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

#include "recent-files-service.h"

#include <XdgDesktopFile>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6xdg/XdgDesktopFile>
#else
#include <qt5xdg/XdgDesktopFile>
#endif

#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QUrl>

#include "utils.h"

#ifdef signals
#undef signals
#endif
#define signals qt_signals
#include <gio/gio.h>

using namespace TaskManager;

namespace {
constexpr int kMaxRecentFiles = 10;
const QString kKylinAppManagerName(QStringLiteral("com.kylin.ProcessManager"));
const QString kKylinAppManagerPath(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"));
const QString kKylinAppManagerInterface(QStringLiteral("com.kylin.ProcessManager.AppLauncher"));
}

RecentFilesService::RecentFilesService(QObject *parent)
    : RecentFilesService(QDir::home().filePath(QStringLiteral(".local/share/recently-used.xbel")), parent)
{
}

RecentFilesService::RecentFilesService(const QString &xbelPath, QObject *parent)
    : QObject(parent)
{
    m_bookmarksManager.reset(new UkuiFileMetadata::BookMarksManager(xbelPath));

    connect(m_bookmarksManager.data(), &UkuiFileMetadata::BookMarksManager::changed,
            this, &RecentFilesService::rebuildCache);
    connect(m_bookmarksManager.data(), &UkuiFileMetadata::BookMarksManager::errorOccurred, this,
            [](const QString &message) {
                qWarning() << "Failed to parse recent files from recently-used.xbel" << message;
            });

    rebuildCache();
}

RecentFilesService::~RecentFilesService() = default;

QList<RecentFileEntry> RecentFilesService::recentFilesForApp(const QString &appId, int limit) const
{
    if (appId.isEmpty() || limit <= 0) {
        return {};
    }

    QList<RecentFileEntry> entries = m_recentFilesByAppId.value(appId);
    if (entries.size() > limit) {
        entries = entries.mid(0, limit);
    }
    return entries;
}

QString RecentFilesService::recentFileDisplayName(const RecentFileEntry &entry) const
{
    if (!entry.bookmark.title().isEmpty()) {
        return entry.bookmark.title();
    }

    const QUrl url = normalizedBookmarkUrl(entry.bookmark);
    if (url.isLocalFile()) {
        return QFileInfo(url.toLocalFile()).fileName();
    }

    return url.fileName();
}

QIcon RecentFilesService::recentFileIcon(const RecentFileEntry &entry) const
{
    if (!entry.bookmark.iconName().isEmpty()) {
        const QIcon themeIcon = QIcon::fromTheme(entry.bookmark.iconName());
        if (!themeIcon.isNull()) {
            return themeIcon;
        }
    }

    if (!entry.bookmark.iconHref().isEmpty()) {
        QUrl iconUrl(entry.bookmark.iconHref());
        if ((!iconUrl.isValid() || iconUrl.scheme().isEmpty()) && !entry.bookmark.iconHref().isEmpty()) {
            iconUrl = QUrl::fromLocalFile(entry.bookmark.iconHref());
        }
        if (iconUrl.isLocalFile()) {
            const QIcon fileIcon(iconUrl.toLocalFile());
            if (!fileIcon.isNull()) {
                return fileIcon;
            }
        }
    }

    const QUrl fileUrl = normalizedBookmarkUrl(entry.bookmark);
    g_autoptr(GFile) file = g_file_new_for_uri(fileUrl.toString().toUtf8().constData());
    if (file) {
        g_autoptr(GFileInfo) info = g_file_query_info(file,
                                                      G_FILE_ATTRIBUTE_STANDARD_ICON,
                                                      G_FILE_QUERY_INFO_NONE,
                                                      nullptr,
                                                      nullptr);

        if (info) {
            GIcon *gIcon = g_file_info_get_icon(info);
            if (G_IS_THEMED_ICON(gIcon)) {
                const gchar *const *iconNames = g_themed_icon_get_names(G_THEMED_ICON(gIcon));
                if (iconNames) {
                    auto p = iconNames;
                    while (*p) {
                        const QIcon icon = QIcon::fromTheme(QString::fromUtf8(*p));
                        if (!icon.isNull()) {
                            return icon;
                        }
                        ++p;
                    }
                }
            }
        }
    }

    return QIcon::fromTheme(QStringLiteral("application-x-desktop"));
}

bool RecentFilesService::openRecentFile(const RecentFileEntry &entry)
{
    const QUrl url = normalizedBookmarkUrl(entry.bookmark);
    if (!url.isValid() || url.isEmpty()) {
        qWarning() << "Invalid recent file url:" << entry.bookmark.href();
        return false;
    }

    if (url.isLocalFile() && !QFileInfo::exists(url.toLocalFile())) {
        qWarning() << "Recent file not exists:" << url.toLocalFile();
        return false;
    }

    QString targetDesktopFile = entry.desktopFile;
    if (targetDesktopFile.isEmpty() && !entry.appId.isEmpty()) {
        targetDesktopFile = Utils::desktopFilePathFromName(entry.appId);
    }

    const QString argument = url.toString();
    if (!targetDesktopFile.isEmpty()) {
        QDBusMessage message = QDBusMessage::createMethodCall(kKylinAppManagerName,
                                                              kKylinAppManagerPath,
                                                              kKylinAppManagerInterface,
                                                              QStringLiteral("LaunchAppWithArguments"));
        message << targetDesktopFile << QStringList{argument};
        auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [url, argument, targetDesktopFile](QDBusPendingCallWatcher *self) {
                    if (!self->isError()) {
                        self->deleteLater();
                        return;
                    }

                    qWarning() << "Fail to call" << kKylinAppManagerInterface << self->error();

                    XdgDesktopFile desktopFile;
                    if (!desktopFile.load(targetDesktopFile)
                        || !desktopFile.startDetached(QStringList{argument})) {
                        QDesktopServices::openUrl(url);
                    }
                    self->deleteLater();
                });
        return true;
    }

