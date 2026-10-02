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
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include "favorites-config.h"
#include "basic-app-model.h"
#include "favorite-folder-helper.h"

#define FOLDER_FILE_PATH ".config/ukui-menu/"
#define FOLDER_FILE_NAME "favorite.json"

namespace UkuiMenu {

QString FavoritesConfig::s_favoritesConfigFile = QDir::homePath() + "/" + FOLDER_FILE_PATH + FOLDER_FILE_NAME;
/**
 * 添加版本号，区分旧文件内容
 */
static const QString FAVORITE_CONFIG_VERSION = QStringLiteral("1.0.0");
FavoritesConfig &FavoritesConfig::instance()
{
    static FavoritesConfig favoritesConfig;
    return favoritesConfig;
}

FavoritesConfig::FavoritesConfig(QObject *parent)
{
    initConfig();
}

void FavoritesConfig::sync(const QStringList &list)
{
    if (m_configList == list) {
        return;
    }
    m_configList = list;
    QDir dir;
    QString folderConfigDir(QDir::homePath() + "/" + FOLDER_FILE_PATH);
    if (!dir.exists(folderConfigDir)) {
        if (!dir.mkdir(folderConfigDir)) {
            qWarning() << "FavoritesConfig::Unable to create configuration file.";
            return;
        }
    }

    QFile file(s_favoritesConfigFile);
    file.open(QFile::WriteOnly);

    QJsonDocument jsonDocument;
    QJsonObject fileObject;

    fileObject.insert("version", FAVORITE_CONFIG_VERSION);
    fileObject.insert("favorites", QJsonArray::fromStringList(m_configList));
    jsonDocument.setObject(fileObject);

    if (file.write(jsonDocument.toJson()) == -1) {
        qWarning() << "FavoritesConfig::Error saving configuration file.";
    }

    file.flush();
    file.close();
}

QStringList FavoritesConfig::getConfig()
{
    return m_configList;
}

void FavoritesConfig::initConfig()
{
    QFile file(s_favoritesConfigFile);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "FavoritesConfig: configuration files open failed.";
        return;
    }

    QJsonDocument jsonDocument(QJsonDocument::fromJson(file.readAll()));
    file.close();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qWarning() << "FavoritesConfig: configuration files format is empty.";
        return;
    }

    // 配置文件补充添加版本号1.0.0
    QStringList list, invalidUrl;
    if (jsonDocument.isObject()) {
        QJsonObject fileObject = jsonDocument.object();
        if (fileObject.value("version").toString() == FAVORITE_CONFIG_VERSION && fileObject.contains("favorites")) {
            for (const QVariant &variant : fileObject.value(QLatin1String("favorites")).toArray().toVariantList()) {
                list << variant.toString();
            }
            invalidUrl = updateInvalidUrlList(list);
            if (invalidUrl.isEmpty()) {
                m_configList = list;
            } else {
                for (const QString &id : invalidUrl) {
                    list.removeAll(id);
                }
                sync(list);
            }
        }
    } else if (jsonDocument.isArray()) {
        for (const QVariant &variant : jsonDocument.array().toVariantList()) {
            list << variant.toString();
        }
        invalidUrl = updateInvalidUrlList(list);
        if (!invalidUrl.isEmpty()) {
            for (const QString&id : invalidUrl) {
                list.removeAll(id);
            }
        }
        sync(list);

    } else {
        qWarning() << "FavoritesConfig: configuration files format is incorrect.";
    }
}

QStringList FavoritesConfig::updateInvalidUrlList(QStringList &list)
{
    QStringList invalidUrl;
    for (auto &url : list) {
        if (url.startsWith(APP_ID_SCHEME)) {
            QString appId = url.mid(6);
            if (FavoriteFolderHelper::instance()->containApp(appId) || appId.isEmpty() || (
                    BasicAppModel::instance()->indexOfApp(appId) < 0)) {
                invalidUrl.append(url);
            }
        } else if (url.startsWith(FOLDER_ID_SCHEME)) {
            bool ok;
            int folderId = url.mid(9).toInt(&ok);
            if (!ok || !FavoriteFolderHelper::instance()->containsFolder(folderId)) {
                invalidUrl.append(url);
            }
        }
    }

    return invalidUrl;
}
} // UkuiMenu
