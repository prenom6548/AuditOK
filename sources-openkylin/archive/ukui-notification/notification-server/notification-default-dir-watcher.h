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

#ifndef NOTIFICATIONDEFAULTDIRWATCHER_H
#define NOTIFICATIONDEFAULTDIRWATCHER_H

#include <QObject>
#include <memory>
#include <QMap>
#include "file-system-watcher.h"
#include "notification-control-register.h"
class NotificationDefaultDirWatcher : public QObject
{
    Q_OBJECT
public:
    explicit NotificationDefaultDirWatcher(const std::unique_ptr<NotificationControlRegister>&, QObject *parent = nullptr);

private Q_SLOTS:
    void onDefaultAppFileAdd(const QString& path, bool isDir);
    void onDefaultAppFileDeleted(const QString& path, bool isDir);
    void onDefaultAppFileModified(const QString& path);

private:
    QString findFirstExistingAncestorDir(const QString& path);
    void updateWatcherPath();
    QStringList getDefaultAppList();
    QStringList getDefaultAppDesktopEntries(const QString& path);
    void refreshDefaultAppListMap();
    void handleDefaultAppFileChanged(const QString& path, bool isDir, bool);
    void registerDefaultApp(const QString& desktop);
    void unRegisterDefaultApp(const QString& desktop);
    void updateDefaultApp(const QString &desktop);

    QStringList m_watcherPaths;
    std::unique_ptr<UkuiSearch::FileSystemWatcher> m_fileSystemWatcher;
    const std::unique_ptr<NotificationControlRegister>& m_notificationControlRegister;
};

#endif // NOTIFICATIONDEFAULTDIRWATCHER_H
