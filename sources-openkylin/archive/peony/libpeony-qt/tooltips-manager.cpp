/*
 * Peony-Qt
 *
 * Copyright (C) 2024, Tianjin KYLIN Information Technology Co., Ltd.
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
 * @author: Renyg <renyangguang@kylinos.cn>
 * @date:   2024-09-23 14:55
 */

#include "tooltips-manager.h"
#include "file-info.h"
#include "file-utils.h"
#include "global-settings.h"

#include <QUrl>
#include <QSet>
#include <QImageReader>
#include <QVariant>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QApplication>
#include <glib.h>

namespace Peony {

// HTML templates for tooltips
// const QString TooltipsManager::HtmlTemplates::BASE_STYLE = R"(
// <style>
//     table { width: 200px; border-collapse: collapse; }
//     td { word-break: break-all; max-width: 0;}
// </style>
// )";

// const QString TooltipsManager::HtmlTemplates::BASE_STYLE_TWO_ROW = R"(
// <style>
//     table { width: 200px; border-collapse: collapse; }
//     td { word-wrap: break-word; max-width: 0; white-space: nowrap;}
// </style>
// )";

const QString TooltipsManager::HtmlTemplates::ONE_ROW_TEMPLATE = R"(
<html>
<head>%1</head>
<body>
    <table>
        <tr><td>%2</td></tr>
    </table>
</body>
</html>
)";

const QString TooltipsManager::HtmlTemplates::FOUR_ROW_TEMPLATE = R"(
<html>
<head>%1</head>
<body>
    <table>
        <tr><td>%2</td></tr>
        <tr><td>%3</td></tr>
        <tr><td>%4</td></tr>
        <tr><td>%5</td></tr>
    </table>
</body>
</html>
)";

const QString TooltipsManager::HtmlTemplates::THREE_ROW_TEMPLATE = R"(
<html>
<head>%1</head>
<body>
    <table>
        <tr><td>%2</td></tr>
        <tr><td>%3</td></tr>
        <tr><td>%4</td></tr>
    </table>
</body>
</html>
)";

const QString TooltipsManager::HtmlTemplates::TWO_ROW_TEMPLATE = R"(
<html>
<head>%1</head>
<body>
    <table>
        <tr><td>%2</td></tr>
        <tr><td>%3</td></tr>
    </table>
</body>
</html>
)";

const QString TooltipsManager::HtmlTemplates::FIVE_ROW_TEMPLATE = R"(
<html>
<head>%1</head>
<body>
    <table>
        <tr><td>%2</td></tr>
        <tr><td>%3</td></tr>
        <tr><td>%4</td></tr>
        <tr><td>%5</td></tr>
        <tr><td>%6</td></tr>
    </table>
</body>
</html>
)";

TooltipsManager::TooltipsManager()
    : m_fontMetrics(QApplication::font())
{
}

TooltipsManager& TooltipsManager::instance()
{
    static TooltipsManager instance;
    return instance;
}

QString TooltipsManager::generateTooltip(FileInfo* info)
{
    if (!info) {
        return QString();
    }

    if (isNetworkFile(info)) {
        return toOneRowTooltip(info->displayName());
    }

    if (isSystemFile(info)) {
        return toOneRowTooltip(info->displayName());
    }

    if (isTrashFile(info)) {
        return toTrashFile(info);
    }

    if (info->uri().endsWith(".desktop")) {
        return toDesktopFile(info);
    }

    if (info->fileType() == tr("Unknow")) {
        return toNormalFile(info);
    }

    if (info->isImageFile()) {
        return toImage(info);
    } else if (info->isVideoFile() || info->isAudioFile()) {
        return toVideoAudio(info);
    } else {
        return toNormalFile(info);
    }
}

QString TooltipsManager::getMediaDuration(const QString& filePath, int timeoutMs)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        return "N/A";
    }

    QProcess process;
    QStringList arguments;
    arguments << "-v" << "error"
              << "-show_entries" << "format=duration"
              << "-of" << "default=noprint_wrappers=1:nokey=1"
              << "-sexagesimal"
              << filePath;

    process.start("ffprobe", arguments);

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        qWarning() << "FFprobe process timed out for file:" << filePath;
        return "N/A";
    }

    if (process.exitCode() != 0) {
        qWarning() << "FFprobe process failed:" << process.errorString();
        return "N/A";
    }

    QString output = process.readAllStandardOutput().trimmed();
    QStringList parts = output.split(".");

    if (parts.isEmpty()) {
        return "N/A";
    }

    // Get the main time part (excluding milliseconds)
    QString mainPart = parts[0];

    // Ensure format is HH:MM:SS
    QStringList timeParts = mainPart.split(":");
    while (timeParts.size() < 3) {
        timeParts.prepend("00");
    }

    // Combine time string, ensuring each part is two digits
    return QString("%1:%2:%3")
        .arg(timeParts[0].rightJustified(2, '0'))
        .arg(timeParts[1].rightJustified(2, '0'))
        .arg(timeParts[2].rightJustified(2, '0'));
}

