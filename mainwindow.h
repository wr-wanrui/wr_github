#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QStack>
#include <QKeyEvent>
#include <QPushButton>
#include <map>

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

    QString operand;
    QString opcode;
    QStack<QString> operands;
    QStack<QString> opcodes;
    QMap<int, QPushButton *> digitBTNs;

    QString calculation(bool *ok = NULL);

private slots:

    void btnNumCliked();

    void btnBinaryOperatorCliked();

    void btnUnaryOperatorCliked();

    void on_btnPeriod_clicked();

    void on_btnDel_clicked();

    void on_btnClearAll_clicked();

    void on_btnEqual_clicked();

    void btnSignClicked();

    virtual void keyPressEvent(QKeyEvent *event);
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
