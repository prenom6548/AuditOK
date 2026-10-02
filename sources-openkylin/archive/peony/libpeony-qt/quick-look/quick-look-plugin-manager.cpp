/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 */

#include "quick-look-plugin-manager.h"

#include "quick-look-plugin-iface.h"

#include <QObject>

using namespace Peony;

static QuickLookPluginManager *global_instance = nullptr;

QuickLookPluginManager *QuickLookPluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new QuickLookPluginManager;
    }
    return global_instance;
}

QuickLookPluginManager::QuickLookPluginManager(QObject *parent) : QObject(parent)
{
}

QuickLookPluginManager::~QuickLookPluginManager()
{
    clearCurrentWindow();
}

bool QuickLookPluginManager::registerPlugin(QuickLookPluginIface *plugin)
{
    if (!plugin || m_hash.value(plugin->name())) {
        return false;
    }

    m_hash.insert(plugin->name(), plugin);
    return true;
}

bool QuickLookPluginManager::unregisterPlugin(QuickLookPluginIface *plugin)
{
    if (!plugin || !m_hash.value(plugin->name())) {
        return false;
    }

    if (m_currentPlugin == plugin) {
        clearCurrentWindow();
        m_currentPlugin = nullptr;
    }

    m_hash.remove(plugin->name());
    return true;
}

const QStringList QuickLookPluginManager::getPluginIds() const
{
    return m_hash.keys();
}

QuickLookPluginIface *QuickLookPluginManager::getPlugin(const QString &pluginId) const
{
    return m_hash.value(pluginId);
}

void QuickLookPluginManager::showQuickLook(const QString &uri, QWidget *parent)
{
    if (uri.isEmpty()) {
        return;
    }

    QuickLookPluginIface *plugin = availablePlugin();
    if (!plugin) {
        return;
    }

    if (m_currentPlugin != plugin) {
        clearCurrentWindow();
        m_currentPlugin = plugin;
    }

    if (!m_currentWindow) {
        m_currentWindow = plugin->createQuickLookWindow();
    }

    if (!m_currentWindow) {
        return;
    }

    m_currentWindow->prepare(uri);
    m_currentWindow->show(parent);
}

void QuickLookPluginManager::closeQuickLook()
{
    if (m_currentWindow) {
        m_currentWindow->closeQuickLook();
    }
}

QuickLookPluginIface *QuickLookPluginManager::availablePlugin() const
{
    for (QuickLookPluginIface *plugin : m_hash) {
        if (plugin && plugin->isEnable()) {
            return plugin;
        }
    }

    return nullptr;
}

void QuickLookPluginManager::clearCurrentWindow()
{
    if (!m_currentWindow) {
        return;
    }

    m_currentWindow->closeQuickLook();
    QObject *object = m_currentWindow->asObject();
    if (object) {
        object->deleteLater();
    } else {
        delete m_currentWindow;
    }
    m_currentWindow = nullptr;
}