bool TooltipsManager::isSystemFile(FileInfo* info)
{
    static const QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    static const QSet<QString> systemUris = {
        "trash:///", "computer:///", "network:///", "recent:///", homeUri
    };

    bool isSystemFile = systemUris.contains(info->uri());
    if (!isSystemFile && !info->targetUri().isEmpty()) {
        isSystemFile = systemUris.contains(info->targetUri());
    }
    return isSystemFile;
}

QString TooltipsManager::toVideoAudio(FileInfo* info)
{
    QString typeLbl = tr("Type: ");
    QString fileType = info->fileType();

    QString duration;
    QString durationLbl = tr("Duration：");

    QString sizeLbl = tr("Size: ");
    bool isElided = info->property("isElided").toBool();

    QString uri = info->uri();
    qint64 modTime = getFileModifiedTime(info);

    if (m_metadataCache.contains(uri) && m_metadataCache.value(uri).modTime == modTime) {
        duration = m_metadataCache.value(uri).duration;
    } else {
        duration = tr("Loading...");
    }

    if (isElided) {
        QString displayName = getDisplayName(info);/*.toHtmlEscaped();*/
        return displayName + "\n" + typeLbl + fileType + "\n" + durationLbl + duration + "\n" + sizeLbl + info->fileSize();
        // return QString(HtmlTemplates::FOUR_ROW_TEMPLATE).arg(
        //     HtmlTemplates::BASE_STYLE,
        //     displayName,
        //     typeLbl + fileType,
        //     durationLbl + duration,
        //     sizeLbl + info->fileSize()
        // );
    } else {
        return typeLbl + fileType + "\n" + durationLbl + duration + "\n" + sizeLbl + info->fileSize();
        // return QString(HtmlTemplates::THREE_ROW_TEMPLATE).arg(
        //     HtmlTemplates::BASE_STYLE,
        //     typeLbl + fileType,
        //     durationLbl + duration,
        //     sizeLbl + info->fileSize()
        // );
    }
}

QString TooltipsManager::toImage(FileInfo* info)
{
    QString typeLbl = tr("Type: ");
    QString fileType = info->fileType();

    QString resolution;
    QString resolutionLbl = tr("Res: ");

    QString sizeLbl = tr("Size: ");
    bool isElided = info->property("isElided").toBool();

    QString uri = info->uri();
    qint64 modTime = getFileModifiedTime(info);

    if (m_metadataCache.contains(uri) && m_metadataCache.value(uri).modTime == modTime) {
        resolution = m_metadataCache.value(uri).resolution;
    } else {
        resolution = tr("Loading...");
    }

    if (isElided) {
        QString displayName = getDisplayName(info)/*.toHtmlEscaped()*/;
        return displayName + "\n" + typeLbl + fileType + "\n" + resolutionLbl + resolution + "\n" + sizeLbl + info->fileSize();
        // return QString(HtmlTemplates::FOUR_ROW_TEMPLATE).arg(
        //     HtmlTemplates::BASE_STYLE,
        //     displayName,
        //     typeLbl + fileType,
        //     resolutionLbl + resolution,
        //     sizeLbl + info->fileSize()
        // );
    } else {
        return typeLbl + fileType + "\n" + resolutionLbl + resolution + "\n" + sizeLbl + info->fileSize();
        // return QString(HtmlTemplates::THREE_ROW_TEMPLATE).arg(
        //     HtmlTemplates::BASE_STYLE,
        //     typeLbl + fileType,
        //     resolutionLbl + resolution,
        //     sizeLbl + info->fileSize()
        // );
    }
}

