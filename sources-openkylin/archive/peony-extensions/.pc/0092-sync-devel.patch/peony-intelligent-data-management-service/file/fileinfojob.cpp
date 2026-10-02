/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "fileinfojob.h"
#include "fileinfo_p.h"
#include "fileinfomanager.h"
#include "condition/condition.h"

#include "ai/aiconditionshelper.h"

#include <peony-qt/global-settings.h>
#include <peony-qt/file-utils.h>
#include <peony-qt/file-meta-info.h>
#include <peony-qt/file-label-model.h>

#include <atomic>

#include <QThreadPool>
#include <QMutexLocker>
#include <QFileInfo>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDateTime>

#include <QUrl>

#undef signals

#include <gio/gio.h>

#define PEONY_FILE_TIME_OPENED "metadata::peony-time-opened"
#define PEONY_FILE_OPENED_COUNT "metadata::peony-opened-count"

namespace UKUI {
namespace IDM {
class FileInfoJobPrivate{
public:
    FileInfoJobPrivate() {
        cancelled = false;
        canFetchInfos = false;
        hasCondition = false;
    }
    ~FileInfoJobPrivate() {
        if (conditionsHelper) {
            delete conditionsHelper;
        }
    }

    QList<QSharedPointer<FileInfo>> infos;
    QSet<QSharedPointer<FileInfo>> resultInfos;

    std::atomic_bool cancelled;
    std::atomic_bool canFetchInfos;
    bool hasCondition;
    bool addWatchByDefault;
    bool handleContentCondition = false;
    QList<std::shared_ptr<Condition>> andConditions;
    QList<std::shared_ptr<Condition>> orConditions;
    QStringList labels;

    AIConditionsHelper *conditionsHelper = nullptr;

    int spaceId = -1;
};
}
}

using namespace UKUI::IDM;

FileInfoJob *FileInfoJob::fromUris(const QStringList &uris, bool addWatchByDefault)
{
    return new FileInfoJob(uris, true, addWatchByDefault);
}

FileInfoJob *FileInfoJob::fromPaths(const QStringList &paths, bool addWatchByDefault)
{
    return new FileInfoJob(paths, false, addWatchByDefault);
}

FileInfoJob *FileInfoJob::fromInfos(const QList<QSharedPointer<FileInfo> > &infos, bool addWatchByDefault)
{
    auto job = new FileInfoJob;
    job->priv->addWatchByDefault = addWatchByDefault;
    job->priv->infos = infos;
    return job;
}

