#include "mainwindow.h"
#include <QPalette>
#include <QApplication>
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    dbusinitial();
    initui();
    dbusconnection();
}

void MainWindow::initui()
{
    this->setObjectName("MainWindow");
//    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    kdk::UkuiStyleHelper::self()->removeHeader(this);
//    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    main = new QVBoxLayout(this);
    titlewidget = new QWidget(this);
    titlelayout = new QHBoxLayout(this);
    titlewidget->setLayout(titlelayout);
    buttonwidget = new KWindowButtonBar(this);
    buttonwidget->maximumButton()->hide();
    buttonwidget->minimumButton()->hide();
    QAction *action1 = new QAction();
    action1->setText(tr("About"));
    QAction *action2 = new QAction();
    action2->setText(tr("Exit"));
    QList<QAction *> actions ;
    actions << action1 << action2;
    buttonwidget->menuButton()->menu()->clear();
    buttonwidget->menuButton()->menu()->addActions(actions);
    connect(buttonwidget->menuButton()->menu(),&QMenu::triggered,this,[=](QAction *act){
        QString str = act->text();
        if(str == tr("Exit"))
        {
            this->close();
        }
        if(str == tr("About"))
        {
            KAboutDialog *dialog = new KAboutDialog;
            QIcon icon(":/kylin-revision-manager.svg");
            dialog->setAppIcon(icon);
            QString version = tr("none");
            dialog->setAppName(tr("kylin-revision-manager"));
            {
                FILE *pp = NULL;
                char *line = NULL;
                size_t len = 0;
                ssize_t read;
                char *q = NULL;

                pp = popen("dpkg -l kylin-revision-manager", "r");
                if(NULL == pp)
                    qInfo()<<" 版本获取不到 ";
                while((read = getline(&line, &len, pp)) != -1){
                q = strrchr(line, '\n');
                *q = '\0';
                QString content = line;
                QStringList list = content.split(" ");
                list.removeAll("");
                if (list.size() >= 3)
                    version = list.at(2);
                }
                free(line);
                pclose(pp);
            }
            dialog->setAppVersion(version);
            dialog->show();
        }
    });
    iconbar = new KIconBar(this);
    titlelayout->setContentsMargins(0,0,0,0);
    titlelayout->addWidget(iconbar);
    titlelayout->addStretch();
    titlelayout->addWidget(buttonwidget);
    iconbar->setIcon(":/kylin-revision-manager.svg");
    iconbar->setWidgetName(tr("Revision"));
    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->setFixedSize(480,560);
    addpages();
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
//    m_stackWidget->setAutoFillBackground(true);
//    m_stackWidget->setBackgroundRole(QPalette::Base);

    main->setContentsMargins(4,0,4,0);
    main->addWidget(titlewidget);
    main->addStretch();
    main->addWidget(m_stackWidget);
    main->addStretch();
    main->setAlignment(m_stackWidget,Qt::AlignHCenter);
}

void MainWindow::dbusinitial()
{
    QDBusInterface *interface;
    interface = new QDBusInterface("org.gnome.SessionManager",
                                   "/org/gnome/SessionManager",
                                   "org.gnome.SessionManager",
                                   QDBusConnection::sessionBus());

    QDBusMessage reply = interface->call(QDBus::Block, "Inhibit", "kylin-inhibit", (quint32)0, "offline-upgrade is playing", (quint32)8);
    m_inhibitValue = reply.arguments().takeFirst().toUInt();
}

QString MainWindow::getversion()
{
    dbusconnect *dbus = dbusconnect::getInstance();
    QDBusReply<QString> mode = dbus->interface->call("getCurOsVersion");
    qDebug()<<"curOsVersion is "<<mode;
    if(!mode.isValid())
    {
        qDebug()<<"获取版本接口异常";
        return "error";
    }
    QString modereply = mode.value();
    qDebug()<<modereply;
    return modereply;
}