    return QDesktopServices::openUrl(url);
}

bool RecentFilesService::pruneMissingLocalFilesForApp(const QString &appId)
{
    if (appId.isEmpty()) {
        return false;
    }

    auto it = m_recentFilesByAppId.find(appId);
    if (it == m_recentFilesByAppId.end()) {
        return false;
    }

    const QList<RecentFileEntry> &entries = *it;
    QList<RecentFileEntry> filteredEntries;
    filteredEntries.reserve(entries.size());
    for (const auto &entry : entries) {
        const QUrl fileUrl = normalizedBookmarkUrl(entry.bookmark);
        if (fileUrl.isLocalFile() && !QFileInfo::exists(fileUrl.toLocalFile())) {
            continue;
        }
        filteredEntries.append(entry);
    }

    if (!hasRecentFilesChanged(entries, filteredEntries)) {
        return false;
    }

    // Keep this prune silent: menuActions() synchronously rebuilds actions right after
    // calling here, and emitting appRecentFilesChanged would re-enter that refresh path.
    *it = filteredEntries;
    return true;
}

void RecentFilesService::rebuildCache()
{
    QHash<QString, QList<RecentFileEntry>> newRecentFilesByAppId;
    if (!m_bookmarksManager || !m_bookmarksManager->isValid()) {
        const auto oldAppIds = m_recentFilesByAppId.keys();
        m_recentFilesByAppId.clear();
        for (const auto &appId : oldAppIds) {
            Q_EMIT appRecentFilesChanged(appId);
        }
        return;
    }

    const UkuiFileMetadata::BookMarksGroup rootGroup = m_bookmarksManager->rootGroup();
    const QList<UkuiFileMetadata::BookMark> allBookmarks = rootGroup.bookmarks();

    for (const auto &bookmark : allBookmarks) {
        if (!isBookmarkAvailable(bookmark)) {
            continue;
        }

        const QList<RecentFileEntry> entries = entriesForBookmark(bookmark);
        for (const auto &entry : entries) {
            insertRecentFile(newRecentFilesByAppId[entry.appId], entry, kMaxRecentFiles);
        }
    }

    QSet<QString> changedAppIds;
    for (auto it = m_recentFilesByAppId.constBegin(); it != m_recentFilesByAppId.constEnd(); ++it) {
        changedAppIds.insert(it.key());
    }
    for (auto it = newRecentFilesByAppId.constBegin(); it != newRecentFilesByAppId.constEnd(); ++it) {
        changedAppIds.insert(it.key());
    }

    const auto oldRecentFilesByAppId = m_recentFilesByAppId;
    m_recentFilesByAppId = newRecentFilesByAppId;

    for (const auto &appId : changedAppIds) {
        if (hasRecentFilesChanged(oldRecentFilesByAppId.value(appId), m_recentFilesByAppId.value(appId))) {
            Q_EMIT appRecentFilesChanged(appId);
        }
    }
}

QList<RecentFileEntry> RecentFilesService::entriesForBookmark(const UkuiFileMetadata::BookMark &bookmark) const
{
    QList<RecentFileEntry> entries;
    if (!m_bookmarksManager) {
        return entries;
    }

    QSet<QString> appIds;
    const auto applications = m_bookmarksManager->freedesktopApplications(bookmark.href());
    for (const auto &application : applications) {
        const QString appId = normalizedAppIdFromApplicationName(application.name());
        if (appId.isEmpty() || appIds.contains(appId)) {
            continue;
        }

        appIds.insert(appId);
        entries.append({appId, desktopFilePathFromBookmarkApplication(application.name()), bookmark});
    }

    return entries;
}

