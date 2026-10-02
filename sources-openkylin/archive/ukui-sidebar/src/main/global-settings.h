/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef UKUI_SIDEBAR_GLOBAL_SETTINGS_H
#define UKUI_SIDEBAR_GLOBAL_SETTINGS_H

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QGSettings>
#include <QDBusInterface>

#define USD_SETTING             "org.ukui.SettingsDaemon.plugins.xsettings"
#define USD_SCALING_FACTOR_KEY  "scalingFactor"

#define CONTROL_CENTER_SETTING      "org.ukui.control-center.personalise"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"
#define CONTROL_CENTER_EFFECT       "effect"

// style schema
#define UKUI_STYLE_SETTING          "org.ukui.style"
// style keys
#define UKUI_STYLE_FONT_KEY         "systemFont"
#define UKUI_STYLE_FONT_SIZE_KEY    "systemFontSize"
#define UKUI_STYLE_NAME_KEY         "styleName"
#define UKUI_STYLE_THEME_COLOR_KEY  "themeColor"
#define UKUI_STYLE_WINDOW_RADIUS_KEY "windowRadius"
// style value
#define DEFAULT_STYLE            "ukui-default"
#define BLACK_STYLE              "ukui-black"
#define WHITE_STYLE              "ukui-white"
#define DARK_STYLE               "ukui-dark"
#define LIGHT_STYLE              "ukui-light"

//dbus
#define DBUS_STATUS_MANAGER_IF   "com.kylin.statusmanager.interface"
#define TABLET_MODE              "tablet-mode"

#define IS_LITE_MODE             "isLiteMode"

// panel
#define UKUI_PANEL_POSITION_KEY                     "panelposition"
#define UKUI_PANEL_SIZE_KEY                         "panelsize"
#define UKUI_PANEL_TYPE_KEY                         "paneltype"
#define UKUI_SETTINGS_ISLAND_POSITION_KEY           "settingsislandposition"
#define UKUI_DATA_ISLAND_POSITION_KEY               "dataislandposition"
#define UKUI_TOPBAR_SIZE_KEY                        "topbarsize"
#define UKUI_PANEL_LENGTH_KEY                       "panellength"

namespace Sidebar {

class GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings *globalInstance(QObject *parent = nullptr);
    ~GlobalSettings() override;

    const QVariant getValue(const QString &key);

    const QStringList getKeys();

    int getPanelLength(QString screenName);

Q_SIGNALS:
    void valueChanged(const QString &key);

private Q_SLOTS:
    void updateTabletStatus(bool isTabletMode);
    void updateIsLiteMode(QMap<QString, QVariant> map);

private:
    explicit GlobalSettings(QObject *parent = nullptr);
    inline void insertValue(const QString &key, const QVariant &value);

    void initStyleSettings();
    void initControlCenterSettings();
    void initUSDSetting();
    void initUSDCenterSettings();
    void initStatusManagerDbus();
    void initPanelMonitor();

    void initPanelGSettings();
    bool isKeysContain(const char* key);
    QMap<QString, QVariant> getPanelLengthMap();

private:
    QDBusInterface *m_statusManagerDBus = nullptr;
    QMap<QString, QVariant> m_cache;
};

class SettingMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList keys READ keys WRITE setKeys NOTIFY keysChanged)
public:
    explicit SettingMonitor(QObject *parent = nullptr);

    QStringList keys();
    void setKeys(QStringList &keys);
    Q_INVOKABLE QVariant getValue(const QString &key);

private Q_SLOTS:
    void valueChangedSlot(const QString &key);

Q_SIGNALS:
    void keysChanged();
    void valueChanged(const QString &key);

private:
    QStringList m_keys;
};

} // UkuiShortcut

#endif //UKUI_SIDEBAR_GLOBAL_SETTINGS_H
