#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <kwidget.h>

#ifdef V11_ENVIRONMENT
    #include "kom_process.h"
#else
    #include <QProcess>
#endif

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

#ifdef V11_ENVIRONMENT
    kom::KomProcess *m_process = nullptr;
#else
    QProcess *m_process = nullptr;
#endif

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