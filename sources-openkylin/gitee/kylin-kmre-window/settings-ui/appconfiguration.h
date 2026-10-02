/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <mutex>
#include <vector>

class QFileSystemWatcher;

class AppConfiguration : public QObject
{
    Q_OBJECT
public:
    typedef struct {
        bool bootFullScreen;    // 全屏启动标志
        bool trayEnabled;       // 应用托盘使能标志
    }AppConfig;

private:
    const QString mAppConfigurationFile;
    std::mutex mAppConfigurationDocLk;
    QDomDocument mAppConfigurationDoc;
    QFileSystemWatcher *mFileWatcher;

    AppConfiguration(/* args */);
    ~AppConfiguration();

    void getAppListFromXml(const QString &filePath, QStringList &appList, 
                            const QString rootTag = "packages", const QString childTag = "item");
    void reLoadAppConfigution();

private slots:
    void slotConfigFileChanged(const QString &path);

public:
    static AppConfiguration *getInstance();

    AppConfig getAppConfig(const QString& pkgName);
    QStringList getAppListByConfig(const QString& cfgName, bool value = true);
    bool setAppBootFullScreen(const QString& pkgName, bool fullScreen);
    bool setAppTray(const QString& pkgName, bool enable);
    bool setAppConfig(const QString& pkgName, const AppConfig& appConfig);
};
