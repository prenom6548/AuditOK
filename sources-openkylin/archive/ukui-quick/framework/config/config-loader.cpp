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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "config-loader.h"
#include <QStandardPaths>
#include <QDir>
#include <QMap>
#include <QDebug>
#include <QVariant>
#include <QStringLiteral>

/*!
    \class UkuiQuick::ConfigLoader
    \inheaderfile config-loader.h
    \inmodule UkuiQuickFramework
    \brief 配置加载器,该类用于加载配置文件.
 */

/*!
    \enum ConfigLoader::Domain
    \brief 配置文件域.

    \value Local 本地配置文件域
    \value Global 全局配置文件域
 */

/*!
    \enum ConfigLoader::Type
    \brief 配置文件格式.

    \value Json JSON格式
    \value Ini INI格式
 */
namespace UkuiQuick {

namespace {

bool isPathComponent(const QString &value)
{
    return !value.isEmpty()
        && value != QStringLiteral(".")
        && value != QStringLiteral("..")
        && !QDir::isAbsolutePath(value)
        && !value.contains(QLatin1Char('/'))
        && !value.contains(QLatin1Char('\\'));
}

}

QMap<ConfigLoader::Domain, QMap<QString, Config*>> ConfigLoader::globalCache = QMap<ConfigLoader::Domain, QMap<QString, Config*>>();

/*!
    \fn Config *ConfigLoader::getConfig(const QString &id, ConfigLoader::Domain domain, const QString &appid)
    \brief 获取配置文件.
    \a id 配置文件 \ cid ,\a domain 配置文件域 , \a appid 应用 \c id
 */
Config *ConfigLoader::getConfig(const QString &id, ConfigLoader::Domain domain, const QString &appid)
{
    QString filename = getFullFileName(id, Json, domain, appid);
    if (filename.isEmpty()) {
        return nullptr;
    }

    const QString cacheKey = (domain == Local) ? (appid + QStringLiteral("_") + id) : id;
    auto &cache = ConfigLoader::globalCache[domain];
    if (cache.contains(cacheKey)) {
        return cache.value(cacheKey);
    }

    auto config = new Config(filename);
    cache.insert(cacheKey, config);

    return config;
}

/*!
    \brief 获取配置文件的完整路径.

    \a id 配置文件 \c id, \a type 配置文件格式, \a domain 配置文件域, \a appid 应用 \c id
*/
QString ConfigLoader::getFullFileName(const QString &id, Type type, Domain domain, const QString &appid)
{
    const QString filename = ConfigLoader::getConfigFileName(id, type);
    if (filename.isEmpty()) {
        return {};
    }

    if (domain == Global) {
        return ConfigLoader::globalConfigPath() + filename;
    }

    if (!isPathComponent(appid)) {
        return {};
    }

    return ConfigLoader::localConfigPath() + appid + QStringLiteral("/") + filename;
}

/*!
    \brief 获取全局配置文件的完整路径.
*/
QString ConfigLoader::globalConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/org.ukui/_ukui-config-global/");
}

/*!
    \brief 获取本地配置文件的完整路径.
*/
QString ConfigLoader::localConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/org.ukui/");
}

/*!
    \brief 获取配置文件的文件名.

    \a id 配置文件 \c id, \a type 配置文件格式
*/
QString ConfigLoader::getConfigFileName(const QString &id, ConfigLoader::Type type)
{
    if (!isPathComponent(id)) {
        return {};
    }

    if (type == Json) {
        return id + QStringLiteral(".json");
    }

    return id + QStringLiteral(".conf");
}

} // UkuiQuick
