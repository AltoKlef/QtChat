// PrivateChatWindow.cpp
#include "privatechatwindow.h"

PrivateChatWindow::PrivateChatWindow(const QString &userName, QWidget *parent)
    : QWidget(parent), userName(userName)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);

    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);

    layout->addWidget(chatDisplay);
    layout->addWidget(messageInput);
    layout->addWidget(sendButton);

    setLayout(layout);

    connect(sendButton, &QPushButton::clicked, this, &PrivateChatWindow::sendMessage);
}

void PrivateChatWindow::sendMessage()
{
    QString message = messageInput->text();
    if (!message.isEmpty()) {
        // Добавить сообщение в окно чата
        chatDisplay->append("Me: " + message);

        // Отправить сообщение пользователю (реализация зависит от вашей логики)
        // ...

        messageInput->clear();
    }
}
