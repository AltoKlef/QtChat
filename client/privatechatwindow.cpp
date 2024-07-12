#include "privatechatwindow.h"
#include "ui_privatechatwindow.h"
#include "QTime"
PrivateChatWindow::PrivateChatWindow(const QString &username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrivateChatWindow)
{
    ui->setupUi(this);
    setWindowTitle("Private Chat with " + username);
    userName=username;

}

PrivateChatWindow::~PrivateChatWindow()
{
    delete ui;
}

void PrivateChatWindow::on_sendButton_clicked()
{
    QString message = ui->messageLineEdit->text();
    if (!message.isEmpty()) {
        emit sendMessage(userName, message);
        appendMessage("|"+QTime::currentTime().toString()+"| "+"Me: " + message);
        ui->messageLineEdit->clear();
    }
}

void PrivateChatWindow::appendMessage(const QString &message)
{
    ui->chat->append(message);
}

void PrivateChatWindow::onWindowClosed()
{
    emit sendMessage(userName, userName+" has closed the chat window.");
}
void PrivateChatWindow::closeEvent(QCloseEvent *event)
{
    onWindowClosed();
    QDialog::closeEvent(event);
}