void MainWindow::addpages()
{
    initpg = new initpage();
    uppg1 = new upgradepage1();
    advoptionpage = new advancedoptions();
    importactivationfilespage = new importactivationfiles();
    uppg2 = new upgradepage2();
    attentionup = new attentionpage();
    preupgradefail = new preupgradefailure();
    preupgradesucc = new preupgradesuccess();
    upgradenotify = new upgradenotification();
    upgradefail = new upgradefailure();
    upgradesucc = new upgradesuccess();
    upgradepro = new upgradeprocess();
    m_stackWidget->addWidget(initpg);
    m_stackWidget->addWidget(uppg1);
    m_stackWidget->addWidget(advoptionpage);
    m_stackWidget->addWidget(importactivationfilespage);
    m_stackWidget->addWidget(uppg2);
    m_stackWidget->addWidget(attentionup);
    m_stackWidget->addWidget(preupgradefail);
    m_stackWidget->addWidget(preupgradesucc);
    m_stackWidget->addWidget(upgradenotify);
    m_stackWidget->addWidget(upgradepro);
    m_stackWidget->addWidget(upgradefail);
    m_stackWidget->addWidget(upgradesucc);


//添加还原界面
    qDebug()<<"get into the 还原界面";
    rspg1 = new restorepage1();
    rspg2 = new restorepage2();
    attentionback = new attentionpage();
    prerestorefail = new prerestorefailure();
    prerestoresucc = new prerestoresucess();
    restorenotify = new restorenotification();
    restorepro = new restoreprogress();
    restorefail = new restorefailure();
    restoresucc = new restoresuccess();
    m_stackWidget->addWidget(rspg1);
    m_stackWidget->addWidget(rspg2);
    m_stackWidget->addWidget(attentionback);
    m_stackWidget->addWidget(prerestorefail);
    m_stackWidget->addWidget(prerestoresucc);
    m_stackWidget->addWidget(restorenotify);
    m_stackWidget->addWidget(restorepro);
    m_stackWidget->addWidget(restorefail);
    m_stackWidget->addWidget(restoresucc);
}

void MainWindow::dbusconnection()
{
    connect(buttonwidget->closeButton(),&QPushButton::clicked,this,&MainWindow::exitapp);
    connect(buttonwidget->minimumButton(),&QPushButton::clicked,this,&MainWindow::showMinimized);
    {
        connect(initpg,&initpage::Upgradesignal,this,&MainWindow::SteptoUpgradePage);
        connect(initpg,&initpage::Restoresignal,this,&MainWindow::SteptoRestorePage);
        connect(initpg,&initpage::closesignal,this,[=]{
            close();
        });
        connect(uppg1,&upgradepage1::exitappsignal,this,&MainWindow::exitapp);
        connect(uppg1,&upgradepage1::nextpage,this,&MainWindow::notificationpage);
        connect(uppg1,&upgradepage1::backpage,this,&MainWindow::showinitpage);
        connect(uppg1,&upgradepage1::showattentionsignal,this,&MainWindow::showattentionup);
        connect(uppg1,&upgradepage1::showadvancedoptionspage,this,&MainWindow::showadvancedoptionspage);
        connect(advoptionpage,&advancedoptions::exitadvancedoptionsettings,this,&MainWindow::firstpage);
        connect(importactivationfilespage,&importactivationfiles::nextpage,this,&MainWindow::notificationpage);
        connect(importactivationfilespage,&importactivationfiles::lastpage,this,&MainWindow::firstpage);
        connect(attentionup,&attentionpage::returnsignal,this,&MainWindow::firstpage);
        connect(upgradenotify,&upgradenotification::gotofistpagesignal,this,&MainWindow::firstpage);
        connect(upgradenotify,&upgradenotification::nextpagesignal,this,&MainWindow::uppage2);
        connect(uppg2,&upgradepage2::checkupdatefinishsignal,this,&MainWindow::checkupdatefinish);
        connect(preupgradefail,&preupgradefailure::exitappsignal,this,&MainWindow::exitapp);
        connect(preupgradesucc,&preupgradesuccess::nextpagesignal,this,&MainWindow::upgradepage);
        connect(upgradepro,&upgradeprocess::upgradefailsignal,this,&MainWindow::upgradefailpage);
        connect(upgradepro,&upgradeprocess::upgradesuccesssignal,this,&MainWindow::upgradesuccesspage);//success指更新进度完成，是否成功需要重启后知道。
        connect(upgradefail,&upgradefailure::exitappsignal,this,&MainWindow::exitapp);
        connect(upgradesucc,&upgradesuccess::exitappsignal,this,&MainWindow::exitapp);
        connect(upgradesucc,&upgradesuccess::rebootsignal,this,&MainWindow::reboot);
    }
    {
        connect(rspg1,&restorepage1::exitappsignal,this,&MainWindow::exitapp);
        connect(rspg1,&restorepage1::nextpage,this,&MainWindow::rspage2);
        connect(rspg1,&restorepage1::showattentionsignal,this,&MainWindow::showattentionback);
        connect(attentionback,&attentionpage::returnsignal,this,&MainWindow::initialpage);
        connect(rspg2,&restorepage2::checkupdatefinishsignal,this,&MainWindow::checkrestorefinish);
        connect(prerestorefail,&prerestorefailure::exitappsignal,this,&MainWindow::exitapp);
        connect(prerestoresucc,&prerestoresucess::nextpagesignal,this,&MainWindow::restorepage);
        connect(restorepro,&restoreprogress::upgradefailsignal,this,&MainWindow::restorefailpage);
        connect(restorepro,&restoreprogress::upgradesuccesssignal,this,&MainWindow::restoresuccesspage);//success指更新进度完成，是否成功需要重启后知道。
        connect(restorefail,&restorefailure::exitappsignal,this,&MainWindow::exitapp);
        connect(restoresucc,&restoresuccess::exitappsignal,this,&MainWindow::exitapp);
        connect(restoresucc,&restoresuccess::rebootsignal,this,&MainWindow::reboot);
    }
}

