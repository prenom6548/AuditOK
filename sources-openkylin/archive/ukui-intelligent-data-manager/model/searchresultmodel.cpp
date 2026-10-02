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

#include "searchresultmodel.h"

#include "modelcontroller.h"
#include "condition/conditioneditor.h"
#include "file/fileinfojob.h"

#include <QTimer>
#include <QDebug>

static SearchResultModel *instance = nullptr;
static SearchResultProxyModel *proxyInstance = nullptr;

SearchResultModel *SearchResultModel::globalInstance()
{
    if (!instance) {
        instance = new SearchResultModel;
    }
    return instance;
}

SearchResultModel::SearchResultModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    delayer = new QTimer;
    delayer->setSingleShot(true);
    delayer->setInterval(500);
    connect(delayer, &QTimer::timeout, this, &SearchResultModel::doUpdateInternal);

    connect(this, &SearchResultModel::searchKeywordChanged, this, &SearchResultModel::delayUpdate);
    connect(this, &SearchResultModel::searchInSpaceChanged, this, &SearchResultModel::delayUpdate);
    connect(this, &SearchResultModel::searchConditionsChanged, this, &SearchResultModel::delayUpdate);
    connect(this, &SearchResultModel::currentSpaceIdChanged, this, &SearchResultModel::delayUpdate);
    connect(this, &SearchResultModel::aiSearchChanged, this, &SearchResultModel::delayUpdate);

    searchTask = new SearchTask;

    connect(searchTask, &SearchTask::searchFinished, this, [=]{
        auto searchJob = UKUI::IDM::FileInfoJob::fromInfos(searchTask->resultInfos);
        searchJob->setAutoDelete(true);
        searchJob->setConditions(this->searchConditions);
        connect(this, &SearchResultModel::aboutToUpdate, searchJob, &UKUI::IDM::FileInfoJob::cancel);
        connect(searchJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            beginResetModel();
            nameMatchFiles = searchJob->getInfos();
            qDebug() << "match files count" << nameMatchFiles.count();
            endResetModel();
            setLoading(false);
        }, Qt::BlockingQueuedConnection);
        searchJob->start();
    });

    connect(searchTask, &SearchTask::searchContentFinished, this, [=](bool successed){
        if (!successed)
            return;
        auto searchContentJob = UKUI::IDM::FileInfoJob::fromInfos(searchTask->contentResultInfos);
        searchContentJob->setAutoDelete(true);
        searchContentJob->setConditions(this->searchConditions);
        connect(this, &SearchResultModel::aboutToUpdate, searchContentJob, &UKUI::IDM::FileInfoJob::cancel);
        connect(searchContentJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            beginResetModel();
            contentMatchFiles = searchContentJob->getInfos();
            qDebug() << "match content files count" << contentMatchFiles.count();
            endResetModel();
            setLoading(false);
        }, Qt::BlockingQueuedConnection);
        searchContentJob->start();
    });
}

QVariant SearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            switch (section) {
            case 0: {
                return tr("File Name");
            }
            case 1: {
                return tr("File Size");
            }
            case 2: {
                return tr("Time Info");
            }
            }
            break;
        }
        }
    }
    return QVariant();
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        // 0为文件名匹配, 分配id为1, 1为内容匹配, 分配id为2
        return createIndex(row, column, row + 1);
    } else {
        // 如果是文件名匹配,则为奇数,从3开始,如果是文件内容匹配则为偶数,从4开始
        bool isFileChildren = (parent.internalId() == 1);
        bool isContentChildren = (parent.internalId() == 2);
        if (isFileChildren) {
            return createIndex(row, column, (row + 1) * 2 + 1);
        } else if (isContentChildren) {
            return createIndex(row, column, (row + 1) * 2 + 2);
        }
    }

    return QModelIndex();
}

QModelIndex SearchResultModel::parent(const QModelIndex &index) const
{
    if (index.internalId() <= 2) {
        return QModelIndex();
    } else {
        bool isFileChildren = ((index.internalId() % 2) == 1);
        if (isFileChildren) {
            return createIndex(0, index.column(), 1);
        } else {
            return createIndex(1, index.column(), 2);
        }
    }

    return QModelIndex();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 2;

    if (parent.isValid() && !parent.parent().isValid()) {
        if (parent.row() == 0) {
            return nameMatchFiles.count();
        } else if (parent.row() == 1) {
            return contentMatchFiles.count();
        }
    }

    return 0;
}

int SearchResultModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

