#include "chatclient.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

ChatClient::ChatClient(QObject *parent) : QObject(parent)
{
    m_clientSocket = new QTcpSocket(this);

    connect(m_clientSocket, &QTcpSocket::connected, this, &ChatClient::connected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &ChatClient::onReadyRead);

    void sendKickRequest(const QString &adminUsername, const QString &targetUsername);
    void sendBanRequest(const QString &adminUsername, const QString &targetUsername);
}

void ChatClient::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_5_12);
    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()) {
                    const QJsonValue typeVal = jsonDoc.object().value("type");
                    if (typeVal.isString()) {
                        if (typeVal.toString() == "ban_result") {
                            emit banResponseReceived(jsonDoc.object());
                        } else if (typeVal.toString() == "kick_result") {
                            emit kickResponseReceived(jsonDoc.object());
                        } else {
                            emit jsonReceived(jsonDoc.object());
                        }
                    }
                }
            }
        } else {
            break;
        }
    }
}

void ChatClient::sendMessage(const QString &text, const QString &type, const QString &target, bool isPrivate)
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if (!text.isEmpty()) {
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_5_12);

        QJsonObject message;
        message["type"] = type;
        message["text"] = text;
        if (isPrivate) {
            message["private"] = true;
            message["target"] = target;
        } else {
            message["private"] = false;
        }
        // 获取当前时间并格式化为"xx:xx"的字符串
        QString timeStr = QDateTime::currentDateTime().toString("hh:mm");
        message["send_time"] = timeStr;

        serverStream << QJsonDocument(message).toJson();
    }
}

void ChatClient::connectToServer(const QHostAddress &address, quint16 port)
{
    m_clientSocket->connectToHost(address, port);
}

void ChatClient::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}

void ChatClient::sendKickRequest(const QString &adminUsername, const QString &targetUsername)
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket未处于连接状态，无法发送踢人请求";
        return;
    }

    QJsonObject request;
    request["type"] = "kick_user";
    request["admin"] = adminUsername;
    request["target"] = targetUsername;

    QDataStream serverStream(m_clientSocket);
    serverStream.setVersion(QDataStream::Qt_5_12);
    if (serverStream.device()->write(QJsonDocument(request).toJson(QJsonDocument::Compact)) == -1) {
        qDebug() << "发送踢人请求失败，可能是网络问题或其他错误";
    } else {
        qDebug() << "踢人请求已发送，管理员：" << adminUsername << "，目标用户：" << targetUsername;
    }
}

void ChatClient::sendBanRequest(const QString &admin, const QString &target)
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QJsonObject banRequest;
    banRequest["type"] = "ban_user";
    banRequest["admin"] = admin;
    banRequest["target"] = target;

    QDataStream serverStream(m_clientSocket);
    serverStream.setVersion(QDataStream::Qt_5_12);
    serverStream << QJsonDocument(banRequest).toJson(QJsonDocument::Compact);
}
