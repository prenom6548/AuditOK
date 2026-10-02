#ifndef POWERMANAGERBRIGHTNESS_H
#define POWERMANAGERBRIGHTNESS_H

#include <QVariant>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusReply>

#include "clib-syslog.h"
#include "usd_global_define.h"
#include "QGSettings/qgsettings.h"
#include "abstract-brightness.h"

class PowerManagerBrightness : public AbstractBrightness
{
    Q_OBJECT
public:
    PowerManagerBrightness(QObject *parent);

    /**
     * @brief getBrightness
     * @return
     */
    int getBrightness() override;

    /**
     * @brief getMaxBrightness
     * @return
     */
    int getMaxBrightness() override;
    /**
     * @brief setBrightness
     * @param brightness
     * @return
     */
     int setBrightness(int brightness) override;

    /**
     * @brief setOutputBrightness
     * @param appName app名称
     * @param outputName 显示器名称
     * @param brightness 亮度
     * @return
     */
     bool setOutputBrightness(QString appName, QString outputName, int brightness) override;

    /**
     * @brief connectTheSignal
     * @return
     */
    int connectTheSignal() override;

    /**
     * @brief isEnhancementBrightness 亮度增强
     * @return
     */
    bool isEnhancementBrightness() override;

    /**
     * @brief isEnable
     * @return
     */
    QString backend();

private Q_SLOTS:
    void doChanged(QString);

private:
    bool hasBrightnessEnhanceSupport();
private:
    QGSettings *m_pPowerManger;
    bool m_isHWBrightnessEnhance = false;
    bool m_isGSBrightnessEnhance = false;
};

#endif // POWERMANAGERBRIGHTNESS_H
