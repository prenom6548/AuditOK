/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef EVENTWATCHER_H
#define EVENTWATCHER_H

#include <QObject>
#include <QProcess>
#include <QGSettings>
#include <QLocalSocket>

#include "common.h"
#include "globalconfig.h"
#include "hardware/huawei/flemingx.h"

#define LOGIN1_DBUS_SET_POWEROFF            "PowerOff"
#define LOGIN1_DBUS_SET_SUSPEND             "Suspend"
#define LOGIN1_DBUS_SET_HIBERNATE           "Hibernate"

class EventWatcher : public QObject
{
    Q_OBJECT
private:
    qulonglong mReserveBrightness;
    qulonglong mDynamicBrightness;

    double m_reserveBrightnessPercent = 0.1;

    QLocalSocket *m_socket = nullptr;
    GlobalConfig *m_globalConfig = nullptr;
    QTimer       *m_pInitTimer = nullptr;
    bool    isSyncGlobal = true;
    static constexpr double DEFAULT_BRIGHTNESS_PERCENT = 70.0;
    int m_brightness350Nit;

public:
    EventWatcher();
    ~EventWatcher();
    /**
     * @brief initEventWatcher
     * 初始化
     */
    void initEventWatcher();

    void initAcpiSocket();
    /**
     * @brief initBrightness
     * 初始化亮度
     */
    void initBrightness();

    /**
     * @brief dealBrightnessMsg
     * 处理亮度信息
     */
    void dealBrightnessMsg();

    /**
     * @brief initSettings
     * 获取用户设置亮度值
     */
    void getUserBrightness();

    /**
     * @brief initConfig
     * 设置亮度值
     */
    void setBrightness();

    void setGsettings(QString key, QVariant value);

    bool isNeedConfig();

    /**
     * @brief controlPowerManagement
     * 控制电源管理dbus建立
     */
    int controlPowerManagement(const QString &type);
    void controlPowerManagement(const QString &type, const int &value);
    void controlPowerManagement(const QString &type, const bool &value);
    void controlPowerManagement(const QString &type, const QString &contrl);
    void controlPowerManagement(const QString &type, const qulonglong &brightness);

    void controlLogin1Manager(QString action);

    /**
     * @brief turnOffDisplay
     * 关闭显示器
     */
    void turnOffDisplay();

    /**
     * @brief turnOnDisplay
     * 打开显示器
     */
    void turnOnDisplay();

    /**
     * @brief suspend
     * 睡眠
     */
    void suspend();

    /**
     * @brief hibernate
     * 休眠
     */
    void hibernate();

    /**
     * @brief PowerOff
     * 关机
     */
    void powerOff();


    void turnOffDisplaySignal(bool state);

    void percentageLowRegulateBrightness(bool);

    static bool checkDeviceModel(const QString &methodName);

    /*
     * ===============================================
     * 以下为成员变量，包括gsettings等设置相关变量
     * ===============================================
     */
    QGSettings *m_powerManagerGsettings;

    QGSettings *m_screenSaverGsettings;

    QGSettings *m_autoBrightnessGsettings = nullptr;

    /**
     * @brief mMaxBrightness
     * 最大亮度值
     */
    qulonglong mMaxBrightness;

    /**
     * @brief mBrightness
     * 当前亮度值
     */
    qulonglong mBrightness;

    /**
     * @brief mUserBrightness
     * 用户配置的亮度值
     */
    qulonglong mUserBrightness;

    bool m_isFlemingX = false;
    
    FlemingX *m_flx = nullptr;
    /**
     * @brief m_isBrightnessEnhancement‌Supported
     * 是否支持亮度增强
     */
    bool m_isBrightnessEnhancementSupported = false;

    enum policy
    {
        Performance = 0,
        Balance,
        EnergySaving,
        Unknown,
    };

    /**
     * @brief handlePowerStateChange
     * 处理电源状态变化，重新设置睡眠转休眠时间
     * @param onBattery 是否使用电池
     */
     void handlePowerStateChange(bool onBattery);

private:
    /**
     * @brief SyncFromGlobalConfig
     * 从全局电源配置同步到当前用户配置
     */
    void syncFromGlobalConfig();

    void setSuspend2HibernateTime(int seconds);

    /**
     * @brief readOnBattery
     * 获取当前电源状态（是否使用电池）
     * @return true表示使用电池，false表示使用AC
     */
    bool readOnBattery();

public Q_SLOTS:
    void handleMiniBrightness();
private Q_SLOTS:
    void userActiveSlot(bool);
    void onInitTimerOut();
signals:
    void allUserSyncFinished();
};

#endif // EVENTWATCHER_H
