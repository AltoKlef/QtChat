#ifndef AUTH_WINDOW_H
#define AUTH_WINDOW_H

#include <QDialog>
#include <QMessageBox>
#include "filter.h"
namespace Ui {
class auth_window;
}

class auth_window : public QDialog
{
    Q_OBJECT

public:
    explicit auth_window(QWidget *parent = nullptr);
    ~auth_window();
    QString getLogin();
    void wrongLogin();
    //String getPass();
private slots:
    void on_authButton_clicked();
    void on_logLineEdit_returnPressed();
    void on_logLineEdit_textEdited(const QString &arg1);
signals:
    void authClicked();
private:
    QString login;
    //QString password;
    Ui::auth_window *ui;
};

#endif // AUTH_WINDOW_H
