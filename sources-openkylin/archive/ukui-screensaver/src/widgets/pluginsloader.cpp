/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "pluginsloader.h"
#include "loginplugininterface.h"

#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <QPluginLoader>
#include <QApplication>

const QString gPluginDir = "/usr/lib/ukui-greeter/plugins";

PluginsLoader::PluginsLoader(QObject *parent) : QThread(parent) {}

PluginsLoader::~PluginsLoader() {}

PluginsLoader &PluginsLoader::instance()
{
    static PluginsLoader pluginsLoader;
    return pluginsLoader;
}

LoginPluginInterface *PluginsLoader::findModuleByName(const QString &name) const
{
    return m_plugins.value(name, nullptr).data();
}

QMap<QString, LoginPluginInterface *> PluginsLoader::findModulesByType(const int type) const
{
    QMap<QString, LoginPluginInterface *> modules;
    for (QSharedPointer<LoginPluginInterface> module : m_plugins.values()) {
        if (module.isNull()) {
            continue;
        }

        if (module->getPluginType() == type) {
            modules.insert(module->name(), module.data());
        }
    }
    return modules;
}

void PluginsLoader::run()
{
    findModule(gPluginDir);
}

void PluginsLoader::findModule(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qInfo() << path << "is not exists.";
        return;
    }
    const QFileInfoList modules = dir.entryInfoList();
    for (QFileInfo module : modules) {
        const QString path = module.absoluteFilePath();
        if (!QLibrary::isLibrary(path)) {
            continue;
        }
        qInfo() << module << "is found";
        QPluginLoader loader(path);

        LoginPluginInterface *moduleInstance = dynamic_cast<LoginPluginInterface *>(loader.instance());
        if (!moduleInstance) {
            qWarning() << loader.errorString();
            continue;
        }

        if (!moduleInstance->needLoad()) {
            continue;
        }
        if (m_plugins.contains(moduleInstance->name())) {
            continue;
        }

        QObject *obj = dynamic_cast<QObject *>(moduleInstance);
        if (obj)
            obj->moveToThread(qApp->thread());

        m_plugins.insert(moduleInstance->name(), QSharedPointer<LoginPluginInterface>(moduleInstance));
        emit moduleFound(moduleInstance);
    }
}

void PluginsLoader::removeModule(const QString &moduleKey)
{
    if (!m_plugins.contains(moduleKey))
        return;

    m_plugins.remove(moduleKey);
}
