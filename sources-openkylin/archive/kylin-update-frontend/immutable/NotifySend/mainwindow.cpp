#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /*分步更新的下载完成弹窗功能*/
    QDBusConnection::systemBus().connect("com.kylin.systemupgrade",
                                         "/com/kylin/systemupgrade",
                                         "com.kylin.systemupgrade.interface",
                                         "UpdateDownloadFinished",this,SLOT(RebootNotify(bool)));
    QDBusConnection::systemBus().connect("com.kylin.systemupgrade",
                                         "/com/kylin/systemupgrade",
                                         "com.kylin.systemupgrade.interface",
                                         "ErrorFeedbackBeforeInstallation",this,SLOT(RebootResult(bool,QString,QString)));
    version=getVersion();
}

void MainWindow::RebootNotify(bool success)
{
    qInfo()<<"[INFO]get signal UpdateDownloadFinished, come into Mainwindow's RebootNotify";
    qInfo()<<"the bool is "<<success;
    qInfo()<<version;
    if(success)
    {
        QStringList actions;
        actions.append("NotifySend --PowercheckNotify");
        actions.append(QObject::tr("Reboot"));
        actions.append("NotifySend --RebootInstallNotify");
        actions.append(QObject::tr("Later"));
        QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             QDBusConnection::sessionBus());
        if(version.isEmpty())
        {
            qInfo()<<"mainwindow 2107 reboot notification";
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
        else/*2309+*/
        {
            qInfo()<<"mainwindow 2309 reboot notification";
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
}

void MainWindow::ButtonAction_2309(uint id, QString key)
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
            qInfo()<<connect(timer,&QTimer::timeout,this,&MainWindow::timenotification_2309);
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

void MainWindow::RebootResult(bool success,QString error_code, QString reason)
{
    if(!success)
    {
        if(error_code == "#0208")
        {
            QDBusInterface iface("org.freedesktop.Notifications",
                                 "/org/freedesktop/Notifications",
                                 "org.freedesktop.Notifications",
                                 QDBusConnection::sessionBus());
            QList<QVariant> args;
            args<<(QObject::tr("System-Upgrade"))
            <<((unsigned int) 0)
            <<("ukui-control-center")
            <<QObject::tr("Low battery level, system update terminated") //控制面板-更新提示
            <<QObject::tr("System updates are carried out when the battery level is not less than 50%.") //显示的具体信息
            <<QStringList()
            <<QVariantMap()
            <<(int)0;
            QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
            if (response.type() == QDBusMessage::ReplyMessage)
            { //判断method的返回值
                qInfo() << "发送成功！ID：" << response.arguments().takeFirst().toString();
                int ID = response.arguments().takeFirst().toUInt(); //保存消息ID
                qInfo()<<"消息ID： "<<ID;
            }
            else
            {
                qInfo() << "发送失败：" << response.type();
            }
            return ;
        }
        else if(error_code == "#0204")
        {
            QDBusInterface iface("org.freedesktop.Notifications",
                                 "/org/freedesktop/Notifications",
                                 "org.freedesktop.Notifications",
                                 QDBusConnection::sessionBus());
            QList<QVariant> args;
            args<<(QObject::tr("System-Upgrade"))
               <<((unsigned int) 0)
              <<("ukui-control-center")
             <<QObject::tr("The system disk space is insufficient to complete the update.") //控制面板-更新提示
            <<""
            <<QStringList()
            <<QVariantMap()
            <<(int)0;
            QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
            if (response.type() == QDBusMessage::ReplyMessage)
            { //判断method的返回值
                qInfo() << "发送成功！ID：" << response.arguments().takeFirst().toString();
                int ID = response.arguments().takeFirst().toUInt(); //保存消息ID
                qInfo()<<"消息ID： "<<ID;
            }
            else
            {
                qInfo() << "发送失败：" << response.type();
            }
            return ;
        }
        else
        {
            QDBusInterface iface("org.freedesktop.Notifications",
                                 "/org/freedesktop/Notifications",
                                 "org.freedesktop.Notifications",
                                 QDBusConnection::sessionBus());
            QList<QVariant> args;
            args<<(QObject::tr("System-Upgrade"))
               <<((unsigned int) 0)
              <<("ukui-control-center")
             <<QObject::tr("Reboot failed.") //控制面板-更新提示
            <<""
            <<QStringList()
            <<QVariantMap()
            <<(int)0;
            QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
            if (response.type() == QDBusMessage::ReplyMessage)
            { //判断method的返回值
                qInfo() << "发送成功！ID：" << response.arguments().takeFirst().toString();
                int ID = response.arguments().takeFirst().toUInt(); //保存消息ID
                qInfo()<<"消息ID： "<<ID;
            }
            else
            {
                qInfo() << "发送失败：" << response.type();
            }
            return ;
        }
    }
}

void MainWindow::timenotification_2309()
{
    timer->stop();
    qInfo()<<"[INFO]timenotification timeout";
    qInfo()<<"mainwindow timenotification_2309";
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

QString MainWindow::getVersion()
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
//            qDebug()<<"version is "<<version_t;
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

MainWindow::~MainWindow()
{
}