QString TooltipsManager::toNormalFile(FileInfo* info)
{
    QString typeLbl = tr("Type: ");
    QString typeUnKnow = tr("Unknow");
    QString fileType = info->fileType().isEmpty() ? typeUnKnow : info->fileType();

    QString modDateLbl = tr("Mod Date: ");
    QString sizeLbl = tr("Size: ");
    bool isElided = info->property("isElided").toBool();
    QString fileSize = getFileOrDirectorySize(info);

    if (isElided) {
        QString displayName = getDisplayName(info);/*.toHtmlEscaped();*/
        if (fileSize.isEmpty()) {
            return displayName + "\n" + typeLbl + fileType + "\n" + modDateLbl + info->modifiedDate();
            // return QString(HtmlTemplates::THREE_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE,
            //     displayName,
            //     typeLbl + fileType,
            //     modDateLbl + info->modifiedDate()
            // );
        } else {
            return displayName+"\n"+typeLbl + fileType+"\n"+modDateLbl + info->modifiedDate()+"\n"+sizeLbl + fileSize; /*QString(HtmlTemplates::FOUR_ROW_TEMPLATE).arg(
                HtmlTemplates::BASE_STYLE,
                displayName,
                typeLbl + fileType,
                modDateLbl + info->modifiedDate(),
                sizeLbl + fileSize
            );*/
        }
    } else {
        if (fileSize.isEmpty()) {
            return typeLbl + fileType + "\n" +  modDateLbl + info->modifiedDate();
            // return QString(HtmlTemplates::TWO_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE_TWO_ROW,
            //     typeLbl + fileType,
            //     modDateLbl + info->modifiedDate()
            // );
        } else {
            return typeLbl + fileType + "\n" +  modDateLbl + info->modifiedDate() + "\n" + sizeLbl + fileSize;
            // return QString(HtmlTemplates::THREE_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE,
            //     typeLbl + fileType,
            //     modDateLbl + info->modifiedDate(),
            //     sizeLbl + fileSize
            // );
        }
    }
}

QString TooltipsManager::getDisplayName(FileInfo* info) const
{
    QString displayName = info->displayName();

    auto settings = GlobalSettings::getInstance();
    bool showFileExtension = settings->isExist(SHOW_FILE_EXTENSION) ?
                                 settings->getValue(SHOW_FILE_EXTENSION).toBool() :
                                 true;

    if (showFileExtension || info->isDir() || !displayName.contains(".")) {
        return displayName;
    }

    return FileUtils::getBaseNameOfFile(displayName);
}

bool TooltipsManager::isTimeConsumingFile(FileInfo* info) const
{
    return info->isImageFile() || info->isVideoFile() || info->isAudioFile();
}

qint64 TooltipsManager::getFileModifiedTime(FileInfo* info) const
{
    if (!info) return 0;

    QFileInfo fileInfo(QUrl(info->uri()).toLocalFile());
    if (fileInfo.exists()) {
        return fileInfo.lastModified().toMSecsSinceEpoch();
    }
    return 0;
}

void TooltipsManager::cleanupMetadataCache()
{
    if (m_metadataCache.size() <= MAX_CACHE_SIZE) {
        return;
    }

    int toRemove = m_metadataCache.size() - MAX_CACHE_SIZE/2;
    auto keys = m_metadataCache.keys();
    for (int i = 0; i < toRemove && !keys.isEmpty(); i++) {
        m_metadataCache.remove(keys.takeFirst());
    }
}

bool TooltipsManager::hasValidMetadataCache(const QString& uri) const
{
    if (!m_metadataCache.contains(uri)) {
        return false;
    }

    auto info = FileInfo::fromUri(uri);
    if (!info) {
        return false;
    }

    qint64 modTime = getFileModifiedTime(info.get());
    return m_metadataCache.value(uri).modTime == modTime;
}

