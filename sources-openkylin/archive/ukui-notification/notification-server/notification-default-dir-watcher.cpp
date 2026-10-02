/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#include "notification-default-dir-watcher.h"
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <ukui-search/application-info.h>
#include <utils.h>
#include "notification-settings/settings-manager.h"
static const QStringList DEFAULT_APP_LIST_PATHS =
    {"/etc/ukui/ukui-notification/default-app",
     "/opt/system/conf/ukui/ukui-notification/default-app"
    };


NotificationDefaultDirWatcher::NotificationDefaultDirWatcher(const std::unique_ptr<NotificationControlRegister>& notificationControlRegister, QObject *parent)
    : QObject{parent}
    , m_fileSystemWatcher(new UkuiSearch::FileSystemWatcher())
    , m_notificationControlRegister(notificationControlRegister)
{
    //监听默认目录
    updateWatcherPath();

    connect(m_fileSystemWatcher.get(), &UkuiSearch::FileSystemWatcher::created, this, &NotificationDefaultDirWatcher::onDefaultAppFileAdd);
    connect(m_fileSystemWatcher.get(), &UkuiSearch::FileSystemWatcher::moveTo, this, &NotificationDefaultDirWatcher::onDefaultAppFileAdd);
    connect(m_fileSystemWatcher.get(), &UkuiSearch::FileSystemWatcher::moved, this, &NotificationDefaultDirWatcher::onDefaultAppFileDeleted);
    connect(m_fileSystemWatcher.get(), &UkuiSearch::FileSystemWatcher::deleted, this, &NotificationDefaultDirWatcher::onDefaultAppFileDeleted);
    connect(m_fileSystemWatcher.get(), &UkuiSearch::FileSystemWatcher::modified, this, &NotificationDefaultDirWatcher::onDefaultAppFileModified);

    connect(&UkuiNotification::Utils::applicationInfo(), &UkuiSearch::ApplicationInfo::appDBItems2BAdd, [this] (const QStringList& desktopEntries){
        //应用安装
        const QStringList defaultAppList = getDefaultAppList();
        for ( const QString& desktopEntry : desktopEntries) {
            const QString desktop = UkuiNotification::Utils::desktopEntry2Name(desktopEntry);
            if (defaultAppList.contains(desktop)) {
                registerDefaultApp(desktop);
                qDebug() << "[NotificationDefaultDirWatcher] Registering default app:" << desktop;
            }
        }
    });

    connect(&UkuiNotification::Utils::applicationInfo(), &UkuiSearch::ApplicationInfo::appDBItems2BDelete, [this] (const QStringList& desktopEntries){
        //应用卸载
        for ( const QString& desktopEntry : desktopEntries) {
            const QString desktop = UkuiNotification::Utils::desktopEntry2Name(desktopEntry);
            unRegisterDefaultApp(desktop);
            qDebug() << "[NotificationDefaultDirWatcher] Unregistering default app:" << desktop;
        }
    });
}

void NotificationDefaultDirWatcher::onDefaultAppFileAdd(const QString &path, bool isDir)
{
    handleDefaultAppFileChanged(path, isDir, false);
}

void NotificationDefaultDirWatcher::onDefaultAppFileDeleted(const QString &path, bool isDir)
{
    handleDefaultAppFileChanged(path, isDir, true);
}

void NotificationDefaultDirWatcher::onDefaultAppFileModified(const QString &path)
{
    handleDefaultAppFileChanged(path, false, true);
}

/**
 * @brief 如果监听的目录不存在，获取上一级目录
 * @param path
 * @return
 */
QString NotificationDefaultDirWatcher::findFirstExistingAncestorDir(const QString &path)
{
    QDir dir(path);
    while (true) {
        const QString absolutePath = dir.absolutePath();
        if (dir.exists()) {
            return absolutePath;
        }

        if (absolutePath == "/") {
            return absolutePath;
        }

        if (!dir.cdUp()) {
            qWarning() << "NotificationDefaultDirWatcher::findFirstExistingAncestorDir: Failed to cdUp from path " << absolutePath;
            return absolutePath;
        }
    }
}

/**
 * @brief DEFAULT_APP_LIST_PATHS
 * 默认目录创建时， 更新监听目录为默认目录
 * 路径删除时，获取默认目录的上一级目录监听
 */

