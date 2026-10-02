/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef POWER_H
#define POWER_H

#include <QDBusContext>
#include <QObject>
#include <QGSettings/QGSettings>

enum UpDeviceType
{
    UP_DEVICE_TYPE_UNKNOWN = 0,
    UP_DEVICE_TYPE_LINE_POWER,
    UP_DEVICE_TYPE_BATTERY,
    UP_DEVICE_TYPE_UPS,
    UP_DEVICE_TYPE_MONITOR,
    UP_DEVICE_TYPE_MOUSE,
    UP_DEVICE_TYPE_KEYBOARD,
    UP_DEVICE_TYPE_PDA,
    UP_DEVICE_TYPE_PHONE,
    UP_DEVICE_TYPE_MEDIA_PLAYER,
    UP_DEVICE_TYPE_TABLET,
    UP_DEVICE_TYPE_COMPUTER,
    UP_DEVICE_TYPE_MAX    
};

class UkuiPowerInterface : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.Power")

    Q_PROPERTY(QString PowerButtonAction READ GetPowerButtonAction WRITE SetPowerButtonAction NOTIFY PowerButtonActionChanged)
    Q_PROPERTY(QString LidCloseAction READ GetLidCloseAction WRITE SetLidCloseAction NOTIFY LidCloseActionChanged)
    Q_PROPERTY(int ScreenTimeoutOnAC READ GetScreenTimeoutOnAC WRITE SetScreenTimeoutOnAC NOTIFY ScreenTimeoutOnACChanged)
    Q_PROPERTY(int ScreenTimeoutOnDC READ GetScreenTimeoutOnDC WRITE SetScreenTimeoutOnDC NOTIFY ScreenTimeoutOnDCChanged)
    Q_PROPERTY(int SleepComputerTimeOnAC READ GetSleepComputerTimeOnAC WRITE SetSleepComputerTimeOnAC NOTIFY SleepComputerTimeOnACChanged)
    Q_PROPERTY(int SleepComputerTimeOnDC READ GetSleepComputerTimeOnDC WRITE SetSleepComputerTimeOnDC NOTIFY SleepComputerTimeOnDCChanged)
    Q_PROPERTY(int ACPowerPolicy READ GetACPowerPolicy WRITE SetACPowerPolicy NOTIFY ACPowerPolicyChanged)
    Q_PROPERTY(int DCPowerPolicy READ GetDCPowerPolicy WRITE SetDCPowerPolicy NOTIFY DCPowerPolicyChanged)
    Q_PROPERTY(QString LowBatteryAction READ GetLowBatteryAction WRITE SetLowBatteryAction NOTIFY LowBatteryActionChanged)
    Q_PROPERTY(int LowBatteryActionThreshold READ GetLowBatteryActionThreshold WRITE SetLowBatteryActionThreshold NOTIFY LowBatteryActionThresholdChanged)
    Q_PROPERTY(int LowBatteryNotifyThreshold READ GetLowBatteryNotifyThreshold WRITE SetLowBatteryNotifyThreshold NOTIFY LowBatteryNotifyThresholdChanged)
    Q_PROPERTY(bool ShowLeftTime READ IsShowLeftTime WRITE SetShowLeftTime NOTIFY ShowLeftTimeChanged)

    const QStringList POWER_BUTTON_ACTION_VALID_LIST = {"interactive", "blank", "suspend", "hibernate", "shutdown"};// 电源按键有效动作：询问、息屏、睡眠、休眠、关机
    const QStringList LID_CLOSE_ACTION_VALID_LIST    = {"nothing", "blank", "suspend", "hibernate", "shutdown"};    // 合盖有效动作：无操作、息屏、睡眠、休眠、关机
    const QStringList LOW_BATTERY_ACTION_VALID_LIST  = {"nothing", "blank", "suspend", "hibernate", "shutdown"};    // 低电量有效动作：无操作、息屏、睡眠、休眠、关机

    const QString KEY_POWER_BUTTON                 = "button-power";
    const QString KEY_LID_BUTTON_AC                = "button-lid-ac";
    const QString KEY_LID_BUTTON_DC                = "button-lid-battery";
    const QString KEY_BRIGHTNESS_AC                = "brightness-ac";
    const QString KEY_SCREEN_TIMEOUT_ON_AC         = "sleep-display-ac";
    const QString KEY_SCREEN_TIMEOUT_ON_DC         = "sleep-display-battery";
    const QString KEY_SLEEP_COMPUTER_ON_AC         = "sleep-computer-ac";
    const QString KEY_SLEEP_COMPUTER_ON_DC         = "sleep-computer-battery";
    const QString KEY_AC_POWER_POLICY              = "power-policy-ac";
    const QString KEY_DC_POWER_POLICY              = "power-policy-battery";
    const QString KEY_LOW_BATTERY_ACTION           = "action-critical-battery";
    const QString KEY_LOW_BATTERY_ACTION_THRESHOLD = "percentage-action";
    const QString KEY_LOW_BATTERY_NOTIFY_THRESHOLD = "percentage-low";
    const QString KEY_DISPLAY_LEFT_TIME            = "display-left-time";