void TooltipsManager::loadMetadataAsync(const QString& uri)
{
    if (m_pendingTooltips.contains(uri)) {
        return;
    }

    auto info = FileInfo::fromUri(uri);
    if (!info || !isTimeConsumingFile(info.get())) {
        return;
    }

    QFutureWatcher<MetadataCache>* watcher = new QFutureWatcher<MetadataCache>(this);
    m_pendingTooltips.insert(uri);

    qint64 modTime = getFileModifiedTime(info.get());

    connect(watcher, &QFutureWatcher<MetadataCache>::finished, this, [this, watcher, uri, info]() {
        MetadataCache metadata = watcher->result();
        m_metadataCache.insert(uri, metadata);
        if (m_metadataCache.size() > MAX_CACHE_SIZE) {
            cleanupMetadataCache();
        }

        Q_EMIT tooltipReady(uri, generateTooltip(info.get()));
        m_pendingTooltips.remove(uri);
        watcher->deleteLater();
    });

    QFuture<MetadataCache> future;
    if (info->isImageFile()) {
        future = QtConcurrent::run([info, modTime]() -> MetadataCache {
            QUrl url = info->uri();
            QImageReader r;
            if (info->uri().startsWith("idm://")) {
                QString targetUri = info->targetUri();
                r.setFileName(targetUri.remove("file://"));
            } else {
                r.setFileName(url.path());
            }
            MetadataCache metadata;
            metadata.modTime = modTime;
            metadata.resolution = QString("%1x%2").arg(r.size().width()).arg(r.size().height());
            return metadata;
        });
    } else if (info->isVideoFile() || info->isAudioFile()) {
        future = QtConcurrent::run([info, modTime]() -> MetadataCache {
            MetadataCache metadata;
            metadata.modTime = modTime;
            metadata.duration = getMediaDuration(info->filePath());
            return metadata;
        });
    }

    watcher->setFuture(future);
}

QString TooltipsManager::toTrashFile(FileInfo* info)
{
    QString deletedDateLbl = tr("Deleted date: ");
    QString deletedDate = info->deletionDate();
    if (deletedDate.isEmpty()) {
        deletedDate = info->modifiedDate();
    }

    QString typeLbl = tr("Type: ");
    QString typeUnKnow = tr("Unknow");
    QString fileType = info->fileType().isEmpty() ? typeUnKnow : info->fileType();

    QString sizeLbl = tr("Size: ");
    QString fileSize = getFileOrDirectorySize(info);

    QString originalPathLbl = tr("Original path: ");
    QString originalPath = getOriginalPath(info);
    int availableWidth = FIXED_ROW_WIDTH - m_fontMetrics.horizontalAdvance(originalPathLbl);
    QString elidedOriginalPath = m_fontMetrics.elidedText(originalPath, Qt::ElideMiddle, availableWidth);

    bool isElided = info->property("isElided").toBool();

    if (isElided) {
        QString displayName = getDisplayName(info)/*.toHtmlEscaped()*/;

        if (fileSize.isEmpty()) {
            return displayName + "\n" + deletedDateLbl + deletedDate + "\n" + typeLbl + fileType + "\n" +  originalPathLbl + elidedOriginalPath;
            // return QString(HtmlTemplates::FOUR_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE,
            //     displayName,
            //     deletedDateLbl + deletedDate,
            //     typeLbl + fileType,
            //     originalPathLbl + elidedOriginalPath.toHtmlEscaped()
            // );
        } else {
            return displayName + "\n" + deletedDateLbl + deletedDate + "\n" + typeLbl + fileType + "\n" + sizeLbl + fileSize + "\n" +  originalPathLbl + elidedOriginalPath;
            // return QString(HtmlTemplates::FIVE_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE,
            //     displayName,
            //     deletedDateLbl + deletedDate,
            //     typeLbl + fileType,
            //     sizeLbl + fileSize,
            //     originalPathLbl + elidedOriginalPath.toHtmlEscaped()
            // );
        }

    } else {
        if (fileSize.isEmpty()) {
            return deletedDateLbl + deletedDate + "\n" + typeLbl + fileType + "\n" +  originalPathLbl + elidedOriginalPath;
            // return QString(HtmlTemplates::THREE_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE,
            //     deletedDateLbl + deletedDate,
            //     typeLbl + fileType,
            //     originalPathLbl + elidedOriginalPath.toHtmlEscaped()
            // );
        } else {
            return  deletedDateLbl + deletedDate + "\n" + typeLbl + fileType + "\n" + sizeLbl + fileSize + "\n" + originalPathLbl + elidedOriginalPath;
            // return QString(HtmlTemplates::FOUR_ROW_TEMPLATE).arg(
            //     HtmlTemplates::BASE_STYLE,
            //     deletedDateLbl + deletedDate,
            //     typeLbl + fileType,
            //     sizeLbl + fileSize,
            //     originalPathLbl + elidedOriginalPath.toHtmlEscaped()
            // );
        }
    }
}

bool TooltipsManager::isTrashFile(FileInfo* info) const
{
    if (!info) {
        return false;
    }

    QString uri = info->uri();
    return uri.startsWith("trash:///");
}

