#include "gamma-brightness.h"

GammaBrightness::GammaBrightness(QObject *parent)
{
}


int GammaBrightness::getBrightness()
{
    if (m_pGmDbusInterface == nullptr) {
        USD_LOG(LOG_DEBUG,"init error!");
        return -1;
    }
    QDBusMessage response =  m_pGmDbusInterface->call("getPrimaryBrightness");

    if (response.type() != QDBusMessage::ReplyMessage) {
        USD_LOG(LOG_DEBUG,"init error!");
        return -1;
    }
    int ret = response.arguments().takeFirst().toInt();
    USD_LOG_SHOW_PARAM1(ret);
    return ret;
}

int GammaBrightness::getMaxBrightness()
{
    return G_DEFAULT_MAX_BRIGHTNESS;
}

int GammaBrightness::setBrightness(int brightness)
{
    if (m_pGmDbusInterface == nullptr) {
        USD_LOG(LOG_DEBUG,"init error!");
        return -1;
    }

    QDBusMessage response =  m_pGmDbusInterface->call("setPrimaryBrightness", "self.", brightness);

    if (response.type() != QDBusMessage::ReplyMessage) {
        return -1;
    }

    return response.arguments().takeFirst().toInt();
}

bool GammaBrightness::setOutputBrightness(QString appName, QString outputName, int brightness)
{
    if (m_pGmDbusInterface == nullptr) {
        USD_LOG(LOG_DEBUG,"init error!");
        return -1;
    }

    QDBusMessage response =  m_pGmDbusInterface->call("setScreenBrightness", appName, outputName, (uint)brightness);

    if (response.type() != QDBusMessage::ReplyMessage) {
        return -1;
    }

    return response.arguments().takeFirst().toInt();
}

int GammaBrightness::connectTheSignal()
{
    m_pGmDbusInterface = new QDBusInterface(DBUS_GM_NAME,DBUS_GM_PATH,DBUS_GM_INTERFACE,
                                                                 QDBusConnection::sessionBus());

    if (!m_pGmDbusInterface->isValid()) {
        USD_LOG(LOG_DEBUG,"can't connect the signal...");
        m_pGmDbusInterface = nullptr;
        return 0;
    }
    m_pGmDbusInterface->call("enablePrimarySignal", true);
    return 1;
}

bool GammaBrightness::isEnhancementBrightness()
{
    return false;
}

QString GammaBrightness::backend()
{
    return "gamma manager";
}
