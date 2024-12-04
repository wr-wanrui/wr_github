#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>

namespace Ui {
class LoginView;
}

class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);
    ~LoginView();

public slots:
    void on_btSignIn_clicked();

signals:
    void LoginSuccess();
    void loginFailed();

private:
    Ui::LoginView *ui;
};

#endif // LOGINVIEW_H
