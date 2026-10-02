#ifndef RESTOREPROGRESS_H
#define RESTOREPROGRESS_H

#include <QObject>
#include <QWidget>

#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QScrollArea>
#include <QTimer>
#include <QIcon>

#include <QDBusInterface>
#include <QDBusReply>
#include "dbusconnect.h"

#include <kprogresscircle.h>
using namespace kdk;
class restoreprogress : public QWidget
{
    Q_OBJECT
public:
    explicit restoreprogress(QWidget *parent = nullptr);

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


    QTimer* m_timer;

    int i = 10;

    dbusconnect *dbus;

    int success = 1;

    QStringList failure;
    QStringList Suggest;
public slots:
    void showcondition(QString itermsNum,int state,int process,QString suggestion);

signals:
    void upgradesuccesssignal();
    void upgradefailsignal();

};

#endif // RESTOREPROGRESS_H
