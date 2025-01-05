#include "chatserver.h"
#include "serverworker.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{

}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    ServerWorker *worker = new ServerWorker(this);
    if (!worker->setSocketDescriptor(socketDescriptor)) {
        delete worker;
        return;
    }

    connect(worker, &ServerWorker::logMessage, this, &ChatServer::logMessage);
    connect(worker, &ServerWorker::jsonReceived, this, &ChatServer::jsonReceived);
    connect(worker, &ServerWorker::disconnectFromClient, this, std::bind(&ChatServer::userDisconnected, this, worker));
    m_clients.append(worker);
    emit logMessage("新的用户连接上了");
}

// 在 loadChatRecordsFromFile 方法中适当添加注释说明格式解析逻辑
QVector<ChatRecord> ChatServer::loadChatRecordsFromFile()
{
    QVector<ChatRecord> records;
    QFile file("chat_history.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(line.toUtf8(), &parseError);
            // 以下注释说明解析出的 JSON 对象需要符合的结构要求
            if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                QJsonObject recordObj = jsonDoc.object();
                ChatRecord record;
                record.message = recordObj.value("message").toString();
                record.sender = recordObj.value("sender").toString();
                record.timestamp = QDateTime::fromString(recordObj.value("timestamp").toString(), Qt::ISODate);
                record.isPrivate = recordObj.value("isPrivate").toBool();
                records.append(record);
            }
        }
        file.close();
    }
    return records;
}

// 在 saveChatRecord 方法中也可以添加类似注释说明存储的 JSON 结构
void ChatServer::saveChatRecord(const ChatRecord &record)
{
    QFile file("chat_history.json");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QJsonObject recordObj;
        recordObj["message"] = record.message;
        recordObj["sender"] = record.sender;
        recordObj["timestamp"] = record.timestamp.toString(Qt::ISODate);
        recordObj["isPrivate"] = record.isPrivate;
        // 这里确保没有错误地设置isBanned字段，假设正常情况下保存记录时不应设置为禁言状态
        recordObj["isBanned"] = m_bannedUsers.contains(record.sender);

        QJsonDocument jsonDoc(recordObj);
        QTextStream out(&file);
        out << jsonDoc.toJson() << "\n";
        file.close();
        m_chatRecords.append(record);
    }
}

QVector<ChatRecord> ChatServer::searchChatRecords(const QDateTime &startTime, const QDateTime &endTime,
        const QString &user, const QString &keyword)
{
    if (m_chatRecords.isEmpty()) {
        m_chatRecords = loadChatRecordsFromFile();
    }
    QVector<ChatRecord> result;
    for (const ChatRecord &record : m_chatRecords) {
        if ((startTime.isNull() || record.timestamp >= startTime) &&
                (endTime.isNull() || record.timestamp <= endTime) &&
                (user.isEmpty() || record.sender == user) &&
                (keyword.isEmpty() || record.message.contains(keyword))) {
            result.append(record);
        }
    }
    return result;
}

void ChatServer::broadcast(const QJsonObject &message, ServerWorker *exclude)
{
    for (ServerWorker *worker : m_clients) {
        worker->sendJson(message);
    }
}

void ChatServer::stopServer()
{
    close();
}

bool ChatServer::isMessageForBotByKeyword(const QString &text)
{
    QStringList keywords = {"天气查询", "笑话", "查询文档"}; // 可以在这里添加更多需要自动触发机器人回复的关键词
    for (const QString &keyword : keywords) {
        if (text.contains(keyword)) {
            return true;
        }
    }
    return false;
}

