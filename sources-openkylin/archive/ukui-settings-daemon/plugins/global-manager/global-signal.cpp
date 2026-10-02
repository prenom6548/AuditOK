#include "global-signal.h"

#include <QDebug>

GlobalSignal::GlobalSignal(QObject *parent)
{
    qRegisterMetaType<SessionStruct>("SessionStruct");
    qRegisterMetaType<SessionStructList>("SessionStructList");
    qRegisterMetaType<QList<QString>>("QList<QString>");

    qDBusRegisterMetaType<QList<QString>>();
    qDBusRegisterMetaType<SessionStruct>();
    qDBusRegisterMetaType<SessionStructList>();


    qRegisterMetaType<UKUILite>("UKUILite");
    qDBusRegisterMetaType<UKUILite>();
    connectUserLogin1Signal();


    QStringList disableOptions = getShutDownDisableOptionFromGlobalManager();
    QStringList intersection = m_allOptions.toSet().intersect(disableOptions.toSet()).toList();
    m_pGMsettings = new QGSettings(GSETTINGS_GLOBALMANAGER_SCHEMA);
    m_pGMsettings->set(DISABLE_POWEROFF_OPTIONS ,(QVariant)intersection);
    m_pGMsettings->set(HIDE_POWEROFF_UI,getHidePowerUiFromGlobalManager());

    connect(m_pGMsettings, SIGNAL(changed(QString)), this, SLOT(doGMsettinsChange(QString)), Qt::DirectConnection);
}

GlobalSignal::~GlobalSignal()
{
    if (m_pGMsettings != nullptr) {
        delete m_pGMsettings;
        m_pGMsettings = nullptr;
    }

    if (m_plogin2 != nullptr) {
        delete m_plogin2;
        m_plogin2 = nullptr;
    }
}

void GlobalSignal::sendUserActiveSignal(QString interface, QVariantMap qvarMap, QStringList strList)
{
    if (interface == "org.freedesktop.login1.Session") {
        QMap<QString,QVariant> mapKey = qvariant_cast<QMap<QString,QVariant>>(qvarMap);
        if (mapKey.contains("Active")) {
            bool isActive = qvariant_cast<bool>(mapKey["Active"]);
            QDBusMessage notifySignal =
                    QDBusMessage::createSignal(DBUS_GC_SIGNAL_PATH, DBUS_GC_SIGNAL_INTERFACE, DBUS_GC_SIGNAL_SIGNAL_Active);

            notifySignal.setArguments({QVariant::fromValue(isActive)});
            QDBusConnection::sessionBus().send(notifySignal);
            Q_EMIT Active(isActive);
            USD_LOG(LOG_DEBUG,"send active:%d",isActive);
        }
    }
}

bool GlobalSignal::isPresenceLightSensor()
{
    QGSettings gsettings(AUTO_BRIGHTNESS_SCHEMA);
    SYS_LOG_SHOW_PARAM1(gsettings.get(HAD_SENSOR_KEY).toBool());
    return gsettings.get(HAD_SENSOR_KEY).toBool();
}

bool GlobalSignal::isSupportGamma()
{
    static bool ret = UsdBaseClass::isSupportGamma();
    return ret;
}

QStringList GlobalSignal::getShutdownDisableOptions()
{
    /**
     * gsettings set org.ukui.SettingsDaemon.plugins.globalManager disable-shutdown-option  ['"switchuser"','"hibernate"','"suspend"','"lockscreen"','"logout"','"restart"','"shutdown"']
     * 全部屏蔽
     */

    QList<QString> disableOption = getShutDownDisableOptionInGsettings();
    QList<QString> intersection = m_allOptions.toSet().intersect(disableOption.toSet()).toList();

    bool ret = m_pGMsettings->get(HIDE_POWEROFF_UI).toBool();

    if (ret) {
        return m_allOptions;
    }

    return intersection;
}

