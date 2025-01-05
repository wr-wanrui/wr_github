#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include "chatclient.h"
#include <QDateTime>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_loginButton_clicked();

    void on_sayButton_clicked();

    void on_logoutButton_clicked();

    void connectedToServer();

    void messageReceived(const QString &sender, const QString &text);

    void sendMessageToBot(const QString &text);

    bool isMessageForBotByKeyword(const QString &text);

    void jsonReceived(const QJsonObject &docObj);

    void userJoined(const QString &user);

    void userLeft(const QString &user);

    void userListReceived(const QStringList &list);

    void on_privateSendButton_clicked();

    void requestChatRecordSearch(const QDateTime &startTime = QDateTime(), const QDateTime &endTime = QDateTime(),
                                 const QString &user = "", const QString &keyword = "");
    void onSearchButtonClicked();

    void onkickButtonClicked();

    void handleKickResponse(const QJsonObject &response);

    void onbanButtonClicked();

    void handleBanResponse(const QJsonObject &response);

    void requestUserListUpdate();

    void on_banButton_clicked();

private:
    Ui::MainWindow *ui;
    ChatClient *m_chatClient;
    bool m_isAdmin;  // 新增变量，用于标记当前登录用户是否为管理员
};
#endif // MAINWINDOW_H
