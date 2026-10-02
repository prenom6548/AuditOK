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

#include "fileitemproxysortfiltermodel.h"
#include <QCollator>
#include <QDateTime>

#include "../file/fileinfo.h"
#include "spacessettings.h"

#include <peony-qt/global-settings.h>

FileItemProxySortFilterModel::FileItemProxySortFilterModel(FileItemModel *sourceModel, QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);

    m_sourceModel = sourceModel;
    m_sourceModel->setParent(this);
    m_collator = new QCollator;
    m_collator->setNumericMode(true);

    setSourceModel(sourceModel);

    updateSortByConfig();
    connect(sourceModel, &FileItemModel::filesLoaded, this, &FileItemProxySortFilterModel::filesLoaded);
    connect(sourceModel, &FileItemModel::requestStart, this, &FileItemProxySortFilterModel::requestStart);
    connect(sourceModel, &FileItemModel::spaceNameChanged, this, &FileItemProxySortFilterModel::spaceNameChanged);
    connect(sourceModel, &FileItemModel::spacesStatusDescriptionChanged, this, &FileItemProxySortFilterModel::spacesStatusDescriptionChanged);
    connect(sourceModel, &FileItemModel::recentFilesChanged, this, [=]{
        invalidate();
    });

    auto settings = SpacesSettings::globalInstance();
    connect(settings, &SpacesSettings::sortConfigChanged, this, [=]{
        updateSortByConfig();
    });
}

FileItemProxySortFilterModel::FileItemProxySortFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_sourceModel = new FileItemModel(this);
    m_collator = new QCollator;
}

FileItemProxySortFilterModel::~FileItemProxySortFilterModel()
{
    if (m_collator) {
        delete m_collator;
        m_collator = nullptr;
    }
}

FileItemProxySortFilterModel::SortType FileItemProxySortFilterModel::sortType() const
{
    return m_sortType;
}

void FileItemProxySortFilterModel::setSortType(const SortType &sortType)
{
    m_sortType = sortType;
    invalidate();
    sort(0, sortOrder());
}

void FileItemProxySortFilterModel::doSort(int sortType, int order, bool userAddedFirst, bool saveSettings)
{
    if (sortType < 0) {
        sortType = m_sortType;
    }

    if (order < 0) {
        order = int(sortOrder());
    }

    m_userAddedFirst = userAddedFirst;

    SortType realSortType = SortType(sortType);
    Qt::SortOrder realSortOrder = Qt::SortOrder(order);

    m_sortType = realSortType;
    invalidate();
    sort(0, realSortOrder);

    auto settings = SpacesSettings::globalInstance();
    if (saveSettings && !settings->getUseGlobalSorting()) {
        settings->setSpacesSortConfig(this->getSpaceId(), realSortType, realSortOrder == Qt::AscendingOrder? 0: 1, userAddedFirst);
    }
}

void FileItemProxySortFilterModel::startLoadRecentFiles()
{
    m_sourceModel->startLoadRecentFiles();
}

void FileItemProxySortFilterModel::startLoadFiles()
{
    m_sourceModel->startLoadFiles();
}

int FileItemProxySortFilterModel::getSpaceId() const
{
    if (m_sourceModel)
        return m_sourceModel->getSpaceId();
    return -1;
}

QString FileItemProxySortFilterModel::getSpaceName() const
{
    if (m_sourceModel)
        return m_sourceModel->getSpaceName();
    return nullptr;
}

QStringList FileItemProxySortFilterModel::getSpaceFilePaths() const
{
    QStringList paths;
    for (int row = 0; row < rowCount(); row++) {
        paths << index(row, 0).data(Qt::UserRole + 1).toString();
    }
    return paths;
}

QString FileItemProxySortFilterModel::getSpaceStatusDescription()
{
    quint64 addedFileCount = getLastAddedFileCount();
    quint64 addedFileTime = getLastAddedFileTimeMsec();
    if (addedFileCount <= 0 || addedFileTime == 0) {
        return nullptr;
    }
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime lastDateTime = QDateTime::fromMSecsSinceEpoch(addedFileTime);
    qint64 days = currentDateTime.daysTo(lastDateTime);
    if (days == 0) {
        quint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        quint64 timeOffset = currentTime - addedFileTime;
        QTime t(0, 0, 0, 0);
        t = t.addSecs(timeOffset/1000);
        QString timeString;
        if (t.hour() > 0) {
            timeString = tr("%1 h").arg(t.hour());
        } else if (t.minute() > 0) {
            timeString = tr("%1 min").arg(t.minute());
        } else {
            timeString = tr("%1 sec").arg(t.second());
        }
        return tr("%1 files added %2 ago").arg(addedFileCount).arg(timeString);
    } else if (days == 1) {
        return tr("%1 files added a day ago").arg(addedFileCount);
    } else {
        return tr("%1 files added %2 days ago").arg(addedFileCount).arg(days);
    }
}

QString FileItemProxySortFilterModel::spaceName()
{
    return getSpaceName();
}

void FileItemProxySortFilterModel::handleFileEvent(UKUI::IDM::FileInfoManager::FileEventType type, const QString &arg1, const QString arg2)
{
    m_sourceModel->handleFileEvent(type, arg1, arg2);
}

bool FileItemProxySortFilterModel::isUserAddedFile(const QString &path)
{
    return m_sourceModel->isUserAddedFile(path);
}

