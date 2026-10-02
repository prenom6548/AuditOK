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

#include "settings.h"
#include "libkypackages.h"

#include <QVariant>
#include <QDebug>
#include <QtQml>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVersionNumber>

#define UKUI_MENU_SCHEMA                "org.ukui.menu.settings"
#define CONTROL_CENTER_SETTING          "org.ukui.control-center.personalise"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"
#define CONTROL_CENTER_EFFECT_KEY       "effect"

#define UKUI_STYLE_SCHEMA               "org.ukui.style"
#define UKUI_STYLE_NAME_KEY             "styleName"
#define UKUI_STYLE_THEME_COLOR_KEY      "themeColor"
#define UKUI_STYLE_SYSTEM_FONT_SIZE     "systemFontSize"
#define UKUI_STYLE_ICON_THEME_NAME_KEY  "iconThemeName"

namespace UkuiMenu {

void SettingModule::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    qRegisterMetaType<GlobalSetting::Key>("GlobalSetting::Key");
    qmlRegisterUncreatableType<GlobalSetting>(uri, versionMajor, versionMinor, "GlobalSetting", "Use enums only.");
}

GlobalSetting *GlobalSetting::instance()
{
    static GlobalSetting setting(nullptr);
    return &setting;
}

GlobalSetting::GlobalSetting(QObject *parent) : QObject(parent)
{
    initStyleSetting();
    initGlobalSettings();
    initUSDSetting();
}

void GlobalSetting::initStyleSetting()
{
    // GCOVR_EXCL_BR_START
    m_cache.insert(StyleName, UKUI_STYLE_VALUE_LIGHT);
    m_cache.insert(ThemeColor, UKUI_STYLE_THEME_COLOR_KEY);
    m_cache.insert(SystemFontSize, UKUI_STYLE_SYSTEM_FONT_SIZE);
    m_cache.insert(Transparency, 1);
    m_cache.insert(EffectEnabled, false);

    if (QGSettings::isSchemaInstalled(UKUI_STYLE_SCHEMA)) {
        QGSettings *settings = new QGSettings(UKUI_STYLE_SCHEMA, {}, this);

        QStringList keys = settings->keys();
        if (keys.contains(UKUI_STYLE_NAME_KEY)) {
            m_cache.insert(StyleName, settings->get(UKUI_STYLE_NAME_KEY));
        }
        if (keys.contains(UKUI_STYLE_THEME_COLOR_KEY)) {
            m_cache.insert(ThemeColor,settings->get(UKUI_STYLE_THEME_COLOR_KEY));
        }
        if (keys.contains(UKUI_STYLE_SYSTEM_FONT_SIZE)) {
            m_cache.insert(SystemFontSize,settings->get(UKUI_STYLE_SYSTEM_FONT_SIZE));
        }
        connect(settings, &QGSettings::changed, this, [this, settings] (const QString &key) {
            if (key == UKUI_STYLE_NAME_KEY) {
                updateData(StyleName, settings->get(key));
                Q_EMIT styleChanged(StyleName);
            } else if (key == UKUI_STYLE_THEME_COLOR_KEY) {
                updateData(ThemeColor, settings->get(key));
                Q_EMIT styleChanged(ThemeColor);
            } else if (key == UKUI_STYLE_SYSTEM_FONT_SIZE) {
                updateData(SystemFontSize, settings->get(key));
                Q_EMIT styleChanged(SystemFontSize);
            } else if (key == UKUI_STYLE_ICON_THEME_NAME_KEY) {
                updateData(IconThemeName, settings->get(key));
                Q_EMIT styleChanged(IconThemeName);
            }
        });
    }

    if (QGSettings::isSchemaInstalled(CONTROL_CENTER_SETTING)) {
        QGSettings *settings = new QGSettings(CONTROL_CENTER_SETTING,{},this);

        QStringList keys = settings->keys();
        if (keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY)) {
            m_cache.insert(Transparency, settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal());
        }
        if (keys.contains(CONTROL_CENTER_EFFECT_KEY)) {
            m_cache.insert(EffectEnabled, settings->get(CONTROL_CENTER_EFFECT_KEY).toBool());
        }

        connect(settings, &QGSettings::changed, this, [this, settings] (const QString &key) {
            if (key == CONTROL_CENTER_TRANSPARENCY_KEY) {
                updateData(Transparency, settings->get(key).toReal());
                Q_EMIT styleChanged(Transparency);
            }
            if (key == CONTROL_CENTER_EFFECT_KEY) {
                updateData(EffectEnabled, settings->get(key).toReal());
                Q_EMIT styleChanged(EffectEnabled);
            }
        });
    }
    // GCOVR_EXCL_BR_STOP
}

QVariant GlobalSetting::get(const GlobalSetting::Key& key)
{
    QMutexLocker locker(&mutex);
    return m_cache.value(key);
}

void GlobalSetting::updateData(const GlobalSetting::Key &key, const QVariant &value)
{
    QMutexLocker locker(&mutex);
    m_cache.insert(key, value);
}

bool GlobalSetting::isSystemApp(const QString &appid) const
{
//    return std::any_of(m_systemApps.constBegin(), m_systemApps.constEnd(), [&appid](const QString &appName) {
//        return appid.contains(appName);
//    });
    bool removable = kdk_package_is_removable_by_desktop(appid.toUtf8().constData());
    qDebug() << appid << "removable from kdk_package is" << removable;
    return !removable;
}

