/*
 * kylin-os-manager
 *
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
 */

#include <sys/stat.h>

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>
#include <QString>
#include <QDebug>

#include "utils.h"
#include "utils-config.h"

static constexpr char TOOLBOX_CONFIG_PATH[] = "/etc/kylin-os-manager/kylin-os-manager-box.conf";
static constexpr char TOOLBOX_INSTALL_ICON_TO_DESKTOP_KEY[] = "copyIconToDesktop";
static constexpr char TOOLBOX_INSTALL_ICON_TO_DESKTOP_CACHE_FILE[] = "installIconToDesktop.cache";

static constexpr char SELF_CONFIG_DIR[] = ".kylin-os-manager.config";

void UtilsConfig::installToolBoxDesktop(void)
{
    QFile file(TOOLBOX_CONFIG_PATH);
    if (!file.exists()) {
        qCritical() << "tool box config file is not exists !";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "open tool box config file fail ! [errno: " << file.error() << "]";
        return;
    }

    QString data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data.toLocal8Bit().data());
    QJsonObject obj = doc.object();

    QStringList installCache = UtilsConfig::getInstallIcon();

    QStringList keys = obj.keys();
    for (int i = 0; i < keys.size(); i++) {
        QJsonObject item = obj.value(keys.at(i)).toObject();
        if (item.contains(TOOLBOX_INSTALL_ICON_TO_DESKTOP_KEY)) {
            QString iconPath = item.value(TOOLBOX_INSTALL_ICON_TO_DESKTOP_KEY).toString();

            if (installCache.contains(iconPath)) {
                continue;
            }

            QFileInfo fileInfo(iconPath);
            if (!fileInfo.exists()) {
                qWarning() << "tool box install icon " << iconPath << " is not exits !";
                continue;
            }

            QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            if (desktopPath.endsWith('/')) {
                desktopPath += fileInfo.fileName();
            } else {
                desktopPath += '/';
                desktopPath += fileInfo.fileName();
            }

            qInfo() << "install tool box icon " << iconPath << " to " << desktopPath;

            if (!QFile::exists(desktopPath)) {
                if (!QFile::copy(iconPath, desktopPath)) {
                    qCritical() << "tool box install icon " << iconPath << " fail !";
                    continue;
                }

                chmod(desktopPath.toLocal8Bit().data(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

                UtilsConfig::saveInstallIcon(iconPath);
            }
        }
    }

    return;
}

void UtilsConfig::saveInstallIcon(QString name)
{
    QString cacheFile = Utils::getSelfCachePath() + TOOLBOX_INSTALL_ICON_TO_DESKTOP_CACHE_FILE;
    QFile file(cacheFile);
    file.open(QIODevice::Append);
    file.write(name.toLocal8Bit());
    file.write("\n");
    file.close();

    return;
}

QStringList UtilsConfig::getInstallIcon(void)
{
    QString cacheFile = Utils::getSelfCachePath() + TOOLBOX_INSTALL_ICON_TO_DESKTOP_CACHE_FILE;

    QFile file(cacheFile);
    if (!file.exists()) {
        return QStringList();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "open installIconToDesktop.cache fail !";
        return QStringList();
    }

    QStringList res;
    while (!file.atEnd()) {
        res << file.readLine().trimmed();
    }

    file.close();
    return res;
}