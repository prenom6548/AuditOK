#include "brightness.h"
/*
 * 主屏机制问题：如果主屏没有在台式机外接的任意一台机器，主屏如何处理？
 * 1：USD无法感知，通过QApplication获取的主屏，与panel、peony实际位置并不相符合。。
*/
Brightness::Brightness(QObject *parent)
{
    QDBusInterface powerInterface("org.ukui.powermanagement",
                                  "/",
                                  "org.ukui.powermanagement.interface",
                                  QDBusConnection::systemBus());
    QDBusReply<bool> replay = powerInterface.call("CanSetBrightness");

    if(replay.isValid()) {
        m_powerManagerSetable = replay.value();
        if (m_powerManagerSetable) {
            m_brightnessManager = new PowerManagerBrightness(this);
        }
    }

    if (!m_powerManagerSetable) {
        if (UsdBaseClass::isWaylandWithKscreen()) {
            if (UsdBaseClass::isWlcom()) {
                m_gammaManagerSetable = true;
                m_brightnessManager = new GammaBrightness(this);
            }
        } else {
            m_gammaManagerSetable = true;
            m_brightnessManager = new GammaBrightness(this);
        }
    }

    if (m_brightnessManager != nullptr) {
        m_brightnessManager->connectTheSignal();
    }

    if (false == m_gammaManagerSetable && false == m_powerManagerSetable) {
        return;
    }
    m_brightCacheTimer = new QTimer();
    connect(m_brightCacheTimer, &QTimer::timeout, this, [=](){
        USD_LOG(LOG_DEBUG,"start set.");
        sendPrimaryStartChanged(m_targetPrimaryBrightness);
        m_brightnessManager->setBrightness(m_targetPrimaryBrightness);
    });

    m_brightCacheTimer->setSingleShot(true);
}

uint Brightness::getPrimaryBrightness()
{
    if (!isEnable()) {
        return -1;
    }

    return m_brightnessManager->getBrightness();
}

bool Brightness::setPrimaryBrightness(uint Brightness)
{
    static QTime startTime = QTime::currentTime();
    static int elapsed = -1;
    elapsed = startTime.msecsTo(QTime::currentTime());
        USD_LOG(LOG_DEBUG,".");
    if (Brightness > m_brightnessManager->getMaxBrightness() || !isEnable()) {
            USD_LOG(LOG_DEBUG,".");
        return false;
    }
    USD_LOG(LOG_DEBUG,".");
    m_targetPrimaryBrightness = Brightness;
/*
 * After multiple dragging, signals with intervals ranging from 100-600ms will still be received.
 * If the signal time difference is less than 700ms,
 * a timer with a timeout of 251ms will be used to filter out duplicate events
*/
    if(elapsed > 0 && elapsed <= 700){
        if(!m_brightCacheTimer->isActive() && elapsed) {
            m_brightCacheTimer->start(251);
            USD_LOG(LOG_DEBUG,"start timer.");
        } else {
            USD_LOG(LOG_DEBUG,"skip timer.");
        }
        return false;
    }
    USD_LOG(LOG_DEBUG,"set it.%d",elapsed);
    m_brightCacheTimer->stop();
    startTime = QTime::currentTime();
    sendPrimaryStartChanged(m_targetPrimaryBrightness);
    m_brightnessManager->setBrightness(m_targetPrimaryBrightness);
    return true;
}

bool Brightness::setOutputBrightness(QString appName, QString outputName, uint Brightness)
{
    static QTime startTime = QTime::currentTime();
    static int elapsed = -1;
    elapsed = startTime.msecsTo(QTime::currentTime());
        USD_LOG(LOG_DEBUG,".");
    if (Brightness > 100 || !isEnable()) {
            USD_LOG(LOG_DEBUG,".");
        return false;
    }
    USD_LOG(LOG_DEBUG,".");
    m_targetPrimaryBrightness = Brightness;
/*
 * After multiple dragging, signals with intervals ranging from 100-600ms will still be received.
 * If the signal time difference is less than 700ms,
 * a timer with a timeout of 251ms will be used to filter out duplicate events
*/
    if(elapsed > 0 && elapsed <= 700){
        if(!m_brightCacheTimer->isActive() && elapsed) {
            m_brightCacheTimer->start(251);
            USD_LOG(LOG_DEBUG,"start timer.");
        } else {
            USD_LOG(LOG_DEBUG,"skip timer.");
        }
        return false;
    }
    USD_LOG(LOG_DEBUG,"set it.%d",elapsed);
    m_brightCacheTimer->stop();
    startTime = QTime::currentTime();
//    sendPrimaryStartChanged(m_targetPrimaryBrightness);
    m_brightnessManager->setOutputBrightness(appName, outputName, m_targetPrimaryBrightness);
    return true;
}

bool Brightness::isEnhancementBrightness()
{
    return m_brightnessManager->isEnhancementBrightness();
}

bool Brightness::isEnable()
{
    static bool ret = ((m_gammaManagerSetable && UsdBaseClass::isSupportGamma()) || m_powerManagerSetable);

    return ret;
}

QString Brightness::backend()
{
    if (!isEnable()) {
        return "disable";
    }

    return m_brightnessManager->backend();
}

void Brightness::sendPrimaryStartChanged(int brightness)
{
    static int lastBrightness = -1;
    if (brightness == lastBrightness) {
        return;
    }

    lastBrightness = brightness;

    QDBusMessage notifySignal =
            QDBusMessage::createSignal(DBUS_GC_BRIGHTNESS_PATH, DBUS_GC_BRIGHTNESS_INTERFACE, DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_START);

    notifySignal.setArguments({QVariant::fromValue((uint)brightness)});
    QDBusConnection::sessionBus().send(notifySignal);
}
