#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QProcess>
#include <QTimer>
#include <kwidget.h>

#ifdef V11_ENVIRONMENT
    #include "kom_process.h"
#else
    #include <QProcess>
#endif

using namespace kdk;
namespace networktelnet

{

class MainWindow : public KWidget
{
    Q_OBJECT

public:
    MainWindow(KWidget *parent = nullptr);
    ~MainWindow();

private:
    QPushButton *m_btnTest = nullptr;
    QLabel *m_label = nullptr;
    QLineEdit *m_lineEditIp = nullptr;
    QLabel *m_labelPort = nullptr;
    QSpinBox *m_spinBox = nullptr;
    QTimer *m_timer = nullptr;
#ifdef V11_ENVIRONMENT
    kom::KomProcess *m_process = nullptr;
#else
    QProcess *m_process = nullptr;
#endif

    QString textTest;
    QString textTesting;
    QString textIP;
    QString textPort;
    QString textConnSuccess;
    QString textConnFail;
    QString textBtnOk;
public slots:
    void slotClickBtn();
    void slotGetOutPut();
    void slotGetError();
    void slotProcessFinish();
    void slotTimeOut();
};

}

#endif