#include "auth_window.h"
#include "ui_auth_window.h"

/**
 * @brief Конструктор класса auth_window.
 *
 * @param parent Указатель на родительский объект.
 */
auth_window::auth_window(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::auth_window)
{
    ui->setupUi(this);
    setWindowTitle("Hello window");

    // Устанавливаем фильтр для ввода, чтобы запретить символ "~"
    NoTildeFilter *filter = new NoTildeFilter;
    ui->logLineEdit->installEventFilter(filter);
}

/**
 * @brief Деструктор класса auth_window.
 */
auth_window::~auth_window()
{
    delete ui;
}

/**
 * @brief Получить логин пользователя.
 *
 * @return QString Логин пользователя.
 */
QString auth_window::getLogin()
{
    return auth_window::login;
}

/**
 * @brief Отобразить сообщение об ошибке, если пользователь уже авторизован.
 */
void auth_window::wrongLogin()
{
    ui->label->setText("Ошибка! Пользователь уже авторизован");
}

/**
 * @brief Обработчик нажатия кнопки авторизации.
 *
 * Этот метод испускает сигнал authClicked, чтобы начать процесс авторизации.
 */
void auth_window::on_authButton_clicked()
{
    emit authClicked();
}

/**
 * @brief Обработчик нажатия клавиши Enter в поле логина.
 *
 * Этот метод испускает сигнал authClicked, чтобы начать процесс авторизации.
 */
void auth_window::on_logLineEdit_returnPressed()
{
    emit authClicked();
}

/**
 * @brief Обработчик изменения текста в поле логина.
 *
 * @param arg1 Новый текст, введенный пользователем.
 */
void auth_window::on_logLineEdit_textEdited(const QString &arg1)
{
    login = arg1;
}
