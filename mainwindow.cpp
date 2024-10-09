#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <math.h>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // this->setStyleSheet("QPushButton{background-color:red}");

    digitBTNs = {{Qt::Key_0, ui->btnNum0},
        {Qt::Key_1, ui->btnNum1},
        {Qt::Key_2, ui->btnNum2},
        {Qt::Key_3, ui->btnNum3},
        {Qt::Key_4, ui->btnNum4},
        {Qt::Key_5, ui->btnNum5},
        {Qt::Key_6, ui->btnNum6},
        {Qt::Key_7, ui->btnNum7},
        {Qt::Key_8, ui->btnNum8},
        {Qt::Key_9, ui->btnNum9},
    };

    foreach (auto btn, digitBTNs )
        connect(btn, SIGNAL(clicked()), this, SLOT(btnNumCliked()));

    // connect(ui->btnNum0, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum1, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum2, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum3, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum4, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum5, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum6, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum7, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum8, SIGNAL(clicked()), SLOT(btnNumCliked()));
    // connect(ui->btnNum9, SIGNAL(clicked()), SLOT(btnNumCliked()));

    connect(ui->btnMultiple, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorCliked()));
    connect(ui->btnPlus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorCliked()));
    connect(ui->btnDivide, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorCliked()));
    connect(ui->btnMultiple, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorCliked()));

    connect(ui->btnPercentage, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorCliked()));
    connect(ui->btnInverse, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorCliked()));
    connect(ui->btnSquare, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorCliked()));
    connect(ui->btnSqrt, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorCliked()));

    // 连接正负号按钮
    connect(ui->btnSign, SIGNAL(clicked()), this, SLOT(btnSignClicked()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{
    double result = 0;
    if (operands.size() == 2 && opcodes.size() > 0) {
        //去操作数
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operands.front().toDouble();
        operands.pop_front();
        //取操作符
        QString op = opcodes.front();
        opcodes.pop_front();

        if (op == "+") {
            result = operand1 + operand2;
        } else if (op == "-") {
            result = operand1 - operand2;
        } else if (op == "*") {
            result = operand1 * operand2;
        } else if (op == "/") {
            result = operand1 / operand2;
        }

        ui->statusbar->showMessage(QString("calculation is in progress : operands is %1,opcode is %2").arg(
                                       operands.size()).arg(
                                       opcodes.size()));
    } else
        ui->statusbar->showMessage(QString("operands is %1,opcode is %2").arg(operands.size()).arg(
                                       opcodes.size()));

    return QString::number(result);
}

void MainWindow::btnNumCliked()
{
    QString digit = qobject_cast<QPushButton *>(sender())->text();
    if (digit == "0" && operand == "0")
        digit = "";
    if (operand == "0" && digit != "0")
        operand = "";
    operand += digit;
    ui->display->setText(operand);

    // QString str = ui->display->text();
    // str += qobject_cast<QPushButton *>(sender())->text();
    // ui->display->setText(str);
    // ui->statusbar->showMessage(qobject_cast<QPushButton *>(sender())->text() + "btn clicked");
}



void MainWindow::on_btnPeriod_clicked()
{
    if (!operand.contains("."))
        operand += qobject_cast<QPushButton *>(sender())->text();
    ui->display->setText(operand);
    // QString str = ui->display->text();
    // if (!str.contains("."))
    //     str += qobject_cast<QPushButton *>(sender())->text();
    // ui->display->setText(str);
}


void MainWindow::on_btnDel_clicked()
{
    operand = operand.left(operand.length() - 1);
    ui->display->setText(operand);
    // QString str = ui->display->text();
    // str = str.left(str.length() - 1);
    // ui->display->setText(str);
}


void MainWindow::on_btnClearAll_clicked()
{
    operand.clear();
    ui->display->setText(operand);
}

void MainWindow::btnBinaryOperatorCliked()
{
    ui->statusbar->showMessage("last operand" + operand);
    QString opcode = qobject_cast<QPushButton *>(sender())->text();

    qDebug() << opcode;

    if (operand != "") {
        operands.push_back(operand);
        operand = "";

        opcodes.push_back(opcode);

        QString result = calculation();
        ui->display->setText(result);
    }
}

void MainWindow::btnUnaryOperatorCliked()
{
    if (operand != "") {
        double result = operand.toDouble();
        operand = "";

        QString op = qobject_cast<QPushButton *>(sender())->text();

        if (op == "%")
            result /= 100.0;
        else if (op == "1/x")
            result = 1 / result;
        else if (op == "x^2")
            result *= result;
        else if (op == "√")
            result = sqrt(result);

        ui->display->setText(QString::number(result));
    }

}

void MainWindow::on_btnEqual_clicked()
{
    if (operand != "") {
        operands.push_back(operand);
        operand = "";
    }
    QString result = calculation();
    ui->display->setText(result);
}

void MainWindow::btnSignClicked()
{
    bool isNegative = operand.startsWith("-");
    if (isNegative) {
        operand = operand.mid(1);
    } else {
        operand = "-" + operand;
    }
    ui->display->setText(operand);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    foreach (auto btnkey, digitBTNs.keys()) {
        if (event->key() == btnkey)
            digitBTNs[btnkey]->animateClick();
    }

    // if (event->key() == Qt::Key_0)
    //     // qDebug() << event->key();
    //     // ui->btnNum0->clicked();
    //     ui->btnNum0->animateClick();
}

