/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef UPM_GSETTINGS_H
#define UPM_GSETTINGS_H

#include <QObject>
#include <QProcess>
#include <QGSettings>
#include <QVariant>

//power gsettings 配置
#define POWER_MANAGER_SETTINGS   "org.ukui.power-manager"

//电源策略(0:节能 1:平衡 2:性能)
#define GSETTINGS_KEY_POWER_POLICY_BATTERY     "powerPolicyBattery"

//空闲关闭显示器
#define GSETTINGS_KEY_SLEEP_DISPLAY_AC         "sleepDisplayAc"
#define GSETTINGS_KEY_SLEEP_DISPLAY_BATTERY    "sleepDisplayBattery"

//空闲睡眠
#define GSETTINGS_KEY_SLEEP_COMPUTER_AC        "sleepComputerAc"
#define GSETTINGS_KEY_SLEEP_COMPUTER_BATTERY   "sleepComputerBattery"
//空闲降低屏幕亮度
#define GSETTINGS_KEY_IDLE_BRIGHTNESS          "idleBrightness"        //空闲状态亮度,降低为当前值的百分之多少
//空闲时切换电源策略
#define GSETTINGS_KEY_IDLE_POWER_POLICY_AUTO   "idlePowerPolicyAuto"

//极低电量
#define GSETTINGS_KEY_PERCENTAGE_ACTION        "percentageAction"
//极低电量时执行
#define GSETTINGS_KEY_ACTION_CRITICAL_BATTERY  "actionCriticalBattery"

//亮度值
#define GSETTINGS_KEY_BRIGHTNESS_AC            "brightnessAc"

//screensaver gsettings 配置
#define SCREEN_SAVER_SETTINGS      "org.ukui.screensaver"
#define GSETTINGS_KEY_IDLE_DELAY   "idleDelay"

//usd gsettings 配置
#define USD_AUTO_BRIGHTENSS_SETTINGS      "org.ukui.SettingsDaemon.plugins.auto-brightness"
#define GSETTINGS_KEY_AUTO_BRIGHTENSS     "autoBrightness"

class UpmGsettings : public QObject
{
    Q_OBJECT
public:
    explicit UpmGsettings(QObject *parent = nullptr);
    ~UpmGsettings();
    static UpmGsettings* self();

    typedef void (UpmGsettings::*pSignalFun)(QVariant);
private:
    QGSettings *m_powerGsettings;
    QStringList m_allPowerKey;
    QGSettings *m_screensaverGsettings;
    QStringList m_allScreensaverKey;
    QGSettings *m_usdAutoBrightnessGsettings;
    QStringList m_allUsdAutoBrightnessKey;

    QHash<QString, QVariant> m_qHashValue;
    QHash<QString, pSignalFun> m_qHashSignalsFuns;

    void initGsettingsConfig();
    void addConfigMonitor(const QString &configName, pSignalFun signalFun);

signals:

public:
    void setGsettingsConfig(const QString &configName, QVariant value);
    bool getGsettingsConfig(const QString &configName, QVariant &value);
    void resetGsettingsConfig(const QString &configName);
};

#endif // UPM_GSETTINGS_H
