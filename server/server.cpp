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
    nextBlockSize=0;
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
        /* qDebug() <<"read..";
        QString str;
        in >> str;
        qDebug()<<str;
        SendToClient(str);*/
        for(;;){
            if(nextBlockSize==0){
                qDebug()<<"nextBlockSize=0";
                if(socket->bytesAvailable()<2){
                    qDebug()<<"Data < 2, break";
                    break;
                }
                in >>nextBlockSize;
                qDebug()<<"nextBlockSize = " << nextBlockSize;
            }
            if(socket->bytesAvailable()<nextBlockSize)  {
                qDebug()<<"Data not full";
                break;
            }
            QString str;
            QTime time;
            in>>time>>str;
            nextBlockSize =0;
            qDebug()<<str;
            SendToClient(str);
            break;
        }

    }
    else{
        qDebug()<<"DataStream error";
    }
}
void Server::SendToClient(QString str){
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out<<quint16(0)<<QTime::currentTime()<<str;
    out.device()->seek(0);
    out<<quint16(Data.size() - sizeof(quint16));
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