public:
    explicit UkuiPowerInterface(QObject *parent = nullptr);
    ~UkuiPowerInterface() override;

public slots:
    /**
     * @brief getPowerSaveLevel 获取节能等级
     * @return  -1:错误信息； 0：正常工作； 1：节能； 2：极低电量
     */
    int GetPowerSaveLevel();

    /**
     * @brief getPowerMode 获取电源工作模式
     * @return  -1:错误信息； 0：性能； 1：平衡； 2：节能
     */
    int GetPowerMode();

    /**
     * @brief SetPowerMode
     * @param mode 0:性能模式 1：平衡模式  2：能效模式
     */
    void SetPowerMode(uint mode);

    /**
     * @brief batteryPercentage 获取电量百分比
     * @return -1:错误信息；1-100：电量百分比
     */
    int BatteryPercentage();

    /**
     * @brief BatteryHealthLevel 获取电池健康度（相对设计容量）
     * @return -1:错误信息；1-100：健康度百分比
     */
    int BatteryHealthLevel();

    /**
     * @brief timeToFull 获取充满电时间
     * @return -1:错误信息；获取充满电时间 单位：秒；
     */
    int TimeToFull();

    /**
     * @brief timeToUse 获取可用时长
     * @return -1:错误信息；可用时长 单位：秒
     */
    int TimeToEmpty();

    /**
     * @brief getBatteryState 获取电池状态
     * @return
     * -1:错误信息； 0：未知状态 1：充电状态 2：放电状态 3：电量为空 4：电池已充满 5：准备充电 6：准备断开充电
     */
    int GetBatteryState();

    /**
     * @brief isBatteryPresent 是否有电池接入
     * @return true:有电池接入；false:无电池接入
     */
    bool IsBatteryPresent();

    /**
     * @brief isLid 是否有盖子
     * @return true:有盖子；false:无盖子
     */
    bool IsLid();

    void onDeviceAdded(const QString &devicePath);

    void onDeviceRemoved(const QString &devicePath);

