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

#include "fileitemmodel.h"

#include "../file/fileinfojob.h"

#include "modelcontroller.h"

#include <QPersistentModelIndex>
#include <QIcon>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

#include <peony-qt/file-watcher.h>
#include <peony-qt/file-enumerator.h>
#include <peony-qt/file-info-job.h>

FileItemModel::FileItemModel(QObject *parent) : QAbstractTableModel(parent)
{
    addedFileTimer = new QTimer(this);
    initRecentTimer();
}

FileItemModel::FileItemModel(int spaceId, const QString &spaceName, QList<QSharedPointer<UKUI::IDM::FileInfo> > infos, QObject *parent)
    : QAbstractTableModel(parent), spaceId(spaceId), spaceName(spaceName)
{
    addedFileTimer = new QTimer(this);
    addedFileTimer->setSingleShot(true);
    addedFileTimer->setInterval(5000);
    initRecentTimer();

    connect(addedFileTimer, &QTimer::timeout, this, [=]{
        lastAddedFileCount = this->infos.count() - tmpFileCount;
        Q_EMIT spacesStatusDescriptionChanged();
    });

    auto infoJob = UKUI::IDM::FileInfoJob::fromInfos(infos);
    connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
        delete infoJob;
    });
    connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
        auto newInfos = infoJob->getInfos();
        int newInfoCounts = newInfos.count();
        if (newInfoCounts > 0) {

            beginInsertRows(QModelIndex(), this->infos.count(), this->infos.count() + newInfoCounts - 1);
            if (newInfoCounts == infos.count()) {
                this->infos << infos;
            } else {
                // fixme: 返回的info列表无序导致判断是否用户手动添加
                QSet<QSharedPointer<UKUI::IDM::FileInfo>> infosSet;
                for (auto info : newInfos) {
                    infosSet.insert(info);
                }
                for (auto userAddedInfo : userAddedInfos) {
                    infosSet.remove(userAddedInfo);
                }

                this->infos << userAddedInfos;
                this->infos << infosSet.values();
            }
            endInsertRows();
        }
        auto infoManager = UKUI::IDM::FileInfoManager::globalInstance();
        connect(infoManager, &UKUI::IDM::FileInfoManager::fileChanged, this, &FileItemModel::handleFileEvent, Qt::UniqueConnection);
        Q_EMIT filesLoaded();
        delete infoJob;
    });

    connect(this, &FileItemModel::requestStart, infoJob, &UKUI::IDM::FileInfoJob::start);
    connect(ModelController::globalInstance(), &ModelController::spaceNameUpdated, this, [=](int spaceId, const QString &spaceName){
        if (spaceId == this->spaceId) {
            this->spaceName = spaceName;
            Q_EMIT spaceNameChanged();
        }
    });
}

FileItemModel::~FileItemModel()
{
    return;
}

int FileItemModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return infos.count();
    return 0;
}

int FileItemModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 4;
    return 0;
}

QVariant FileItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto info = infos.at(index.row());
    int column = index.column();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole: {
        switch (column) {
        case 0:
            return info->displayName();
        case 1:
            return info->sizeFormat();
        case 2:
            return info->dateString();
        case 3:
            return info->parentPath();
        default:
            break;
        }
    }
    case Qt::DecorationRole: {
        if (column == 0)
            return QIcon::fromTheme(info->iconName(), QIcon::fromTheme("unknown"));
        else
             break;
    }
    case Qt::UserRole + 1:
        return info->path();
    case Qt::UserRole + 2:
        return info->mimeType();
    case Qt::UserRole + 3:
        return info->fileType();
    case Qt::UserRole + 4:
        return info->dateString();
    case Qt::UserRole + 5:
        return info->timeModifiedMsec();
    case Qt::UserRole + 6:
        return info->size();
    case Qt::UserRole + 7:
        return info->sizeFormat();
    case Qt::UserRole + 8:
        return info->parentPath();
    case Qt::UserRole + 9:
        return index.row() < userAddedInfos.count();
    case Qt::UserRole + 10:
        return info->labelColors();
    case Qt::UserRole + 11:
        return info->timeAccessMsec();
    case Qt::UserRole + 12:
        return info->createDateString();
    case Qt::UserRole + 13:
        return info->openDateString();
    case Qt::UserRole + 14:
        return info->modifyDateString();
    case Qt::UserRole + 15:
        return info->timeRecentModified();
    case Qt::UserRole + 16:
        return info->openCount();
    default:
        break;
    }
    // FIXME: Implement me!
    return QVariant();
}

