#include "auth_window.h"
#include "ui_auth_window.h"

auth_window::auth_window(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::auth_window)
{
    ui->setupUi(this);

}

auth_window::~auth_window()
{
    delete ui;
}

QString auth_window::getLogin()
{
    return auth_window::login;
}

void auth_window::wrongLogin()
{
    ui->label->setText("Пользователь уже авторизован, повторите попытку");
}

void auth_window::on_authButton_clicked()
{
    emit authClicked();
}


void auth_window::on_logLineEdit_returnPressed()
{
    emit authClicked();
}


void auth_window::on_logLineEdit_textEdited(const QString &arg1)
{
    login=arg1;
}

