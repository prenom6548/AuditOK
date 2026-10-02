#include "desktopnotify.h"

DesktopNotify::DesktopNotify(QWidget *parent) : QWidget(parent)
{
    version=getVersion();
    QDBusInterface iface("com.kylin.systemupgrade",
                         "/com/kylin/systemupgrade",
                         "com.kylin.systemupgrade.interface",
                         QDBusConnection::systemBus());
    QDBusMessage result = iface.call("CheckInstallRequired");
    int  reboot =result.arguments().value(0).toInt();
    qInfo()<<"reboot result is"<<reboot;
    if(reboot)
    {
        DesktopNotify::RebootNotify();
    }
}

void DesktopNotify::RebootNotify()
{
    qInfo()<<"[INFO]get signal UpdateDownloadFinished, come into DesktopNotify's RebootNotify";
    QStringList actions;
    actions.append("NotifySend --PowercheckNotify");
    actions.append(QObject::tr("Reboot"));
    actions.append("NotifySend --RebootInstallNotify");
    actions.append(QObject::tr("Later"));
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if(version.isEmpty())// == "2107"||version == "2203"||version == "2303"
    {
        qInfo()<<"DesktopNotify 2107";
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
    }
    else
    {
        qInfo()<<"DesktopNotify 2309";
        QList<QVariant> args;
        QVariantMap hints;
        hints.insert("sound-name","Complete");
        hints.insert("x-ukui-popup-timeout",-1);
        args<<(QObject::tr("System-Upgrade"))
        <<((unsigned int) 0)
        <<("ukui-control-center")
        <<QObject::tr("Update download completed, do you want to restart the system?") //控制面板-更新提示
        <<QObject::tr("After restarting, the new system will be used.") //显示的具体信息
        <<actions
        <<hints
        <<(int)0;
        QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
        qInfo()<<" 2309 buttonaction conenct"
               <<QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                                       "/org/freedesktop/Notifications",
                                                       "org.freedesktop.Notifications",
                                                       "ActionInvoked",this,SLOT(ButtonAction_2309(uint,QString)));
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
    }
    return ;
}

void DesktopNotify::ButtonAction_2309(uint id, QString key)
{
    qInfo()<<"the id is" << id <<"and ID is "<<ID;
    if(id == ID)
    {
        if(key=="NotifySend --PowercheckNotify")
        {
            qInfo()<<"立即重启";
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
            QDBusConnection::sessionBus().disconnect("org.freedesktop.Notifications",
                                                 "/org/freedesktop/Notifications",
                                                 "org.freedesktop.Notifications",
                                                 "ActionInvoked",this,SLOT(ButtonAction_2309(uint,QString)));
            return;
        }
        else if(key == "NotifySend --RebootInstallNotify")
        {
            qInfo()<<"稍后重启，24小时提醒功能添加";
            timer = new QTimer(this);
            qInfo()<<connect(timer,&QTimer::timeout,this,&DesktopNotify::timenotification_2309);
            QDBusConnection::sessionBus().disconnect("org.freedesktop.Notifications",
                                                 "/org/freedesktop/Notifications",
                                                 "org.freedesktop.Notifications",
                                                 "ActionInvoked",this,SLOT(ButtonAction_2309(uint,QString)));
            timer->start(24*60*60*1000);
//            timer->start(2*60*1000);
            return;
        }
    }
}

void DesktopNotify::timenotification_2309()
{
    timer->stop();
    qInfo()<<"[INFO]timenotification_2309 timeout";
    qInfo()<<"DesktopNotify timenotification_2309";
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
    hints.insert("sound-name","Complete");
    hints.insert("x-ukui-popup-timeout",-1);
    args<<(QObject::tr("System-Upgrade"))
       <<((unsigned int) 0)
      <<("ukui-control-center")
         <<QObject::tr("Update download completed, do you want to restart the system?") //控制面板-更新提示
        <<QObject::tr("After restarting, the new system will be used.") //显示的具体信息
    <<actions
    <<hints
    <<(int)0;
    QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    qInfo() <<QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                         "/org/freedesktop/Notifications",
                                         "org.freedesktop.Notifications",
                                         "ActionInvoked",this,SLOT(ButtonAction_2309(uint,QString)));
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

QString DesktopNotify::getVersion()
{
//    QFile file("/etc/os-release");
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//    {
//        // 文件无法打开，处理错误
//         return version_t;
//    }

//    // 使用 QTextStream 读取文件内容
//    QTextStream in(&file);
//    while (!in.atEnd())
//    {
//        QString line = in.readLine();
//        if(line.contains("KYLIN_RELEASE_ID"))
//        {
//            QStringList versionlist = line.split("=");
//            QString version1 = versionlist.at(1);
//            QString version2 = version1.remove(QChar('"'));
////            qDebug()<<"version2 is "<<version2;
//            version_t = version2;
//        }
////        qDebug() << line; // 将每一行的内容打印出来
//    }

//    // 关闭文件
//    file.close();
//    return version_t;

    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QDBusMessage result = iface.call("GetServerInformation");
    if (!result.errorMessage().isEmpty())
    {
        qInfo() << "DBus调用失败：" << result.errorMessage();
        return version_t;
    }
    else
    {
        qInfo() << "DBus调用成功";
        version_t = result.arguments().value(2).toString();
        qInfo()<< version_t;
        return version_t;
    }
}

DesktopNotify::~DesktopNotify()
{
}
