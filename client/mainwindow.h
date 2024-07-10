#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTime>
#include "auth_window.h"
#include "privatechatwindow.h"
#include "qlistwidget.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void display();
private slots:
    void on_pushButton_2_clicked();
    void on_lineEdit_returnPressed();
    void on_onlineButton_clicked();

private:
    Ui::MainWindow *ui;
    auth_window auth;
    QString username,password;
    QTcpSocket *socket;
    QByteArray Data;
    quint16 nextBlockSize;
    void authorizeUser();
    void connectToServer();
    bool isConnected;
    void SendToServer(const QString &command, const QString &data);
    void processResponse(const QString &command, const QString &data);
    void updateOnlineUsers(const QStringList &userList);
    void handleError(QAbstractSocket::SocketError socketError);
    void onConnected();
    void createSocket();
    void handleDisconnection();
    void openChatWindow(QListWidgetItem *item);
public slots:
    void slotReadyRead();
};
#endif // MAINWINDOW_H
