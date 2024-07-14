/**
 * @file server.cpp
 * @brief Реализация класса Server и связанных с ним функций.
 */

#include "server.h"
#include <QDebug>
#include <QDataStream>
#include <QTime>
#include <QSettings>
#include <QTextStream>
#include <QCoreApplication>

/**
 * @brief Конструктор класса Server.
 * Инициализирует сервер, читая конфигурацию и начиная прослушивание на указанном порту.
 */
Server::Server() {
    QSettings settings(":/con/config.ini", QSettings::IniFormat);
    int port = settings.value("Server/Port").toInt();
    qDebug() << "port:" << port;
    if (this->listen(QHostAddress::Any, port)) {
        qDebug() << "Сервер запущен";
    } else {
        qDebug() << "Ошибка запуска сервера";
    }
    nextBlockSize = 0;
}

/**
 * @brief Деструктор класса Server.
 * Обеспечивает отключение всех клиентов и правильное закрытие сервера.
 */
Server::~Server() {
    for (QTcpSocket *socket : clients) {
        socket->disconnectFromHost();
    }
    close();
}

/**
 * @brief Обрабатывает входящие соединения.
 * Создает новый сокет для входящего соединения и подключает сигналы для чтения данных и отключения клиента.
 * @param socketDescriptor Дескриптор сокета для входящего соединения.
 */
void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *socket = new QTcpSocket;
    if (socket->setSocketDescriptor(socketDescriptor)) {
        connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::slotClientDisconnected);
        qDebug() << "Клиент подключен:" << socketDescriptor;
    } else {
        delete socket;
        qDebug() << "Ошибка в incomingConnection";
    }
}

/**
 * @brief Слот для обработки сигнала readyRead от клиентского сокета.
 * Считывает данные из сокета, обрабатывает команды и осуществляет связь с клиентом.
 */
void Server::slotReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qDebug() << "Ошибка приведения sender к QTcpSocket";
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
            qDebug() << "Данные еще не полностью доступны";
            break;
        }

        nextBlockSize = 0;
        processCommand(command, data, socket);
    }
}

/**
 * @brief Отправляет сообщение конкретному клиенту.
 * @param socket Сокет клиента для отправки сообщения.
 * @param command Команда для отправки.
 * @param message Сообщение для отправки.
 */
void Server::SendToClient(QTcpSocket *socket, const QString &command, const QString &message) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << command << message;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    socket->write(data);
}

/**
 * @brief Отправляет сообщение всем подключенным клиентам.
 * @param command Команда для отправки.
 * @param message Сообщение для отправки.
 */
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

/**
 * @brief Слот для обработки отключения клиента.
 * Удаляет клиента из списка подключенных клиентов и обновляет список пользователей для всех клиентов.
 */
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
        qDebug() << "Клиент отключен:" << user;
        updateAllClientsUserList();
    }
}

/**
 * @brief Обрабатывает аутентификацию клиента.
 * Проверяет, используется ли уже логин, и отправляет соответствующий ответ клиенту.
 * @param data Данные логина, отправленные клиентом.
 * @param socket Сокет клиента.
 */
void Server::handleAuth(const QString &data, QTcpSocket *socket) {
    QString login = data;
    if (!clients.contains(login)) {
        clients[login] = socket;
        qDebug() << "Пользователь авторизован:" << login;
        SendToClient(socket, "AUTH_SUCCESS", "Добро пожаловать " + login);
        updateAllClientsUserList();
    } else {
        qDebug() << "Ошибка авторизации для пользователя:" << login;
        SendToClient(socket, "AUTH_FAIL", "Логин уже используется");
    }
}

/**
 * @brief Обрабатывает рассылку сообщений от клиента.
 * @param data Данные сообщения, отправленные клиентом.
 * @param login Логин клиента, отправившего сообщение.
 */
void Server::handleMessage(const QString &data, const QString &login) {
    QString message = QString("%1 %2: %3").arg(QTime::currentTime().toString(), login, data);
    qDebug() << "Рассылка сообщения:" << message;
    SendToAllClients("MESSAGE", message);
}

/**
 * @brief Обрабатывает личные сообщения между клиентами.
 * @param toUser Получатель личного сообщения.
 * @param message Содержание личного сообщения.
 * @param fromUser Отправитель личного сообщения.
 */
void Server::handlePrivateMessage(const QString &toUser, const QString &message, const QString &fromUser) {
    if (clients.contains(toUser)) {
        QTcpSocket *toSocket = clients[toUser];
        SendToClient(toSocket, "PRIVATE_MESSAGE", QTime::currentTime().toString() + "~" + fromUser + "~" + message);
        qDebug() << "Личное сообщение от" << fromUser << "для" << toUser << ":" << message;
    }
}

/**
 * @brief Обрабатывает команды, отправленные клиентом.
 * Направляет команду в соответствующую функцию-обработчик в зависимости от типа команды.
 * @param command Команда, отправленная клиентом.
 * @param data Данные, связанные с командой.
 * @param socket Сокет клиента.
 */
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
        qDebug() << "Неизвестная команда";
    }
}

/**
 * @brief Обновляет список пользователей для всех подключенных клиентов.
 * Отправляет обновленный список онлайн-пользователей всем клиентам.
 */
void Server::updateAllClientsUserList() {
    QStringList userList = clients.keys();
    QString userListString = userList.join(',');

    for (QTcpSocket *client : clients.values()) {
        SendToClient(client, "UPDATE_USERS", userListString);
    }
}

/**
 * @brief Обновляет список пользователей для конкретного клиента.
 * Отправляет обновленный список онлайн-пользователей указанному клиенту.
 * @param socket Сокет клиента для отправки списка пользователей.
```cpp
 */
void Server::UpdateUserList(QTcpSocket *socket) {
    QStringList userList = clients.keys();
    QString userListString = userList.join(',');
    qDebug() << "Список пользователей обновлен";
    SendToClient(socket, "UPDATE_USERS", userListString);
}
