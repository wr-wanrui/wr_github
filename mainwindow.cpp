#include "mainwindow.h"
#include "chatclient.h"
#include "idatabase.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);

    m_chatClient = new ChatClient(this);
    m_isAdmin = false;  // 初始化管理员标记为f

    connect(m_chatClient, &ChatClient::connected, this, &MainWindow::connectedToServer);
    connect(m_chatClient, &ChatClient::jsonReceived, this, &MainWindow::jsonReceived);


    // 初始化私聊对象下拉框，确保能正常显示和使用
    ui->privateTargetComboBox = findChild<QComboBox *>("privateTargetComboBox");
    if (ui->privateTargetComboBox == nullptr) {
        qDebug() << "未能找到 privateTargetComboBox 控件";
    }

    // 初始化搜索结果文本编辑框控件
    ui->searchResultTextEdit = findChild<QTextEdit *>("searchResultTextEdit");
    if (ui->searchResultTextEdit == nullptr) {
        qDebug() << "未能找到 searchResultTextEdit 控件";
    }
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);

    // 初始化管理员操作目标用户下拉框
    ui->adminTargetComboBox = findChild<QComboBox *>("adminTargetComboBox");
    if (ui->adminTargetComboBox == nullptr) {
        qDebug() << "未能找到 adminTargetComboBox 控件";
    }

    bool isConnected = QObject::connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);
    if (isConnected) {
        qDebug() << "searchButton点击事件与onSearchButtonClicked函数连接成功";
    } else {
        qDebug() << "searchButton点击事件与onSearchButtonClicked函数连接失败";
    }
    // 测试向searchResultTextEdit添加文本
    ui->searchResultTextEdit->setPlainText("这是一个测试文本");

    connect(ui->kickButton, &QPushButton::clicked, this, &MainWindow::onkickButtonClicked);
    connect(ui->banButton, &QPushButton::clicked, this, &MainWindow::onbanButtonClicked);

    // 根据是否为管理员设置踢人按钮的可用性（示例，可根据实际需求调整显示方式等）
    ui->kickButton->setEnabled(m_isAdmin);
    // 根据是否为管理员设置禁言按钮的可用性
    ui->banButton->setEnabled(m_isAdmin);

    connect(m_chatClient, &ChatClient::banResponseReceived, this,
            &MainWindow::handleBanResponse);  // 连接禁言响应信号到处理函数
    connect(m_chatClient, &ChatClient::kickResponseReceived, this,
            &MainWindow::handleKickResponse);  // 新增连接踢人响应信号
}
MainWindow::~MainWindow()
{
    delete ui;
}

// 登录按钮点击事件处理函数，添加昵称验证逻辑
void MainWindow::on_loginButton_clicked()
{
    QString username = ui->usernameEdit->text();
    QString result = IDatabase::getInstance().userLogin(username);
    if (result == "loginok") {
        if (username == "管理员") {
            m_isAdmin = true;
        } else {
            m_isAdmin = false;
        }
        m_chatClient->connectToServer(QHostAddress(ui->serverEdit->text()), 1967);
    } else {
        QMessageBox::critical(this, "登录失败", "输入的用户名不存在，请核对后重新输入");
    }
}

// 发送消息按钮点击事件处理函数等其他函数保持不变，此处省略部分代码展示

void MainWindow::on_sayButton_clicked()
{
    QString inputText = ui->sayLineEdit->text();
    QString username = ui->usernameEdit->text();
    if (isMessageForBotByKeyword(inputText)) {
        ui->roomTextEdit->append(QString("[%1] : %2").arg(username).arg(inputText));
        sendMessageToBot(inputText);
    } else if (!inputText.isEmpty()) {
        m_chatClient->sendMessage(inputText);
    }
}


void MainWindow::on_logoutButton_clicked()
{
    m_chatClient->disconnectFromHost();
    ui->stackedWidget->setCurrentWidget(ui->loginPage);


    for (auto aItem : ui->userListWidget->findItems(ui->usernameEdit->text(), Qt::MatchExactly)) {
        qDebug("remove");
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }
}

