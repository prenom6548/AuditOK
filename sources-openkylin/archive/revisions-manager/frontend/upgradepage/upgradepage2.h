#ifndef UPGRADEPAGE2_H
#define UPGRADEPAGE2_H

#include <QObject>
#include <QWidget>
#include <QLabel>
//#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QScrollArea>
#include <QDBusInterface>
#include <QDBusReply>
#include <QIcon>

#include <kprogresscircle.h>
#include "dbusconnect.h"
using namespace kdk;
class upgradepage2: public QWidget
{
    Q_OBJECT
public:
    explicit upgradepage2(QWidget *parent = nullptr);

    void initialdbus();
    void initui();
    void dbusconnection();

    QLabel* icon;
    QLabel* title;
    QLabel* bottom;
    QLabel* middle;
    KProgressCircle* progress;

    QWidget* topwidget;
    QWidget* midwidget;

    QVBoxLayout* main;
    QHBoxLayout* titlelayout;
    QHBoxLayout* boxlayout;

    int i = 10;

    dbusconnect *dbus;

    int success = 1;
    QStringList failure;
    QStringList Suggest;
    bool finishsignalsent;

public slots:
    void showcondition(QString itermsNum,int state,int process,QString suggestion);

signals:
    void checkupdatefinishsignal();
};

#endif // UPGRADEPAGE2_H
