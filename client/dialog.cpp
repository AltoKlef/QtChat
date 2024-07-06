#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

QString Dialog::getLogin() const
{
    return login;
}




void Dialog::on_loginButton_clicked()
{
    login = ui->lineEdit->text();
    if (!login.isEmpty()) {
        accept();  // Закрыть диалог и вернуть QDialog::Accepted
    } else {
        ui->label->setText("Please enter a login");
    }
}

