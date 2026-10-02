#include "updatedbus.h"
#include "connection.h"
#include <QMutexLocker>
#define PROGRAM_NAME "control-upgrade"
#define PATH_MAX_LEN 1024
#define PID_STRING_LEN 64

UpdateDbus* UpdateDbus::updateMutual = nullptr;
using namespace std;

UpdateDbus* UpdateDbus::getInstance(QObject *parent)
{
    static QMutex mutex;
    if(nullptr == updateMutual)
    {
        QMutexLocker locker(&mutex);
        updateMutual = new UpdateDbus(parent);
    }
    return updateMutual;
}

UpdateDbus::UpdateDbus(QObject *parent)
    :QObject(parent)
{
    qRegisterMetaType<AppMsg>("AppMsg"); //注册信号槽类型
    qRegisterMetaType<AppAllMsg>("AppAllMsg"); //注册信号槽类型
    strategy_interface = new QDBusInterface("com.kylin.UpgradeStrategies",
                                            "/com/kylin/UpgradeStrategies",
                                            "com.kylin.UpgradeStrategies.interface",
                                            QDBusConnection::systemBus());
    interface = new QDBusInterface("com.kylin.systemupgrade",
                                   "/com/kylin/systemupgrade",
                                   "com.kylin.systemupgrade.interface",
                                   QDBusConnection::systemBus());
    qDebug() << "interface 成功";
    emit startDbusFinished();
}

bool UpdateDbus::UpgradeAll()
{
    interface->asyncCall("UpdateDownloadAll");
    return true;
}






void UpdateDbus::SetDownloadLimit(QString value,bool whetherlimit)
{
    interface->call("SetDownloadspeedMax",value,whetherlimit);
}

void UpdateDbus::SetAutoUpgrade(bool gs,bool allow)
{
    if(allow)
        interface->call("GetSetDatabaseInfo",gs,"display","autoupdate_allow=true");
    else
        interface->call("GetSetDatabaseInfo",gs,"display","autoupdate_allow=false");
}

void UpdateDbus::SetAutoUpgradeState(bool allow)
{
    strategy_interface->call("SetAutoUpgradeState",allow);
}

QString UpdateDbus::GetAutoUpgrade(bool gs)
{
    QDBusPendingReply<QString> reply = interface->call("GetSetDatabaseInfo",gs,"display","autoupdate_allow");
    return reply;
}

int UpdateDbus::GetDownloadLimit(void)
{
    QDBusPendingReply<int> reply = interface->call("GetDownloadspeedLimitValue");
    if (!reply.isValid())
    {
        qDebug()<<"error getting download speed limit value";
        return -1;
    }
    if (reply.argumentAt(0)==true)
    {
        return reply.argumentAt(1).toInt();
    }
    else
    {
        return -2;
    }
}

void UpdateDbus::onRequestSendDesktopNotify(QString message)
{
    qInfo()<<"get in onRequestSendDesktopNotify fuction";
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if (!notifyMsg.compare(message)) {
        return ;
    }
    QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                  "/com/kylin/systemupgrade",
                                  "com.kylin.systemupgrade.interface",
                                  QDBusConnection::systemBus() );
    QDBusMessage modelist = upgrade_interface.call("GetConfigValue","InstallMode","shutdown_install");
    QString  mode =modelist.arguments().value(1).toString();
    qInfo()<<"[INFO]the mode is "<<mode;
    if(mode !="True")
    {
        notifyMsg = message;
        QList<QVariant> args;
        QVariantMap hints;
        hints.insert("sound-name","Complete");
        args<<(tr("System-Upgrade"))
            <<((unsigned int) 0)
            <<("ukui-control-center")
            <<tr("ukui-control-center-update") //显示的是什么类型的信息  控制面板-更新提示
            <<message //显示的具体信息
            <<QStringList()
            <<hints
            <<(int)-1;
        iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    }
}




bool UpdateDbus::DistUpgradePartial(bool status,QString pkgName)
{
    interface->asyncCall("DistUpgradePartial",status,pkgName);
    return true;
}

bool UpdateDbus::DistUpgradeSystem(bool status)
{
    interface->asyncCall("DistUpgradeSystem",status);
    return true;
}

bool UpdateDbus::CancelDownload()
{
    interface->asyncCall("CancelDownload");
    return true;
}

void UpdateDbus::callDBusUpdateSource()
{
   QDBusPendingCall call = interface->asyncCall("UpdateDetect");
   if (call.isValid()) {
       qDebug() << "UpdateDetect 失败";
   }
   qDebug() << "源管理器：" <<"Call updateSourcePackages" ;
}

void UpdateDbus::insertInstallStates(QString item, QString info)
{
    interface->asyncCall("InsertInstallState",item,info);
}

UpdateDbus::~UpdateDbus()
{
}