void MainWindow::restoresuccesspage()
{
    qDebug()<<"jump into the restoresuccess";
    m_stackWidget->setCurrentWidget(restoresucc);
    restoresucc->m_timer->start();
}

void MainWindow::restorefailpage()
{
    restorelist=restorepro->failure;
    restoresuggestionlist=restorepro->Suggest;
    qDebug()<<"jump into the restorefailpage";
    if(!restorelist.isEmpty())
    {
        qDebug()<<"upgrade fail";
        QString var;
        foreach (var, restorelist) {
            qDebug()<<var<<"var is var"<<restorelist;
            int number = restorelist.indexOf(var);
            qDebug()<<"var number is "<<number;
            QString var2 = restoresuggestionlist.value(number);
            QString reason;
            if(var2.isEmpty())
            {
                reason = var + "失败";
                qDebug()<<"var2.isEmpty()"<<reason;
//                restorefail->content->append(reason);
            }
            else
            {
                reason = var + "失败，"+ var2;
                qDebug()<<"var2.is Not Empty()"<<reason;
//                restorefail->content->append(reason);
            }
//            upgradefail->content->append(reason);
        }
    }
    this->showNormal();
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    titlewidget->show();
    m_stackWidget->setCurrentWidget(restorefail);
}

void MainWindow::restorepage()
{
    qDebug()<<"jump into the restorepage";
    dbus = dbusconnect::getInstance();
    dbus->interface->asyncCall("fallbackVersion");
    m_stackWidget->setCurrentWidget(restorepro);
//    restorepro->m_timer->start();
}

void MainWindow::restorenotificationpage()
{
    qDebug()<<"get into restorenotificationpage page";
    m_stackWidget->setCurrentWidget(restorenotify);
}

void MainWindow::checkrestorefinish()
{
    restorelist=rspg2->failure;
    restoresuggestionlist=rspg2->Suggest;
    qDebug()<<"list list is "<<restorelist;
    if(restorelist.isEmpty())
    {
        qDebug()<<"check success";
        m_stackWidget->setCurrentWidget(prerestoresucc);
        prerestoresucc->m_timer->start();
    }
    else
    {
        qDebug()<<"check fail";
        QString var;
        foreach (var, restorelist) {
            qDebug()<<var<<"var is var"<<restorelist;
            int number = restorelist.indexOf(var);
            qDebug()<<"var number is "<<number;
            QString var2 = restoresuggestionlist.value(number);
            qDebug()<<"var2 value is "<<var2;
            if(var2.isEmpty())
            {
                QString reason = var + "失败";
                qDebug()<<"var2.isEmpty()"<<reason;
                prerestorefail->content->append(reason);
            }
            else
            {
                QString reason = var + "失败，"+ var2;
                qDebug()<<"var2.is Not Empty()"<<reason;
                prerestorefail->content->append(reason);
            }
//            upgradefail->content->append(reason);
        }
        setAutoFillBackground(true);
        setBackgroundRole(QPalette::Base);
        this->showNormal();
        titlewidget->show();
        m_stackWidget->setCurrentWidget(prerestorefail);
    }
}

void MainWindow::initialpage()
{
    m_stackWidget->setCurrentWidget(rspg1);
}

void MainWindow::rspage2()//需要和stackwidget里面的部件联动。接受信号，然后上下页
{
    qInfo()<<"MainWindow::rspage2";
    m_stackWidget->setCurrentWidget(rspg2);
    showFullScreen();
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    m_stackWidget->setAutoFillBackground(true);
    m_stackWidget->setBackgroundRole(QPalette::Base);

    titlewidget->hide();
    dbus = dbusconnect::getInstance();
    dbus->interface->asyncCall("preFallbackCheck");
}

void MainWindow::reboot()
{
    qDebug()<<"reboot plz";
    system("reboot");
}

void MainWindow::upgradesuccesspage()
{
    qDebug()<<"jump into the upgradesuccess";
    m_stackWidget->setCurrentWidget(upgradesucc);
    upgradesucc->m_timer->start();
}