bool SearchResultModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.internalId() <= 2) {
        return true;
    }
    return false;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto internalId = index.internalId();
    if (internalId <= 2) {
        bool isFile = internalId == 1;
        switch (index.column()) {
        case 0: {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole: {
                return isFile? tr("File Name Match"): tr("File Content Match");
            }
            default:
                return QVariant();
            }
        }
        default:
            return QVariant();
        }
    } else {
        QSharedPointer<UKUI::IDM::FileInfo> info;
        bool isFileChildren = ((internalId % 2) == 1);
        if (isFileChildren) {
            int realRow = ((internalId - 3) / 2);
            info = nameMatchFiles.at(realRow);
        } else {
            int realRow = ((internalId - 4) / 2);
            info = contentMatchFiles.at(realRow);
        }

        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
            switch (index.column()) {
            case 0: {
                return info->displayName();
            }
            case 1: {
                return info->sizeFormat();
            }
            case 2: {
                return info->dateString();
            }
            default:
                break;
            }
        }
        case Qt::UserRole + 1: {
            return info->path();
        }
        case Qt::UserRole + 4: {
            return info->dateString();
        }
        case Qt::UserRole + 7: {
            return info->sizeFormat();
        }
        case Qt::DecorationRole: {
            if (index.column() == 0) {
                return QIcon::fromTheme(info->iconName());
            }
        }
        default:
            break;
        }
    }

    return QVariant();
}

bool SearchResultModel::getSearchInSpace() const
{
    return searchInSpace;
}

void SearchResultModel::setSearchInSpace(bool newSearchInSpace)
{
    if (searchInSpace == newSearchInSpace)
        return;
    searchInSpace = newSearchInSpace;
    emit searchInSpaceChanged();
}

const QString &SearchResultModel::getSearchKeyword() const
{
    return searchKeyword;
}

void SearchResultModel::setSearchKeyword(const QString &newSearchKeyword)
{
    if (searchKeyword == newSearchKeyword)
        return;
    searchKeyword = newSearchKeyword;

    for (auto condition : reversedConditions) {
        condition->deleteLater();
    }
    reversedConditions.clear();

    auto condition = QSharedPointer<Condition>(new Condition(Condition::ConditionMatchRule));
    condition->setConditionMatchRule(Condition::NameOrContent);
    condition->setIsAndCondition(false);

    auto condition1 = QSharedPointer<Condition>(new Condition(Condition::FileName));
    condition1->setFileNameCondition(true, QStringList() << newSearchKeyword);
    condition1->setIsAndCondition(false);

    auto condition2 = QSharedPointer<Condition>(new Condition(Condition::FileContent));
    condition2->setFileContentCondition(newSearchKeyword);
    condition2->setIsAndCondition(false);

    reversedConditions << condition;
    reversedConditions << condition1;
    reversedConditions << condition2;

    emit searchKeywordChanged();
}

const QList<QSharedPointer<Condition>> &SearchResultModel::getSearchConditions() const
{
    return searchConditions;
}

void SearchResultModel::setSearchConditions(const QList<QSharedPointer<Condition>> &newSearchConditions)
{
    if (searchConditions == newSearchConditions)
        return;

    searchConditions.clear();

    searchConditions = newSearchConditions;
    emit searchConditionsChanged();
}

int SearchResultModel::getCurrentSpaceId() const
{
    return currentSpaceId;
}

void SearchResultModel::setCurrentSpaceId(int newCurrentSpaceId)
{
    if (currentSpaceId == newCurrentSpaceId)
        return;
    currentSpaceId = newCurrentSpaceId;
    emit currentSpaceIdChanged();
}

const QList<QSharedPointer<Condition>> &SearchResultModel::getReversedConditions() const
{
    return reversedConditions;
}

void SearchResultModel::delayUpdate()
{
    Q_EMIT aboutToUpdate();
    delayer->start();
}

void SearchResultModel::doUpdateInternal()
{
    this->beginResetModel();
    nameMatchFiles.clear();
    contentMatchFiles.clear();
    this->endResetModel();

    if (searchKeyword.isEmpty()) {
        return;
    }

    setLoading(true);

    qDebug() << "do update internal" << searchInSpace << currentSpaceId << searchKeyword << reversedConditions << searchConditions;

    QList<QSharedPointer<Condition>> conditions;
    conditions << reversedConditions;
    conditions << searchConditions;
    auto conditionsDescription = ConditionEditor::pharseConditions(conditions);

    if (!searchInSpace) {
        searchTask->cancel();
        searchTask->setConditionsDescription(conditionsDescription);
        searchTask->start();
    } else {
        auto job = UKUI::IDM::FileInfoJob::fromPaths(this->searchInSpacePaths);
        QList<QSharedPointer<Condition>> conditions;
        conditions << reversedConditions << searchConditions;
        job->setConditions(conditions);
        job->setAutoDelete(true);
        connect(this, &SearchResultModel::aboutToUpdate, job, &UKUI::IDM::FileInfoJob::cancel);
        connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            this->beginResetModel();
            nameMatchFiles = job->getInfos();
            // fixme: 用内容搜索的结果结合当前空间内的文件列表进行筛选填充
            //contentMatchFiles = job->getInfos();
            this->endResetModel();
            setLoading(false);
        }, Qt::BlockingQueuedConnection);
        job->start();
    }
}

