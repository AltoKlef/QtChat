#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTime>
class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();
    void SendToClient(QTcpSocket *socket, const QString &command, const QString &message);
    void SendToAllClients(const QString &command, const QString &message);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void slotReadyRead();
    void slotClientDisconnected();

private:
    QHash<QString, QTcpSocket*> clients;
    quint16 nextBlockSize;

    void processCommand(const QString &command, const QString &data, QTcpSocket *socket);
    void handleAuth(const QString &data, QTcpSocket *socket);
    void handleMessage(const QString &data, QTcpSocket *socket);
    void handlePrivateMessage(const QString &toUser, const QString &message, QTcpSocket *fromSocket);
    void updateAllClientsUserList();
    void UpdateUserList(QTcpSocket *socket);
    void handlePrivateMessage(const QString &toUser, const QString &message, const QString &fromUser);
};

#endif // SERVER_H
