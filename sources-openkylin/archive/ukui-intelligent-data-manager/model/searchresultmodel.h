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

#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "file/fileinfo.h"
#include "condition/condition.h"
#include "search/searchtask.h"

class QTimer;

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class SearchResultProxyModel;
    friend class ConditionEditor;
public:
    static SearchResultModel *globalInstance();
    explicit SearchResultModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool getSearchInSpace() const;
    void setSearchInSpace(bool newSearchInSpace);

    const QString &getSearchKeyword() const;
    void setSearchKeyword(const QString &newSearchKeyword);

    const QList<QSharedPointer<Condition> > &getSearchConditions() const;
    void setSearchConditions(const QList<QSharedPointer<Condition> > &newSearchConditions);

    int getCurrentSpaceId() const;
    void setCurrentSpaceId(int newCurrentSpaceId);

    const QList<QSharedPointer<Condition> > &getReversedConditions() const;

    const QList<QSharedPointer<UKUI::IDM::FileInfo> > &getNameMatchFiles() const;

    const QList<QSharedPointer<UKUI::IDM::FileInfo> > &getContentMatchFiles() const;

    void setSearchInSpacePaths(const QStringList &paths);

    bool getAiSearch() const;
    void setAiSearch(bool newAiSearch);

    bool getLoading() const;
    void setLoading(bool newLoading);

signals:
    void searchInSpaceChanged();
    void searchKeywordChanged();
    void searchConditionsChanged();
    void currentSpaceIdChanged();

    void aboutToUpdate();

    void aiSearchChanged();

    void loadingChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

protected slots:
    void delayUpdate();
    void doUpdateInternal();
    void saveSearchAsSpace();

private:
    QList<QSharedPointer<Condition>> reversedConditions;

    bool searchInSpace = false;
    QString searchKeyword = nullptr;
    QList<QSharedPointer<Condition>> searchConditions;
    int currentSpaceId = -1;

    bool aiSearch = false;
    bool loading = false;

    Q_PROPERTY(bool searchInSpace READ getSearchInSpace WRITE setSearchInSpace NOTIFY searchInSpaceChanged)
    Q_PROPERTY(QString searchKeyword READ getSearchKeyword WRITE setSearchKeyword NOTIFY searchKeywordChanged)
    Q_PROPERTY(QList<QSharedPointer<Condition>> searchConditions READ getSearchConditions WRITE setSearchConditions NOTIFY searchConditionsChanged)
    Q_PROPERTY(int currentSpaceId READ getCurrentSpaceId WRITE setCurrentSpaceId NOTIFY currentSpaceIdChanged)
    Q_PROPERTY(bool aiSearch READ getAiSearch WRITE setAiSearch NOTIFY aiSearchChanged)
    Q_PROPERTY(bool loading READ getLoading WRITE setLoading NOTIFY loadingChanged)

    QList<QSharedPointer<UKUI::IDM::FileInfo>> nameMatchFiles;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> contentMatchFiles;
    QStringList searchInSpacePaths;
    QTimer *delayer = nullptr;

    SearchTask *searchTask = nullptr;
};

inline QHash<int, QByteArray> SearchResultModel::roleNames() const
{
    auto orignalRoleNames = QAbstractItemModel::roleNames();
    orignalRoleNames.insert(Qt::UserRole + 1, "path");
    orignalRoleNames.insert(Qt::UserRole + 2, "mimeType");
    orignalRoleNames.insert(Qt::UserRole + 3, "fileType");
    orignalRoleNames.insert(Qt::UserRole + 4, "displayTime");
    orignalRoleNames.insert(Qt::UserRole + 5, "mtime");
    orignalRoleNames.insert(Qt::UserRole + 6, "size");
    orignalRoleNames.insert(Qt::UserRole + 7, "displaySize");
    orignalRoleNames.insert(Qt::UserRole + 8, "parentPath");
    return orignalRoleNames;
}

class SearchResultProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    static SearchResultProxyModel *globalInstance();
    explicit SearchResultProxyModel();

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

    bool getSearchInSpace() const;
    void setSearchInSpace(bool newSearchInSpace);

    const QString &getSearchKeyword() const;
    void setSearchKeyword(const QString &newSearchKeyword);

    const QList<QSharedPointer<Condition> > getSearchConditions() const;
    void setSearchConditions(const QList<QSharedPointer<Condition> > &newSearchConditions);

    Q_INVOKABLE void saveSearchAsSpace();
    Q_INVOKABLE void clearCondition();

    int getCurrentSpaceId() const;
    void setCurrentSpaceId(int newCurrentSpaceId);

    /*!
     * \brief setSearchInSpacePaths
     * 搜索界面切换空间时设置的数据源,用于进行空间内数据匹配
     * \param paths
     * 需要进行筛选的文件路径,一般是从空间页面的model获取
     */
    Q_INVOKABLE void setSearchInSpacePaths(const QStringList &paths);

    bool getAiSearch() const;
    void setAiSearch(bool newAiSearch);

    bool getLoading() const;
    void setLoading(bool newLoading);

signals:
    void searchInSpaceChanged();
    void searchKeywordChanged();
    void searchConditionsChanged();
    void currentSpaceIdChanged();

    void setSearchKeywordReqeust(const QString &keyword);

    void aiSearchChanged();

    void loadingChanged();

private:
    SearchResultModel *sourceModel = nullptr;
    int currentSpaceId = -1;

    Q_PROPERTY(bool searchInSpace READ getSearchInSpace WRITE setSearchInSpace NOTIFY searchInSpaceChanged)
    Q_PROPERTY(QString searchKeyword READ getSearchKeyword WRITE setSearchKeyword NOTIFY searchKeywordChanged)
    Q_PROPERTY(QList<QSharedPointer<Condition>> searchConditions READ getSearchConditions WRITE setSearchConditions NOTIFY searchConditionsChanged)
    Q_PROPERTY(int currentSpaceId READ getCurrentSpaceId WRITE setCurrentSpaceId NOTIFY currentSpaceIdChanged)
    Q_PROPERTY(bool aiSearch READ getAiSearch WRITE setAiSearch NOTIFY aiSearchChanged)
    Q_PROPERTY(bool loading READ getLoading WRITE setLoading NOTIFY loadingChanged)
};

#endif // SEARCHRESULTMODEL_H
