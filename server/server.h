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
    QVector <QTcpSocket*> Sockets;
    QByteArray Data;
    void SendToClient(QString str);
    quint16 nextBlockSize;
public slots:
    void incomingConnection(qintptr socketDescription);
    void slotReadyRead();
    void slotClientDisconnected();
};

#endif // SERVER_H
