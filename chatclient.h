#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDateTime>

class ChatClient : public QObject
{
    Q_OBJECT

public:
    explicit ChatClient(QObject *parent = nullptr);

signals:
    void connected();
    void messageReceived(const QString &text);
    void jsonReceived(const QJsonObject &docObj);
    void banResponseReceived(const QJsonObject &response);  // 新增信号，用于传递禁言响应
    void kickResponseReceived(const QJsonObject &response);  // 新增踢人响应信号
private:
    QTcpSocket *m_clientSocket;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text, const QString &type = "message", const QString &target = "",
                     bool isPrivate = false);
    void connectToServer(const QHostAddress &address, quint16 port);
    void disconnectFromHost();

    // 新增函数声明，用于发送踢人操作消息到服务器
    void sendKickRequest(const QString &adminUsername, const QString &targetUsername);
    // 新增函数声明，用于发送禁言操作消息到服务器
    void sendBanRequest(const QString &adminUsername, const QString &targetUsername);
};

#endif // CHATCLIENT_H
