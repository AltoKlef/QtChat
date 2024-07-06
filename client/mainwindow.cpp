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
    authorizeUser();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::display()
{
    auth.show();
}

void MainWindow::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    QString str;
    QTime time;

    while (true) {
        in.startTransaction();
        if (nextBlockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint16)) {
                qDebug()<<"nextBlockSize=0";
                in.rollbackTransaction();
                break;
            }

            in >> nextBlockSize;
            qDebug()<<"nextBlockSize = " << nextBlockSize;
        }

        if (socket->bytesAvailable() < nextBlockSize) {
            qDebug()<<"Data not full";
            in.rollbackTransaction();
            break;
        }
        in >> time >> str;

        if (!in.commitTransaction()) {
            qDebug() << "Data not fully available yet";
            break;
        }
        nextBlockSize = 0;
        ui->textBrowser->append(time.toString() + ": " + str);
        break;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if(ui->lineEdit->text()==""){
        return;
    }
    SendToServer(ui->lineEdit->text());
}

void MainWindow::SendToServer(QString str)
{
    qDebug()<<str;
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << username<<str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    socket->write(Data);
    ui->lineEdit->clear();
}
void MainWindow::connectToServer(){
    socket->connectToHost("127.0.0.1",2323);
}
void MainWindow::authorizeUser()
{
    auth.getLogin();
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
    SendToServer(ui->lineEdit->text());
}

