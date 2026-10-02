#include "rfkill-state.h"
#include "usd_base_class.h"

#include "rfkillswitch.h"
#include "clib-syslog.h"

#include <QDebug>

#define MEDIA_SCHEMA   "org.ukui.SettingsDaemon.plugins.media-keys-state"
static QString s_rfkillState = "rfkill-state";

static QString s_netDbusName        = "org.freedesktop.NetworkManager";
static QString s_netDbusPath        = "/org/freedesktop/NetworkManager";
static QString s_netDbusInterface   = "org.freedesktop.NetworkManager";
static QString s_propInterface      = "org.freedesktop.DBus.Properties";
static QString s_netPropRadio       = "RadioFlags";
static QString s_netPropWlanState   = "WirelessEnabled";

RfkillState::RfkillState(QObject *parent) : QObject(parent)
{
    initialization();
}

RfkillState::~RfkillState()
{
    if (m_rfkillSettings != nullptr) {
        delete m_rfkillSettings;
        m_rfkillSettings = nullptr;
    }
}

RfkillState *RfkillState::self()
{
    static RfkillState rfkillState;
    return &rfkillState;
}

void RfkillState::initialization()
{
    if (!QGSettings::isSchemaInstalled(QByteArray(MEDIA_SCHEMA))) {
        SYS_LOG(LOG_WARNING, "Gsettings %s is not installed!", QString(MEDIA_SCHEMA).toUtf8().data());
        return;
    }

    m_rfkillSettings = new QGSettings(MEDIA_SCHEMA);
    if (m_rfkillSettings == nullptr) {
        SYS_LOG(LOG_WARNING, "Init m_rfkillSettings error!");
        return;
    }

    connect(m_rfkillSettings, SIGNAL(changed(QString)),  this, SLOT(doSettingsChangeAction(const QString&)));

    if (!UsdBaseClass::isNotebook()) {
        SYS_LOG(LOG_NOTICE, "Device is not notebook, don't have flight mode!");
        setSettingsState(-1);
        return;
    }

    QVariant state = getGlobalRfkillState();
    setSettingsState(state.isValid()? state.toBool() : 0);

    // 用户状态切换信号
    QDBusConnection::sessionBus().connect(QString(),\
                                          QStringLiteral("/GlobaSignal"),\
                                          QStringLiteral("org.ukui.SettingsDaemon.GlobalSignal"),\
                                          "Active", this, SLOT(onUserActive(bool)));
}


bool RfkillState::isSupportWlan()
{
    QDBusInterface networkDbus(s_netDbusName,
                               s_netDbusPath,
                               s_propInterface,
                               QDBusConnection::systemBus());

    if (!networkDbus.isValid()) {
        SYS_LOG(LOG_WARNING, "NetworkManager interface init error! Dbus may be unavailable");
        return false;
    }

    QDBusReply<QVariant> reply = networkDbus.call("Get", s_netDbusInterface, s_netPropRadio);
    return reply.isValid()? (reply.value().toUInt() & 0X01) : false;
}

bool RfkillState::getWlanState()
{
    QDBusInterface networkDbus(s_netDbusName,
                               s_netDbusPath,
                               s_propInterface,
                               QDBusConnection::systemBus());

    if (!networkDbus.isValid()) {
        SYS_LOG(LOG_WARNING, "NetworkManager interface init error! Dbus may be unavailable");
        return false;
    }

    QDBusReply<QVariant> reply = networkDbus.call("Get", s_netDbusInterface, s_netPropWlanState);
    return reply.isValid()? reply.value().toBool() : false;
}

bool RfkillState::setWlanState(bool state)
{
    QDBusInterface networkDbus(s_netDbusName,
                               s_netDbusPath,
                               s_propInterface,
                               QDBusConnection::systemBus());

    if (!networkDbus.isValid()) {
        SYS_LOG(LOG_WARNING, "NetworkManager interface init error! Dbus may be unavailable");
        return false;
    }

    QDBusMessage msg = networkDbus.call("Set", s_netDbusInterface, s_netPropWlanState, QVariant::fromValue(QDBusVariant(state)));
    if (!msg.errorMessage().isEmpty()) {
        SYS_LOG(LOG_WARNING, "Set network property WirelessEnabled error, error msg is %s", msg.errorMessage().toUtf8().data());
    }
    return msg.errorMessage().isEmpty();
}

int RfkillState::getFlightState() const
{
    return getSettingsState();
}

void RfkillState::setFlightState(bool state)
{
    setSettingsState(state);
}

int RfkillState::getBluetooth()
{
    return RfkillSwitch::instance()->getCurrentBluetoothMode();
}

void RfkillState::setBluetooth(bool state)
{
    RfkillSwitch::instance()->toggleBluetoothMode(state);
}

void RfkillState::setSettingsState(int state)
{
    if (m_rfkillSettings != nullptr && m_rfkillSettings->keys().contains(s_rfkillState)) {
        m_rfkillSettings->set(s_rfkillState, state);
    }
}

int RfkillState::getSettingsState() const
{
    if (m_rfkillSettings == nullptr || !m_rfkillSettings->keys().contains(s_rfkillState)) {
        return -1;
    }
    return m_rfkillSettings->get(s_rfkillState).toInt();

}

void RfkillState::setGlobalRfkillState(int state)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/globalconfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());
    globalManager.asyncCall("SetFlightModeState", state);
}

QVariant RfkillState::getGlobalRfkillState()
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                          "/globalconfig", \
                          SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                          QDBusConnection::systemBus());


    QDBusReply<int> reply = globalManager.call("GetFlightModeState");
    if (reply.isValid()) {
        return reply.value();
    }
    USD_LOG(LOG_ERR,"globalManager dbus interface failed .");
    return QVariant(QVariant::Invalid);
}

void RfkillState::doSettingsChangeAction(const QString &key)
{

    if (key == s_rfkillState) {
        int state = getSettingsState();
        setGlobalRfkillState(state);
    }
}

void RfkillState::onUserActive(bool state)
{
    //用户登录状态改变，获取全局配置，设置飞行模式状态
    if (state) {
        QVariant state = getGlobalRfkillState();
        if (state.isValid()) {
            setSettingsState(state.toBool());
        }
    }
}
