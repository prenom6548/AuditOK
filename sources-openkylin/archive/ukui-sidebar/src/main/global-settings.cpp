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

#include <glib.h>
#include <gio/gio.h>

#include <mutex>
#include <QDBusReply>
#include <QDebug>

#include "global-settings.h"

#define UKUI_PANEL_SETTING          "org.ukui.panel.settings"

using namespace Sidebar;

static std::once_flag onceFlag;
static GlobalSettings *g_globalSettings = nullptr;
static GSettings * m_settings = nullptr;
static GSettingsSchema * m_schema = nullptr;
static const char *m_panelLengthKey = "panellength";

GlobalSettings *GlobalSettings::globalInstance(QObject *parent)
{
    std::call_once(onceFlag, [ & ] {
        g_globalSettings = new GlobalSettings(parent);
    });
    return g_globalSettings;
}

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
    // 1.加载主题 gsetting
    initStyleSettings();
    initControlCenterSettings();
    // 2.加载平板模式设置
    initStatusManagerDbus();
    initPanelMonitor();
    // 3.缩放相关
//    initUSDCenterSettings();
    initUSDSetting();

    initPanelGSettings();
}

GlobalSettings::~GlobalSettings()
{
    if (m_settings) {
        g_object_unref(m_settings);
    }
    if (m_schema) {
        g_settings_schema_unref(m_schema);
    }
    g_globalSettings = nullptr;
}

const QVariant GlobalSettings::getValue(const QString &key)
{
    if (m_cache.contains(key)) {
        return m_cache.value(key);
    }

    return {};
}

const QStringList GlobalSettings::getKeys()
{
    return m_cache.keys();
}

int GlobalSettings::getPanelLength(QString screenName)
{
    if (!m_settings || !m_schema) return -1;
    if (!isKeysContain(m_panelLengthKey)) return -1;

    QMap<QString, QVariant> map = getPanelLengthMap();
    if (!map.contains(screenName)) {
        return -1;
    }
    return map.value(screenName).toInt();
}

void GlobalSettings::initStyleSettings()
{
    insertValue(UKUI_STYLE_NAME_KEY, DEFAULT_STYLE);
    insertValue(UKUI_STYLE_WINDOW_RADIUS_KEY, 12);

    const QByteArray id(UKUI_STYLE_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto *settings = new QGSettings(id, QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == UKUI_STYLE_NAME_KEY || key == UKUI_STYLE_THEME_COLOR_KEY || key == UKUI_STYLE_WINDOW_RADIUS_KEY) {
                insertValue(key, settings->get(key));
                Q_EMIT valueChanged(key);
            }
        });

        QStringList keys = settings->keys();
        if (keys.contains(UKUI_STYLE_NAME_KEY)) {
            insertValue(UKUI_STYLE_NAME_KEY, settings->get(UKUI_STYLE_NAME_KEY));
        }
        if (keys.contains(UKUI_STYLE_WINDOW_RADIUS_KEY)) {
            insertValue(UKUI_STYLE_WINDOW_RADIUS_KEY, settings->get(UKUI_STYLE_WINDOW_RADIUS_KEY));
        }
    }
}

void GlobalSettings::insertValue(const QString &key, const QVariant &value)
{
    m_cache.insert(key,value);
}

void GlobalSettings::initStatusManagerDbus()
{
    m_cache.insert(TABLET_MODE, false);
    //dbus
    m_statusManagerDBus = new QDBusInterface(DBUS_STATUS_MANAGER_IF, "/" , DBUS_STATUS_MANAGER_IF, QDBusConnection::sessionBus(), this);
    if (m_statusManagerDBus && m_statusManagerDBus->isValid()) {
        //平板模式切换
        connect(m_statusManagerDBus, SIGNAL(mode_change_signal(bool)), this, SLOT(updateTabletStatus(bool)));

        QDBusReply<bool> message = m_statusManagerDBus->call("get_current_tabletmode");
        if (message.isValid()) {
            m_cache.insert(TABLET_MODE, message.value());
        }
    }
}

