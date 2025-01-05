#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QObject>
#include "serverworker.h"
#include <QVector>
#include <QJsonObject>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

struct AdminUser {
    QString username;
};

// 定义聊天记录结构体
struct ChatRecord {
    QString message;
    QString sender;
    QDateTime timestamp;
    bool isPrivate;
    bool isBanned;  // 新增这个成员，用于标识用户是否被禁言
};


class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);

    bool isAdmin(const QString &username);  // 新增函数声明，用于验证是否为管理员

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    QVector<ServerWorker *>m_clients;
    QVector<ChatRecord> loadChatRecordsFromFile();
    void saveChatRecord(const ChatRecord &record);  // 新增函数声明，用于保存聊天记录
    QVector<ChatRecord> m_chatRecords;
    QVector<ChatRecord> searchChatRecords(const QDateTime &startTime = QDateTime(), const QDateTime &endTime = QDateTime(),
                                          const QString &user = "", const QString &keyword = "");
    void broadcast(const QJsonObject &message, ServerWorker *exclude);

    // 新增函数声明，用于处理管理员踢人操作
    void kickUser(const QString &adminUsername, const QString &targetUsername);
    // 新增函数声明，用于处理管理员禁言操作
    void banUser(const QString &adminUsername, const QString &targetUsername) ;



signals:
    void logMessage(const QString &msg);

public slots:
    void stopServer();
    bool isMessageForBotByKeyword(const QString &text);
    void jsonReceived(ServerWorker *sender, const QJsonObject &docObj);
    void userDisconnected(ServerWorker *sender);

private:
    QSet<QString> m_bannedUsers;
    QStringList bannedUsers;  // 记录被禁言的用户列表，简单示例，可优化数据结构
};

#endif // CHATSERVER_H
