#ifndef GAMMABRIGHTNESS_H
#define GAMMABRIGHTNESS_H


#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusInterface>

#include "clib-syslog.h"
#include "usd_global_define.h"
#include "abstract-brightness.h"

class GammaBrightness : public AbstractBrightness
{
    Q_OBJECT
public:
    GammaBrightness(QObject *parent);

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
    int connectTheSignal();

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

private:
    QDBusInterface *m_pGmDbusInterface = nullptr;
};

#endif // GAMMABRIGHTNESS_H