void GlobalSettings::updateTabletStatus(bool isTabletMode)
{
    m_cache.insert(TABLET_MODE, isTabletMode);
    Q_EMIT valueChanged(TABLET_MODE);
}

void GlobalSettings::initControlCenterSettings()
{
    insertValue(CONTROL_CENTER_EFFECT, false);
    insertValue(CONTROL_CENTER_TRANSPARENCY_KEY, 1.0);
    const QByteArray id(CONTROL_CENTER_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto *settings = new QGSettings(id, QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == CONTROL_CENTER_TRANSPARENCY_KEY || key == CONTROL_CENTER_EFFECT) {
                insertValue(key, settings->get(key));
                Q_EMIT valueChanged(key);
            }
        });

        QStringList keys = settings->keys();
        if (keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY)) {
            insertValue(CONTROL_CENTER_TRANSPARENCY_KEY, settings->get(CONTROL_CENTER_TRANSPARENCY_KEY));
        }
        if (keys.contains(CONTROL_CENTER_EFFECT)) {
            insertValue(CONTROL_CENTER_EFFECT, settings->get(CONTROL_CENTER_EFFECT));
        }
    }
}

void GlobalSettings::initUSDSetting()
{
    m_cache.insert(IS_LITE_MODE, false);

    const QString service = QStringLiteral("org.ukui.SettingsDaemon");
    const QString path = QStringLiteral("/GlobalSignal");
    const QString interface = QStringLiteral("org.ukui.SettingsDaemon.GlobalSignal");

    QDBusInterface dBusInterface(service, path, interface);
    if (dBusInterface.isValid()) {
        QDBusReply<QString> reply = dBusInterface.call(QStringLiteral("getUKUILiteAnimation"));
        if (reply.isValid()) {
            QMap<QString, QVariant> m;
            m.insert("animation", reply.value());
            updateIsLiteMode(m);
        }
    }

    QDBusConnection::sessionBus().connect(service, path, interface, "UKUILiteChanged", this, SLOT(updateIsLiteMode));
}

void GlobalSettings::initPanelMonitor()
{
    m_cache.insert(UKUI_PANEL_POSITION_KEY, 0);
    m_cache.insert(UKUI_PANEL_SIZE_KEY, 48);

    const QByteArray id(UKUI_PANEL_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto panelGSetting = new QGSettings(id, QByteArray(), this);

        QStringList keys = panelGSetting->keys();
        if (keys.contains(UKUI_PANEL_POSITION_KEY)) {
            m_cache.insert(UKUI_PANEL_POSITION_KEY, panelGSetting->get(UKUI_PANEL_POSITION_KEY));
        }
        if (keys.contains(UKUI_PANEL_SIZE_KEY)) {
            m_cache.insert(UKUI_PANEL_SIZE_KEY, panelGSetting->get(UKUI_PANEL_SIZE_KEY));
        }
        if (keys.contains(UKUI_PANEL_TYPE_KEY)) {
            m_cache.insert(UKUI_PANEL_TYPE_KEY, panelGSetting->get(UKUI_PANEL_TYPE_KEY));
        }
        if (keys.contains(UKUI_SETTINGS_ISLAND_POSITION_KEY)) {
            m_cache.insert(UKUI_SETTINGS_ISLAND_POSITION_KEY, panelGSetting->get(UKUI_SETTINGS_ISLAND_POSITION_KEY));
        }
        if (keys.contains(UKUI_DATA_ISLAND_POSITION_KEY)) {
            m_cache.insert(UKUI_DATA_ISLAND_POSITION_KEY, panelGSetting->get(UKUI_DATA_ISLAND_POSITION_KEY));
        }
        if (keys.contains(UKUI_TOPBAR_SIZE_KEY)) {
            m_cache.insert(UKUI_TOPBAR_SIZE_KEY, panelGSetting->get(UKUI_TOPBAR_SIZE_KEY));
        }

        connect(panelGSetting, &QGSettings::changed, this, [this, panelGSetting] (const QString &key) {
            if (key == UKUI_PANEL_POSITION_KEY || key == UKUI_PANEL_SIZE_KEY ||
                key == UKUI_PANEL_TYPE_KEY     || key == UKUI_SETTINGS_ISLAND_POSITION_KEY ||
                key == UKUI_DATA_ISLAND_POSITION_KEY || key == UKUI_TOPBAR_SIZE_KEY) {
                insertValue(key, panelGSetting->get(key));
                Q_EMIT valueChanged(key);
            }
            if (key == UKUI_PANEL_LENGTH_KEY) {
                Q_EMIT valueChanged(key);
            }
        });
    }
}

