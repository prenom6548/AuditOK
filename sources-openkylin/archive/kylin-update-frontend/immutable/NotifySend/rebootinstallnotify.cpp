#include "rebootinstallnotify.h"
/*2309以下版本使用(不含)*/
RebootInstallNotify::RebootInstallNotify(QWidget *parent) : QWidget(parent)
{
    qInfo()<<"[INFO]get in RebootInstallNotify";
    timer = new QTimer(this);
    timer1 = new QTimer(this);
//    timer->setInterval(10);
    qInfo()<<connect(timer,&QTimer::timeout,this,&RebootInstallNotify::timenotification_2107);
    connect(timer1,&QTimer::timeout,this,&RebootInstallNotify::hourdebug);
    qInfo()<<"[INFO]finish dbus connection";
    timer->start(24*60*60*1000);
//    timer->start(2*60*1000);
    timer1->start(60*60*1000);
}

void RebootInstallNotify::timenotification_2107()
{
    timer->stop();
    qInfo()<<"[INFO]timenotification_2107 timeout";
    qInfo()<<"RebootInstallNotify timenotification_2107";
    QStringList actions;
    actions.append("NotifySend --PowercheckNotify");
    actions.append(QObject::tr("Reboot"));
    actions.append("NotifySend --RebootInstallNotify");
    actions.append(QObject::tr("Later"));
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    QVariantMap hints;
    args<<(QObject::tr("System-Upgrade"))
       <<((unsigned int) 0)
      <<("ukui-control-center")
         <<QObject::tr("Update download completed, do you want to restart the system?") //控制面板-更新提示
        <<QObject::tr("After restarting, the new system will be used.") //显示的具体信息
    <<actions
    <<hints
    <<(int)0;
    QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    if (response.type() == QDBusMessage::ReplyMessage)
    { //判断method的返回值
        qInfo() << "发送成功！ID：" << response.arguments().takeFirst().toString();
        ID = response.arguments().takeFirst().toUInt(); //保存消息ID
        qInfo()<<"消息ID： "<<ID;
    }
    else
    {
        qInfo() << "发送失败：" << response.type();
    }
    return ;
}

void RebootInstallNotify::ButtonAction(int id, QString key)
{
    if(id == ID)
    {
        if(key=="NotifySend --PowercheckNotify")
        {
            QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                       "/com/kylin/systemupgrade",
                                       "com.kylin.systemupgrade.interface",
                                       QDBusConnection::systemBus() );
            QDBusMessage rebootresult = upgrade_interface.call("DeployLatestUpdate","reboot");
            int  reboot =rebootresult.arguments().value(0).toInt();
            QString reason = rebootresult.arguments().value(1).toString();
            if(!reboot)
            {
                qInfo()<<"reboot success!";
            }
            QDBusConnection::systemBus().disconnect("org.freedesktop.Notifications",
                                                 "/org/freedesktop/Notifications",
                                                 "org.freedesktop.Notifications",
                                                 "ActionInvoked",this,SLOT(ButtonAction(int,QString)));
            return;
        }
        else
        {
            QDBusConnection::systemBus().disconnect("org.freedesktop.Notifications",
                                                 "/org/freedesktop/Notifications",
                                                 "org.freedesktop.Notifications",
                                                 "ActionInvoked",this,SLOT(ButtonAction(int,QString)));
            return;
        }
    }

}

void RebootInstallNotify::hourdebug()
{
    qInfo()<<"hourdebug";
    timer1->stop();
    timer1->start(60*60*1000);
}
