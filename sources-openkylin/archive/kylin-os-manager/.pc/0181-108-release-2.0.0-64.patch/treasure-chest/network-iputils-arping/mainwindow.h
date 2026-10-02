#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QProcess>
#include <kwidget.h>

using namespace kdk;
namespace networkipa
{

class MainWindow : public KWidget
{
    Q_OBJECT

public:
    MainWindow(KWidget *parent = nullptr);
    ~MainWindow();

private:
    QLabel *m_label = nullptr;
    QLineEdit *m_lineEditIp = nullptr;
    QPushButton *m_btnStart = nullptr;
    QProcess *m_process = nullptr;

    QString textTest;
    QString textTesting;
    QString textMsgSuccess;
    QString textMsgFail;
    QString textMsgFailLocalIP;
    QString textMsgFailIPError;
    QString textMsgFailNetWorkError;
    QString textMsgBtn;
public slots:
    void slotClickBtn();
    void slotGetOutPut();
    void slotGetError();
    void slotProcessFinish();
};

}

#endif