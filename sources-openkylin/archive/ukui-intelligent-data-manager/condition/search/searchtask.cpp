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

#include "searchtask.h"

#include "conditioneditor.h"
#include "fileinfojob.h"

#include <QWriteLocker>
#include <QReadLocker>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileInfo>

#include <QDebug>

SearchTask::SearchTask(QObject *parent)
    : QObject{parent}
{
    ukuiSearchTask = new UkuiSearch::UkuiSearchTask;
    ukuiSearchTask->setInformNum(100);
    ukuiSearchTask->setMaxResultNum();
    ukuiSearchTask->initSearchPlugin(UkuiSearch::SearchProperty::SearchType::File);
    queue = ukuiSearchTask->init();

    connect(ukuiSearchTask, &UkuiSearch::UkuiSearchTask::searchError, this, [=](){
        Q_EMIT searchFinished(false);
    });
    connect(ukuiSearchTask, &UkuiSearch::UkuiSearchTask::reachInformNum, this, &SearchTask::handleSearchResult);
    connect(ukuiSearchTask, &UkuiSearch::UkuiSearchTask::searchFinished, this, [=](){
        handleSearchResult();
        searchDone = true;
        Q_EMIT searchFinished(true);

        if (searchContentDone || !needSearchFileContent) {
            if (!cancelled)
                Q_EMIT finished();
        }
    });

    ukuiSearchContentTask = new UkuiSearch::UkuiSearchTask;
    ukuiSearchContentTask->setInformNum(100);
    ukuiSearchContentTask->setMaxResultNum();
    ukuiSearchContentTask->initSearchPlugin(UkuiSearch::SearchProperty::SearchType::FileContent);
    contentQueue = ukuiSearchContentTask->init();

    connect(ukuiSearchContentTask, &UkuiSearch::UkuiSearchTask::searchError, this, [=](size_t searchId, QString msg){
        qCritical() << "search content failed," << searchId << msg;
        Q_EMIT searchContentFinished(false);

        searchContentDone = true;

        if (searchDone || !needSearchFile) {
            if (!cancelled)
                Q_EMIT finished();
        }
    });
    connect(ukuiSearchContentTask, &UkuiSearch::UkuiSearchTask::reachInformNum, this, &SearchTask::handleSearchResult);
    connect(ukuiSearchContentTask, &UkuiSearch::UkuiSearchTask::searchFinished, this, [=](){
        handleSearchResult();
        searchContentDone = true;
        Q_EMIT searchContentFinished(true);

        if (searchDone || !needSearchFile) {
            if (!cancelled)
                Q_EMIT finished();
        }
    });
}

void SearchTask::setSpaceId(int spaceId)
{
    this->spaceId = spaceId;
}

SearchTask::~SearchTask()
{
    ukuiSearchTask->stop();
    queue->clear();
    delete ukuiSearchTask;
    ukuiSearchContentTask->stop();
    contentQueue->clear();
    delete ukuiSearchContentTask;
    resultInfos.clear();
    contentResultInfos.clear();
}

const QVariant &SearchTask::getConditionsDescription() const
{
    return conditionsDescription;
}

