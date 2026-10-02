/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#include "yamlhelper.h"
#include <yaml-cpp/yaml.h>
#include <QDir>
#include "syslog.h"

#include "utils.cpp"

#define _PERMISSION "_permission"
#define _DEFAULT "_default"
#define _TYPE "_type"
#define _SUMMARY "_summary"
#define _DESCRIPTION "_description"

namespace
{
    QVariant yamlNodeToVariant(const YAML::Node &node)
    {
        if (node.IsScalar())
        {
            try
            {
                return QVariant(QString::fromStdString(node.as<std::string>()));
            }
            catch (const YAML::BadConversion &)
            {
                try
                {
                    return QVariant(QString::number(node.as<int>()));
                }
                catch (const YAML::BadConversion &)
                {
                    try
                    {
                        return QVariant(QString::number(node.as<double>()));
                    }
                    catch (const YAML::BadConversion &)
                    {
                        try
                        {
                            return QVariant(QString("%1").arg(node.as<bool>() ? "true" : "false"));
                        }
                        catch (const YAML::BadConversion &)
                        {
                            return QVariant();
                        }
                    }
                }
            }
        }
        else if (node.IsSequence())
        {
            QList<QVariant> list;
            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
            {
                list.append(yamlNodeToVariant(*it));
            }
            return list;
        }
        else if (node.IsMap())
        {
            QMap<QString, QVariant> map;
            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
            {
                QString key = QString::fromStdString(it->first.as<std::string>());
                QVariant value = yamlNodeToVariant(it->second);
                map.insert(key, value);
            }
            return map;
        }
        return QVariant();
    }

    QMap<QString, QVariant> yamlToQMap(const YAML::Node &root)
    {
        if (!root.IsMap())
            throw std::runtime_error("Root YAML node must be a map");
        return yamlNodeToVariant(root).toMap();
    }

    bool isFinalOrStable(const QMap<QString, QVariant> &map, int mergeType)
    {
        if (!map.contains(_PERMISSION))
            return false;
        QString permission = map[_PERMISSION].toString();
        return (mergeType == 1 && permission == "final") || (mergeType == 2 && (permission == "final" || permission == "stable"));
    }
}
YamlHelper::YamlHelper() {}

QMap<QString, QVariant> YamlHelper::loadYamlDirs(const QStringList &dirs)
{
    QMap<QString, QVariant> result;
    QStringList tmp = dirs;

    if (tmp.isEmpty())
    {
        YAML::Node conf2Node = YAML::LoadFile("/etc/kylin-config/conf2.yaml");
        std::list<std::string> stdDirs = conf2Node["dirs"].as<std::list<std::string>>();
        for (std::list<std::string>::const_iterator it = stdDirs.begin(); it != stdDirs.end(); ++it)
        {
            tmp.append(QString::fromStdString(*it));
        }
    }

    result = loadYamlDir("/etc/kylin-config/basic");

    for (QString &dir : tmp)
    {
        if (dir == "basic")
            continue;

        QDir dire(QString("/etc/kylin-config/%1").arg(dir)), opt_dire(QString("/opt/system/conf/kylin/kylin-config/%1").arg(dir));

        QStringList files = dire.entryList(QStringList() << "*.yaml" << "*.yml") + opt_dire.entryList(QStringList() << "*.yaml" << "*.yml");
        std::sort(files.begin(), files.end(), sortString);
        
        for (QString &file : files)
        {
            QString real_path = QFile::exists(dire.filePath(file)) ? dire.filePath(file) : opt_dire.filePath(file);
            QMap<QString, QVariant> map = loadYamlFile(real_path);
            if (map.isEmpty())
                continue;

            mergeSelfVersion(map);
            overrideConfig(result, map, true);
        }
    }

    return result;
}
QMap<QString, QVariant> YamlHelper::loadYamlDir(const QString &dir)
{
    QMap<QString, QVariant> result;

    QString opt_dir = dir;
    opt_dir.replace("/etc/kylin-config", "/opt/system/conf/kylin/kylin-config");

    QDir dire(dir), opt_dire(opt_dir);
    QStringList files = dire.entryList(QStringList() << "*.yaml" << "*.yml") + opt_dire.entryList(QStringList() << "*.yaml" << "*.yml");
    std::sort(files.begin(), files.end(), sortString);

    for (QString &file : files)
    {
        QString real_path = QFile::exists(dire.filePath(file)) ? dire.filePath(file) : opt_dire.filePath(file);
        QMap<QString, QVariant> map = loadYamlFile(real_path);
        if (map.isEmpty())
            continue;
        mergeMap(result, map, 1, true);
    }
    return result;
}