void MainWindow::connectedToServer()
{
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    m_chatClient->sendMessage(ui->usernameEdit->text(), "login");

    ui->privateTargetComboBox->clear();
}

void MainWindow::messageReceived(const QString &sender, const QString &text)
{
    ui->roomTextEdit->append(QString("%1 : %2").arg(sender).arg(text));
}

bool MainWindow::isMessageForBotByKeyword(const QString &text)
{
    QStringList keywords = {"天气查询", "笑话", "帮助文档"}; // 可以在这里添加更多需要自动触发机器人回复的关键词
    for (const QString &keyword : keywords) {
        if (text.contains(keyword)) {
            return true;
        }
    }
    return false;
}

void MainWindow::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");
    if (typeVal.isNull() || !typeVal.isString())
        return;

    if (typeVal.toString().compare("message", Qt::CaseInsensitive) == 0) {
        const QJsonValue textVal = docObj.value("text");
        const QJsonValue senderVal = docObj.value("sender");
        const QJsonValue privateVal = docObj.value("private");
        const QJsonValue sendTimeVal = docObj.value("send_time");  // 获取发送时间字段值
        if (typeVal.isNull() || !typeVal.isString())
            return;

        if (senderVal.isNull() || !senderVal.isString())
            return;

        QString text = textVal.toString();
        QString sender = senderVal.toString();
        QString timeStr = "";
        if (sendTimeVal.isString()) {
            timeStr = QString("%1").arg(sendTimeVal.toString());  // 构建时间显示格式
        }

        // 判断是否是聊天机器人的回复消息（这里假设服务器返回的聊天机器人消息sender为"AI助手"）
        if (sender == "AI助手") {
            if (privateVal.toBool()) {
                ui->roomTextEdit->append(QString("[%1] [私聊用户：AI助手] %2").arg(timeStr).arg(text));
            } else {
                ui->roomTextEdit->append(QString("[%1] AI助手 : %2").arg(timeStr).arg(text));
            }
            return;  // 直接返回，不再进行普通消息的显示逻辑
        }


        if (privateVal.toBool()) {
            ui->roomTextEdit->append(QString("[%1] [私聊用户：%2] %3").arg(timeStr).arg(sender).arg(text));
        } else {
            ui->roomTextEdit->append(QString("[%1] %2 : %3").arg(timeStr).arg(sender).arg(text));
        }


    }  else if (typeVal.toString().compare("search_records", Qt::CaseInsensitive) == 0) {
        // 输出整个接收到的包含search_records类型消息的JSON数据
        qDebug() << "接收到的搜索记录消息JSON数据: " << QJsonDocument(docObj).toJson();

        const QJsonValue recordsVal = docObj.value("records");
        if (recordsVal.isNull() || !recordsVal.isArray())
            return;

        QString resultText = "消息记录搜索结果如下：\n";
        resultText += QString("时间\t发送者\t消息内容\t是否私聊\n");
        QJsonArray recordsArray = recordsVal.toArray();
        for (const QJsonValue &recordVal : recordsArray) {
            if (recordVal.isObject()) {
                QJsonObject recordObj = recordVal.toObject();
                QString message = recordObj.value("message").toString();
                QString sender = recordObj.value("sender").toString();
                QString timestamp = recordObj.value("timestamp").toString();
                bool isPrivate = recordObj.value("isPrivate").toBool();
                resultText += QString("%1\t%2\t%3\t%4\n").arg(timestamp).arg(sender).arg(message).arg(isPrivate ? "是" : "否");
            }
        }
        // 设置搜索结果文本到searchResultTextEdit控件中
        if (ui->searchResultTextEdit) {
            ui->searchResultTextEdit->setPlainText(resultText);
            qDebug() << "已将搜索结果文本设置到searchResultTextEdit控件中";
        }
    }   else if (typeVal.toString().compare("newuser", Qt::CaseInsensitive) == 0) {
        const QJsonValue usernameVal = docObj.value("username");
        if (usernameVal.isNull() || !usernameVal.isString())
            return;

        userJoined(usernameVal.toString());
    } else if (typeVal.toString().compare("userdisconnected", Qt::CaseInsensitive) == 0) {
        const QJsonValue usernameVal = docObj.value("username");
        if (usernameVal.isNull() || !usernameVal.isString())
            return;

        userLeft(usernameVal.toString());
    } else if (typeVal.toString().compare("userlist", Qt::CaseInsensitive) == 0) {

        const QJsonValue userlistVal = docObj.value("userlist");
        const QJsonValue updateVal = docObj.value("update");
        if (userlistVal.isNull() || !userlistVal.isArray())
            return;

        if (updateVal.toBool()) {
            QStringList userList = userlistVal.toVariant().toStringList();
            ui->privateTargetComboBox->clear();
            ui->privateTargetComboBox->addItems(userList);
            ui->adminTargetComboBox->clear();
            ui->adminTargetComboBox->addItems(userList);
            qDebug() << userlistVal.toVariant().toStringList();
            userListReceived(userlistVal.toVariant().toStringList());
        }
    } else if (typeVal.toString().compare("search_records", Qt::CaseInsensitive) == 0) {
        const QJsonValue recordsVal = docObj.value("records");
        if (recordsVal.isNull() || !recordsVal.isArray())
            return;

        QString resultText;
        QJsonArray recordsArray = recordsVal.toArray
                                  ();
        for (const QJsonValue &recordVal : recordsArray) {
            if (recordVal.isObject()) {
                QJsonObject recordObj = recordVal.toObject();
                QString message = recordObj.value("message").toString();
                QString sender = recordObj.value("sender").toString();
                QString timestamp = recordObj.value("timestamp").toString();
                bool isPrivate = recordObj.value("isPrivate").toBool();
                resultText += QString("[%1] %2: %3\n").arg(timestamp).arg(sender).arg(message);
            }
        }
        // 设置搜索结果文本到searchResultTextEdit控件中
        if (ui->searchResultTextEdit) {
            ui->searchResultTextEdit->setPlainText(resultText);
        }
    }
}