void FileItemModel::startLoadRecentFiles()
{
#ifdef HAVE_SET_QUERY_DISPLAY_NAME_WITH_VOLUMES
    Peony::FileInfoJob::setQueryDisplayNameWithVolumes(false);
#endif
    auto enumerator = new Peony::FileEnumerator;
    enumerator->setEnumerateDirectory("recent:///");
    connect(enumerator, &Peony::FileEnumerator::enumerateFinished, this, [=]{
        QStringList uris = enumerator->getChildrenUris();
        UKUI::IDM::FileInfoJob *job = UKUI::IDM::FileInfoJob::fromUris(uris);
        connect(job, &UKUI::IDM::FileInfoJob::cancelled, [=]{
            delete job;
        });
        connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            beginResetModel();
            this->infos = job->getInfos();
            endResetModel();
            delete job;
            Peony::FileWatcher *watcher = new Peony::FileWatcher("recent:///");
            watcher->setMonitorChildrenChange(true);
            connect(this, &FileItemModel::destroyed, watcher, &Peony::FileWatcher::deleteLater);
            connect(watcher, &Peony::FileWatcher::fileCreated, this, [=](const QString &uri){
                auto infoJob = UKUI::IDM::FileInfoJob::fromUris(QStringList() << uri);
                connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
                    delete infoJob;
                });
                connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                    auto newInfos = infoJob->getInfos();
                    beginInsertRows(QModelIndex(), 0, newInfos.count() - 1);
                    auto tmpInfo = newInfos;
                    tmpInfo << this->infos;
                    this->infos = tmpInfo;
                    endInsertRows();
                    delete infoJob;
                });
                infoJob->start();
            });
            connect(watcher, &Peony::FileWatcher::fileDeleted, this, [=](const QString &uri){
                beginResetModel();
                infos.removeOne(UKUI::IDM::FileInfo::fromUri(uri));
                endResetModel();
            });
            connect(watcher, &Peony::FileWatcher::fileChanged, this, [=](const QString &uri){
                QTimer::singleShot(1000, this, [=]{
                    auto info = UKUI::IDM::FileInfo::fromUri(uri);
                    QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
                    infos << info;
                    auto infoJob = UKUI::IDM::FileInfoJob::fromInfos(infos);
                    infoJob->setAutoDelete(true);
                    connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, &FileItemModel::handleRecentFileChanged);
                    infoJob->start();
                });
            });
            watcher->startMonitor();
            Q_EMIT filesLoaded();
            enumerator->deleteLater();
        });
        job->start();
    });
    enumerator->enumerateAsync();
}

void FileItemModel::startLoadFiles()
{
    Q_EMIT requestStart();
}

int FileItemModel::getSpaceId() const
{
    return spaceId;
}

QString FileItemModel::getSpaceName() const
{
    return spaceName;
}