bool GlobalSignal::isHidePoweroffUi()
{
    bool ret = m_pGMsettings->get(HIDE_POWEROFF_UI).toBool();

    if (!ret) {
        QList<QString> disableOption = getShutDownDisableOptionInGsettings();
        QList<QString> intersection = m_allOptions.toSet().intersect(disableOption.toSet()).toList();
        USD_LOG_SHOW_PARAM1(intersection.count());
        if (intersection.count()>=m_allOptions.count()) {
            return true;
        }
    }
    return m_pGMsettings->get(HIDE_POWEROFF_UI).toBool();
}

QMap<QString, QVariant> GlobalSignal::getUKUILiteLevel()
{
    QMap<QString, QVariant> ret;
    ret = getUKUILite();
    if (ret.count() > 0) {
        return ret;
    }

    ret.insert(UKUI_LITE_FUNCTION, "normal");
    ret.insert(UKUI_LITE_ANIMATION, "normal");
    return ret;
}

QString GlobalSignal::getUKUILiteFunction()
{
    return m_pGMsettings->get(UKUI_LITE_FUNCTION_KEY).toString();
}

QString GlobalSignal::getUKUILiteAnimation()
{
    return m_pGMsettings->get(UKUI_LITE_ANIMATION_KEY).toString();
}

void GlobalSignal::doGMsettinsChange(QString key)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/globalconfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());

    if (key == DISABLE_POWEROFF_OPTIONS) {
        QStringList disableOption = getShutDownDisableOptionInGsettings();
        QStringList intersection = m_allOptions.toSet().intersect(disableOption.toSet()).toList();

        QList<QVariant> arguments;
        arguments << QVariant(QString::fromLatin1(MODULE_NAME)) << QVariant(QString::fromLatin1(DISABLE_POWEROFF_OPTIONS)) << QVariant::fromValue(QDBusVariant(intersection));
        QDBusPendingCall reply  = globalManager.asyncCallWithArgumentList("setGlobalConf", arguments);
        if (reply.isError()) {
            USD_LOG(LOG_DEBUG,"error! %s %s",reply.reply().errorMessage().toLatin1().data());
        }
    } else if (key == HIDE_POWEROFF_UI){
        bool ret = m_pGMsettings->get(HIDE_POWEROFF_UI).toBool();

        QList<QVariant> arguments;
        arguments << QVariant(QString::fromLatin1(MODULE_NAME)) << QVariant(QString::fromLatin1(HIDE_POWEROFF_UI)) << QVariant::fromValue(QDBusVariant(ret));
        QDBusPendingCall reply = globalManager.asyncCallWithArgumentList("setGlobalConf", arguments);
        if (reply.isError()) {
            USD_LOG(LOG_ERR,"error! %s %s", reply.reply().errorMessage().toLatin1().data());
        }
    } else if (key == UKUI_LITE_FUNCTION_KEY || key == UKUI_LITE_ANIMATION_KEY) {
        Q_EMIT UKUILiteChanged(getUKUILite());
    }/* else if (key == UKUI_LITE_MODE) {
        //QGsettings
        * Not all values that a QVariant can hold can be serialized into a
        * setting. Basic types (integers, doubles, strings) and string lists are
        * supporte
        * // so ukui-session can't set qMap in gsettings..
        QMap<QString, QVariant> sendLite = getUKUILite();

        if (sendLite.count() > 0) {
            Q_EMIT UKUILiteChanged(sendLite);
        }
    }*/
}

void GlobalSignal::connectUserActiveSignal()
{
    m_plogin1 = new  DBusLogin1Interface("org.freedesktop.login1",
                              "/org/freedesktop/login1/user/self",
                              "org.freedesktop.login1.User",
                              QDBusConnection::systemBus());
    SessionStructList lqvar =  qvariant_cast<SessionStructList>(m_plogin1->property("Sessions"));
}