QString TooltipsManager::getOriginalPath(FileInfo* info) const
{
    GFile *file = g_file_new_for_uri(info->uri().toUtf8().constData());
    GFileInfo *gInfo = g_file_query_info(file,
                                         G_FILE_ATTRIBUTE_TRASH_ORIG_PATH,
                                         G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                         nullptr,
                                         nullptr);
    auto origin_path = g_file_info_get_attribute_byte_string(gInfo, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH);

    if (!origin_path) {
        QString path = info->property("orig-path").toString();
        if (!path.isEmpty()) {
            origin_path = path.toUtf8().constData();
        }
    }

    QUrl url = QUrl(QString(origin_path));
    QString originalPath = url.path();

    g_object_unref(gInfo);
    g_object_unref(file);
    return originalPath;
}

QString TooltipsManager::getFileOrDirectorySize(FileInfo* info)
{
    QString fileSize;
    if (info->isDir()) {
        QString uri = info->uri();
        if (uri.startsWith("trash:///")) {
            uri = info->targetUri();
        }
        if (hasValidDirectorySizeCache(uri)) {
            auto cache = m_directorySizeCache.value(uri);
            if (cache.calculated && !cache.size.isEmpty()) {
                fileSize = cache.size;
            } else {
                fileSize = "";
            }
        } else {
            fileSize = calculateDirectorySizeSync(uri, DIRECTORY_SIZE_TIMEOUT_MS);
        }
    } else {
        fileSize = info->fileSize();
    }

    return fileSize;
}

bool TooltipsManager::hasValidDirectorySizeCache(const QString& uri) const
{
    if (!m_directorySizeCache.contains(uri)) {
        return false;
    }

    auto info = FileInfo::fromUri(uri);
    if (!info) {
        return false;
    }

    qint64 modTime = getFileModifiedTime(info.get());
    return m_directorySizeCache.value(uri).modTime == modTime;
}

void TooltipsManager::cleanupDirectorySizeCache()
{
    if (m_directorySizeCache.size() <= MAX_CACHE_SIZE) {
        return;
    }

    int toRemove = m_directorySizeCache.size() - MAX_CACHE_SIZE/2;
    auto keys = m_directorySizeCache.keys();
    for (int i = 0; i < toRemove && !keys.isEmpty(); i++) {
        m_directorySizeCache.remove(keys.takeFirst());
    }
}

QString TooltipsManager::calculateDirectorySizeSync(const QString& uri, int timeoutMs)
{
    auto info = FileInfo::fromUri(uri);
    if (!info) {
        return "";
    }

    qint64 currentModTime = getFileModifiedTime(info.get());
    if (m_directorySizeCache.contains(uri)) {
        auto cache = m_directorySizeCache.value(uri);
        if (cache.modTime == currentModTime) {
            if (cache.calculated && !cache.size.isEmpty()) {
                return cache.size;
            } else {
                return "";
            }
        }
    }

    QUrl url(uri);
    QString dirPath = url.toLocalFile();
    if (uri.startsWith("ftp://") || uri.startsWith("sftp://") || uri.startsWith("smb://")) {
        dirPath = info->filePath();
    }
    if (dirPath.isEmpty()) {
        return "";
    }

    if (uri == "filesafe:///") {
        dirPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.box";
    }

    bool isSymLink = info->isSymbolLink();
    QProcess process;
    QString command = "/usr/bin/du";
    QStringList arguments;

    if (isSymLink) {
        arguments << "-L" << "-s" << "-b" << dirPath;
    } else {
        arguments << "-s" << "-b" << dirPath;
    }

    process.start(command, arguments);

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        //process.waitForFinished(800);
        qDebug() << "Directory size calculation timeout for:" << uri;

        // Cache the failed result
        DirectorySizeCache cache;
        cache.modTime = currentModTime;
        cache.calculated = false;
        cache.size = "";
        m_directorySizeCache[uri] = cache;

        if (m_directorySizeCache.size() > MAX_CACHE_SIZE) {
            cleanupDirectorySizeCache();
        }

        return "";
    }

    QString result = process.readAllStandardOutput().trimmed();
    if (result.isEmpty()) {
        DirectorySizeCache cache;
        cache.modTime = currentModTime;
        cache.calculated = false;
        cache.size = "";
        m_directorySizeCache[uri] = cache;

        if (m_directorySizeCache.size() > MAX_CACHE_SIZE) {
            cleanupDirectorySizeCache();
        }

        return "";
    }

    QStringList parts = result.split(QRegExp("\\s+"));
    if (parts.isEmpty()) {
        DirectorySizeCache cache;
        cache.modTime = currentModTime;
        cache.calculated = false;
        cache.size = "";
        m_directorySizeCache[uri] = cache;

        if (m_directorySizeCache.size() > MAX_CACHE_SIZE) {
            cleanupDirectorySizeCache();
        }
        return "";
    }

    bool ok;
    qint64 size = parts.first().toLongLong(&ok);
    if (!ok) {
        DirectorySizeCache cache;
        cache.modTime = currentModTime;
        cache.calculated = false;
        cache.size = "";
        m_directorySizeCache[uri] = cache;

        if (m_directorySizeCache.size() > MAX_CACHE_SIZE) {
            cleanupDirectorySizeCache();
        }
        return "";
    }

    QString formattedSize = formatFileSize(size);

    // Cache the successful result
    DirectorySizeCache cache;
    cache.modTime = currentModTime;
    cache.calculated = true;
    cache.size = formattedSize;
    m_directorySizeCache[uri] = cache;

    if (m_directorySizeCache.size() > MAX_CACHE_SIZE) {
        cleanupDirectorySizeCache();
    }

    return formattedSize;
}

