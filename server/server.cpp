#include "server.h"
#include <QDebug>
#include <QTcpSocket>
#include <QDataStream>
#include <QTime>
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
        qDebug() << command << "data:" << data;
        if (!in.commitTransaction()) {
            qDebug() << "Data not fully available yet";
            break;
        }

        nextBlockSize = 0;
        processCommand(command, data, socket);
    }
}

void Server::handleAuth(const QString &data, QTcpSocket *socket) {
    QString login = data;
    if (!clients.contains(login)) {
        clients[login] = socket;
        qDebug() << "User authorized:" << login;
        SendToClient(socket, "AUTH_SUCCESS", "Welcome " + login);
        updateAllClientsUserList();
    } else {
        qDebug() << "Authorization failed for user:" << login;
        SendToClient(socket, "AUTH_FAIL", "Login already in use");
    }
}

void Server::handleMessage(const QString &data, const QString &login) {
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
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    for (QTcpSocket *socket : clients.values()) {
        socket->write(data);
    }
}

void Server::slotClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QString user;
        for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
            if (it.value() == socket) {
                user = it.key();
                break;
            }
        }
        clients.remove(user);
        socket->deleteLater();
        qDebug() << "Client disconnected:" << user;
        updateAllClientsUserList();
    }
}

void Server::updateAllClientsUserList() {
    QStringList userList = clients.keys();
    QString userListString = userList.join(',');

    for (QTcpSocket *client : clients.values()) {
        SendToClient(client, "UPDATE_USERS", userListString);
    }
}

void Server::UpdateUserList(QTcpSocket *socket) {
    QStringList userList = clients.keys();
    QString userListString = userList.join(',');
    qDebug() << "User list updated";
    SendToClient(socket, "UPDATE_USERS", userListString);
}

void Server::processCommand(const QString &command, const QString &data, QTcpSocket *socket) {
    if (command == "AUTH") {
        handleAuth(data, socket);
    } else if (command == "MESSAGE") {
        QString login;
        for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
            if (it.value() == socket) {
                login = it.key();
                break;
            }
        }
        handleMessage(data, login);
    } else if (command == "PRIVATE_MESSAGE") {
        QStringList parts = data.split("~");
        if (parts.size() == 2) {
            QString toUser = parts[0];
            QString message = parts[1];
            QString fromUser;
            for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
                if (it.value() == socket) {
                    fromUser = it.key();
                    break;
                }
            }
            handlePrivateMessage(toUser, message, fromUser);
        }
    } else if (command == "ONLINE") {
        UpdateUserList(socket);
    } else {
        qDebug() << "Unknown command";
    }
}

void Server::handlePrivateMessage(const QString &toUser, const QString &message, const QString &fromUser) {
    if (clients.contains(toUser)) {
        QTcpSocket *toSocket = clients[toUser];
        SendToClient(toSocket, "PRIVATE_MESSAGE", QTime::currentTime().toString() + "~" + fromUser + "~" + message);
        qDebug() << "Private message from" << fromUser << "to" << toUser << ":" << message;
    }
}
