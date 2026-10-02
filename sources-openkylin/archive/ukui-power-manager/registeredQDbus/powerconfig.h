/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef POWERCONFIG_H
#define POWERCONFIG_H

#include <QObject>
#include <QSettings>
#include <QFile>

#define POWER_MANAGER_CONF_PATH   "/etc/power/power-manager.conf"
#define POWER_MANAGER_NO_THIS_KEY "no-this-key"

#define CONFIG_GROUP_CONFIG       "config"

#define POWER_MANAGER_RUN_TIMES   "powerManagerRunTimes"
//电源策略(0:节能 1:平衡 2:性能)
#define GSETTINGS_KEY_POWER_POLICY_AC          "powerPolicyAc"
#define GSETTINGS_KEY_POWER_POLICY_BATTERY     "powerPolicyBattery"
#define GSETTINGS_KEY_POWER_POLICY_BATTERY_BACKUP  "powerPolicyBatteryBackup"
//空闲关闭显示器
#define GSETTINGS_KEY_SLEEP_DISPLAY_AC         "sleepDisplayAc"
#define GSETTINGS_KEY_SLEEP_DISPLAY_BATTERY    "sleepDisplayBattery"
//空闲睡眠
#define GSETTINGS_KEY_SLEEP_COMPUTER_AC        "sleepComputerAc"
#define GSETTINGS_KEY_SLEEP_COMPUTER_BATTERY   "sleepComputerBattery"
//盒盖事件触发操作
#define GSETTINGS_KEY_BUTTON_LID_AC            "buttonLidAc"
#define GSETTINGS_KEY_BUTTON_LID_BATTERY       "buttonLidBattery"

//低电量百分百通知
#define GSETTINGS_KEY_PERCENTAGE_LOW           "percentageLow"
//极低电量
#define GSETTINGS_KEY_PERCENTAGE_ACTION        "percentageAction"
//极低电量时执行
#define GSETTINGS_KEY_ACTION_CRITICAL_BATTERY  "actionCriticalBattery"

//低电量时自动开启节能模式
#define GSETTINGS_KEY_BATTERY_SAVE_SWITCH      "batterySaveSwitch"
//低电量时自动开启节能模式
#define GSETTINGS_KEY_LOW_BATTERY_AUTO_SAVE    "lowBatteryAutoSave"
//节能模式降低亮度
#define GSETTINGS_KEY_BATTERY_SAVE_REDUCE_BRIGHTNESS    "batterySaveReduceBrightness"

//亮度值
#define GSETTINGS_KEY_BRIGHTNESS_AC            "brightnessAc"

class PowerConfig : public QObject
{
    Q_OBJECT
public:
    explicit PowerConfig(QObject *parent = nullptr);
    ~PowerConfig();

    int getPowerManagerRunTimes();
    void updatePowerConfig(const QString &key, const QString &value);
    bool getPowerConfig(const QString &key, QVariant &value);

private:
    int m_powerManagerRunTimes = 0;
    QSettings *m_qsettings;
signals:

};

#endif // POWERCONFIG_H
