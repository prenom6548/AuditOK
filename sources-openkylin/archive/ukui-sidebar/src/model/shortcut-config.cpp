/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "shortcut-config.h"

#include <config-loader.h>
#include <QDebug>

static const QStringList SHORTCUT_ADDED_LIST = {"WiFi",
                                                "BluetoothShortcut",
                                                "StatusChangeShortcut",
                                                "AutoRotationShortcut",
                                                "EyeCareShortcut",
                                                "DarkModeShortcut",
                                                "Clock",
                                                "Notebook",
                                                "MultiScreenShortcut",
                                                "ScreenShot",
                                                "ServicesAndSupportsShortcut",
                                                "SplitviewManagerShortcut"};

static const QStringList SHORTCUT_NOT_ADDED_LIST = {"FlightModeShortcut",
                                                    "ClipboardShortcut",
                                                    "DNDModeShortcut",
                                                    "PowerModeShortcut",
                                                    "VPN",
                                                    "ConnectivityShortcut"};
using namespace UkuiShortcut;

ShortcutConfig &ShortcutConfig::instance()
{
    static ShortcutConfig shortcutConfig;
    return shortcutConfig;
}

void ShortcutConfig::setShortcutOrder(const int &indexFrom, const int &indexTo, PluginStateType::AddState state)
{
    if (indexFrom < 0 || indexTo < 0) return;

    if (state == PluginStateType::Add) {
        if ((indexFrom > m_addedList.length() - 1) || indexTo > m_addedList.length() - 1) return;
        m_addedList.move(indexFrom, indexTo);
        m_config->setValue(QStringLiteral("addedList"), m_addedList);
    } else {
        if ((indexFrom > m_notAddedList.length() - 1) || indexTo > m_notAddedList.length() - 1) return;
        m_notAddedList.move(indexFrom, indexTo);
        m_config->setValue(QStringLiteral("notAddedList"), m_notAddedList);
    }

    m_config->forceSync();
    verify();
}

void ShortcutConfig::setShortcutOrder(const QString &idFrom, const int &indexTo, PluginStateType::AddState stateTo)
{
    if (stateTo == PluginStateType::Add) {
        if (m_addedList.contains(idFrom)) return;
        m_notAddedList.removeAll(idFrom);
        m_addedList.insert(indexTo, idFrom);
    } else {
        if (m_notAddedList.contains(idFrom)) return;
        m_addedList.removeAll(idFrom);
        m_notAddedList.insert(indexTo, idFrom);
    }
    m_config->setValue(QStringLiteral("addedList"), m_addedList);
    m_config->setValue(QStringLiteral("notAddedList"), m_notAddedList);
    m_config->forceSync();
    verify();
}

void ShortcutConfig::changeShortcutState(const QString &idFrom, PluginStateType::AddState stateFrom, bool atEnd)
{
    if (stateFrom == PluginStateType::Add) {
        if (!m_addedList.contains(idFrom)) return;
        m_addedList.removeAll(idFrom);
        if (atEnd) {
            m_notAddedList.append(idFrom);
        } else {
            m_notAddedList.prepend(idFrom);
        }
    } else {
        if (!m_notAddedList.contains(idFrom)) return;
        m_notAddedList.removeAll(idFrom);
        if (atEnd) {
            m_addedList.append(idFrom);
        } else {
            m_addedList.prepend(idFrom);
        }
    }
    m_config->setValue(QStringLiteral("addedList"), m_addedList);
    m_config->setValue(QStringLiteral("notAddedList"), m_notAddedList);
    m_config->forceSync();
    verify();
}

void ShortcutConfig::recordShortcut(const QString &key)
{
    if (m_addedList.contains(key) || m_notAddedList.contains(key)) {
        return;
    }

    m_notAddedList.append(key);
    m_config->setValue(QStringLiteral("notAddedList"), m_notAddedList);
    m_config->forceSync();
    verify();
}

PluginStateType::AddState ShortcutConfig::getShortcutState(const QString &key)
{
    if (m_addedList.contains(key)) {
        return PluginStateType::Add;
    } else {
        return PluginStateType::NotAdd;
    }
}

int ShortcutConfig::getShortcutIndex(const QString &key, PluginStateType::AddState state)
{
    if (state == PluginStateType::Add) {
        return m_addedList.indexOf(key);
    } else {
        return m_notAddedList.indexOf(key);
    }
}

ShortcutConfig::ShortcutConfig(QObject *parent) : QObject{parent}
{
    initConfig();
    verify();
}

void ShortcutConfig::initConfig()
{
    if (!m_config) {
        m_config = UkuiQuick::ConfigLoader::getConfig("org.ukui.shortcut", UkuiQuick::ConfigLoader::Local, "ukui-sidebar");
    }
    const auto &data = m_config->data();

    //版本号校验
    if (!data.contains(QStringLiteral("version"))) {
        m_config->setValue(QStringLiteral("version"), QStringLiteral("1.0"));
        m_config->setValue(QStringLiteral("addedList"), SHORTCUT_ADDED_LIST);
        m_config->setValue(QStringLiteral("notAddedList"), SHORTCUT_NOT_ADDED_LIST);
        m_addedList = SHORTCUT_ADDED_LIST;
        m_notAddedList = SHORTCUT_NOT_ADDED_LIST;
    } else {

        if (!data.contains(QStringLiteral("addedList"))) {
            m_config->setValue(QStringLiteral("addedList"), SHORTCUT_ADDED_LIST);
            m_addedList = SHORTCUT_ADDED_LIST;
        } else {
            m_addedList = m_config->getValue(QStringLiteral("addedList")).toStringList();
        }

        if (!data.contains(QStringLiteral("notAddedList"))) {
            m_config->setValue(QStringLiteral("notAddedList"), SHORTCUT_NOT_ADDED_LIST);
            m_notAddedList = SHORTCUT_NOT_ADDED_LIST;
        } else {
            m_notAddedList = m_config->getValue(QStringLiteral("notAddedList")).toStringList();
        }
    }

    connect(m_config, &UkuiQuick::Config::configChanged, this, &ShortcutConfig::configChanged);
    m_config->forceSync();
}

void ShortcutConfig::verify()
{
    QSet<QString> setAdd(m_addedList.begin(), m_addedList.end());
    QSet<QString> setNotAdd(m_notAddedList.begin(), m_notAddedList.end());

    if (setAdd.intersects(setNotAdd)) {

        qDebug() << "shortcut config duplicated";
        setAdd.intersect(setNotAdd);

        for (const QString &id : setAdd) {
            m_notAddedList.removeAll(id);
        }

        m_config->setValue(QStringLiteral("notAddedList"), m_notAddedList);
        m_config->forceSync();
    }
}
