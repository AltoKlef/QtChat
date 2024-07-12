#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
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
    connect(ui->userList, &QListWidget::itemClicked, this, &MainWindow::chatClicked);
    setWindowTitle("General chat");
    nextBlockSize = 0;
    isConnected = false;
}

MainWindow::~MainWindow()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected();
    }
    delete ui;
    qDeleteAll(chatWindows);
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



void MainWindow::SendToServer(const QString &command, const QString &data) {
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << command << data;
    out.device()->seek(0);
    out << quint16(packet.size() - sizeof(quint16));
    socket->write(packet);
}
void MainWindow::processResponse(const QString &command, const QString &data)
{
    if (command == "AUTH_SUCCESS") {
        auth.hide();
        MainWindow::show();
        ui->textBrowser->append(data);
        qDebug() << "Autorised";
    } else if (command == "AUTH_FAIL") {
        qDebug() << "not authorised";
        auth.wrongLogin();
    } else if (command == "MESSAGE") {
        ui->textBrowser->append(data);
    } else if (command == "PRIVATE_MESSAGE") {
        qDebug()<<"private";
        QStringList parts = data.split("~");
        if (parts.size() == 3) {
            QString time = parts[0];
            QString fromUser = parts[1];
            QString message=parts[2];
            if(chatWindows.contains(fromUser)){
                chatWindows[fromUser]->appendMessage("|"+time+ "|  "+fromUser +": " + message);
            }
            else{
                openChatWindow(fromUser);
                chatWindows[fromUser]->appendMessage("|"+time+ "|  "+fromUser +": " + message);
            }

        }
    } else if (command == "UPDATE_USERS") {
        QStringList userList = data.split(',');
        updateOnlineUsers(userList);
    } else {
        qDebug() << "Unknown command";
    }
}

void MainWindow::connectToServer(){
    if (socket->state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "Reconnecting";
        QSettings settings("config.ini", QSettings::IniFormat);
        QString ip = settings.value("Server/IP").toString();
        int port = settings.value("Server/Port").toInt();
        qDebug()<<ip<<"   "<<port;
        socket->connectToHost(ip, port);
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
        auth.raise();
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

void MainWindow::updateOnlineUsers(const QStringList &userList) {
    ui->userList->clear();
    ui->userList->addItems(userList);
}

void MainWindow::on_onlineButton_clicked()
{
    SendToServer("ONLINE", username);
}

void MainWindow::openChatWindow(QString userName)
{
    if (chatWindows.contains(userName)) {
        PrivateChatWindow *chatWindow = chatWindows[userName];
        chatWindow->show();
        chatWindow->raise(); // Поднять окно на передний план
    } else {
        qDebug()<<userName;
        PrivateChatWindow *chatWindow = new PrivateChatWindow(userName, this);
        chatWindows.insert(userName, chatWindow);
        connect(chatWindow, &PrivateChatWindow::sendMessage, this, &MainWindow::handlePrivateMessage);
        chatWindow->show();
    }
}

void MainWindow::removeChatWindow(const QString &userName)
{
    if (chatWindows.contains(userName)) {
        PrivateChatWindow *chatWindow = chatWindows[userName];
        delete chatWindow;
    }
}
void MainWindow::handlePrivateMessage(const QString &toUser, const QString &message)
{
    SendToServer("PRIVATE_MESSAGE", toUser+"~"+message);
}
void MainWindow::chatClicked(QListWidgetItem *item){
    openChatWindow(item->text());
}




void MainWindow::on_sendButton_clicked()
{
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    SendToServer("MESSAGE", ui->lineEdit->text());
    ui->lineEdit->clear();
}


void MainWindow::on_exitButton_clicked()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected();
    }
    qApp->quit();
}

