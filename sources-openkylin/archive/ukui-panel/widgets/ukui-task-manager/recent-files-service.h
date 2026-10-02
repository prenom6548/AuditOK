/*
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: buxiaoqing <buxiaoqing@kylinos.cn>
 *
 */

#ifndef RECENTFILESSERVICE_H
#define RECENTFILESSERVICE_H

#include <QHash>
#include <QIcon>
#include <QMetaType>
#include <QObject>
#include <QScopedPointer>
#include <QSet>
#include <QUrl>

#include <bookmark.h>
#include <bookmarks-manager.h>

namespace TaskManager {

struct RecentFileEntry
{
    QString appId;
    QString desktopFile;
    UkuiFileMetadata::BookMark bookmark;
};

class RecentFilesService : public QObject
{
    Q_OBJECT
public:
    explicit RecentFilesService(QObject *parent = nullptr);
    explicit RecentFilesService(const QString &xbelPath, QObject *parent = nullptr);
    ~RecentFilesService() override;

    QList<RecentFileEntry> recentFilesForApp(const QString &appId, int limit = 10) const;
    QString recentFileDisplayName(const RecentFileEntry &entry) const;
    QIcon recentFileIcon(const RecentFileEntry &entry) const;
    bool openRecentFile(const RecentFileEntry &entry);
    bool pruneMissingLocalFilesForApp(const QString &appId);

Q_SIGNALS:
    void appRecentFilesChanged(const QString &appId);

private:
    void rebuildCache();
    QList<RecentFileEntry> entriesForBookmark(const UkuiFileMetadata::BookMark &bookmark) const;
    void insertRecentFile(QList<RecentFileEntry> &entries, const RecentFileEntry &entry, int limit) const;
    bool isMoreRecent(const RecentFileEntry &left, const RecentFileEntry &right) const;
    bool isBookmarkAvailable(const UkuiFileMetadata::BookMark &bookmark) const;
    QString normalizedAppIdFromDesktopFile(const QString &desktopFilePath) const;
    QString normalizedAppIdFromApplicationName(const QString &applicationName) const;
    bool hasRecentFilesChanged(const QList<RecentFileEntry> &oldEntries, const QList<RecentFileEntry> &newEntries) const;
    QUrl normalizedBookmarkUrl(const UkuiFileMetadata::BookMark &bookmark) const;
    QString desktopFilePathFromBookmarkApplication(const QString &applicationName) const;

private:
    QScopedPointer<UkuiFileMetadata::BookMarksManager> m_bookmarksManager;
    QHash<QString, QList<RecentFileEntry>> m_recentFilesByAppId;
};

}

Q_DECLARE_METATYPE(TaskManager::RecentFileEntry)

#endif // RECENTFILESSERVICE_H