QMap<QString, QVariant> YamlHelper::loadYamlFile(const QString &file)
{
    try
    {
        return yamlToQMap(YAML::LoadFile(file.toStdString()));
    }
    catch (const std::exception &e)
    {
        syslog(LOG_ERR, "Load %s failed: %s", file.toLatin1().data(), e.what());
        return QMap<QString, QVariant>();
    }
}

void YamlHelper::mergeMap(QMap<QString, QVariant> &dest, const QMap<QString, QVariant> &src, int mergeType, bool passTypeChanged)
{
    if (isFinalOrStable(dest, mergeType))
        return;
    if (passTypeChanged && dest.contains(_TYPE) && dest[_TYPE] != src[_TYPE])
        return;

    for (QMap<QString, QVariant>::const_iterator it = src.begin(); it != src.end(); ++it)
    {
        const QString &key = it.key();
        const QVariant &value = it.value();

        if (!dest.contains(key))
        {
            dest.insert(key, value);
        }
        else
        {
            QVariant &destValue = dest[key];
            if (destValue.type() == QVariant::Map && value.type() == QVariant::Map)
            {
                QMap<QString, QVariant> destMap = destValue.toMap();
                mergeMap(destMap, value.toMap(), mergeType, passTypeChanged);
                destValue = destMap;
            }
            else
            {
                destValue = value;
            }
        }
    }
}

void YamlHelper::overrideConfig(QMap<QString, QVariant> &dest, const QMap<QString, QVariant> &src, bool withoutVersion)
{
    for (QMap<QString, QVariant>::const_iterator it = src.begin(); it != src.end(); ++it)
    {
        const QString &app = it.key();
        const QVariant &value = it.value();

        bool isGroupFile = value.toMap().first().toMap().contains(_DEFAULT);
        if (isGroupFile)
        {
            groupOverride(dest, app, value.toMap());
        }
        else
        {
            if (withoutVersion)
            {
                appOverrideWithoutVersion(dest, app, value.toMap());
            }
            else
            {
                appOverride(dest, app, value.toMap());
            }
        }
    }
}

void YamlHelper::groupOverride(QMap<QString, QVariant> &dest, const QString &group, const QMap<QString, QVariant> &groupData)
{
    for (QMap<QString, QVariant>::iterator it = dest.begin(); it != dest.end(); ++it)
    {
        QVariant &appValue = it.value();
        QMap<QString, QVariant> appData = appValue.toMap();

        for (QString &version : appData.keys())
        {
            QVariant &versionValue = appData[version];
            if (versionValue.type() != QVariant::Map)
                continue;

            QMap<QString, QVariant> versionData = versionValue.toMap();
            if (versionData.contains(group))
            {
                QMap<QString, QVariant> destGroupData = versionData[group].toMap();
                mergeMap(destGroupData, groupData, 1, true);
                versionData[group] = destGroupData;
                appData[version] = versionData;
                appValue = appData;
            }
        }
    }
}

void YamlHelper::appOverride(QMap<QString, QVariant> &dest, const QString &app, const QMap<QString, QVariant> &appData)
{
    Q_UNUSED(dest)
    Q_UNUSED(app)
    Q_UNUSED(appData)
}

void YamlHelper::appOverrideWithoutVersion(QMap<QString, QVariant> &dest, const QString &app, const QMap<QString, QVariant> &appData)
{
    if (dest.contains(app))
    {
        QVariant &destAppValue = dest[app];
        QMap<QString, QVariant> destAppData = destAppValue.toMap();

        for (QString &destVersion : destAppData.keys())
        {
            QVariant &destVersionValue = destAppData[destVersion];
            if (destVersionValue.type() != QVariant::Map)
                continue;

            QMap<QString, QVariant> destVersionData = destVersionValue.toMap();
            QMap<QString, QVariant> versionData = appData.first().toMap();
            mergeMap(destVersionData, versionData, 1, true);

            destVersionValue = destVersionData;
            destAppData[destVersion] = destVersionValue;
            destAppValue = destAppData;
        }
    }
    else
    {
        dest[app] = appData;
    }
}

void YamlHelper::mergeSelfVersion(QMap<QString, QVariant> &src)
{
    for (QString &srcApp : src.keys())
    {
        QVariant &srcAppValue = src[srcApp];
        QMap<QString, QVariant> srcAppData = srcAppValue.toMap();

        if (srcAppData.first().toMap().contains(_DEFAULT))
            continue;

        QStringList srcVersions = srcAppData.keys();
        std::sort(srcVersions.begin(), srcVersions.end(), compareVersions);

        QMap<QString, QVariant> data;
        for (QString &srcVersion : srcVersions)
        {
            QMap<QString, QVariant> srcVersionData = srcAppData[srcVersion].toMap();
            mergeMap(data, srcVersionData, 1, true);
        }

        srcAppData.clear();
        srcAppData[srcVersions[0]] = data;
        srcAppValue = srcAppData;
    }
}