void MainWindow::upgradefailpage()
{
    upgradelist=upgradepro->failure;
    upgradesuggestionlist=upgradepro->Suggest;
    qDebug()<<"jump into the upgradefail";
//    m_stackWidget->setCurrentWidget(upgradefail);
    if(!upgradelist.isEmpty())
    {
        qDebug()<<"upgrade fail";
        QString var;
        foreach (var, upgradelist) {
            qDebug()<<var<<"var is var"<<upgradelist;
            int number = upgradelist.indexOf(var);
            qDebug()<<"var number is "<<number;
            QString var2 = upgradesuggestionlist.value(number);
            QString reason;
            if(var2.isEmpty())
            {
                reason = var + "失败";
                qDebug()<<"var2.isEmpty()"<<reason;
                upgradefail->content->append(reason);
            }
            else
            {
                reason = var + "失败，"+ var2;
                qDebug()<<"var2.is Not Empty()"<<reason;
                upgradefail->content->append(reason);
            }
        }
    }
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    this->showNormal();
    titlewidget->show();
    m_stackWidget->setCurrentWidget(upgradefail);
}

void MainWindow::upgradepage()
{
    qDebug()<<"jump into the restorepage";
    dbus = dbusconnect::getInstance();
    QString path = uppg1->s;
    qDebug()<<"upgrade"<<path;
    dbus->interface->asyncCall("upgradeVersion",path);
    m_stackWidget->setCurrentWidget(upgradepro);
//    upgradepro->m_timer->start();
}

void MainWindow::showadvancedoptionspage()
{
    advoptionpage->setupselecteditems();
    m_stackWidget->setCurrentWidget(advoptionpage);
}

void MainWindow::showimportactivationfilespage()
{
    m_stackWidget->setCurrentWidget(importactivationfilespage);
}

void MainWindow::showinitpage()
{
    m_stackWidget->setCurrentWidget(initpg);
}

void MainWindow::notificationpage()
{
    qDebug()<<"get into notification page";
    m_stackWidget->setCurrentWidget(upgradenotify);
}

void MainWindow::checkupdatefinish()
{
    upgradelist=uppg2->failure;
    upgradesuggestionlist=uppg2->Suggest;
    qDebug()<<"list list is "<<upgradelist;
    if(upgradelist.isEmpty())
    {
        qDebug()<<"check success";
        m_stackWidget->setCurrentWidget(preupgradesucc);
        preupgradesucc->m_timer->start();
    }
    else
    {
        qDebug()<<"check fail";
        QString var;
        foreach (var, upgradelist) {
            qDebug()<<var<<"checkupdatefinish var is var"<<upgradelist;
            int number = upgradelist.indexOf(var);
            qDebug()<<"checkupdatefinish var number is "<<number;
            QString var2 = upgradesuggestionlist.value(number);
            qDebug()<<"checkupdatefinish var2 value is "<<var2;
            QString reason = var + "失败，"+ var2;
            preupgradefail->content->append(reason);
        }
        setAutoFillBackground(true);
        setBackgroundRole(QPalette::Base);
        this->showNormal();
        titlewidget->show();
        m_stackWidget->setCurrentWidget(preupgradefail);
    }
}

void MainWindow::exitapp()
{
    this->close();
}

void MainWindow::SteptoUpgradePage()
{
    m_stackWidget->setCurrentWidget(uppg1);
}

void MainWindow::SteptoRestorePage()
{
    m_stackWidget->setCurrentWidget(rspg1);
}

void MainWindow::uppage2()//需要和stackwidget里面的部件联动。接受信号，然后上下页
{
    this->showFullScreen();
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    m_stackWidget->setAutoFillBackground(true);
    m_stackWidget->setBackgroundRole(QPalette::Base);
    titlewidget->hide();
    m_stackWidget->setCurrentWidget(uppg2);
    dbus = dbusconnect::getInstance();
    QString path = uppg1->s;
    qDebug()<<"upgrade"<<path;
    dbus->interface->asyncCall("preUpgradeCheck",path);
}


void MainWindow::lastpage()
{
    if(pagenumber>0)
        pagenumber=pagenumber-1;
    m_stackWidget->setCurrentIndex(pagenumber);
}

void MainWindow::showattentionup()
{
    m_stackWidget->setCurrentWidget(attentionup);
}
void MainWindow::showattentionback()
{
    m_stackWidget->setCurrentWidget(attentionback);
}
void MainWindow::firstpage()
{
    m_stackWidget->setCurrentWidget(uppg1);
}

MainWindow::~MainWindow()
{
    QDBusInterface *interface;
    interface = new QDBusInterface("org.gnome.SessionManager",
                                   "/org/gnome/SessionManager",
                                   "org.gnome.SessionManager",
                                   QDBusConnection::sessionBus());
    QDBusMessage reply = interface->call("Uninhibit", m_inhibitValue);
}