bool FileInfoJob::queryInfoSyncWithConditions(QSharedPointer<FileInfo> &info, const QList<std::shared_ptr<Condition>> &andConditions, const QList<std::shared_ptr<Condition>> &orConditions)
{
    g_autoptr (GFile) gfile = g_file_new_for_uri(info->priv->uri.toUtf8().constData());
    g_autoptr (GFileInfo) gfileinfo = g_file_query_info(gfile,
                                                        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME","
                                                        G_FILE_ATTRIBUTE_STANDARD_ICON","
                                                        G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
                                                        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
                                                        G_FILE_ATTRIBUTE_STANDARD_SIZE","
                                                        G_FILE_ATTRIBUTE_TIME_ACCESS","
                                                        G_FILE_ATTRIBUTE_TIME_MODIFIED","
                                                        G_FILE_ATTRIBUTE_TIME_CREATED,
                                                        G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    if (!gfileinfo) {
        return false;
    } else {
        updateInfoWithGFileInfo(info, gfile, gfileinfo);
        if (orConditions.isEmpty() && andConditions.isEmpty())
            return true;
        else
            return infoMatchCondtions(info, andConditions, orConditions, false);
    }
}

FileInfoJob::~FileInfoJob()
{
    delete priv;
}

void FileInfoJob::start()
{
    // fixme: not use QThreadPool::globalInstance()
    QThreadPool::globalInstance()->start(this);
}

void FileInfoJob::cancel()
{
    priv->cancelled = true;
}

void FileInfoJob::setConditions(const QList<std::shared_ptr<Condition> > &conditions)
{
    priv->andConditions.clear();
    priv->orConditions.clear();
    if (conditions.isEmpty()) {
        priv->hasCondition = false;
        return;
    }
    for (auto condition : conditions) {
        if (condition->isAndCondition()) {
            priv->andConditions << condition;
        } else {
            priv->orConditions << condition;
        }
    }
    priv->hasCondition = true;
    if (!priv->conditionsHelper) {
        priv->conditionsHelper = new AIConditionsHelper;
    }
}

void FileInfoJob::setSpaceId(int spaceId)
{
    priv->spaceId = spaceId;
}

void FileInfoJob::setLabels(const QStringList &labels)
{
    if (labels.isEmpty()) {
        priv->labels.clear();
        return;
    }
    priv->labels = labels;
    if (!priv->conditionsHelper) {
        priv->conditionsHelper = new AIConditionsHelper;
    }
}

void FileInfoJob::setHandleContentCondition(bool handleContentCondition)
{
    priv->handleContentCondition = handleContentCondition;
}

void FileInfoJob::run()
{
    priv->canFetchInfos = false;

    Q_EMIT started();

    while (!priv->infos.isEmpty()) {
        auto info = priv->infos.takeFirst();
        GError *error = nullptr;
        g_autoptr (GFile) gfile = g_file_new_for_uri(info->priv->uri.toUtf8().constData());
        g_autoptr (GFileInfo) gfileinfo = g_file_query_info(gfile,
                                                            G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME","
                                                            G_FILE_ATTRIBUTE_STANDARD_ICON","
                                                            G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
                                                            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
                                                            G_FILE_ATTRIBUTE_STANDARD_SIZE","
                                                            G_FILE_ATTRIBUTE_TIME_ACCESS","
                                                            G_FILE_ATTRIBUTE_TIME_MODIFIED","
                                                            G_FILE_ATTRIBUTE_TIME_CREATED","
                                                            G_FILE_ATTRIBUTE_RECENT_MODIFIED","
                                                            PEONY_FILE_OPENED_COUNT","
                                                            PEONY_FILE_TIME_OPENED","
                                                            "metadata::peony-file-label-ids",
                                                            G_FILE_QUERY_INFO_NONE, nullptr, &error);
        if (error) {
            QString msg = error->message;
            g_error_free (error);
            {
                QMutexLocker l(&info->priv->mutex);
                info->priv->state = FileInfoPrivate::Invalid;
            }
            Q_EMIT errored(msg);
            continue;
        }
        if (priv->cancelled) {
            priv->infos.clear();
            priv->resultInfos.clear();
            Q_EMIT cancelled();
            return;
        }
        updateInfoWithGFileInfo(info, gfile, gfileinfo);

        bool match = true;

        if (priv->hasCondition) {
            match = infoMatchCondtions(info, priv->andConditions, priv->orConditions, priv->handleContentCondition);
        }

        if (!priv->labels.isEmpty()) {
            if (match) {
                // 如果基本条件不匹配则不做标签匹配
                match = infoMatchLabels(info, priv->labels);
            }
        }

        if (match) {
            if (priv->addWatchByDefault) {
                FileInfoManager::globalInstance()->addFileToWatcher(info->uri());
            }
            priv->resultInfos << info;
            Q_EMIT infoUpdated(info);
        }
    }

    priv->canFetchInfos = true;

    Q_EMIT finished();
}

QList<QSharedPointer<FileInfo> > FileInfoJob::getInfos()
{
    if (priv->canFetchInfos) {
        return priv->resultInfos.toList();
    }

    return QList<QSharedPointer<FileInfo>>();
}

void FileInfoJob::handleCancelWithSpaceId(int spaceId)
{
    if (spaceId == priv->spaceId) {
        cancel();
    }
}

void FileInfoJob::updateInfoWithGFileInfo(QSharedPointer<FileInfo> &info, GFile *gfile, GFileInfo *gfileinfo)
{
    g_autofree gchar *path = g_file_get_path(gfile);
    QString displayName = g_file_info_get_attribute_string(gfileinfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
    GIcon *gicon = g_file_info_get_icon(gfileinfo);
    QString iconName = Peony::FileUtils::getIconStringFromGIconThreadSafety(gicon);

    guint64 timeAcessSec = g_file_info_get_attribute_uint64(gfileinfo, G_FILE_ATTRIBUTE_TIME_ACCESS);
    guint64 timeModifiedSec = g_file_info_get_attribute_uint64(gfileinfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);
    guint64 timeCreatedSec = g_file_info_get_attribute_uint64(gfileinfo, G_FILE_ATTRIBUTE_TIME_CREATED);

    if (g_file_info_has_attribute(gfileinfo, PEONY_FILE_TIME_OPENED)) {
        QString peonyTimeOpenedString = g_file_info_get_attribute_string(gfileinfo, PEONY_FILE_TIME_OPENED);
        timeAcessSec = peonyTimeOpenedString.toULongLong();
    }

    int openCount = 0;
    if (g_file_info_has_attribute(gfileinfo, PEONY_FILE_OPENED_COUNT)) {
        QString openCountString = g_file_info_get_attribute_string(gfileinfo, PEONY_FILE_OPENED_COUNT);
        openCount = openCountString.toULongLong();
    }

    guint64 timeAcessMsec = timeAcessSec * 1000;
    guint64 timeModifiedMsec = timeModifiedSec * 1000;
    guint64 timeCreatedMsec = timeCreatedSec * 1000;
    gint64 timeRecentModified = g_file_info_get_attribute_int64(gfileinfo, G_FILE_ATTRIBUTE_RECENT_MODIFIED);

    const gchar *ctype = g_file_info_get_content_type(gfileinfo);
    QString contentType = ctype;
    g_autofree gchar *ftype = g_content_type_get_description(ctype);
    QString fileType = ftype;

    g_autofree gchar *formatsize = nullptr;
    guint64 size = g_file_info_get_size(gfileinfo);
    if (contentType != "inode/directory") {
        formatsize = g_format_size(size);
    }

    if (!path) {
        if (g_file_info_has_attribute(gfileinfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI)) {
            QUrl url(g_file_info_get_attribute_as_string(gfileinfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI));
            path = g_strdup(url.toLocalFile().toUtf8().constData());
        }
    }

    QString dateString;
    QString createString = Peony::GlobalSettings::getInstance()->transToSystemTimeFormat(timeCreatedSec);
    createString = tr("Create Date: %1").arg(createString);
    QString accessString = Peony::GlobalSettings::getInstance()->transToSystemTimeFormat(timeAcessSec);
    accessString = tr("Open Date: %1").arg(accessString);
    QString modifiyString = Peony::GlobalSettings::getInstance()->transToSystemTimeFormat(timeModifiedSec);
    //modifiyString = tr("Modify Date: %1").arg(modifiyString);
    if (timeAcessSec > timeModifiedSec) {
        // 显示打开时间
        dateString = accessString;
    } else if (timeModifiedSec > timeCreatedSec) {
        // fixme: 由于WPS等文件修改时是以临时文件移动方式进行原文件覆盖，所以创建时间和修改时间在保存后是一致的，这里先显示修改时间，后续需要结合版本判断的信息来处理此处显示的内容
        // 显示修改时间
        dateString = modifiyString;
    } else {
        // 显示创建时间
        dateString = createString;
    }

    QString parentPath = path;
    parentPath.truncate(parentPath.lastIndexOf('/'));

    // 标记信息
    auto metaInfo = Peony::FileMetaInfo::fromGFileInfo(info->priv->uri, gfileinfo);
    auto labelStrings = metaInfo->getMetaInfoStringList("peony-file-label-ids");
    QList<int> labelIds;
    QList<QVariant> labelColors;
    // fixme: 线程安全问题
    auto labelModel = FileLabelModel::getGlobalModel();
    for (QString labelString : labelStrings) {
        int labelId = labelString.toInt();
        auto item = labelModel->itemFromId(labelId);
        if (item) {
            labelColors << item->color();
            labelIds << labelId;
        }
    }

    QString version = "test";
    // thread safety
    {
        QMutexLocker l(&info->priv->mutex);
        info->priv->path = path;
        info->priv->parentPath = parentPath;
        info->priv->name = displayName;
        info->priv->iconName = iconName;
        info->priv->timeAccessMsec = timeAcessMsec;
        info->priv->timeModifiedMsec = timeModifiedMsec;
        info->priv->timeCreatedMsec = timeCreatedMsec;
        info->priv->openDateString = accessString;
        info->priv->modifyDateString = modifiyString;
        info->priv->createDateString = createString;
        info->priv->dateString = modifiyString;
        info->priv->version = version;
        info->priv->mimeType = contentType;
        info->priv->fileType = fileType;
        info->priv->size = size;
        info->priv->sizeFormat = formatsize;
        info->priv->labelIds = labelIds;
        info->priv->labelColors = labelColors;
        info->priv->timeRecentModified = timeRecentModified;
        info->priv->openCount = openCount;
        info->priv->state = FileInfoPrivate::State::Valid;
        //info->priv->ischecked = false;
    }
}

bool FileInfoJob::infoMatchCondtions(QSharedPointer<FileInfo> &info, const QList<std::shared_ptr<Condition> > &andConditions, const QList<std::shared_ptr<Condition> > &orConditions, bool handleContentCondition)
{
    QString displayName = info->displayName();
    quint64 size = info->size();
    QString contentType = info->mimeType();
    quint64 timeModifiedMsec = info->timeModifiedMsec();
    QString path = info->path();

    bool match = true;

    // 如果符合所有条件, 则match最终会为true, 如果没有或条件则match初始值为true, 否则为false
    match = orConditions.count() == 0;

    // 或条件, 只需要满足其中之一即可通过
    for (auto condition : orConditions) {
        // 如果存在条件匹配则跳过, 并且设置match为true
        switch (condition->type()) {
        case Condition::FileName: {
            match = condition->isFileNameMatchedWithKeywords(displayName);
            break;
        }
        case Condition::FileContent: {
            // 或条件的文件内容匹配目前仅用于普通搜索，这里可以跳过
           // if (handleContentCondition) {
                QString contentDecription = condition->getFileContentContent();
                AIConditionsHelper helper;
                match = helper.isFileMatchedWithDescriptionCondition(info->path(), contentDecription);
            //}
            break;
        }
        case Condition::FileSize: {
            match = condition->isFileSizeMatchedWithSize(size);
            break;
        }
        case Condition::FileType: {
            int subType = condition->getFileTypeSelectionType();
            if (subType < 0) {
                match = condition->isFileTypeMatchedWithSuffixes(displayName);
            } else {
                match = condition->isFileTypeMatchedWithSubType(subType, displayName, contentType);
            }
            break;
        }
        case Condition::DateRange: {
            match = condition->isFileMatchedWithDate(0, timeModifiedMsec);
            break;
        }
        case Condition::SearchLocation: {
            match = condition->isFileMatchedWithSearchLocation(path);
            break;
        }
        default: {
            match = false;
            break;
        }
        }

        // 如果有一个条件匹配, 则跳出循环
        if (match) {
            break;
        }
    }

    if (!match) {
        return match;
    } else {
        // 如果存在不匹配的且条件, 则跳过此次流程
        for (auto condition : andConditions) {
            // 如果存在条件不匹配则跳过, 并且设置match为false
            switch (condition->type()) {
            case Condition::FileName: {
                match = condition->isFileNameMatchedWithKeywords(displayName);
                break;
            }
            case Condition::FileContent: {
                // 与条件的文件内容搜索目前结合语义搜索使用，这里需要区分是语义搜索还是动态添加场景
                //if (handleContentCondition) {
                    QString contentDecription = condition->getFileContentContent();
                    AIConditionsHelper helper;
                    match = helper.isFileMatchedWithDescriptionCondition(info->path(), contentDecription);
                //}
                break;
            }
            case Condition::FileType: {
                int subType = condition->getFileTypeSelectionType();
                if (subType < 0) {
                    match = condition->isFileTypeMatchedWithSuffixes(displayName);
                } else {
                    match = condition->isFileTypeMatchedWithSubType(subType, displayName, contentType);
                }
                break;
            }
            case Condition::FileSize: {
                match = condition->isFileSizeMatchedWithSize(size);
                break;
            }
            case Condition::SearchLocation: {
                match = condition->isFileMatchedWithSearchLocation(path);
                break;
            }
            case Condition::DateRange: {
                match = condition->isFileMatchedWithDate(0, timeModifiedMsec);
                break;
            }
            default:
                match = true;
                break;
            }

            // 存在不匹配, 则跳出循环
            if (!match) {
                break;
            }
        }
    }

    return match;
}

bool FileInfoJob::infoMatchLabels(QSharedPointer<FileInfo> &info, const QStringList &labels)
{
    QUrl url = info->uri();
    QString filePath = url.toLocalFile();
    return priv->conditionsHelper->isFileMatchedWithLables(filePath, labels);
}

bool FileInfoJob::infoMatchCondtions(const QFileInfo &fileinfo, const QList<std::shared_ptr<Condition>> &andConditions, const QList<std::shared_ptr<Condition> > &orConditions)
{
    QString displayName = fileinfo.fileName();
    QString path = fileinfo.absoluteFilePath();
    quint64 size = fileinfo.size();

    QString contentType;
    if (fileinfo.isDir()) {
        contentType = "inode/directory";
    } else {
        //QMimeDatabase db;
        //QMimeType mimeType = db.mimeTypeForFile(fileinfo.absoluteFilePath(), QMimeDatabase::MatchExtension); // fixme: 搜索根目录匹配时调用此方法内存会飙升
        g_autofree gchar *contenttype = g_content_type_guess(path.toUtf8().constData(), nullptr, size, nullptr);
        contentType = contenttype;
    }
    quint64 timeModifiedMsec = fileinfo.fileTime(QFile::FileModificationTime).toMSecsSinceEpoch();

    QSharedPointer<FileInfo> info = QSharedPointer<FileInfo>(new FileInfo);
    info->priv->name = displayName;
    info->priv->size = size;
    info->priv->mimeType = contentType;
    info->priv->timeModifiedMsec = timeModifiedMsec;
    info->priv->path = path;

    return infoMatchCondtions(info, andConditions, orConditions, false);
}

FileInfoJob::FileInfoJob(const QStringList &args, bool isUriFormat, bool addWatchByDefault, QObject *parent) : QObject(parent), QRunnable()
{
    setAutoDelete(false);
    priv = new FileInfoJobPrivate;
    priv->addWatchByDefault = addWatchByDefault;
    if (isUriFormat) {
        for (auto arg : args) {
            priv->infos.append(FileInfo::fromUri(arg));
        }
    } else {
        for (auto arg : args) {
            priv->infos.append(FileInfo::fromPath(arg));
        }
    }
}
void FileInfoJob::updateFileInfoDataformat(QSharedPointer<FileInfo> &info)
{
    GError *error = nullptr;
    g_autoptr (GFile) gfile = g_file_new_for_uri(info->priv->uri.toUtf8().constData());
    g_autoptr (GFileInfo) gfileinfo = g_file_query_info(gfile,
                                                        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME","
                                                        G_FILE_ATTRIBUTE_STANDARD_ICON","
                                                        G_FILE_ATTRIBUTE_STANDARD_TARGET_URI","
                                                        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
                                                        G_FILE_ATTRIBUTE_STANDARD_SIZE","
                                                        G_FILE_ATTRIBUTE_TIME_ACCESS","
                                                        G_FILE_ATTRIBUTE_TIME_MODIFIED","
                                                        G_FILE_ATTRIBUTE_TIME_CREATED","
                                                        G_FILE_ATTRIBUTE_RECENT_MODIFIED","
                                                        PEONY_FILE_OPENED_COUNT","
                                                        PEONY_FILE_TIME_OPENED","
                                                        "metadata::peony-file-label-ids",
                                                        G_FILE_QUERY_INFO_NONE, nullptr, &error);
    if (error) {
        QString msg = error->message;
        g_error_free (error);
        {
            QMutexLocker l(&info->priv->mutex);
            info->priv->state = FileInfoPrivate::Invalid;
        }
        return;
    }
    updateInfoWithGFileInfo(info, gfile, gfileinfo);
}
FileInfoJob::FileInfoJob()
{
    setAutoDelete(false);
    priv = new FileInfoJobPrivate;
    priv->addWatchByDefault = true;
}
