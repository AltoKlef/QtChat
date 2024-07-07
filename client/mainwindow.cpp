#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket= new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    connect(&auth, &auth_window::authClicked, this, &MainWindow::authorizeUser);
    nextBlockSize=0;
    connectToServer();
    //authorizeUser();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::display()
{
    auth.show();
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
            qDebug()<<"socket->bytesAvailable(): "<<socket->bytesAvailable();
            qDebug()<<"nextBlockSize: "<<nextBlockSize;
        }

        if (socket->bytesAvailable() < nextBlockSize) {
            in.rollbackTransaction();
            break;
        }

        in >> command >> data;
        qDebug()<<command<<" "<<data;

        if (!in.commitTransaction()) {
            qDebug() << "Data not fully available yet";
            break;
        }

        nextBlockSize = 0;
        processResponse(command, data);
        break;
    }
}
void MainWindow::processResponse(const QString &command, const QString &data) {
    if (command == "AUTH_SUCCESS") {
        auth.hide();
        MainWindow::show();
        ui->textBrowser->append(data);
        qDebug()<<"Autorised";
    } else if (command == "AUTH_FAIL") {
        qDebug()<<"not autorised";
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
    socket->connectToHost("127.0.0.1",2323);
}
void MainWindow::authorizeUser()
{
    username=auth.getLogin();
    qDebug()<<username;
    SendToServer("AUTH",username);
    //qDebug<<username;
    /*Dialog Dialog(this);
    if (Dialog.exec() == QDialog::Accepted) {
        userLogin = Dialog.getLogin();
        qDebug()<<userLogin;
        connectToServer();
    }*/

}


void MainWindow::on_lineEdit_returnPressed()
{
    if(ui->lineEdit->text()==""){
        return;
    }
    SendToServer("MESSAGE",ui->lineEdit->text());
    ui->lineEdit->clear();
}

void MainWindow::on_pushButton_2_clicked()
{
    if(ui->lineEdit->text()==""){
        return;
    }
    SendToServer("MESSAGE",ui->lineEdit->text());
    ui->lineEdit->clear();
}

void MainWindow::updateOnlineUsers(const QStringList &userList) {
    ui->userList->clear();
    ui->userList->addItems(userList);
}
