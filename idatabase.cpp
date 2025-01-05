#include "idatabase.h"
#include <QUuid>

// 初始化数据库连接的函数实现
void IDatabase::initDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");
    QString aFile = "D:/Qt/lab4a.db";
    database.setDatabaseName(aFile);
    if (!database.open()) {
        qDebug() << "failed to open database";
    } else {
        qDebug() << "open database is ok";
    }
}


// 修改后的用户登录验证函数，着重验证昵称是否存在
QString IDatabase::userLogin(QString username)
{
    QSqlQuery query;
    query.prepare("SELECT username FROM user WHERE username = :USERNAME");
    query.bindValue(":USERNAME", username);
    query.exec();
    if (query.next()) {
        return "loginok";
    }
    return "wrongUsername";
}

IDatabase::IDatabase(QObject *parent) : QObject(parent)
{
    initDatabase();
}