// todo:
// 1. 批处理
// 2. 文件移动和重命名场景
// 3. 数据库同步
void FileItemModel::handleFileEvent(UKUI::IDM::FileInfoManager::FileEventType type, const QString &arg1, const QString &arg2)
{
    using namespace UKUI::IDM;
    switch (type) {
    // fixme: 使用pluma等文本编辑器修改文件后无法监听到后续事件
    case FileInfoManager::FileEventType::FileIgnoredAutomatic: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        QString path = arg1;
        auto info = UKUI::IDM::FileInfo::fromPath(path, false);
        if (this->infos.contains(info)) {
            auto infoJob = FileInfoJob::fromPaths(QStringList()<<path, false);
            connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
                delete infoJob;
            });
            connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                auto infos = infoJob->getInfos();
                for (auto info : infos) {
                    int index = this->infos.indexOf(info);
                    if (index != -1) {
                        QPersistentModelIndex pindexStart = this->index(index, 0);
                        QPersistentModelIndex pindexEnd = this->index(index, 3);
                        Q_EMIT dataChanged(pindexStart, pindexEnd);
                        FileInfoManager::globalInstance()->addFileToWatcher(info->uri());
                    }
                }
                delete infoJob;
            });
            connect(infoJob, &FileInfoJob::errored, this, [=]{
                // 删除文件
                removeFile(path);
            });
            infoJob->start();
        }
        break;
    }
    case FileInfoManager::FileEventType::FileCreated: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        startCountAddedFiles();
        // 添加文件
        QString path = arg1;
        auto infoJob = FileInfoJob::fromPaths(QStringList()<<path);
        connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
            delete infoJob;
        });
        connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            auto infos = infoJob->getInfos();
            int newInfoCount = infos.count();
            beginInsertRows(QModelIndex(), 0, newInfoCount - 1);
            auto tmpInfos = infos << this->infos;
            this->infos = tmpInfos;
            endInsertRows();
            delete infoJob;
        });
        infoJob->start();
        break;
    }
    case FileInfoManager::FileEventType::UserRemoveFile: {
        auto infoToBeRemoved = FileInfo::fromPath(arg1);
        int indexToBeRemoved = infos.indexOf(infoToBeRemoved);
        int autoAddedFilesStartIndex = userAddedInfos.count();
        if (indexToBeRemoved >= 0 && indexToBeRemoved < autoAddedFilesStartIndex && autoAddedFilesStartIndex > 0) {
            beginRemoveRows(QModelIndex(), indexToBeRemoved, indexToBeRemoved);
            infos.removeAt(indexToBeRemoved);
            userAddedInfos.removeAt(indexToBeRemoved);
            endRemoveRows();
        } else {
            qWarning() << "failed to get index of" << arg1;
        }
        break;
    }
    case FileInfoManager::FileEventType::UserAddFile: {
        QString path = arg1;
        auto info = FileInfo::fromPath(path);
        if (infos.contains(info)) {
            Q_EMIT reportError(QObject::tr("File %1 is existed in space, please not add file repeatly.").arg(path));
        }

        startCountAddedFiles();
        auto infoJob = FileInfoJob::fromPaths(QStringList()<<path);
        connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
            delete infoJob;
        });
        connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            auto infos = infoJob->getInfos();
            insertFiles(infos, false);
            delete infoJob;
        });
        infoJob->start();
        break;
    }
    //case FileInfoManager::FileEventType::FileMoveIn:
    //case FileInfoManager::FileEventType::FileMoveOut:
    //case FileInfoManager::FileEventType::FileIgnoredAutomatic:
    case FileInfoManager::FileEventType::FileDeleted: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        // 文件无法监听自身删除，故只能在FileIgnoredAutomatic进行判断文件是否删除
        // 文件删除则从空间中移除，model中也移除
        QString path = arg1;
        removeFile(path);
        break;
    }
    case FileInfoManager::FileEventType::FileMoved: {
        auto oldInfo = FileInfo::fromPath(arg1);
        int index = this->infos.indexOf(oldInfo);
        if (index < 0)
            return;

        beginRemoveRows(QModelIndex(), index, index);
        infos.removeAt(index);
        int autoAddedFilesStartIndex = userAddedInfos.count();
        if (index < autoAddedFilesStartIndex) {
            QUrl url = QUrl::fromLocalFile(arg2);
            auto proxyModel = qobject_cast<FileItemProxySortFilterModel *>(parent());
            ModelController::globalInstance()->userAddFilesToSpace(QList<QUrl>() << url, proxyModel);
            userAddedInfos.removeAt(index);
        }
        endRemoveRows();

        break;
    }
    case FileInfoManager::FileEventType::FileAttrChanged:
    case FileInfoManager::FileEventType::FileModified: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        // 更新文件信息
        QString path = arg1;
        auto info = UKUI::IDM::FileInfo::fromPath(path, false);
        if (this->infos.contains(info)) {
            auto infoJob = FileInfoJob::fromPaths(QStringList()<<path);
            connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
                delete infoJob;
            });
            connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                auto infos = infoJob->getInfos();
                for (auto info : infos) {
                    int index = this->infos.indexOf(info);
                    if (index != -1) {
                        QPersistentModelIndex pindexStart = this->index(index, 0);
                        QPersistentModelIndex pindexEnd = this->index(index, 3);
                        Q_EMIT dataChanged(pindexStart, pindexEnd);
                    }
                }
                delete infoJob;
            });
            infoJob->start();
        }
        break;
    }
    default:
        break;
    }
}

bool FileItemModel::isUserAddedFile(const QString &path)
{
    auto info = UKUI::IDM::FileInfo::fromPath(path);
    return userAddedInfos.contains(info);
}

bool FileItemModel::isUserAddedFile(int index)
{
    if (index < 0)
        return false;
    return index < userAddedInfos.count();
}

void FileItemModel::handleFilesAdded(const QList<QSharedPointer<UKUI::IDM::FileInfo> > &addInfos, int spaceId)
{
    if (this->spaceId == spaceId) {
        if (!addInfos.isEmpty()) {
            startCountAddedFiles();
            insertFiles(addInfos, true);
        }
    }
}

