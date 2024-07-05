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

    in.startTransaction(); // Начинаем транзакцию

    QString str;
    QTime time;

    // Читаем данные из потока
    in >> time >> str;

    // Проверяем, успешно ли завершилась транзакция
    if (!in.commitTransaction()) {
        qDebug() << "Data not fully available yet";
        return;
    }

    qDebug() << "Received:" << str;

    // Отправляем полученные данные обратно всем клиентам
    SendToClient(str);
}

void Server::SendToClient(QString str) {
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << QTime::currentTime() << str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));

    for (QTcpSocket *socket : Sockets) {
        socket->write(Data);
    }
}
void Server::slotClientDisconnected()
{
    socket = (QTcpSocket*)sender();
    Sockets.erase(std::remove(Sockets.begin(), Sockets.end(), socket), Sockets.end());
    socket->deleteLater();
}
