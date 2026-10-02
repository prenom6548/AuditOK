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
#include <QJsonDocument>

#include "file/fileinfo.h"
#include "condition/condition.h"
#include "search/searchtask.h"

class QTimer;

class SearchResultModel : public QObject
{
    Q_OBJECT
public:
    static SearchResultModel *globalInstance();
    explicit SearchResultModel(QObject *parent = nullptr);
    ~SearchResultModel();
    bool getSearchInSpace() const;
    void setSearchInSpace(bool newSearchInSpace);

    const QString &getSearchKeyword() const;
    void setSearchKeyword(const QString &newSearchKeyword);

    const QList<std::shared_ptr<Condition>> &getSearchConditions() const;
    void setSearchConditions(const QList<std::shared_ptr<Condition>> &newSearchConditions);

    int getCurrentSpaceId() const;
    void setCurrentSpaceId(int newCurrentSpaceId);

    const QList<std::shared_ptr<Condition>> &getReversedConditions() const;

    const QList<QSharedPointer<UKUI::IDM::FileInfo> > &getNameMatchFiles() const;

    const QList<QSharedPointer<UKUI::IDM::FileInfo> > &getContentMatchFiles() const;

    void setSearchInSpacePaths(const QStringList &paths);
    bool getAiSearch() const;
    void setAiSearch(bool newAiSearch);

    bool getLoading() const;
    void setLoading(bool newLoading);

    const QStringList &getLabels() const;
    void setLabels(const QStringList &newLabels);
    const QStringList &getcurrentLabels() const;
    void setcurrentLabels(const QStringList &newLabels);

    bool getHasResult() const;
    void setHasResult(bool newHasResult);
    QStringList getContentResult();
  Q_SIGNALS:
    void searchInSpaceChanged();
    void searchKeywordChanged();
    void searchConditionsChanged();
    void currentSpaceIdChanged();
    void updateSearchResult();
    void aboutToUpdate();

    void aiSearchChanged();

    void loadingChanged();

    void labelsChanged();

    void hasResultChanged();

protected:

protected Q_SLOTS:
    void delayUpdate();
    void doUpdateInternal();
    void saveSearchAsSpace(const QString &spaceName);

private:
    QList<std::shared_ptr<Condition>> reversedConditions;

    bool searchInSpace = false;
    QString searchKeyword = nullptr;
    QList<std::shared_ptr<Condition>> searchConditions;
    QStringList labels;
    QStringList currentlabels;
    int currentSpaceId = -1;
    QJsonDocument aiConditionDescription;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> currentContentMatchFiles;

    bool aiSearch = true;
    bool loading = false;
    bool hasResult = false;
    bool searchContentFinish = false;
    bool searchTextContentFinish = false;
    Q_PROPERTY(bool searchInSpace READ getSearchInSpace WRITE setSearchInSpace NOTIFY searchInSpaceChanged)
    Q_PROPERTY(QString searchKeyword READ getSearchKeyword WRITE setSearchKeyword NOTIFY searchKeywordChanged)
    //Q_PROPERTY(QList<QSharedPointer<Condition>> searchConditions READ getSearchConditions WRITE setSearchConditions NOTIFY searchConditionsChanged)
    Q_PROPERTY(QStringList labels READ getLabels WRITE setLabels NOTIFY labelsChanged)
    Q_PROPERTY(int currentSpaceId READ getCurrentSpaceId WRITE setCurrentSpaceId NOTIFY currentSpaceIdChanged)
    Q_PROPERTY(bool aiSearch READ getAiSearch WRITE setAiSearch NOTIFY aiSearchChanged)
    Q_PROPERTY(bool loading READ getLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool hasResult READ getHasResult WRITE setHasResult NOTIFY hasResultChanged)

