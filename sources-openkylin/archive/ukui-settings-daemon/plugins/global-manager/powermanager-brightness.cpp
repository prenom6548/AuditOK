#include "powermanager-brightness.h"


PowerManagerBrightness::PowerManagerBrightness(QObject *parent)
{
    m_pPowerManger = new QGSettings(POWER_MANAGER_SCHEMA);
    m_isHWBrightnessEnhance = hasBrightnessEnhanceSupport();
    m_isGSBrightnessEnhance = m_pPowerManger->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toInt();
}

int PowerManagerBrightness::getBrightness()
{
    return m_pPowerManger->get(BRIGHTNESS_AC_KEY).toInt();
}

int PowerManagerBrightness::getMaxBrightness()
{
    bool switchState = m_pPowerManger->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toInt();
    int maxBrightness = (switchState && isEnhancementBrightness()) ?
                G_ENHANCED_MAX_BRIGHTNESS : G_DEFAULT_MAX_BRIGHTNESS;

    return maxBrightness;
}

int PowerManagerBrightness::setBrightness(int brightness)
{
    USD_LOG_SHOW_PARAM1(brightness);
    return m_pPowerManger->trySet(BRIGHTNESS_AC_KEY, brightness);
}

bool PowerManagerBrightness::setOutputBrightness(QString appName, QString outputName, int brightness)
{
    return false;
}

int PowerManagerBrightness::connectTheSignal()
{
    connect(m_pPowerManger, SIGNAL(changed(QString)), this, SLOT(doChanged(QString)));
    return 0;
}

bool PowerManagerBrightness::isEnhancementBrightness()
{
    return m_isHWBrightnessEnhance && m_isGSBrightnessEnhance;
}

QString PowerManagerBrightness::backend()
{
    return "power manager";
}

void PowerManagerBrightness::doChanged(QString key)
{
    if (key == BRIGHTNESS_AC_KEY) {
        uint brightness = m_pPowerManger->get(key).toInt();
        bool switchState = m_pPowerManger->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toInt();
        int maxBrightness = (switchState && isEnhancementBrightness()) ?
                    G_ENHANCED_MAX_BRIGHTNESS : G_DEFAULT_MAX_BRIGHTNESS;

        if (brightness >= 0 && brightness <= maxBrightness) {
            QDBusMessage notifySignal =
                    QDBusMessage::createSignal(DBUS_GC_BRIGHTNESS_PATH,
                                               DBUS_GC_BRIGHTNESS_INTERFACE, DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_END);

            notifySignal.setArguments({QVariant::fromValue(brightness)});
            QDBusConnection::sessionBus().send(notifySignal);
        }
    } else if (key == BRIGHTNESS_ENHANCEMENT_SWITCH) {
        if (!m_isHWBrightnessEnhance) {
            return;
        }
        m_isGSBrightnessEnhance = m_pPowerManger->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toInt();
        QDBusMessage notifySignal =
                QDBusMessage::createSignal(DBUS_GC_BRIGHTNESS_PATH,
                                           DBUS_GC_BRIGHTNESS_INTERFACE, DBUS_GC_BRIGHTNESS_SIGNAL_ENHANCEMENT_CHANGED);

        notifySignal.setArguments({QVariant::fromValue(m_isGSBrightnessEnhance)});
        QDBusConnection::sessionBus().send(notifySignal);
    }
}

bool PowerManagerBrightness::hasBrightnessEnhanceSupport()
{
    QDBusInterface powerInterface("org.ukui.powermanagement",
                                  "/",
                                  "org.ukui.powermanagement.interface",
                                  QDBusConnection::systemBus());

    if (!powerInterface.isValid()) {
        SYS_LOG(LOG_WARNING, "powerInterface create error");
        return false;
    }

    QDBusReply<bool> reply = powerInterface.call("IsBrightnessEnhancementSupported");

    if(!reply.isValid()) {
        QString error = reply.error().message();
        SYS_LOG(LOG_WARNING, "errorMsg:%s", error.toLatin1().data());
        return false;
    }
    SYS_LOG(LOG_WARNING, "IsBrightnessEnhancementSupported:%d", reply.value());
    return reply.value();
}