void SearchResultModel::saveSearchAsSpace()
{
    // fixme: 当搜索结未完成时不应该保存
    QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
    infos << nameMatchFiles;
    infos << contentMatchFiles;
    QList<QSharedPointer<Condition>> conditions;
    conditions << reversedConditions;
    conditions << searchConditions;
    auto conditonsDescription = ConditionEditor::pharseConditions(conditions);
    ModelController::globalInstance()->onSpaceCreateRequestWithFiles(this->getSearchKeyword(), conditonsDescription, true, infos);
}

bool SearchResultModel::getLoading() const
{
    return loading;
}

void SearchResultModel::setLoading(bool newLoading)
{
    if (loading == newLoading)
        return;
    loading = newLoading;
    emit loadingChanged();
}

bool SearchResultModel::getAiSearch() const
{
    return aiSearch;
}

void SearchResultModel::setAiSearch(bool newAiSearch)
{
    if (aiSearch == newAiSearch)
        return;
    aiSearch = newAiSearch;
    emit aiSearchChanged();
}

const QList<QSharedPointer<UKUI::IDM::FileInfo> > &SearchResultModel::getContentMatchFiles() const
{
    return contentMatchFiles;
}

void SearchResultModel::setSearchInSpacePaths(const QStringList &paths)
{
    searchInSpacePaths = paths;
}

const QList<QSharedPointer<UKUI::IDM::FileInfo> > &SearchResultModel::getNameMatchFiles() const
{
    return nameMatchFiles;
}

SearchResultProxyModel *SearchResultProxyModel::globalInstance()
{
    if (!proxyInstance) {
        proxyInstance = new SearchResultProxyModel;
    }
    return proxyInstance;
}

SearchResultProxyModel::SearchResultProxyModel()
{
    this->sourceModel = SearchResultModel::globalInstance();

    connect(this->sourceModel, &SearchResultModel::searchInSpaceChanged, this, &SearchResultProxyModel::searchInSpaceChanged);
    connect(this->sourceModel, &SearchResultModel::searchKeywordChanged, this, &SearchResultProxyModel::searchKeywordChanged);
    connect(this->sourceModel, &SearchResultModel::searchConditionsChanged, this, &SearchResultProxyModel::searchConditionsChanged);
    connect(this->sourceModel, &SearchResultModel::currentSpaceIdChanged, this, &SearchResultProxyModel::currentSpaceIdChanged);
    connect(this->sourceModel, &SearchResultModel::aiSearchChanged, this, &SearchResultProxyModel::aiSearchChanged);
    connect(this->sourceModel, &SearchResultModel::loadingChanged, this, &SearchResultProxyModel::loadingChanged);

    setSourceModel(this->sourceModel);
}

bool SearchResultProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

bool SearchResultProxyModel::getSearchInSpace() const
{
    return this->sourceModel->getSearchInSpace();
}

void SearchResultProxyModel::setSearchInSpace(bool newSearchInSpace)
{
    this->sourceModel->setSearchInSpace(newSearchInSpace);
}

const QString &SearchResultProxyModel::getSearchKeyword() const
{
    return this->sourceModel->getSearchKeyword();
}

void SearchResultProxyModel::setSearchKeyword(const QString &newSearchKeyword)
{
    this->sourceModel->setSearchKeyword(newSearchKeyword);
}

const QList<QSharedPointer<Condition> > SearchResultProxyModel::getSearchConditions() const
{
    return this->sourceModel->getSearchConditions();
}

void SearchResultProxyModel::setSearchConditions(const QList<QSharedPointer<Condition>> &newSearchConditions)
{
    this->sourceModel->setSearchConditions(newSearchConditions);
}

void SearchResultProxyModel::saveSearchAsSpace()
{
    this->sourceModel->saveSearchAsSpace();
}

void SearchResultProxyModel::clearCondition()
{
    this->sourceModel->setSearchConditions(QList<QSharedPointer<Condition>>());
}

int SearchResultProxyModel::getCurrentSpaceId() const
{
    return this->sourceModel->getCurrentSpaceId();
}

void SearchResultProxyModel::setCurrentSpaceId(int newCurrentSpaceId)
{
    this->sourceModel->setCurrentSpaceId(newCurrentSpaceId);
}

void SearchResultProxyModel::setSearchInSpacePaths(const QStringList &paths)
{
    this->sourceModel->setSearchInSpacePaths(paths);
}

bool SearchResultProxyModel::getAiSearch() const
{
    return this->sourceModel->getAiSearch();
}

void SearchResultProxyModel::setAiSearch(bool newAiSearch)
{
    this->sourceModel->setAiSearch(newAiSearch);
}

bool SearchResultProxyModel::getLoading() const
{
    return this->sourceModel->getLoading();
}

void SearchResultProxyModel::setLoading(bool newLoading)
{
    this->sourceModel->setLoading(newLoading);
}
