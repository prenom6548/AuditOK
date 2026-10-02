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

#ifndef FILEITEMPROXYSORTFILTERMODEL_H
#define FILEITEMPROXYSORTFILTERMODEL_H

#include <QSortFilterProxyModel>

#include "fileitemmodel.h"

class QCollator;

class FileItemProxySortFilterModel : public QSortFilterProxyModel
{
    friend class ModelController;
    Q_OBJECT
    Q_PROPERTY(SortType sortType READ sortType WRITE setSortType)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder)
    Q_PROPERTY(QString spaceName READ spaceName NOTIFY spaceNameChanged)
    Q_PROPERTY(bool userAddedFirst READ userAddedFirst CONSTANT)
public:
    enum SortType {
        MostUsed,
        AddMode, //手动添加优先，自动添加优先两种模式
        FileName,
        Date, // 创建时间，修改时间，访问时间？
        FileType,
        FileSize,
        RecentModified // 最近访问排序时间
    };
    Q_ENUM(SortType)

    explicit FileItemProxySortFilterModel(FileItemModel *sourceModel, QObject *parent = nullptr);
    explicit FileItemProxySortFilterModel(QObject *parent = nullptr);
    ~FileItemProxySortFilterModel();

    SortType sortType() const;
    void setSortType(const SortType &sortType);

    Q_INVOKABLE void doSort(int sortType, int order, bool userAddedFirst, bool saveSettings = true);

    Q_INVOKABLE void startLoadRecentFiles();
    Q_INVOKABLE void startLoadFiles();

    Q_INVOKABLE int getSpaceId() const;
    Q_INVOKABLE QString getSpaceName() const;

    Q_INVOKABLE QStringList getSpaceFilePaths() const;

    Q_INVOKABLE QString getSpaceStatusDescription();

    QString spaceName();

    void handleFileEvent(UKUI::IDM::FileInfoManager::FileEventType type, const QString &arg1, const QString arg2);

    bool isUserAddedFile(const QString &path);

    void setIsRecent(bool isRecent);

    bool userAddedFirst() const;

signals:
    void filesLoaded();
    void requestStart();
    void spaceNameChanged();
    void spacesStatusDescriptionChanged();

public slots:
    void handleSpaceRemoved(int spaceId);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

    QString getDisplayName(const QModelIndex &index) const;
    bool isDirectory(const QModelIndex &index) const;
    quint64 getModificationTime(const QModelIndex &index) const;
    QString getFileMimeType(const QModelIndex &index) const;
    quint64 getFileSize(const QModelIndex &index) const;
    QString getFileSizeFormat(const QModelIndex &index) const;
    quint64 getFileAccessTime(const QModelIndex &index) const;
    qint64 getFileRecentModifiedTimeMsec(const QModelIndex &index) const;
    quint64 getFileOpenCount(const QModelIndex &index) const;

    quint64 getLastAddedFileCount() const;
    quint64 getLastAddedFileTimeMsec() const;

    void updateSortByConfig();

private:
    SortType m_sortType = MostUsed;

    bool m_userAddedFirst = true;

    QCollator *m_collator = nullptr;

    FileItemModel *m_sourceModel = nullptr;

    bool m_isRecent = false;
};

#endif // FILEITEMPROXYSORTFILTERMODEL_H