    QList<QSharedPointer<UKUI::IDM::FileInfo>> nameMatchFiles;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> contentMatchFiles;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> textContentMatchFiles;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> spaceBatchFileinfoList;
    QStringList searchInSpacePaths;
    QTimer *delayer = nullptr;
    int m_loadNameMatchFileCount = 0;
    bool m_loadNameMatchflefinish = false;
    int m_allCount = 0;
    SearchTask *searchTask = nullptr;
    bool aiIndexenable = false;
    bool isAiSubsystemavaliable = false;
    bool isFirstbatch = true;
    int  lastUpdateCount = 0;
    int  forceUpdateCount = 0;
};

//class SearchResultProxyModel : public QSortFilterProxyModel
//{
//    Q_OBJECT
//public:
//    static SearchResultProxyModel *globalInstance();
//    explicit SearchResultProxyModel();

//    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
//    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

//    bool getSearchInSpace() const;
//    void setSearchInSpace(bool newSearchInSpace);

//    const QString &getSearchKeyword() const;
//    void setSearchKeyword(const QString &newSearchKeyword);

//    const QList<QSharedPointer<Condition> > getSearchConditions() const;
//    void setSearchConditions(const QList<QSharedPointer<Condition> > &newSearchConditions);

//    Q_INVOKABLE void saveSearchAsSpace(const QString &spaceName);
//    Q_INVOKABLE void clearCondition();
//    Q_INVOKABLE void updateSearchResult();

//    int getCurrentSpaceId() const;
//    void setCurrentSpaceId(int newCurrentSpaceId);

//    /*!
//     * \brief setSearchInSpacePaths
//     * 搜索界面切换空间时设置的数据源,用于进行空间内数据匹配
//     * \param paths
//     * 需要进行筛选的文件路径,一般是从空间页面的model获取
//     */
//    Q_INVOKABLE void setSearchInSpacePaths(const QStringList &paths);

//    bool getAiSearch() const;
//    void setAiSearch(bool newAiSearch);

//    bool getLoading() const;
//    void setLoading(bool newLoading);

//    const QStringList &getLabels() const;
//    void setLabels(const QStringList &newLabels);
//    const QStringList &getcurrentLabels() const;
//    void setcurrentLabels(const QStringList &newLabels);

//    bool getHasResult() const;
//    void setHasResult(bool newHasResult);
//    Q_INVOKABLE void setAiavaliabtype(int type);

//Q_SIGNALS:
//    void searchInSpaceChanged();
//    void searchKeywordChanged();
//    void searchConditionsChanged();
//    void currentSpaceIdChanged();

//    void setSearchKeywordReqeust(const QString &keyword);

//    void aiSearchChanged();

//    void loadingChanged();

//    void labelsChanged();

//    void hasResultChanged();

//    //void searchend(QString path, QIcon icon,bool isuseadd,bool hasResult);

//private:
//    SearchResultModel *sourceModel = nullptr;
//    int currentSpaceId = -1;
//    QStringList labels;

//    Q_PROPERTY(bool searchInSpace READ getSearchInSpace WRITE setSearchInSpace NOTIFY searchInSpaceChanged)
//    Q_PROPERTY(QString searchKeyword READ getSearchKeyword WRITE setSearchKeyword NOTIFY searchKeywordChanged)
//    Q_PROPERTY(QList<QSharedPointer<Condition>> searchConditions READ getSearchConditions WRITE setSearchConditions NOTIFY searchConditionsChanged)
//    Q_PROPERTY(int currentSpaceId READ getCurrentSpaceId WRITE setCurrentSpaceId NOTIFY currentSpaceIdChanged)
//    Q_PROPERTY(bool aiSearch READ getAiSearch WRITE setAiSearch NOTIFY aiSearchChanged)
//    Q_PROPERTY(bool loading READ getLoading WRITE setLoading NOTIFY loadingChanged)
//    Q_PROPERTY(QStringList labels READ getLabels WRITE setLabels NOTIFY labelsChanged)
//    Q_PROPERTY(bool hasResult READ getHasResult WRITE setHasResult NOTIFY hasResultChanged)
//};

#endif // SEARCHRESULTMODEL_H
