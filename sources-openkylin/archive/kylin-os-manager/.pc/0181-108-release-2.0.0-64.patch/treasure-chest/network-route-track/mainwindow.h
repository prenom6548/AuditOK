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
    QProcess *m_process = nullptr;

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