#include "server.h"
#include <QDebug>
#include <QTcpSocket>
#include <QDataStream>

Server::Server() {
    if (this->listen(QHostAddress::Any, 2323)) {
        qDebug() << "Server started";
    } else {
        qDebug() << "Server start error";
    }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *clientSocket = new QTcpSocket;
    if (clientSocket->setSocketDescriptor(socketDescriptor)) {
        connect(clientSocket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);

        Sockets.push_back(clientSocket);
        qDebug() << "Client connected:" << socketDescriptor;
    } else {
        delete clientSocket;
        qDebug() << "Error in incomingConnection";
    }
}

void Server::slotReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qDebug() << "Error casting sender to QTcpSocket";
        return;
    }

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    if(in.status()==QDataStream::Ok){
        qDebug() <<"read..";
        QString str;
        in >> str;
        qDebug()<<str;
    }
    else{
        qDebug()<<"DataStream error";
    }
}
void Server::SendToClient(QString str){
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out<<str;
    socket->write(Data);

}
