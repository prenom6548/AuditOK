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
#define POWER_MANAGER_SETTINGS                 "org.ukui.power-manager"

//电源策略(0:节能 1:平衡 2:性能)
#define GSETTINGS_KEY_POWER_POLICY_AC          "powerPolicyAc"
#define GSETTINGS_KEY_POWER_POLICY_BATTERY     "powerPolicyBattery"

#define GSETTINGS_KEY_DISPLAY_LEFT_TIME        "displayLeftTime"

#define GSETTINGS_KEY_PERCENTAGE_LOW           "percentageLow"
#define GSETTINGS_KEY_PERCENTAGE_CRITICAL      "percentageCritical"

//任务栏 gsettings 配置
#define PANEL_SETTINGS                         "org.ukui.panel.settings"
#define GSETTINGS_KEY_PANEL_SIZE               "panelsize"
#define GSETTINGS_KEY_PANEL_POSITION           "panelposition"

//style gsettings 配置
#define STYLE_SETTINGS                         "org.ukui.style"

#define GSETTINGS_KEY_SYSTEM_FONT_SIZE         "systemFontSize"
#define GSETTINGS_KEY_STYLE_NAME               "styleName"

//control-center gsettings 配置
#define CONTROL_CENTER_SETTINGS                "org.ukui.control-center.personalise"

#define GSETTINGS_KEY_TRANSPARENCY_KEY         "transparency"
//dbus配置
#define UPOWER_SERVICE                         "org.ukui.upower"
#define UPOWER_DISPLAY_PATH                    "/upower/BatteryInfo"
#define UPOWER_DISPLAY_INTERFACE               "org.ukui.upower.battery"

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

    QGSettings *m_panelGsettings;
    QStringList m_allPanelKey;

    QGSettings *m_styleGsettings;
    QStringList m_allStyleKey;

    QGSettings *m_controlCenterGsettings;
    QStringList m_allControlCenterKey;

    QHash<QString, QVariant> m_qHashValue;
    QHash<QString, pSignalFun> m_qHashSignalsFuns;

    void initPowerGsettings();
    void initPanelGsettings();
    void initStyleGsettings();
    void initControlCenterGsettings();

    void addConfigMonitor(const QString &configName, pSignalFun signalFun);

Q_SIGNALS:
    void powerPolicyAcChanged(QVariant);
    void powerPolicyBatteryChanged(QVariant);
    void displayLeftTime(QVariant);

    void percentageActionChanged(QVariant);
    void percentageLowChanged(QVariant);

    void panelsizeChanged(QVariant);
    void panelpositionChanged(QVariant);

    void systemFontSizeChanged(QVariant);
    void styleNameChanged(QVariant);

    void transparencyChanged(QVariant);

private Q_SLOTS:
    void dealGsettingsKeyChanged(const QString &key);

public:
    void setGsettingsConfig(const QString &configName, QVariant value);
    bool getGsettingsConfig(const QString &configName, QVariant &value);

    // 直接读取 percentageLow，避免缓存导致的延迟
    int percentageLowValue() const;
};

#endif // UPM_GSETTINGS_H
