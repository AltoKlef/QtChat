// server.h

#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
class Server : public QTcpServer {
    Q_OBJECT

public:
    Server();
    ~Server();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void slotReadyRead();
    void slotClientDisconnected();

private:
    void handleAuth(const QString &data, QTcpSocket *socket);
    void handleMessage(const QString &data, const QString &login);
    void handlePrivateMessage(const QString &toUser, const QString &message, const QString &fromUser);
    void SendToClient(QTcpSocket *socket, const QString &command, const QString &message);
    void SendToAllClients(const QString &command, const QString &message);
    void processCommand(const QString &command, const QString &data, QTcpSocket *socket);
    void updateAllClientsUserList();
    void UpdateUserList(QTcpSocket *socket);

    quint16 nextBlockSize;
    QMap<QString, QTcpSocket *> clients;
};


#endif // SERVER_H
