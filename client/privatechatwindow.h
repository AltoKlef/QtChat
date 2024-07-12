#ifndef PRIVATECHATWINDOW_H
#define PRIVATECHATWINDOW_H

#include <QDialog>

namespace Ui {
class PrivateChatWindow;
}

class PrivateChatWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PrivateChatWindow(const QString &userName, QWidget *parent = nullptr);
    ~PrivateChatWindow();
    void appendMessage(const QString &message);
    void closeEvent(QCloseEvent *event);
signals:
    void sendMessage(const QString &toUser, const QString &message);

private slots:
    void on_sendButton_clicked();

private:
    Ui::PrivateChatWindow *ui;
    QString userName;
    void onWindowClosed();

};

#endif // PRIVATECHATWINDOW_H