QStringList GlobalSetting::systemApps() const
{
    return m_systemApps;
}

bool GlobalSetting::isDefaultFavoriteApp(const QString &appid) const
{
    return std::any_of(m_defaultFavoriteApps.constBegin(), m_defaultFavoriteApps.constEnd(), [&appid](const QString &id) {
        return appid == id;
    });
}

QStringList GlobalSetting::defaultFavoriteApps() const
{
    return m_defaultFavoriteApps;
}

void GlobalSetting::initGlobalSettings()
{
    if (QFile::exists(UKUI_MENU_GLOBAL_CONFIG_FILE)) {
        QSettings settings(UKUI_MENU_GLOBAL_CONFIG_FILE, QSettings::IniFormat);
        settings.beginGroup("System Apps");
        m_systemApps.append(settings.allKeys());
        settings.endGroup();

        //=
        settings.beginGroup("Default Favorite Apps");
        QMap<int, QString> apps;
        for (const auto &key : settings.allKeys()) {
            apps.insert(key.toInt(), settings.value(key).toString());
        }

        for (const auto &app : apps) {
            m_defaultFavoriteApps.append(app);
        }
        settings.endGroup();
    }
}

void GlobalSetting::initUSDSetting()
{
    m_cache.insert(IsLiteMode, false);

    // GCOVR_EXCL_START
    const QString service = QStringLiteral("org.ukui.SettingsDaemon");
    const QString path = QStringLiteral("/GlobalSignal");
    const QString interface = QStringLiteral("org.ukui.SettingsDaemon.GlobalSignal");

    QDBusInterface dBusInterface(service, path, interface);
    if (dBusInterface.isValid()) {
        QDBusReply<QString> reply = dBusInterface.call(QStringLiteral("getUKUILiteAnimation"));
        if (reply.isValid()) {
            QMap<QString, QVariant> m;
            m.insert("animation", reply.value());
            onSysModeChanged(m);
        }
    }

    QDBusConnection::sessionBus().connect(service, path, interface, "UKUILiteChanged", this, SLOT(onSysModeChanged));
    // GCOVR_EXCL_STOP
}

void GlobalSetting::onSysModeChanged(QMap<QString, QVariant> map)
{
    if (map.contains("animation")) {
        m_cache.insert(IsLiteMode, (map.value("animation").toString() == QStringLiteral("lite")));
        Q_EMIT styleChanged(IsLiteMode);
    }
}

MenuSetting *MenuSetting::instance()
{
    static MenuSetting setting(nullptr);
    return &setting;
}

MenuSetting::MenuSetting(QObject *parent) : QObject(parent)
{
    initSettings();
}

void MenuSetting::initSettings()
{
    m_cache.insert(UKUI_MENU_ANIMATION_DURATION, {300});
    m_cache.insert(FOLLOW_UKUI_PANEL, {true});
    m_cache.insert(MENU_WIDTH, {766});
    m_cache.insert(MENU_HEIGHT, {688});
    m_cache.insert(MENU_MARGIN, {8});
    m_cache.insert(FULL_SCREEN, {false});
    m_cache.insert(APP_LIST_DISPLAY_MODE, {"all"});

    QByteArray id{UKUI_MENU_SCHEMA};
    // GCOVR_EXCL_BR_START
    if (QGSettings::isSchemaInstalled(id)) {
        m_gSettings = new QGSettings(id, "/", this);

        if (!m_gSettings) {
            qWarning() << "Unable to initialize settings.";
            return;
        }

        //1.x版本号校验,更改开始菜单默认尺寸并添加version字段
        if (!m_gSettings->keys().contains("version")) {
            m_gSettings->set("width", 766);
            m_gSettings->set("height", 688);
        } else {
            QString version = m_gSettings->get("version").toString();
            QVersionNumber versionNumber = QVersionNumber::fromString(version);
            if (versionNumber.majorVersion() < 1) {
                m_gSettings->set("width", 766);
                m_gSettings->set("height", 688);
                if (m_gSettings->trySet("version", "1.0")) {
                    version = "1.0";
                    versionNumber = QVersionNumber::fromString(version);
                }
            }

            if (versionNumber.majorVersion() == 1 && versionNumber.minorVersion() == 0) {
                if (m_gSettings->keys().contains(FULL_SCREEN) && m_gSettings->keys().contains(APP_LIST_DISPLAY_MODE)) {
                    if (m_gSettings->trySet("version", "1.1")) {
                        version = "1.1";
                        versionNumber = QVersionNumber::fromString(version);
                    }
                }
            }
        }

        for (const auto &key: m_gSettings->keys()) {
            m_cache.insert(key, m_gSettings->get(key));
        }

        connect(m_gSettings, &QGSettings::changed, this, &MenuSetting::updateValue);
    }
    // GCOVR_EXCL_BR_STOP
}

QVariant MenuSetting::get(const QString &key)
{
    QMutexLocker locker(&mutex);
    return m_cache.value(key, {});
}

bool MenuSetting::set(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&mutex);
    if (m_gSettings) {
        if (m_gSettings->keys().contains(key)) {
            return m_gSettings->trySet(key, value);
        }
//        m_cache.insert(key,value);
//        Q_EMIT changed(key);
    }

    return false;
}

void MenuSetting::updateValue(const QString &key)
{
    mutex.lock();
    m_cache.insert(key, m_gSettings->get(key));
    mutex.unlock();
    Q_EMIT changed(key);
}

} // UkuiMenu
