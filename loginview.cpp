#include "loginview.h"
#include "ui_loginview.h"
#include "idatabase.h"

LoginView::LoginView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginView)
{
    ui->setupUi(this);
}

LoginView::~LoginView()
{
    delete ui;
}

void LoginView::on_btSignIn_clicked()
{
    QString status = IDatabase::getInstance().userLogin(ui->InputUserName->text(), ui->InputUserPasword->text());

    if (status == "loginok")
        emit LoginSuccess();
}