void NotificationDefaultDirWatcher::updateWatcherPath()
{
    if (!m_watcherPaths.isEmpty()) {
        for (const QString& path : m_watcherPaths) {
            m_fileSystemWatcher->removeWatch(path);
        }
    }
    for (const QString& defaultPath : DEFAULT_APP_LIST_PATHS) {
        m_watcherPaths << findFirstExistingAncestorDir(defaultPath);
    }
    m_fileSystemWatcher->addWatch(m_watcherPaths);
    qDebug() << "Notification watch path is updated:" << m_watcherPaths;
    //刷新默认列表
    refreshDefaultAppListMap();
}

/**
 * @brief 获取默认的列表
 * @return
 */
QStringList NotificationDefaultDirWatcher::getDefaultAppList()
{
    QStringList defaultAppList;
    for (const QString& defaultPath : DEFAULT_APP_LIST_PATHS)
    {
        QDir dir(defaultPath);
        if (!dir.exists()) {
            continue;
        }
        for (const QString &fileName : dir.entryList(QDir::Files)) {
            const QStringList desktopEntries = getDefaultAppDesktopEntries(dir.absoluteFilePath(fileName));
            for (const QString& appDesktop : desktopEntries) {
                if (!defaultAppList.contains(appDesktop)) {
                    defaultAppList << appDesktop;
                }
            }
        }
    }
    return defaultAppList;
}

/**
 * @brief 读取文件的"DesktopEntries"内容
 * @param path
 * @return
 */
QStringList NotificationDefaultDirWatcher::getDefaultAppDesktopEntries(const QString& path)
{
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "NotificationDefaultDirWatcher::getDefaultAppDesktopEntries" << file.fileName() << "is not exists";
        return QStringList();
    }
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "NotificationDefaultDirWatcher::getDefaultAppDesktopEntries" << file.fileName() << "open failed";
        return QStringList();
    }
    QByteArray byteArray = file.readAll();
    file.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "NotificationDefaultDirWatcher::getDefaultAppDesktopEntries Incorrect configuration files " << file.fileName() << "JSON parse error";
        return QStringList();
    }
    QJsonObject jsonObject = jsonDocument.object();
    if (!jsonObject.contains("DesktopEntries")) {
        return QStringList();
    }
    return jsonObject.value("DesktopEntries").toString().split(",");
}

/**
 * @brief 加载DEFAULT_APP_LIST_PATHS 下的默认配置
 */
void NotificationDefaultDirWatcher::refreshDefaultAppListMap()
{
    const QStringList newDefaultApplist = getDefaultAppList();
    QStringList oldDefaultApplist;
    QStringList otherApplist;
    UkuiNotification::SettingsManager::self()->getAppList(oldDefaultApplist, otherApplist);

    //新增
    QStringList newAddList =  StringListDiff(newDefaultApplist, oldDefaultApplist);
    //删除
    QStringList newRemoveList =  StringListDiff(oldDefaultApplist, newDefaultApplist);

    for (const QString& desktop : newAddList) {
        if (otherApplist.contains(desktop)) {
            //新增默认应用，已经存在配置，需要更新配置来源
            updateDefaultApp(desktop);
        } else {
            registerDefaultApp(desktop);
        }
    }

    for (const QString& desktop : newRemoveList) {
        unRegisterDefaultApp(desktop);
    }
}

/**
 * @brief 默认应用的配置文件改变
 * @param path
 * @param isDir
 * @param isDelete
 */
void NotificationDefaultDirWatcher::handleDefaultAppFileChanged(const QString &path, bool isDir, bool isDelete)
{
    if (isDir) {
        //改变的目录符合预期时，进行更新
        if (DEFAULT_APP_LIST_PATHS.contains(path)) {
            updateWatcherPath();
        }
    } else {
        //读取默认配置内容，更改配置
        for (const QString& defaultAppPath : DEFAULT_APP_LIST_PATHS) {
            if (path.startsWith(defaultAppPath) && path.endsWith(QStringLiteral(".json"))) {
                refreshDefaultAppListMap();
            }
        }
    }
}

void NotificationDefaultDirWatcher::registerDefaultApp(const QString &desktop)
{
    m_notificationControlRegister->registerNotificationControl(desktop, NotificationControlRegister::RegisterMode::Default);
}

void NotificationDefaultDirWatcher::unRegisterDefaultApp(const QString &desktop)
{
    m_notificationControlRegister->unRegisterNotificationControl(desktop);
}

void NotificationDefaultDirWatcher::updateDefaultApp(const QString &desktop)
{
    m_notificationControlRegister->updateNotificationControl(desktop, NotificationControlRegister::RegisterMode::Default);
}