private slots:
    void onGSettingsChanged(const QString &key);

    /**
     * @brief GetPowerButtonAction
     * @return 按电源键执行的操作,返回空则是未知错误
     */
    Q_INVOKABLE QString GetPowerButtonAction();

    /**
     * @brief SetPowerButtonAction
     * @param action 按电源键执行的操作，可用{"interactive", "blank", "suspend", "hibernate", "shutdown"}
     * @return 0 有效； -1 无效,例如给的string不是可用范围内的
     */
    Q_INVOKABLE int SetPowerButtonAction(QString action);

    /**
     * @brief GetLidCloseAction
     * @return 合盖执行的动作
     */
    Q_INVOKABLE QString GetLidCloseAction();

    /**
     * @brief SetLidCloseAction
     * @param action
     * @return 0 有效； -1 无效,例如给的string不是可用范围内的
     */
    Q_INVOKABLE int SetLidCloseAction(QString action);

    /**
     * @brief GetScreenTimeoutOnAC
     * @return 电源供电情况下，息屏时间，单位s(秒)
     */
    Q_INVOKABLE int GetScreenTimeoutOnAC();

    /**
     * @brief SetScreenTimeoutOnAC
     * @param time 电源供电情况下，息屏时间，单位s(秒)
     */
    Q_INVOKABLE void SetScreenTimeoutOnAC(int time);

    /**
     * @brief GetScreenTimeoutOnDC
     * @return 电池供电情况下，息屏时间，单位s(秒)
     */
    Q_INVOKABLE int GetScreenTimeoutOnDC();

    /**
     * @brief SetScreenTimeoutOnDC
     * @param time 电池供电情况下，息屏时间，单位s(秒)
     */
    Q_INVOKABLE void SetScreenTimeoutOnDC(int time);

    /**
     * @brief GetSleepComputerTimeOnAC
     * @return 电源供电情况下，睡眠时间，单位s(秒)
     */
    Q_INVOKABLE int GetSleepComputerTimeOnAC();

    /**
     * @brief SetSleepComputerTimeOnAC
     * @param time 电源供电情况下，睡眠时间，单位s(秒)
     */
    Q_INVOKABLE void SetSleepComputerTimeOnAC(int time);

    /**
     * @brief GetSleepComputerTimeOnDC
     * @return 电池供电情况下，睡眠时间，单位s(秒)
     */
    Q_INVOKABLE int GetSleepComputerTimeOnDC();

    /**
     * @brief SetSleepComputerTimeOnDC
     * @param time 电池供电情况下，睡眠时间，单位s(秒)
     */
    Q_INVOKABLE void SetSleepComputerTimeOnDC(int time);

    /**
     * @brief GetACPowerPolicy
     * @return  电源供电情况下，电源模式策略 -1:错误信息； 0：性能； 1：平衡； 2：节能
     */
    Q_INVOKABLE int GetACPowerPolicy();

    /**
     * @brief SetACPowerPolicy
     * @param policy 电源供电情况下，电源模式策略  0：性能； 1：平衡； 2：节能
     */
    Q_INVOKABLE void SetACPowerPolicy(int policy);

    /**
     * @brief GetDCPowerPolicy
     * @return 电池供电情况下，电源模式策略 -1:错误信息； 0：性能； 1：平衡； 2：节能
     */
    Q_INVOKABLE int GetDCPowerPolicy();

    /**
     * @brief SetDCPowerPolicy
     * @param policy 电池供电情况下，电源模式策略  0：性能； 1：平衡； 2：节能
     */
    Q_INVOKABLE void SetDCPowerPolicy(int policy);

    /**
     * @brief GetLowBatteryAction
     * @return 获取低电量时，系统要执行的动作 {"nothing", "blank", "suspend", "hibernate", "shutdown"};   有效动作：无操作、息屏、睡眠、休眠、关机
     */
    Q_INVOKABLE QString GetLowBatteryAction();

    /**
     * @brief SetLowBatteryAction
     * @param action 设置低电量是系统要执行的动作：无操作、息屏、睡眠、休眠、关机 {"nothing", "blank", "suspend", "hibernate", "shutdown"};
     */
    Q_INVOKABLE void SetLowBatteryAction(QString action);

    /**
     * @brief GetLowBatteryActionThreshold‌
     * @return 获取执行低电量动作的阈值：有效范围0-10，整数int，默认5（5%）
     */
    Q_INVOKABLE int GetLowBatteryActionThreshold();

    /**
     * @brief SetLowBatteryActionThreshold‌
     * @param value 设置执行低电量动作的阈值：有效范围0-10，整数int，默认5（5%）
     */
    Q_INVOKABLE void SetLowBatteryActionThreshold(int value);

    /**
     * @brief GetLowBatteryNotifyThreshold‌
     * @return 低电量通知的阈值，10，20，30，40可用，配置后，低于设置的阈值时会发送低电量通知（一般在系统右上角）
     */
    Q_INVOKABLE int GetLowBatteryNotifyThreshold();

    /**
     * @brief SetLowBatteryNotifyThreshold‌
     * @param value 设置低电量通知的阈值，10，20，30，40可用，配置后，低于设置的阈值时会发送低电量通知（一般在系统右上角）
     */
    Q_INVOKABLE void SetLowBatteryNotifyThreshold(int value);

    /**
     * @brief IsShowLeftTime 获取是否显示剩余时间的开关状态
     * @return true:显示剩余时间；false:不显示剩余时间
     */
    Q_INVOKABLE bool IsShowLeftTime();

    /**
     * @brief SetShowLeftTime 设置是否显示剩余时间的开关
     * @param enabled true:显示剩余时间；false:不显示剩余时间
     */
    Q_INVOKABLE void SetShowLeftTime(bool enabled);

signals:
    void PowerButtonActionChanged();
    void LidCloseActionChanged();
    void ScreenTimeoutOnACChanged();
    void ScreenTimeoutOnDCChanged();
    void SleepComputerTimeOnACChanged();
    void SleepComputerTimeOnDCChanged();
    void ACPowerPolicyChanged();
    void DCPowerPolicyChanged();
    void LowBatteryActionChanged();
    void LowBatteryActionThresholdChanged();
    void LowBatteryNotifyThresholdChanged();
    void ShowLeftTimeChanged();

private:
    void getBatteryDbusPath();
    QVariant getBatteryProperty(const QString &propertyName);
    int getDeviceType(const QString &dBusPath);

    QString m_batteryPath;
    QGSettings *m_powerGSettings = nullptr;
};


#endif // POWER_H