void GlobalSettings::initPanelGSettings()
{
    GSettingsSchemaSource *source;

    source = g_settings_schema_source_get_default();
    m_schema = g_settings_schema_source_lookup(source, "org.ukui.panel.settings", true);

    if (!m_schema) {
        m_settings = nullptr;
        return;
    }

    m_settings = g_settings_new_with_path("org.ukui.panel.settings", "/org/ukui/panel/settings/");
}

bool GlobalSettings::isKeysContain(const char *key)
{
    if (!m_settings || !m_schema) return false;

        gchar **keys = g_settings_schema_list_keys(m_schema);
        if (g_strv_contains(keys, key)) {
            g_strfreev(keys);
            return true;
        } else {
            g_strfreev(keys);
            return false;
        }
}

QMap<QString, QVariant> GlobalSettings::getPanelLengthMap()
{
    GVariant *gvalue = g_settings_get_value(m_settings, m_panelLengthKey);

    GVariantIter iter;
    QMap<QString, QVariant> map;
    const gchar *key;
    size_t str_len;
    GVariant *val = NULL;
    g_variant_iter_init (&iter, gvalue);
    QVariant qvar;

    while (g_variant_iter_next (&iter, "{&sv}", &key, &val)) {
        if (g_variant_is_of_type(val, G_VARIANT_TYPE_UINT32)) {
            qvar = QVariant::fromValue(static_cast<quint32>(g_variant_get_uint32(val)));
            map.insert(key, qvar);
        }
    }

    g_variant_unref(gvalue);

    return map;
}

void GlobalSettings::updateIsLiteMode(QMap<QString, QVariant> map)
{
    if (map.contains("animation")) {
        insertValue(IS_LITE_MODE, map.value("animation").toString() == QStringLiteral("lite"));
        Q_EMIT valueChanged(IS_LITE_MODE);
    }
}

void GlobalSettings::initUSDCenterSettings()
{
    insertValue(USD_SCALING_FACTOR_KEY, 1.0);
    const QByteArray id(USD_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto *settings = new QGSettings(id, QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == USD_SCALING_FACTOR_KEY) {
                insertValue(key, settings->get(key));
                Q_EMIT valueChanged(key);
            }
        });

        QStringList keys = settings->keys();
        if (keys.contains(USD_SCALING_FACTOR_KEY)) {
            insertValue(USD_SCALING_FACTOR_KEY, settings->get(USD_SCALING_FACTOR_KEY));
        }
    }
}

SettingMonitor::SettingMonitor(QObject *parent) : QObject(parent)
{
    connect(GlobalSettings::globalInstance(), &GlobalSettings::valueChanged,
            this, &SettingMonitor::valueChangedSlot);
}

QStringList SettingMonitor::keys()
{
    return m_keys;
}

void SettingMonitor::setKeys(QStringList &keys)
{
    if (keys == m_keys) {
        return;
    }

    m_keys.clear();
    m_keys.append(keys);

    Q_EMIT keysChanged();
}

void SettingMonitor::valueChangedSlot(const QString &key)
{
    if (m_keys.contains(key)) {
        Q_EMIT valueChanged(key);
    }
}

QVariant SettingMonitor::getValue(const QString &key)
{
    return GlobalSettings::globalInstance()->getValue(key);
}