// 新增发送消息给聊天机器人的函数（示例，根据实际需求调整）
void MainWindow::sendMessageToBot(const QString &text)
{
    if (!text.isEmpty()) {
        m_chatClient->sendMessage(text, "message", "AI助手",
                                  false);  // 假设聊天机器人的名称为"AI助手"，发送广播消息给它
    }
}


void MainWindow::userJoined(const QString &user)
{
    ui->userListWidget->addItem(user);

    // 用户加入时，将新用户添加到私聊目标下拉框
    ui->privateTargetComboBox->addItem(user);
}

void MainWindow::userLeft(const QString &user)
{
    for (auto aItem : ui->userListWidget->findItems(user, Qt::MatchExactly)) {
        qDebug("remove");
        ui->userListWidget->removeItemWidget(aItem);
        delete aItem;
    }

    // 用户离开时，从私聊目标下拉框移除该用户
    int index = ui->privateTargetComboBox->findText(user);
    if (index != -1) {
        ui->privateTargetComboBox->removeItem(index);
    }
}

void MainWindow::userListReceived(const QStringList &list)
{
    ui->userListWidget->clear();
    ui->userListWidget->addItems(list);
}

void MainWindow::on_privateSendButton_clicked()
{
    QString target = ui->privateTargetComboBox->currentText();
    QString text = ui->privateMessageLineEdit->text();
    if (!target.isEmpty() && !text.isEmpty()) {
        m_chatClient->sendMessage(text, "message", target, true);
    }
}

void MainWindow::onSearchButtonClicked()
{
    qDebug() << "onSearchButtonClicked函数被执行了";
    QDateTime startTime = ui->startTimeEdit->dateTime();
    QDateTime endTime = ui->endTimeEdit->dateTime();
    QString user = ui->userSearchEdit->text();
    QString keyword = ui->keywordSearchEdit->text();

    // 获取roomTextEdit中的文本内容
    QString roomText = ui->roomTextEdit->toPlainText();
    if (roomText.isEmpty()) {
        ui->searchResultTextEdit->setPlainText("roomTextEdit中暂无文本内容，请先进行聊天产生记录后再搜索哦。");
        return;
    }
    QStringList roomTextLines = roomText.split("\n");

    QString resultText = "根据关键字搜索roomTextEdit的结果如下：\n";
    for (const QString &line : roomTextLines) {
        if (line.contains(keyword)) {
            resultText += line + "\n";
        }
    }

    // 设置筛选后的结果文本到searchResultTextEdit控件中
    ui->searchResultTextEdit->setPlainText(resultText);

    requestChatRecordSearch(startTime, endTime, user, keyword);
}

