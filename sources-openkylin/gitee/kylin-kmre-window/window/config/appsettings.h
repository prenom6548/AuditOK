/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
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
#include <QSize>
#include <QMap>
#include <QDomDocument>
#include <mutex>
#include <tuple>
#include "singleton.h"

class QJsonObject;
class QFileSystemWatcher;

class AppSettings: public QObject, public kmre::Singleton<AppSettings>
{
    Q_OBJECT
public:
    typedef struct {
        bool bootFullScreen;    // 全屏启动标志
        bool bootMaxSize;       // 上次退出时全屏标志
        int bootWidth;          // 上次退出时宽度
        int bootHeight;         // 上次退出时高度
        bool trayEnabled;       // 应用托盘使能标志
    }AppConfig;

    void updateSettings();
    bool isAppSupportShow(const QString &pkgName);
    bool isAppSupportDDS(const QString &pkgName);
    bool isMultiperEnabled();
    std::tuple<bool, QString> isAppWarned(const QString& pkgName);
    bool isAppSupportMultiper(const QString &pkgName);
    bool isAppMultiperEnabled(const QString &pkgName);
    bool isAppSupportKmreData(const QString &pkgName);
    void initAppMultipliersList(const QString &jsonStr);
    void updateAppMultipliersList(const QString &pkgName, int multiplier);
    QString getAppMultipliersList();
    void initAndroidConfigFile();
    void initAndroidDataPath();
    AppConfig getAppConfig(const QString& pkgName);
    bool setAppBootSize(const QString& pkgName, const QSize& size);
    bool setAppBootMaxSize(const QString& pkgName, bool maxsize);
    bool setAppBootFullScreen(const QString& pkgName, bool fullScreen);
    bool setAppTray(const QString& pkgName, bool enable);
    bool setAppConfig(const QString& pkgName, const AppConfig& appConfig);

private:
    explicit AppSettings();
    ~AppSettings(){}

    void checkMultiperSettings();
    int updateMultiperSettingsFromJson(const QByteArray &jsonStr);
    void checkDDSSettings();
    void checkKmreDataAppConfig();
    void getAppListFromXml(const QString &filePath, QStringList &appList, 
                            const QString rootTag = "packages", const QString childTag = "item");
    void reLoadAppConfigution();

    friend Singleton<AppSettings>;

private slots:
    void slotConfigFileChanged(const QString &path);
    
private:
    QStringList mUnsupportShowAppList;

    QString mMultiplierConfigFile;
    
    bool mIsHostSupportDDS = false;
    QString mSupportDDSAppListFile;
    QStringList mSupportDDSAppList;

    QString mWarningConfigFile;

    std::mutex mSupportMultiAppMapLk;
    QMap<QString, bool> mSupportMultiAppMap;

    QString mSupportKmreDataAppListFile;
    QStringList mSupportKmreDataAppList;

    QString mAppConfigurationFile;
    std::mutex mAppConfigurationDocLk;
    QDomDocument mAppConfigurationDoc;

    QFileSystemWatcher *mFileWatcher;
};