void RecentFilesService::insertRecentFile(QList<RecentFileEntry> &entries, const RecentFileEntry &entry, int limit) const
{
    for (int i = 0; i < entries.size(); ++i) {
        if (entries.at(i).bookmark.href() != entry.bookmark.href()) {
            continue;
        }

        if (entries.at(i).desktopFile.isEmpty() && !entry.desktopFile.isEmpty()) {
            entries.removeAt(i);
            break;
        }
        return;
    }

    int insertIndex = 0;
    for (; insertIndex < entries.size(); ++insertIndex) {
        if (!isMoreRecent(entries.at(insertIndex), entry)) {
            break;
        }
    }

    entries.insert(insertIndex, entry);
    if (entries.size() > limit) {
        entries.removeLast();
    }
}

bool RecentFilesService::isMoreRecent(const RecentFileEntry &left, const RecentFileEntry &right) const
{
    const QDateTime leftModifiedTime = QDateTime::fromString(left.bookmark.modified(), Qt::ISODate);
    const QDateTime rightModifiedTime = QDateTime::fromString(right.bookmark.modified(), Qt::ISODate);
    const bool leftTimeIsValid = leftModifiedTime.isValid();
    const bool rightTimeIsValid = rightModifiedTime.isValid();

    if (leftTimeIsValid && rightTimeIsValid && leftModifiedTime != rightModifiedTime) {
        return leftModifiedTime > rightModifiedTime;
    }
    if (leftTimeIsValid != rightTimeIsValid) {
        return leftTimeIsValid;
    }

    return left.bookmark.href() > right.bookmark.href();
}

bool RecentFilesService::isBookmarkAvailable(const UkuiFileMetadata::BookMark &bookmark) const
{
    const QUrl fileUrl = normalizedBookmarkUrl(bookmark);
    if (fileUrl.isLocalFile() && !QFileInfo::exists(fileUrl.toLocalFile())) {
        qWarning() << "Recent file not exists:" << fileUrl.toLocalFile();
        return false;
    }

    return true;
}

QString RecentFilesService::normalizedAppIdFromDesktopFile(const QString &desktopFilePath) const
{
    const QFileInfo info(desktopFilePath);
    const QString fileName = info.fileName();
    if (!fileName.endsWith(QStringLiteral(".desktop"))) {
        return {};
    }
    return fileName.left(fileName.size() - 8);
}

QString RecentFilesService::normalizedAppIdFromApplicationName(const QString &applicationName) const
{
    if (applicationName.isEmpty()) {
        return {};
    }

    const QString desktopBackedAppId = normalizedAppIdFromDesktopFile(applicationName);
    if (!desktopBackedAppId.isEmpty()) {
        return desktopBackedAppId;
    }

    const QString candidate = QFileInfo(applicationName).fileName();
    if (candidate.isEmpty()
        || candidate.contains(QLatin1Char('/'))
        || candidate.contains(QLatin1Char(' '))
        || candidate.toLower() != candidate) {
        return {};
    }

    return candidate;
}

bool RecentFilesService::hasRecentFilesChanged(const QList<RecentFileEntry> &oldEntries, const QList<RecentFileEntry> &newEntries) const
{
    if (oldEntries.size() != newEntries.size()) {
        return true;
    }

    for (int i = 0; i < oldEntries.size(); ++i) {
        const auto &oldEntry = oldEntries.at(i);
        const auto &newEntry = newEntries.at(i);
        if (oldEntry.appId != newEntry.appId
            || oldEntry.desktopFile != newEntry.desktopFile
            || oldEntry.bookmark.href() != newEntry.bookmark.href()
            || oldEntry.bookmark.title() != newEntry.bookmark.title()
            || oldEntry.bookmark.modified() != newEntry.bookmark.modified()) {
            return true;
        }
    }

    return false;
}

QUrl RecentFilesService::normalizedBookmarkUrl(const UkuiFileMetadata::BookMark &bookmark) const
{
    QUrl fileUrl(bookmark.href());
    if ((!fileUrl.isValid() || fileUrl.scheme().isEmpty()) && !bookmark.href().isEmpty()) {
        fileUrl = QUrl::fromLocalFile(bookmark.href());
    }
    return fileUrl;
}

QString RecentFilesService::desktopFilePathFromBookmarkApplication(const QString &applicationName) const
{
    if (applicationName.isEmpty()) {
        return {};
    }

    const QFileInfo fileInfo(applicationName);
    if (fileInfo.isAbsolute() && fileInfo.exists() && fileInfo.fileName().endsWith(QStringLiteral(".desktop"))) {
        return fileInfo.absoluteFilePath();
    }

    return {};
}