QString TooltipsManager::formatFileSize(qint64 size) const
{
    if (size <= 0) {
        return "0 B";
    }

    char *size_full = g_format_size(size);
    if (!size_full) {
        return "0 B";
    }

    QString result = QString(size_full);
    g_free(size_full);

    if (result.isEmpty()) {
        return "0 B";
    }

    return result;
}

QString TooltipsManager::toDesktopFile(FileInfo* info)
{
    if (!info || !info->uri().endsWith(".desktop")) {
        return QString();
    }

    QUrl url(info->uri());
    QString filePath = url.toLocalFile();

    if (filePath.isEmpty() || !QFileInfo(filePath).exists()) {
        // 如果是特殊URI（如trash:///中的desktop文件），尝试获取target URI
        QString targetUri = info->targetUri();
        if (!targetUri.isEmpty()) {
            QUrl targetUrl(targetUri);
            filePath = targetUrl.toLocalFile();
        }
    }

    QString comment = readDesktopComment(filePath);

    // 如果没有找到Comment字段，使用displayName作为兜底
    if (comment.isEmpty()) {
        comment = getDisplayName(info);
    }

    return toOneRowTooltip(comment.toHtmlEscaped());
}

QString TooltipsManager::readDesktopComment(const QString& filePath) const
{
    if (filePath.isEmpty() || !QFileInfo(filePath).exists()) {
        return QString();
    }

    GKeyFile* keyFile = g_key_file_new();
    GError* error = nullptr;

    // 加载desktop文件
    if (!g_key_file_load_from_file(keyFile, filePath.toUtf8().constData(), G_KEY_FILE_NONE, &error)) {
        if (error) {
            qWarning() << "Failed to load desktop file:" << filePath << "Error:" << error->message;
            g_error_free(error);
        }
        g_key_file_free(keyFile);
        return QString();
    }

    // 获取本地化的Comment字段
    gchar* comment = g_key_file_get_locale_string(keyFile,
                                                  G_KEY_FILE_DESKTOP_GROUP,
                                                  G_KEY_FILE_DESKTOP_KEY_COMMENT,
                                                  nullptr,  // 使用当前系统locale
                                                  &error);

    QString result;
    if (comment) {
        result = QString::fromUtf8(comment);
        g_free(comment);
    } else if (error) {
        // 如果没有找到Comment字段，不算错误
        if (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
            qWarning() << "Failed to get Comment from desktop file:" << filePath << "Error:" << error->message;
        }
        g_error_free(error);
    }

    g_key_file_free(keyFile);
    return result;
}

QString TooltipsManager::toOneRowTooltip(const QString& oneRowText) const
{
    return oneRowText;
    // return QString(HtmlTemplates::ONE_ROW_TEMPLATE).arg(
    //     HtmlTemplates::BASE_STYLE,
    //     oneRowText
    // );
}

bool TooltipsManager::isNetworkFile(FileInfo *info) const
{
    if (!info) {
        return false;
    }

    QString uri = info->uri();
    return uri.startsWith("network:///");
}

} // namespace Peony