void ChatServer::jsonReceived(ServerWorker *sender, const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");
    if (typeVal.isNull() || !typeVal.isString())
        return;

    if (typeVal.toString().compare("message", Qt::CaseInsensitive) == 0) {
        const QString text = docObj.value("text").toString().trimmed();
        if (text.isEmpty())
            return;

        if (m_bannedUsers.contains(sender->userName())) {
            QString reply = "你已被管理员禁止发言，如有疑问请联系管理员。";
            QJsonObject message;
            message["type"] = "message";
            message["text"] = reply;
            message["sender"] = "Server";
            message["private"] = false;
            message["send_time"] = QDateTime::currentDateTime().toString("hh:mm");
            sender->sendJson(message);
            return;
        }


        const QJsonValue textVal = docObj.value("text");
        const QJsonValue privateVal = docObj.value("private");
        const QJsonValue targetVal = docObj.value("target");
        if (typeVal.isNull() || !typeVal.isString())
            return;
        if (text.isEmpty())
            return;

        ChatRecord record;
        record.message = text;
        record.sender = sender->userName();
        record.timestamp = QDateTime::currentDateTime();
        record.isPrivate = privateVal.toBool();

        // 判断消息内容中是否包含特定关键词来确定是否是发给聊天机器人的消息
        if (isMessageForBotByKeyword(text)) {
            QString reply;
            if (text.contains("天气查询")) {
                reply = "抱歉呀，暂时无法查询真实天气呢，你可以通过专业的天气软件查看哦。";
            } else if (text.contains("笑话")) {
                reply = "许仙给老婆买了一顶帽子，白娘子戴上之后就死啦，因为那是顶鸭（压）舌（蛇）帽呀，哈哈。";
            } else if (text.contains("帮助文档")) {
                reply = "目前暂无详细帮助文档呢，你可以联系管理员咨询相关问题哦。";
            } else {
                reply = "不太明确你想查询什么呀，你可以换个问题试试哦。";
            }

            record.message = reply;
            record.sender = "AI助手";  // 设置为聊天机器人的名称
            record.isPrivate =
                false;  // 这里假设聊天机器人回复都是广播形式，可根据需求调整为私聊回复等情况
            saveChatRecord(record);

            QJsonObject message;
            message["type"] = "message";
            message["text"] = reply;
            message["sender"] = "AI助手";
            message["private"] = false;
            message["send_time"] = record.timestamp.toString("hh:mm");
            broadcast(message, nullptr);
            return;  // 直接返回，不再进行普通消息的转发逻辑
        }

        saveChatRecord(record);

        if (privateVal.toBool()) {
            // 处理私聊消息，查找接收方并转发
            QString target = targetVal.toString();
            for (ServerWorker *worker : m_clients) {
                if (worker->userName() == target) {
                    QJsonObject privateMessage;
                    privateMessage["type"] = "message";
                    privateMessage["text"] = text;
                    privateMessage["sender"] = sender->userName();
                    privateMessage["private"] = true;
                    // 添加发送时间字段
                    privateMessage["send_time"] = record.timestamp.toString("hh:mm");
                    worker->sendJson(privateMessage);
                    break;
                }
            }
        } else {
            QJsonObject message;
            message["type"] = "message";
            message["text"] = text;
            message["sender"] = sender->userName();
            // 添加发送时间字段
            message["send_time"] = record.timestamp.toString("hh:mm");
            broadcast(message, sender);
        }
    } else if (typeVal.toString().compare("search_records", Qt::CaseInsensitive) == 0) {
        const QJsonValue startTimeVal = docObj.value("start_time");
        const QJsonValue endTimeVal = docObj.value("endTime");
        const QJsonValue userVal = docObj.value("user");
        const QJsonValue keywordVal = docObj.value("keyword");

        // 输出整个请求的JSON数据，方便查看包含keyword的原语句
        qDebug() << "接收到的搜索记录请求JSON数据: " << QJsonDocument(docObj).toJson();

        QDateTime startTime = QDateTime::fromString(startTimeVal.toString(), Qt::ISODate);
        QDateTime endTime = QDateTime::fromString(endTimeVal.toString(), Qt::ISODate);
        QString user = userVal.toString();
        QString keyword = keywordVal.toString();

        QVector<ChatRecord> searchResults = searchChatRecords(startTime, endTime, user, keyword);

        QJsonArray resultsArray;
        for (const ChatRecord &record : searchResults) {
            QJsonObject resultObj;
            resultObj["message"] = record.message;
            resultObj["sender"] = record.sender;
            resultObj["timestamp"] = record.timestamp.toString(Qt::ISODate);
            resultObj["isPrivate"] = record.isPrivate;
            resultsArray.append(resultObj);
        }

        QJsonObject response;
        response["type"] = "search_records";
        response["records"] = resultsArray;
        sender->sendJson(response);
    } else if (typeVal.toString().compare("login", Qt::CaseInsensitive) == 0) {
        const QJsonValue usernameVal = docObj.value("text");
        if (usernameVal.isNull() || !usernameVal.isString())
            return;

        sender->setUserName(usernameVal.toString());
        QJsonObject connectedMessage;
        connectedMessage["type"] = "newuser";
        connectedMessage["username"] = usernameVal.toString();
        broadcast(connectedMessage, sender);

        QJsonObject userListMessage;
        userListMessage["type"] = "userlist";
        userListMessage["update"] = true;
        QJsonArray userlist;
        for (ServerWorker *worker : m_clients) {
            if (worker == sender)
                userlist.append(worker->userName() + "*");
            else
                userlist.append(worker->userName() );
        }
        userListMessage["userlist"] = userlist;
        sender->sendJson(userListMessage);
    }
}

void ChatServer::userDisconnected(ServerWorker *sender)
{
    m_clients.removeAll(sender);
    const QString userName = sender->userName();
    if (!userName.isEmpty()) {
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = "userdisconnected";
        disconnectedMessage["username"] = userName;
        broadcast(disconnectedMessage, nullptr);
        emit logMessage(userName + "disconnected");
    }
    sender->deleteLater();
}

// 实现处理踢人操作的函数
void ChatServer::kickUser(const QString &adminUsername, const QString &targetUsername)
{
    if (!isAdmin(adminUsername)) {
        qDebug() << "非管理员尝试执行踢人操作，已拒绝。";
        return;
    }
    if (adminUsername.isEmpty() || targetUsername.isEmpty()) {
        return;
    }
    // 已有的查找目标用户并执行踢人操作的逻辑
    ServerWorker *targetWorker = nullptr;
    for (ServerWorker *worker : m_clients) {
        if (worker->userName() == targetUsername) {
            targetWorker = worker;
            break;
        }
    }
    if (targetWorker) {
        QJsonObject kickMessage;
        kickMessage["type"] = "user_kicked";
        kickMessage["admin"] = adminUsername;
        kickMessage["kicked_user"] = targetUsername;
        broadcast(kickMessage, nullptr);
        targetWorker->disconnectFromClient();
    }
}

void ChatServer::banUser(const QString &adminUsername, const QString &targetUsername)
{
    if (!isAdmin(adminUsername)) {
        qDebug() << "非管理员尝试执行禁言操作，已拒绝。管理员用户名：" << adminUsername;
        return;
    }
    if (adminUsername.isEmpty() || targetUsername.isEmpty()) {
        return;
    }
    // 根据操作决定是添加还是移除禁言用户（假设这里先只考虑添加禁言用户的情况，如果要实现解禁功能则需要相应修改此处逻辑）
    m_bannedUsers.insert(targetUsername);
    QJsonObject banMessage;
    banMessage["type"] = "user_banned";
    banMessage["admin"] = adminUsername;
    banMessage["banned_user"] = targetUsername;
    broadcast(banMessage, nullptr);
}

bool ChatServer::isAdmin(const QString &username)
{
    return username == "管理员";
}
