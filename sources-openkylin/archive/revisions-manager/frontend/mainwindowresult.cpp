#include "mainwindowresult.h"

MainWindowResult::MainWindowResult(QWidget *parent) : QWidget(parent)
{
//    dbusinitial();
    initui();
    dbusconnection();
}

void MainWindowResult::initui()
{
    this->setObjectName("MainWindow");
//    this->setStyleSheet("QWidget{background: #F2F2F2;border-radius:12px}");
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    main = new QHBoxLayout(this);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0,0);
    shadow->setColor("#444444");
    shadow->setBlurRadius(20);
    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->setGraphicsEffect(shadow);
//    m_stackWidget->setStyleSheet("QStackedWidget{background: #FFFFFF}");
    m_stackWidget->setFixedSize(480,520);
    addpages();
    main->setMargin(20);
    main->addWidget(m_stackWidget);
}

void MainWindowResult::dbusinitial(QString state)
{
    qDebug()<<"change the state"<<state;
    status = state;
}

QStringList MainWindowResult::getlist()
{
    QDBusInterface upgrade_interface( "com.kylin.RevisionsManager",
                                     "/com/kylin/RevisionsManager",
                                     "com.kylin.RevisionsManager.interface",
                                     QDBusConnection::systemBus() );
    QDBusMessage modelist = upgrade_interface.call("getFailedPackageList");
    QStringList reason = modelist.arguments().value(0).toStringList();
//    QString reason = modelist.arguments().value(1).toString();
//    if(!reason.isEmpty())
//    {
//        qDebug()<<"获取状态接口异常";
//        return "error";
//    }
    return reason;
}

void MainWindowResult::reset()
{
    QDBusInterface upgrade_interface( "com.kylin.RevisionsManager",
                                     "/com/kylin/RevisionsManager",
                                     "com.kylin.RevisionsManager.interface",
                                     QDBusConnection::systemBus() );
    QDBusMessage reset = upgrade_interface.call("addNoticeCount");
    qDebug()<<"addNoticeCount reset";
}

QString MainWindowResult::getreason()
{
    QDBusInterface upgrade_interface( "com.kylin.RevisionsManager",
                                     "/com/kylin/RevisionsManager",
                                     "com.kylin.RevisionsManager.interface",
                                     QDBusConnection::systemBus() );
    QDBusMessage modelist = upgrade_interface.call("getState");
    QString reason = modelist.arguments().value(1).toString();
    if(reason.isNull())
    {
        qDebug()<<"获取状态接口异常";
        return "error";
    }
    return reason;
}

QString MainWindowResult::getState()
{
    QDBusInterface upgrade_interface( "com.kylin.RevisionsManager",
                                     "/com/kylin/RevisionsManager",
                                     "com.kylin.RevisionsManager.interface",
                                     QDBusConnection::systemBus() );
    QDBusMessage modelist = upgrade_interface.call("getState");
    QString result = modelist.arguments().value(0).toString();
    if(result.isNull())
    {
        qDebug()<<"获取状态接口异常";
        return "error";
    }
    return result;
}

QString MainWindowResult::getversion()
{
    dbusconnect *dbus = dbusconnect::getInstance();
    QDBusReply<QString> mode = dbus->interface->call("getCurOsVersion");
    if(!mode.isValid())
    {
        qDebug()<<"获取版本接口异常";
        return "error";
    }
    QString modereply = mode.value();
    qDebug()<<modereply;
    return modereply;
}

void MainWindowResult::addpages()
{
    QString state = getState();
    if(state == "upsuccess")
    {
        qDebug()<<"get into the 升级结果界面";
        upsucc = new UpgradeSucc();
//        upsucc->setStyleSheet("border-radius:6px");
        m_stackWidget->addWidget(upsucc);
        m_stackWidget->setCurrentWidget(upsucc);
        reset();
    }
    else if (state == "upfailed")
    {
        qDebug()<<"get into the 升级结果界面";
        upfail = new UpgradeFail();
//        upfail->setStyleSheet("border-radius:6px");
        m_stackWidget->addWidget(upfail);
        QString reason = getreason();
        upfail->content->append(reason);
        m_stackWidget->setCurrentWidget(upfail);
        reset();
    }
    else if (state == "backsuccess")
    {
        qDebug()<<"get into the 还原结果界面";
        resucc = new RestoreSucc();
//        resucc->setStyleSheet("border-radius:6px");
//        m_stackWidget->setStyleSheet("QStackedWidget{background: #FFFFFF}");
        m_stackWidget->addWidget(resucc);
        m_stackWidget->setCurrentWidget(resucc);
        reset();
    }
    else if (state == "backfailed")
    {
        qDebug()<<"get into the 还原结果界面";
        refail = new RestoreFail();
//        refail->setStyleSheet("border-radius:6px");
//        m_stackWidget->setStyleSheet("QStackedWidget{background: #FFFFFF}");
        m_stackWidget->addWidget(refail);
        QString reason = getreason();
        refail->content->append(reason);
        m_stackWidget->setCurrentWidget(refail);
        reset();
    }
    else{
        qDebug()<<"close";
        this->close();
        reset();
    }
}

void MainWindowResult::dbusconnection()
{

        connect(refail,&RestoreFail::exitappsignal,this,&MainWindowResult::exitapp);
        connect(resucc,&RestoreSucc::exitappsignal,this,&MainWindowResult::exitapp);
        connect(this,SIGNAL(exitappsignal()),this,SLOT(exitapp()));
        connect(upfail,&UpgradeFail::exitappsignal,this,&MainWindowResult::exitapp);
        connect(upnote,&UpgradeNote::exitappsignal,this,&MainWindowResult::exitapp);
        connect(upsucc,&UpgradeSucc::exitappsignal,this,&MainWindowResult::exitapp);
        connect(this,&MainWindowResult::exitappsignal,this,&MainWindowResult::exitapp);
        connect(upnote,&UpgradeNote::showdeblistsignal,this,&MainWindowResult::showdeblist);
        connect(upfaillist,&upgradefaillist::backtopagesignal,this,&MainWindowResult::shownote);
}

void MainWindowResult::exitapp()
{
    qDebug()<<"method exitapp";
    this->close();
}

void MainWindowResult::shownote()
{
    qDebug()<<"set stackwidget to upnote";
    m_stackWidget->setCurrentWidget(upnote);
}

void MainWindowResult::showdeblist()
{
    qDebug()<<"set stackwidget to showdedlist";
    m_stackWidget->setCurrentWidget(upfaillist);
}


MainWindowResult::~MainWindowResult()
{
}

