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
    nextBlockSize = 0;
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *socket = new QTcpSocket;
    if (socket->setSocketDescriptor(socketDescriptor)) {
        connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::slotClientDisconnected);

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
        handleMessage(data, socket);
    } else {
        qDebug() << "Unknown command";
    }
}

void Server::handleAuth(const QString &data, QTcpSocket *socket) {
    QString login = data;
    if (!clients.values().contains(login)) {
        clients[socket] = login;
        qDebug() << "User authorized:" << login;
        SendToClient(socket, "AUTH_SUCCESS", "Welcome " + login);
        updateAllClientsUserList();
    } else {
        qDebug() << "Authorization failed for user:" << login;
        SendToClient(socket, "AUTH_FAIL", "Login already in use");
    }
}


void Server::handleMessage(const QString &data, QTcpSocket *socket) {
    QString login = clients[socket];
    QString message = QString("%1 %2: %3").arg(QTime::currentTime().toString(), login, data);
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
    qDebug()<<"data: "<<message;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    for (QTcpSocket *socket : clients.keys()) {
        socket->write(data);
    }
}


void Server::slotClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        clients.remove(socket);
        socket->deleteLater();
        qDebug() << "Client disconnected";
        updateAllClientsUserList();
    }
}
void Server::updateAllClientsUserList() {
    QStringList userList = clients.values();
    QString userListString = userList.join(',');

    for (QTcpSocket *client : clients.keys()) {
        SendToClient(client, "UPDATE_USERS", userListString);
    }
}

