#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QDBusInterface>
#include <QDBusReply>

#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QGraphicsDropShadowEffect>

#include <kwindowsystem.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <kiconbar.h>
#include <kwindowbuttonbar.h>
#include <kaboutdialog.h>
#include "upgradepage/initpage.h"
#include "upgradepage/upgradepage1.h"
#include"upgradepage/advancedoptions.h"
#include"upgradepage/importactivationfilespage.h"
#include "upgradepage/upgradepage2.h"
#include "upgradepage/attentionpage.h"
#include "upgradepage/preupgradefailure.h"
#include "upgradepage/preupgradesuccess.h"
#include "upgradepage/upgradenotification.h"
#include "upgradepage/upgradeprocess.h"
#include "upgradepage/upgradefailure.h"
#include "upgradepage/upgradesuccess.h"

#include "restorepage/restorepage1.h"
#include "restorepage/restorepage2.h"
#include "restorepage/prerestorefailure.h"
#include "restorepage/prerestoresucess.h"
#include "restorepage/restorenotification.h"
#include "restorepage/restoreprogress.h"
#include "restorepage/restorefailure.h"
#include "restorepage/restoresuccess.h"

#include "dbusconnect.h"

using namespace kdk;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initui();
    void dbusinitial();
    void dbusconnection();

    QString getversion();
    void addpages();

    initpage *initpg;
    upgradepage1 *uppg1;
    advancedoptions *advoptionpage;
    importactivationfiles *importactivationfilespage;
    upgradepage2 *uppg2;
    attentionpage *attentionup;
    attentionpage *attentionback;
    preupgradefailure *preupgradefail;
    preupgradesuccess *preupgradesucc;
    upgradenotification *upgradenotify;
    upgradeprocess *upgradepro;
    upgradefailure *upgradefail;
    upgradesuccess *upgradesucc;

    restorepage1 *rspg1;
    restorepage2 *rspg2;
    prerestorefailure *prerestorefail;
    prerestoresucess *prerestoresucc;
    restorenotification *restorenotify;
    restoreprogress *restorepro;
    restorefailure *restorefail;
    restoresuccess *restoresucc;

    int pagenumber=0;

    dbusconnect *dbus;

    QString version;
    QStringList upgradelist;
    QStringList upgradesuggestionlist;
    QStringList restorelist;
    QStringList restoresuggestionlist;
    QVBoxLayout* main;
    QWidget *titlewidget;
    QHBoxLayout *titlelayout;
    KWindowButtonBar *buttonwidget;
    KIconBar *iconbar;

    int m_inhibitValue;
protected:

private:
    QStackedWidget* m_stackWidget = nullptr;

public slots:
    void SteptoUpgradePage();
    void SteptoRestorePage();
    void exitapp();
    void showattentionup();
    void showattentionback();
    void firstpage();
    void notificationpage();
    void upgradepage();
    void restorepage();
    void initialpage();
    void showadvancedoptionspage();
    void showimportactivationfilespage();
    void showinitpage();
    void uppage2();
    void lastpage();
    void checkupdatefinish();
    void rspage2();
    void restorenotificationpage();
    void checkrestorefinish();
    void upgradesuccesspage();
    void restoresuccesspage();
    void upgradefailpage();
    void restorefailpage();
    void reboot();
//private slots :
//    void slotChangeStyle(QString key);

//    void bootOptionsFilter(QString opt);

//    void initConnect();

//public slots:
//    void methodupgrade(int Index);
//    void methodupgrade();

};
#endif // MAINWINDOW_H
