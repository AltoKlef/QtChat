#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::handleDisconnection);
    connect(&auth, &auth_window::authClicked, this, &MainWindow::authorizeUser);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::handleError);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);

    nextBlockSize = 0;
    isConnected = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::display()
{
    auth.show();
}

void MainWindow::onConnected()
{
    isConnected = true;
    username = auth.getLogin();
    SendToServer("AUTH", username);
}

void MainWindow::slotReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    QString command;
    QString data;

    while (true) {
        in.startTransaction();

        if (nextBlockSize == 0) {
            if (socket->bytesAvailable() < 2) {
                in.rollbackTransaction();
                break;
            }
            in >> nextBlockSize;
            qDebug() << "socket->bytesAvailable(): " << socket->bytesAvailable();
            qDebug() << "nextBlockSize: " << nextBlockSize;
        }

        if (socket->bytesAvailable() < nextBlockSize) {
            in.rollbackTransaction();
            break;
        }

        in >> command >> data;
        qDebug() << command << " " << data;

        if (!in.commitTransaction()) {
            qDebug() << "Data not fully available yet";
            break;
        }

        nextBlockSize = 0;
        processResponse(command, data);
    }
}

void MainWindow::processResponse(const QString &command, const QString &data) {
    if (command == "AUTH_SUCCESS") {
        auth.hide();
        this->show();
        ui->textBrowser->append(data);
        qDebug() << "Authorized";
    } else if (command == "AUTH_FAIL") {
        qDebug() << "Not authorized";
        auth.wrongLogin();
    } else if (command == "MESSAGE") {
        ui->textBrowser->append(data);
    } else if (command == "UPDATE_USERS") {
        QStringList userList = data.split(',');
        updateOnlineUsers(userList);
    } else {
        qDebug() << "Unknown command";
    }
}

void MainWindow::SendToServer(const QString &command, const QString &data) {
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << command << data;
    out.device()->seek(0);
    out << quint16(packet.size() - sizeof(quint16));
    socket->write(packet);
}

void MainWindow::connectToServer(){
    if (socket->state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "Reconnecting";
        socket->connectToHost("127.0.0.1", 2323);
    }
}

void MainWindow::authorizeUser()
{
    qDebug() << auth.getLogin();
    qDebug() << "isConnected: " << isConnected;
    if (!isConnected) {
        connectToServer();
    } else {
        SendToServer("AUTH", auth.getLogin());
    }
}

void MainWindow::handleError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage;

    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        errorMessage = "The host was not found.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorMessage = "The connection was refused by the peer.";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMessage = "The remote host closed the connection.";
        socket->disconnectFromHost();
        this->hide();
        auth.show();
        break;
    default:
        errorMessage = socket->errorString();
    }
    qDebug() << errorMessage;
    isConnected = false;
    QMessageBox::critical(this, "Connection Error", errorMessage);
    //auth.connectionError(errorMessage);
}

void MainWindow::handleDisconnection() {
    isConnected = false;
    qDebug() << "Disconnected from server";
    this->hide();
    auth.show();
}

void MainWindow::on_lineEdit_returnPressed()
{
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    SendToServer("MESSAGE", ui->lineEdit->text());
    ui->lineEdit->clear();
}

void MainWindow::on_pushButton_2_clicked()
{
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    SendToServer("MESSAGE", ui->lineEdit->text());
    ui->lineEdit->clear();
}

void MainWindow::updateOnlineUsers(const QStringList &userList) {
    ui->userList->clear();
    ui->userList->addItems(userList);
}

void MainWindow::on_onlineButton_clicked()
{
    SendToServer("ONLINE", username);
}