void GlobalSignal::connectUserActiveSignalWithPath(QString path)
{
    USD_LOG(LOG_DEBUG,"connect:%s..",path.toLatin1().data());
    m_ploginWithPath = new  QDBusInterface("org.freedesktop.login1",
                                           path,
                                           "org.freedesktop.DBus.Properties",
                                           QDBusConnection::systemBus());
   connect(m_ploginWithPath, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)), this, SLOT(sendUserActiveSignal(QString,QVariantMap,QStringList)));
}

void GlobalSignal::connectUserLogin1Signal()
{
    m_plogin2 = new  QDBusInterface("org.freedesktop.login1",
                                    "/org/freedesktop/login1/user/self",
                                    "org.freedesktop.DBus.Properties",
                                    QDBusConnection::systemBus());


    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1/user/self"
                                                          ,"org.freedesktop.DBus.Properties",
                                                          "Get");

     QDBusMessage reply1 = m_plogin2->call("Get", "org.freedesktop.login1.User", "Sessions");

     QVariant v = reply1.arguments().first();
     QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();

     SessionStructList members;
     arg.beginArray();
     while(!arg.atEnd()) {
         SessionStruct m;
         arg >> m;
         members << m;
         USD_LOG(LOG_DEBUG,"ready connect %s..",m.userPath.path().toLatin1().data());
         connectUserActiveSignalWithPath(m.userPath.path());
         break;
     }
     arg.endArray();
}

QVariant GlobalSignal::getGlobalConfig(QString schema, QString key)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                          "/globalconfig", \
                          SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                          QDBusConnection::systemBus());

    QDBusReply<QVariant> reply = globalManager.call("getGlobalConf", schema, key);

    if (reply.isValid()) {
        return reply.value();
    } else {
        USD_LOG(LOG_ERR,"globalManager dbus interface failed .");
    }
    return QVariant(0);
}

QList<QString> GlobalSignal::getShutDownDisableOptionInGsettings()
{
    QList<QString> disableOption = m_pGMsettings->get(DISABLE_POWEROFF_OPTIONS).toStringList();
    for (int var = 0; var < disableOption.count(); ++var) {
        disableOption[var] = disableOption[var].toLower();
    }
    QList<QString> intersection = m_allOptions.toSet().intersect(disableOption.toSet()).toList();
    USD_LOG_SHOW_PARAM1(intersection.count());
    return intersection;
}

QStringList GlobalSignal::getShutDownDisableOptionFromGlobalManager()
{
    QStringList values = getGlobalConfig(MODULE_NAME, DISABLE_POWEROFF_OPTIONS).toStringList();
    return values;
}

QMap<QString, QVariant> GlobalSignal::getUKUILite()
{
    QMap<QString, QVariant> lite;
    lite.insert(UKUI_LITE_FUNCTION_KEY , m_pGMsettings->get(UKUI_LITE_FUNCTION_KEY));
    lite.insert(UKUI_LITE_ANIMATION_KEY , m_pGMsettings->get(UKUI_LITE_ANIMATION_KEY));
    return lite;
#if 0
    QMap<QString, QVariant> getLite = m_pGMsettings->get(UKUI_LITE_MODE).value<QMap<QString, QVariant>>();
    QMap<QString, QVariant> sendLite;
    QStringList liteMode = {"normal", "lite"};
    QString function;
    QString animation;
    function = getLite[UKUI_LITE_FUNCTION].toString();
    animation = getLite[UKUI_LITE_ANIMATION].toString();

    if(liteMode.contains(function,Qt::CaseInsensitive) && liteMode.contains(animation, Qt::CaseInsensitive)) {
        sendLite.insert(UKUI_LITE_FUNCTION, function.toLower());
        sendLite.insert(UKUI_LITE_ANIMATION, animation.toLower());
    } else {
        USD_LOG_SHOW_PARAMS(function.toLatin1().data());
        USD_LOG_SHOW_PARAMS(animation.toLatin1().data());
    }
    return sendLite;
#endif
}

bool GlobalSignal::getHidePowerUiFromGlobalManager()
{
    return getGlobalConfig(MODULE_NAME, HIDE_POWEROFF_UI).toBool();
}