void FileItemProxySortFilterModel::setIsRecent(bool isRecent)
{
    setDynamicSortFilter(true);
    setSortType(RecentModified);
    sort(0);
    m_isRecent = isRecent;
}

void FileItemProxySortFilterModel::handleSpaceRemoved(int spaceId)
{
    if (spaceId < 0) {
        return;
    }
    if (spaceId == getSpaceId()) {
        this->deleteLater();
    }
}

bool FileItemProxySortFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_isRecent) {
        qint64 modifytime = getModificationTime(source_left);
        qint64 accesstime = getFileAccessTime(source_left);
        qint64 leftRecentModifedTime = modifytime>accesstime ? modifytime:accesstime;
        modifytime = getModificationTime(source_right);
        accesstime = getFileAccessTime(source_right);
        qint64 rightRecentModifedTime = modifytime>accesstime ? modifytime:accesstime;
        return leftRecentModifedTime >= rightRecentModifedTime;
    }

    // 文件夹优先
    bool leftIsDirectory = isDirectory(source_left);
    bool rightIsDirectory = isDirectory(source_right);
    if (!leftIsDirectory) {
        if (rightIsDirectory) {
            return sortOrder() == Qt::AscendingOrder? false: true;
        }
    } else if (!rightIsDirectory) {
        return sortOrder() == Qt::AscendingOrder? true: false;
    }

    switch (m_sortType) {
    case MostUsed: {
        // 文件打开总次数
        quint64 leftOpenCount = getFileOpenCount(source_left);
        quint64 rightOpenCount = getFileOpenCount(source_right);
        if (leftOpenCount != rightOpenCount) {
            return leftOpenCount < rightOpenCount;
        }
        // 文件最近一次访问时间
        quint64 leftAtime = getFileAccessTime(source_left);
        quint64 rightAtime = getFileAccessTime(source_right);
        if (leftAtime < rightAtime) {
            return true;
        }
        if (rightAtime > leftAtime) {
            return false;
        }
        break;
    }
    case AddMode: {
        bool leftIsUserAdded = m_sourceModel->isUserAddedFile(source_left.row());
        bool rightIsUserAdded = m_sourceModel->isUserAddedFile(source_right.row());
        bool lesser = rightIsUserAdded && !leftIsUserAdded;
        bool greater = leftIsUserAdded && !rightIsUserAdded;
        bool same = !lesser && !greater;
        if (same) {
            break;
        } else if (lesser) {
            return !m_userAddedFirst;
        } else {
            return m_userAddedFirst;
        }
        break;
    }
    case FileName: {
        // go to name sort
        break;
    }
    case Date: {
        quint64 leftMtime = getModificationTime(source_left);
        quint64 rightMtime = getModificationTime(source_right);
        bool lesser = leftMtime <= rightMtime;
        return lesser;
        break;
    }
    case FileType: {
        QString leftType = getFileMimeType(source_left);
        QString rightType = getFileMimeType(source_right);
        if (leftType != rightType) {
            return m_collator->compare(leftType, rightType) < 0;
        }
        break;
    }
    case FileSize: {
        quint64 leftSize = getFileSize(source_left);
        quint64 rightSize = getFileSize(source_right);
        if (leftSize != rightSize) {
            return leftSize < rightSize;
        }
        break;
    }

    default:
        break;
    }

namesort:
    QString leftName = getDisplayName(source_left);
    QString rightName = getDisplayName(source_right);
    return m_collator->compare(leftName, rightName) <= 0;
}

QString FileItemProxySortFilterModel::getDisplayName(const QModelIndex &index) const
{
    return index.data().toString();
}

bool FileItemProxySortFilterModel::isDirectory(const QModelIndex &index) const
{
    QString mimeType = index.data(Qt::UserRole + 2).toString();
    if (mimeType == "inode/directory") {
        return true;
    }
    return false;
}

quint64 FileItemProxySortFilterModel::getModificationTime(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 5).toULongLong();
}

QString FileItemProxySortFilterModel::getFileMimeType(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 2).toString();
}

quint64 FileItemProxySortFilterModel::getFileSize(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 6).toULongLong();
}

QString FileItemProxySortFilterModel::getFileSizeFormat(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 7).toString();
}

quint64 FileItemProxySortFilterModel::getFileAccessTime(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 11).toULongLong();
}

qint64 FileItemProxySortFilterModel::getFileRecentModifiedTimeMsec(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 15).toLongLong();
}

quint64 FileItemProxySortFilterModel::getFileOpenCount(const QModelIndex &index) const
{
    return index.data(Qt::UserRole + 16).toULongLong();
}

quint64 FileItemProxySortFilterModel::getLastAddedFileCount() const
{
    return m_sourceModel->getLastAddedFileCount();
}

quint64 FileItemProxySortFilterModel::getLastAddedFileTimeMsec() const
{
    return m_sourceModel->getLastAddedFileTimeMsec();
}

void FileItemProxySortFilterModel::updateSortByConfig()
{
    if (getSpaceId() < 0)
        return;
    auto settings = SpacesSettings::globalInstance();
    auto sortSettings = settings->getSpacesSortConfig(this->getSpaceId());
    int sortType = sortSettings.value("sortType").toInt();
    int sortOrder = sortSettings.value("sortOrder").toInt();
    bool userAddedFirst = sortSettings.value("userAddedFirst").toBool();
    doSort(sortType, sortOrder, userAddedFirst, false);
}

bool FileItemProxySortFilterModel::userAddedFirst() const
{
    return m_userAddedFirst;
}
