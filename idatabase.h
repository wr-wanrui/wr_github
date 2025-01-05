#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QDataWidgetMapper>

class IDatabase : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例的静态函数
    static IDatabase &getInstance()
    {
        static IDatabase instance;
        return instance;
    }

    // 用户登录验证函数，只验证用户名（昵称）是否存在于数据库中
    QString userLogin(QString username);

private:
    explicit IDatabase(QObject *parent = nullptr);
    IDatabase(IDatabase const &) = delete;
    void operator=(IDatabase const &) = delete;

    QSqlDatabase database;

    // 初始化数据库连接的函数
    void initDatabase();

signals:

public:


};

#endif // IDATABASE_H
