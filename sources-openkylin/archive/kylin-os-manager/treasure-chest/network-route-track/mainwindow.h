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
namespace networkrt
{

class MainWindow : public KWidget
{
    Q_OBJECT

public:
    MainWindow(KWidget *parent = nullptr);
    ~MainWindow();

private:
    QPushButton *m_btnStart = nullptr;
    QLabel *m_label = nullptr;
    QLineEdit *m_lineEditIp = nullptr;
    QTextEdit *m_textEditOutPut = nullptr;
#ifdef V11_ENVIRONMENT
    kom::KomProcess *m_process = nullptr;
#else
    QProcess *m_process = nullptr;
#endif

    QString strStrat;
    QString strStop;
public slots:
    void slotClickBtn();
    void slotGetOutPut();
    void slotGetError();
    void slotProcessFinish();
};

}

#endif