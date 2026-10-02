#ifndef MAINWINDOWRESULT_H
#define MAINWINDOWRESULT_H

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QDBusInterface>
#include <QDBusReply>

#include <QGraphicsDropShadowEffect>

#include "resultpage/restorefail.h"
#include "resultpage/restoresucc.h"
#include "resultpage/upgradefail.h"
#include "resultpage/upgradenote.h"
#include "resultpage/upgradesucc.h"
#include "resultpage/upgradefaillist.h"

#include "dbusconnect.h"

class MainWindowResult : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindowResult(QWidget *parent = nullptr);
    ~MainWindowResult();

    void initui();
    void dbusinitial(QString state);
    void dbusconnection();

    void reset();
    QString getversion();
    QString getState();
    QString getreason();
    QStringList getlist();
    void addpages();

    RestoreFail *refail;
    RestoreSucc *resucc;
    UpgradeFail *upfail;
    UpgradeNote *upnote;
    UpgradeSucc *upsucc;
    upgradefaillist *upfaillist;

    int pagenumber=0;

    dbusconnect *dbus;

    QString version;
    QStringList upgradelist;
    QStringList restorelist;

    QStringList deblist;

    QVBoxLayout* main1;
    QHBoxLayout* main;

    QString status;
protected:

private:
    QStackedWidget* m_stackWidget = nullptr;

public slots:
    void exitapp();
    void showdeblist();
    void shownote();

signals:
    void exitappsignal();
};

#endif // MAINWINDOWRESULT_H