void MainWindow::onkickButtonClicked()
{
    if (!m_isAdmin) {
        QMessageBox::warning(this, "权限不足", "你没有权限执行踢人操作，请联系管理员。");
        return;
    }
    QString adminUsername = ui->usernameEdit->text();
    QString targetUsername = ui->adminTargetComboBox->currentText();
    if (targetUsername.isEmpty()) {
        QMessageBox::warning(this, "操作目标未选择", "请选择要踢除的用户。");
        return;
    }

    // 可以添加一个确认提示框，询问是否确定要踢人，避免误操作
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认踢人",
                                        QString("确定要将用户 %1 踢出聊天室吗？").arg(targetUsername),
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // 发送踢人请求
        m_chatClient->sendKickRequest(adminUsername, targetUsername);
        // 可以添加日志记录，方便后续查看操作情况
        qDebug() << "已发送踢人请求，管理员：" << adminUsername << "，目标用户：" << targetUsername;
    }
}

void MainWindow::handleKickResponse(const QJsonObject &response)
{
    const QJsonValue resultVal = response.value("result");
    if (resultVal.isString()) {
        QString result = resultVal.toString();
        if (result == "success") {
            QString targetUsername = ui->adminTargetComboBox->currentText();
            QMessageBox::information(this, "踢人成功", QString("已成功将用户 %1 踢出聊天室。").arg(targetUsername));
            // 移除被踢用户在界面相关控件中的显示，比如从用户列表、私聊目标下拉框等移除
            userLeft(targetUsername);
            // 直接调用函数，不接收返回值，因为它本身返回void
            qDebug() << "已尝试更新用户列表";
        } else if (result == "failure") {
            QMessageBox::warning(this, "踢人失败", "踢人操作失败，请检查相关设置或联系技术支持。");
        }
    }
}


// 禁言按钮点击事件处理函数
void MainWindow::onbanButtonClicked()
{
    if (!m_isAdmin) {
        QMessageBox::warning(this, "权限不足", "你没有权限执行禁言操作，请联系管理员。");
        return;
    }
    QString adminUsername = ui->usernameEdit->text();
    QString targetUsername = ui->adminTargetComboBox->currentText();
    if (targetUsername.isEmpty()) {
        QMessageBox::warning(this, "操作目标未选择", "请选择要禁言的用户。");
        return;
    }

    m_chatClient->sendBanRequest(adminUsername, targetUsername);
}

// 新增的禁言响应处理函数
void MainWindow::handleBanResponse(const QJsonObject &response)
{
    const QJsonValue resultVal = response.value("result");
    if (resultVal.isString()) {
        QString result = resultVal.toString();
        if (result == "success") {
            QMessageBox::information(this, "禁言成功", "已成功禁言该用户。");
        } else if (result == "failure") {
            QMessageBox::warning(this, "禁言失败", "禁言操作失败，请检查相关设置或联系技术支持。");
        }
    }
}

void MainWindow::requestChatRecordSearch(const QDateTime &startTime, const QDateTime &endTime, const QString &user,
        const QString &keyword)
{
    if (startTime > endTime) {
        qDebug() << "搜索的开始时间不能晚于结束时间，请重新选择时间范围";
        return;
    }

    QJsonObject searchRequest;
    searchRequest["type"] = "search_records";
    searchRequest["start_time"] = startTime.toString(Qt::ISODate);
    searchRequest["end_time"] = endTime.toString(Qt::ISODate);
    searchRequest["user"] = user;
    searchRequest["keyword"] = keyword;
    qDebug() << "组装的查询请求JSON数据为: " << QJsonDocument(searchRequest).toJson();
    m_chatClient->sendMessage(QJsonDocument(searchRequest).toJson(QJsonDocument::Compact), "request");
}

