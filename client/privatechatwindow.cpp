/**
 * @file privatechatwindow.cpp
 * @brief Реализация класса PrivateChatWindow для управления приватными чатами.
 */

#include "privatechatwindow.h"
#include "ui_privatechatwindow.h"
#include "QTime"
#include <QKeyEvent>
#include "filter.h"

/**
 * @brief Конструктор класса PrivateChatWindow.
 * Инициализирует окно приватного чата с указанным пользователем.
 * @param username Имя пользователя, с которым ведется приватный чат.
 * @param parent Родительский виджет.
 */
PrivateChatWindow::PrivateChatWindow(const QString &username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrivateChatWindow)
{
    ui->setupUi(this);
    setWindowTitle("Private Chat with " + username);
    userName = username;
    NoTildeFilter *filter = new NoTildeFilter;
    ui->messageLineEdit->installEventFilter(filter); // Устанавливаем фильтр событий
}

/**
 * @brief Деструктор класса PrivateChatWindow.
 * Освобождает ресурсы, связанные с окном приватного чата.
 */
PrivateChatWindow::~PrivateChatWindow()
{
    delete ui;
}

/**
 * @brief Слот для обработки нажатия кнопки отправки сообщения.
 * Отправляет сообщение пользователю, если строка сообщения не пуста.
 */
void PrivateChatWindow::on_sendButton_clicked()
{
    QString message = ui->messageLineEdit->text();
    if (!message.isEmpty()) {
        emit sendMessage(userName, message);
        appendMessage("|" + QTime::currentTime().toString() + "|  Me: " + message);
        ui->messageLineEdit->clear();
    }
}

/**
 * @brief Добавляет сообщение в окно чата.
 * @param message Сообщение для добавления в чат.
 */
void PrivateChatWindow::appendMessage(const QString &message)
{
    ui->chat->append(message);
}

/**
 * @brief Обрабатывает закрытие окна.
 * Отправляет сообщение о закрытии чата и завершает работу диалогового окна.
 */
void PrivateChatWindow::onWindowClosed()
{
    emit sendMessage(userName, "User has closed the chat window.");
}

/**
 * @brief Обрабатывает событие закрытия окна.
 * Вызывает onWindowClosed() и завершает работу диалогового окна.
 * @param event Событие закрытия окна.
 */
void PrivateChatWindow::closeEvent(QCloseEvent *event)
{
    onWindowClosed();
    QDialog::closeEvent(event);
}

/**
 * @brief Слот для обработки нажатия кнопки выхода из чата.
 * Закрывает окно приватного чата.
 */
void PrivateChatWindow::on_exitButton_clicked()
{
    this->close();
}