void FileItemModel::handleFilesChangedForConditionsChanged(bool needReload, const QList<QSharedPointer<UKUI::IDM::FileInfo> > &resultFiles, int spaceId)
{
    // fixme
}

void FileItemModel::handleFilesReloaded(const QList<QSharedPointer<UKUI::IDM::FileInfo> > &infos, int spaceId)
{
    if (this->spaceId == spaceId) {
        auto reloadedInfos = userAddedInfos;
        reloadedInfos << infos;
        beginResetModel();
        tmpFileCount = 0;
        addedFileTimer->start();
        this->infos = reloadedInfos;
        endResetModel();
    }
}

void FileItemModel::handleRecentFileChanged()
{
    recentFileTimer->start();
}

QHash<int, QByteArray> FileItemModel::roleNames() const
{
    auto orignalRoleNames = QAbstractTableModel::roleNames();
    orignalRoleNames.insert(Qt::UserRole + 1, "path");
    orignalRoleNames.insert(Qt::UserRole + 2, "mimeType");
    orignalRoleNames.insert(Qt::UserRole + 3, "fileType");
    orignalRoleNames.insert(Qt::UserRole + 4, "displayTime");
    orignalRoleNames.insert(Qt::UserRole + 5, "mtime");
    orignalRoleNames.insert(Qt::UserRole + 6, "size");
    orignalRoleNames.insert(Qt::UserRole + 7, "displaySize");
    orignalRoleNames.insert(Qt::UserRole + 8, "parentPath");
    orignalRoleNames.insert(Qt::UserRole + 9, "isUserAdded");
    orignalRoleNames.insert(Qt::UserRole + 10, "labelColors");
    orignalRoleNames.insert(Qt::UserRole + 11, "atime");
    orignalRoleNames.insert(Qt::UserRole + 12, "createDate");
    orignalRoleNames.insert(Qt::UserRole + 13, "openDate");
    orignalRoleNames.insert(Qt::UserRole + 14, "modifyDate");
    orignalRoleNames.insert(Qt::UserRole + 15, "recentMtime");
    orignalRoleNames.insert(Qt::UserRole + 16, "openCount");
    return orignalRoleNames;
}

void FileItemModel::startCountAddedFiles()
{
    if (!addedFileTimer->isActive()) {
        lastAddedFileTimeMsec = QDateTime::currentMSecsSinceEpoch();
        tmpFileCount = infos.count();
        addedFileTimer->start();
    }
}

void FileItemModel::initRecentTimer()
{
    recentFileTimer = new QTimer(this);
    recentFileTimer->setSingleShot(true);
    recentFileTimer->setInterval(500);

    connect(recentFileTimer, &QTimer::timeout, this, [=]{
        Q_EMIT recentFilesChanged();
    });
}

void FileItemModel::removeFile(const QString &filePath)
{
    auto info = UKUI::IDM::FileInfo::fromPath(filePath);

    int index = infos.indexOf(info);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        infos.removeAt(index);
        if (index < userAddedInfos.count()) {
            userAddedInfos.removeAt(index);
        }
        endRemoveRows();
        ModelController::globalInstance()->fileDeletedRequest(filePath);
    }
}

quint64 FileItemModel::getLastAddedFileTimeMsec() const
{
    return lastAddedFileTimeMsec;
}

QList<QSharedPointer<UKUI::IDM::FileInfo>> FileItemModel::insertFiles(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, bool isAutoAdded)
{
    QList<QSharedPointer<UKUI::IDM::FileInfo>> infosToBeAdded;
    for (auto info : infos) {
        if (this->infos.contains(info))
            continue;
        infosToBeAdded.append(info);
    }

    if (!infosToBeAdded.isEmpty()) {
        int insertPos;
        if (isAutoAdded) {
            insertPos = this->infos.count();
            beginInsertRows(QModelIndex(), insertPos, insertPos + infosToBeAdded.count() - 1);
            this->infos.append(infosToBeAdded);
            endInsertRows();
        } else {
            insertPos = this->userAddedInfos.count();
            beginInsertRows(QModelIndex(), insertPos, insertPos + infosToBeAdded.count() - 1);
            for (auto info : infosToBeAdded) {
                this->infos.insert(insertPos, info);
                insertPos++;
            }
            this->userAddedInfos.append(infosToBeAdded);
            endInsertRows();
        }
    }

    return infosToBeAdded;
}

quint64 FileItemModel::getLastAddedFileCount() const
{
    return lastAddedFileCount;
}
