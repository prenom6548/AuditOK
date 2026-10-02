#ifndef UPGRADENOTIFICATION_H
#define UPGRADENOTIFICATION_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QTextBrowser>
#include <QAbstractSlider>
#include <QAbstractScrollArea>

class upgradenotification : public QWidget
{
    Q_OBJECT
public:
    explicit upgradenotification(QWidget *parent = nullptr);

    QLabel* title;
    QTextEdit *content;
    QPushButton *nextbtn;
    QPushButton *backbtn;
    QCheckBox *check;

    QTextBrowser* license_text;

    QHBoxLayout *titlelayout;
    QVBoxLayout *main;
    QHBoxLayout *btnlayout;
    QWidget* btnwidget;

    void initui();
    void dbusconnection();

//    void statechange();

public slots:
    void statechange(int num);
    void gotofirstpage();
    void nextpage();

signals:
    void gotofistpagesignal();
    void nextpagesignal();
};

#endif // UPGRADENOTIFICATION_H
