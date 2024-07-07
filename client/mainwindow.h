#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTime>
#include "auth_window.h"
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

private:
    Ui::MainWindow *ui;
    auth_window auth;
    QString username,password;
    QTcpSocket *socket;
    QByteArray Data;
    quint16 nextBlockSize;
    void authorizeUser();
    void connectToServer();

    void SendToServer(const QString &command, const QString &data);
    void processResponse(const QString &command, const QString &data);
    void updateOnlineUsers(const QStringList &userList);
public slots:
    void slotReadyRead();
};
#endif // MAINWINDOW_H
