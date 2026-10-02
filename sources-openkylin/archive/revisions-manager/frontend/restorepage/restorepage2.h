#ifndef RESTOREPAGE2_H
#define RESTOREPAGE2_H

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

#include <kprogresscircle.h>
#include "dbusconnect.h"
using namespace kdk;
class restorepage2 : public QWidget
{
    Q_OBJECT
public:
    explicit restorepage2(QWidget *parent = nullptr);

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
    void checkupdatefinishsignal();

};

#endif // RESTOREPAGE2_H
