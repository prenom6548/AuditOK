#include "globalconfig.h"
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusMetaType>

#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "common.h"

#define DBUS_GC_NAME    "org.ukui.SettingsDaemon"
#define DBUS_GC_SIGNAL_PATH    "/GlobalSignal"
#define DBUS_GC_SIGNAL "org.ukui.SettingsDaemon.GlobalSignal"

#define DBUS_GC_INTERFACE_PATH "/globalconfig"
#define DBUS_GC_INTERFACE "com.settings.daemon.interface"

/*com.kylin.ukui.SettingsDaemon*/
#define SETTINGS_DAEMON_SYSTEMDBUS_NAME            "com.kylin.ukui.SettingsDaemon"
#define SETTINGS_DAEMON_SYSTEMDBUS_PATH            "/globalconfig"
#define SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE       "com.kylin.ukui.SettingsDaemon.interface"


#define DBUS_POWER_MANAGER_SETTINGS "powerManager"
#define DBUS_POWER_FULL_CONFIG   "config"


GlobalConfig::GlobalConfig(QObject *parent) : QObject(parent)
{

}

GlobalConfig::~GlobalConfig()
{
    if(m_isInit==true){
        m_isInit = false;
    }
}

void GlobalConfig::initGlobalConfig()
{
    if(m_isInit==true){
        return;
    }

    const bool ok = QDBusConnection::sessionBus().connect(
        DBUS_GC_NAME,
        DBUS_GC_SIGNAL_PATH,
        DBUS_GC_SIGNAL,
        "Active",
        this,
        SLOT(userActiveSlot(bool))
    );
    if (!ok) {
        qWarning() << "[GC] connect Active(bool) failed on session bus:"
                   << QDBusConnection::sessionBus().lastError().message();
    } else {
        qInfo() << "[GC] connect Active(bool) ok on session bus";
    }

    m_isInit = true;
}

void GlobalConfig::userActiveSlot(bool isLogin)
{
    qInfo() << "GlobalConfig user active " << isLogin;
    Q_EMIT(userActiveSignal(isLogin));
}

bool GlobalConfig::writeGlobalParam(const QString & key, const QVariant &value)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME,
                                 SETTINGS_DAEMON_SYSTEMDBUS_PATH,
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE,
                                 QDBusConnection::systemBus());
    QList<QVariant> arguments;
    arguments<<QVariant(QString::fromLatin1(DBUS_POWER_MANAGER_SETTINGS))
             <<QVariant(key)
             << QVariant::fromValue(QDBusVariant(value));
    QDBusPendingCall reply  = globalManager.asyncCallWithArgumentList("setGlobalConf", arguments);
    qDebug() << __func__ << arguments;
    qDebug() << __func__ << value;
    if (reply.isError()) {
        qInfo() <<"writeGlobalParam:"<< reply.error();
        return false;
    }
    qDebug() <<"writeGlobalParam:"<< key<<": "<<value;
    return true;
}

bool GlobalConfig::readAllGlobalParam(QVariantMap &oMap)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME,
                                 SETTINGS_DAEMON_SYSTEMDBUS_PATH,
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE,
                                 QDBusConnection::systemBus());

    QDBusReply<QVariantMap> reply = globalManager.call("getSchemaGlobalConf", DBUS_POWER_MANAGER_SETTINGS);

    if (!reply.isValid()) {
        qWarning() << "[GC] getSchemaGlobalConf failed:" << reply.error();
        return false;                   // 调用失败
    }

    oMap = reply.value();               // 调用成功
    qInfo() << "[GC] keys count =" << oMap.size() << "keys=" << oMap.keys();

    return true;
}
