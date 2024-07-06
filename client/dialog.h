#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    QString getLogin() const;

private slots:
    void on_loginButton_clicked();
private:
    Ui::Dialog *ui;
    QString login;
};

#endif // LOGINDIALOG_H
