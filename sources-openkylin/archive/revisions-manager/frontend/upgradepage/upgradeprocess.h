#ifndef UPGRADEPROCESS_H
#define UPGRADEPROCESS_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QScrollArea>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>
#include <QIcon>
#include <kprogresscircle.h>
#include "dbusconnect.h"
using namespace kdk;
class upgradeprocess : public QWidget
{
    Q_OBJECT
public:
    explicit upgradeprocess(QWidget *parent = nullptr);

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
    int tmp = 0;

    QStringList failure;
    QStringList Suggest;
public slots:
    void showcondition(QString itermsNum,int state,int process,QString suggestion);

signals:
    void upgradesuccesssignal();
    void upgradefailsignal();
};

#endif // UPGRADEPROCESS_H
