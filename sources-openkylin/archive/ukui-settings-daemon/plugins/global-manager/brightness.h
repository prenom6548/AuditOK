#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QDBusReply>
#include <QDBusInterface>

#include "clib-syslog.h"
#include "usd_base_class.h"
#include "abstract-brightness.h"
#include "gamma-brightness.h"
#include "powermanager-brightness.h"

class Brightness: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DBUS_GC_BRIGHTNESS_INTERFACE)
public:
    Brightness(QObject *parent = nullptr);

public Q_SLOTS:

    /**
     * @brief getPrimaryBrightness
     * @return
     */
    Q_SCRIPTABLE uint getPrimaryBrightness();

    /**
     * @brief setPrimaryBrightness
     * @return
     */
    Q_SCRIPTABLE bool setPrimaryBrightness(uint Brightness);

    /**
     * @brief setOutputBrightness
     * @return
     */
    Q_SCRIPTABLE bool setOutputBrightness(QString appName, QString outputName, uint Brightness);

    /**
     * @brief isEnable
     * @return
     */
    Q_SCRIPTABLE bool isEnable();

    /**
     * @brief isEnable
     * @return
     */
    Q_SCRIPTABLE QString backend();

    /**
     * @brief isEnhancementBrightness
     * @return
     */
    Q_SCRIPTABLE bool isEnhancementBrightness();
Q_SIGNALS:
    /**
     * @brief primaryChanged
     * @param Bright
     * @return
     */
    void enhancementBrightnessSwitchChanged(bool switchState);

private:
    void sendPrimaryStartChanged(int brightness);

private:
    bool m_powerManagerSetable = false;
    bool m_gammaManagerSetable = false;

    uint m_targetPrimaryBrightness = 0;
    QTimer *m_brightCacheTimer;
    AbstractBrightness *m_brightnessManager = nullptr;
};

#endif // BRIGHTNESS_H
