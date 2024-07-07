#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTime>
class Server: public QTcpServer
{
    Q_OBJECT

public:
    Server();
    QTcpSocket *socket;
private:
    QByteArray Data;
    void SendToClient(QString str);
    quint16 nextBlockSize;
    QMap<QTcpSocket*, QString> clients;  // Сокеты и их логины
    void SendToClient(QTcpSocket *socket, const QString &command, const QString &message);
    void SendToAllClients(const QString &message);
    void processCommand(const QString &command, const QString &data, QTcpSocket *socket);
    void handleAuth(const QString &data, QTcpSocket *socket);
    void handleMessage(const QString &data, QTcpSocket *socket);
    void SendToAllClients(const QString &command, const QString &message);
    void updateAllClientsUserList();
    void UpdateUserList(QTcpSocket *socket);
public slots:
    void incomingConnection(qintptr socketDescription);
    void slotReadyRead();
    void slotClientDisconnected();
private slots:

};

#endif // SERVER_H
