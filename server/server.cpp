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
    QTcpSocket *socket = new QTcpSocket;
    if (socket->setSocketDescriptor(socketDescriptor)) {
        connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

        Sockets.push_back(socket);
        qDebug() << "Client connected:" << socketDescriptor;
    } else {
        delete socket;
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
        SendToClient(str);
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
    //socket->write(Data);
    for(int i=0;i<Sockets.size();i++){
        Sockets[i]->write(Data);
    }
}
void Server::slotClientDisconnected()
{
    socket = (QTcpSocket*)sender();
    Sockets.erase(std::remove(Sockets.begin(), Sockets.end(), socket), Sockets.end());
    socket->deleteLater();
}
