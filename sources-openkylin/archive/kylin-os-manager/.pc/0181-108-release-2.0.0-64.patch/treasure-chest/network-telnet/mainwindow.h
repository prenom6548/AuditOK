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
    QProcess *m_process = nullptr;
    QTimer *m_timer = nullptr;

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