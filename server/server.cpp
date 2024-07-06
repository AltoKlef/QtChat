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
    QString command;
    QString data;
    QString userLogin;

    while (true) {
        in.startTransaction();

        if (nextBlockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint16)) {
                in.rollbackTransaction();
                break;
            }

            in >> nextBlockSize;
        }

        if (socket->bytesAvailable() < nextBlockSize) {
            in.rollbackTransaction();
            break;
        }

        in >> command >> data;

        if (!in.commitTransaction()) {
            qDebug() << "Data not fully available yet";
            break;
        }

        nextBlockSize = 0;
        processCommand(command, data, socket);
        break;
    }
}

void Server::processCommand(const QString &command, const QString &data, QTcpSocket *socket) {
    if (command == "AUTH") {
        handleAuth(data, socket);
    } else if (command == "MESSAGE") {
        handleMessage(data);
    } else {
        qDebug() << "Unknown command";
    }
}

void Server::handleAuth(const QString &data, QTcpSocket *socket) {
    QString login = data;  // В реальном приложении также нужно учитывать пароль и проверять его
    // Проверка логина в базе данных (пример без реальной БД)
    if (login == "validUser") {
        // Авторизация успешна
        qDebug() << "User authorized:" << login;
        SendToClient(socket, "AUTH_SUCCESS", "Welcome " + login);
    } else {
        // Авторизация не удалась
        qDebug() << "Authorization failed for user:" << login;
        SendToClient(socket, "AUTH_FAIL", "Invalid login");
    }
}

void Server::handleMessage(const QString &data) {
    QString message = data;
    qDebug() << "Broadcasting message:" << message;
    SendToAllClients("MESSAGE", message);
}

void Server::SendToClient(QTcpSocket *socket, const QString &command, const QString &message) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << command << message;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    socket->write(data);
}

void Server::SendToAllClients(const QString &command, const QString &message) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << command << message;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    for (QTcpSocket *socket : qAsConst(Sockets)) {
        socket->write(data);
    }
}

void Server::slotClientDisconnected()
{
    socket = (QTcpSocket*)sender();
    Sockets.erase(std::remove(Sockets.begin(), Sockets.end(), socket), Sockets.end());
    socket->deleteLater();
}