void SearchTask::setConditionsDescription(const QVariant &newConditionsDescription)
{
    if (conditionsDescription == newConditionsDescription)
        return;

    needSearchFile = false;
    needSearchFileContent = false;
    hasContentSearchAndCondition = false;

    conditionsDescription = newConditionsDescription;

    auto newConditions = ConditionEditor::generateContitionsFromDescription(newConditionsDescription);

    rwlock.lockForWrite();

    andConditions.clear();
    orConditions.clear();
    conditions = newConditions;

    if (ukuiSearchTask->isSearching(UkuiSearch::SearchProperty::SearchType::File)) {
        ukuiSearchTask->stop();
    }
    ukuiSearchTask->clearAllConditions();

    if (ukuiSearchContentTask->isSearching(UkuiSearch::SearchProperty::SearchType::FileContent)) {
        ukuiSearchContentTask->stop();
    }
    ukuiSearchContentTask->clearAllConditions();

    UkuiSearch::SearchResultProperties properties = {UkuiSearch::SearchProperty::FilePath};
    bool hasDateRangeCondition = false;
    bool hasSearchLocationCondition = false;

    for (auto condition : conditions) {
        bool shouldAddCondition = true;

        switch (condition->type()) {
        case Condition::FileName: {
            auto keywords = condition->getFileNameKeywords();
            if (keywords.isEmpty()) {
                shouldAddCondition = false;
                break;
            }
            if (condition->getFileNameIsContains()) {
                needSearchFile = true;
                for (auto keyword : keywords) {
                    ukuiSearchTask->addKeyword(keyword);
                }
            }
            break;
        }
        case Condition::FileSize:
        case Condition::FileType: {
            needSearchFile = true;
            ukuiSearchTask->setMatchAllIfNoKeyword(true);
            break;
        }
        case Condition::FileContent: {
            auto contentKeyword = condition->getFileContentContent();
            if (contentKeyword.isEmpty()) {
                shouldAddCondition = false;
                break;
            }
            if (condition->isAndCondition()) {
                hasContentSearchAndCondition = true;
            }
            needSearchFileContent = true;
            ukuiSearchContentTask->addKeyword(condition->getFileContentContent());
            break;
        }
        case Condition::SearchLocation: {
            auto path = condition->getSearchLocationPath();
            if (path.isEmpty()) {
                shouldAddCondition = false;
                break;
            }
            needSearchFile = true;
            //ukuiSearchTask->setMatchAllIfNoKeyword(true);
            if (condition->getSearchLocationIsContains()) {
                ukuiSearchTask->addSearchDir(path);
                ukuiSearchContentTask->addSearchDir(path);
            }
            break;
        }
        case Condition::DateRange: {
            // 在线程中查询时间信息
            needSearchFile = true;
            ukuiSearchTask->setMatchAllIfNoKeyword(true);
            hasDateRangeCondition = true;
            break;
        }
        case Condition::ConditionMatchRule: {
            needSearchFile = true;
            needSearchFileContent = true;
            break;
        }
        default:
            break;
        }

        if (condition->isAndCondition()) {
            andConditions << condition;
        } else {
            orConditions << condition;
        }
    }

    if (hasContentSearchAndCondition) {
        // 如果包含了与逻辑的文件内容关键字条件，则不进行文件名搜索
        needSearchFile = false;
    }

    rwlock.unlock();

    // 根据条件对搜索task进行设置
    if (hasDateRangeCondition)
        properties.append(UkuiSearch::SearchProperty::ModifiedTime);

    ukuiSearchTask->setResultProperties(UkuiSearch::SearchProperty::SearchType::File, properties);
    ukuiSearchContentTask->setResultProperties(UkuiSearch::SearchProperty::SearchType::FileContent, properties);

    if (!hasSearchLocationCondition) {
        ukuiSearchTask->addSearchDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        ukuiSearchContentTask->addSearchDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    }

    ukuiSearchTask->setRecurse(true);
    ukuiSearchContentTask->setRecurse(true);

    // 清空旧条件的结果
    queue->clear();
    contentQueue->clear();
    resultInfos.clear();
    contentResultInfos.clear();

    emit conditionsDescriptionChanged();
}

void SearchTask::start()
{
    cancelled = false;
    // 如果条件不完全，比如文件名条件未设置文件名，搜索路径未设置路径，则不需要执行搜索
    if (!needSearchFile && !needSearchFileContent) {
        if (!cancelled)
            Q_EMIT finished();
        return;
    }

    if (needSearchFile)
        ukuiSearchTask->startSearch(UkuiSearch::SearchProperty::SearchType::File);

    if (needSearchFileContent)
        ukuiSearchContentTask->startSearch(UkuiSearch::SearchProperty::SearchType::FileContent);
}

void SearchTask::cancel()
{
    cancelled = true;

    if (needSearchFile)
        ukuiSearchTask->stop();

    if (needSearchFileContent)
        ukuiSearchContentTask->stop();

    resultInfos.clear();
    contentResultInfos.clear();
}

void SearchTask::handleCancelWithSpaceId(int spaceId)
{
    if (spaceId == this->spaceId) {
        cancel();
        deleteLater();
    }
}

void SearchTask::handleSearchResult()
{
    while (!queue->isEmpty()) {
        auto item = queue->dequeue();
        QString path = item.getValue(UkuiSearch::SearchProperty::FilePath).toString();
        QVariant timeVar = item.getValue(UkuiSearch::SearchProperty::ModifiedTime);
        if (timeVar.isValid()) {
            QDateTime modificationTimeDate = timeVar.toDateTime();
            // fixme: 判断日期相关条件
        }
        // fixme: 判断大小相关条件
        // fixme: 判断类型相关条件

        if (spaceId >= 0) {
            QFileInfo fileinfo(path);
            if (UKUI::IDM::FileInfoJob::infoMatchCondtions(fileinfo, andConditions, orConditions)) {
                auto info = UKUI::IDM::FileInfo::fromPath(path);
                resultInfos << info;
            }
        } else {
            auto info = UKUI::IDM::FileInfo::fromPath(path);
            resultInfos << info;
        }
    }

    while (!contentQueue->isEmpty()) {
        auto item = contentQueue->dequeue();
        QString path = item.getValue(UkuiSearch::SearchProperty::FilePath).toString();
        QVariant timeVar = item.getValue(UkuiSearch::SearchProperty::ModifiedTime);
        if (timeVar.isValid()) {
            QDateTime modificationTimeDate = timeVar.toDateTime();
            // fixme: 判断日期相关条件
        }
        // fixme: 判断大小相关条件
        // fixme: 判断类型相关条件

        if (spaceId >= 0) {
            QFileInfo fileinfo(path);
            if (UKUI::IDM::FileInfoJob::infoMatchCondtions(fileinfo, andConditions, orConditions)) {
                auto info = UKUI::IDM::FileInfo::fromPath(path);
                contentResultInfos << info;
            }
        } else {
            auto info = UKUI::IDM::FileInfo::fromPath(path);
            contentResultInfos << info;
        }
    }
}
