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
#include "file/fileinfojob.h"

#include "ai/aisearchtask.h"

#include <QTimer>
#include <QThread>
#include <QJsonObject>
#include <QDebug>
#include <QIcon>

//SearchResultModel *SearchResultModel::globalInstance()
//{
//    if (!instance) {
//        instance = new SearchResultModel;
//    }
//    return instance;
//}

SearchResultModel::SearchResultModel(QObject *parent)
    : QObject(parent)
{
    delayer = new QTimer;
    delayer->setSingleShot(true);
    delayer->setInterval(500);
    connect(delayer, &QTimer::timeout, this, &SearchResultModel::doUpdateInternal);

    connect(this, &SearchResultModel::searchKeywordChanged, this, &SearchResultModel::delayUpdate);
//    connect(this, &SearchResultModel::searchInSpaceChanged, this, &SearchResultModel::delayUpdate);
//    connect(this, &SearchResultModel::searchConditionsChanged, this, &SearchResultModel::delayUpdate);
//    connect(this, &SearchResultModel::labelsChanged, this, &SearchResultModel::delayUpdate);
//    connect(this, &SearchResultModel::currentSpaceIdChanged, this, &SearchResultModel::delayUpdate);
//    connect(this, &SearchResultModel::aiSearchChanged, this, &SearchResultModel::delayUpdate);
//    connect(this, &SearchResultModel::updateSearchResult, this, &SearchResultModel::delayUpdate);

    searchTask = new SearchTask;

    QString currentSearhPath = ModelController::globalInstance()->getCurrentSearchpath();
    if(!currentSearhPath.isEmpty()){
        searchTask->setSearchPath(currentSearhPath);
    }
    connect(ModelController::globalInstance(), &ModelController::cancelSearchRequest, searchTask, &SearchTask::cancel);
    connect(searchTask, &SearchTask::searchFinished, this, [=]{
        Q_EMIT searchTask->searchLoadBatch(true);
    });
    connect(searchTask, &SearchTask::searchLoadBatch, this, [=](bool finish,bool forceupdate){
        if(isFirstbatch){
            qDebug() <<"first batch search keyword:=="<<searchKeyword;
            isFirstbatch = false;
        }
        int count = searchTask->resultInfos.size();
        int start = lastUpdateCount;
        int length = count - lastUpdateCount;
        if(forceupdate){           
            forceUpdateCount += length;
        }else{
            if (!searchInSpace) {
                if(finish){
                    // if(count < SEARCH_LOAD_BATCH){
                    //     if(forceUpdateCount == count){
                    //         length = 0;
                    //     }else{
                    //         start = lastUpdateCount;
                    //         length = count - lastUpdateCount;
                    //     }
                    // }else{
                    //     length = count - lastUpdateCount;
                    // }
                    m_loadNameMatchflefinish = true;
                    m_allCount = searchTask->resultInfos.size();
                    qDebug() << "match files count" << count;
                }
            }else{
                if(finish){
                    m_loadNameMatchflefinish = true;
                    m_allCount +=  spaceBatchFileinfoList.size();
                }
            }
        }
        auto info = searchInSpace? spaceBatchFileinfoList:searchTask->resultInfos.mid(start,length);
        QSet<QSharedPointer<UKUI::IDM::FileInfo>> set = QSet<QSharedPointer<UKUI::IDM::FileInfo>>::fromList(info);
        QStringList list;
        nameMatchFiles = set.values();
        for(auto fileinfo:nameMatchFiles){
            list<<fileinfo->uri();
        }
        m_loadNameMatchFileCount += nameMatchFiles.size();
        ModelController::globalInstance()->fileReloadByBatch(list,m_loadNameMatchflefinish && m_allCount == m_loadNameMatchFileCount);
        lastUpdateCount = searchTask->resultInfos.size();
    //     auto searchJob = UKUI::IDM::FileInfoJob::fromInfos(set.values(),false);
    //     searchJob->setAutoDelete(true);
    //     searchJob->setConditions(this->searchConditions);
    //     connect(this, &SearchResultModel::aboutToUpdate, searchJob, &UKUI::IDM::FileInfoJob::cancel);
    //     connect(ModelController::globalInstance(), &ModelController::cancelSearchRequest, searchJob, &UKUI::IDM::FileInfoJob::cancel);
    //     connect(searchJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
    //             nameMatchFiles = searchJob->getInfos();
    //             m_loadNameMatchFileCount += nameMatchFiles.size();
    //             QStringList list;
    //             if (nameMatchFiles.count() > 0){
    //                 setHasResult(true);
    //                 setLoading(false);
    //                 for(auto fileinfo:nameMatchFiles){
    //                     >size())<<QString::number(fileinfo->timeModifiedMsec());
    //                     list.append(fileinfolist);
    //                 }
    //             }
    //             if(m_allCount == m_loadNameMatchFileCount){
    //                 qDebug()<<"########"<<m_allCount << "******"<< m_loadNameMatchFileCount;
    //             }
    //             ModelController::globalInstance()->fileReloadByBatch(list,m_loadNameMatchflefinish && m_allCount == m_loadNameMatchFileCount);
    //         }, Qt::BlockingQueuedConnection);
    //     searchJob->start();
     });

    connect(searchTask, &SearchTask::searchContentFinished, this, [=](bool successed){
        if (!successed){
            if(searchContentFinish){
                QStringList list;
                list = getContentResult();
                ModelController::globalInstance()->SearchContentLoad(list);
            }
            searchTextContentFinish = true;
            return;
        }
        textContentMatchFiles = searchTask->contentResultInfos;
        searchTextContentFinish = true;
        QStringList list;
        if(searchContentFinish){
            list = getContentResult();
            ModelController::globalInstance()->SearchContentLoad(list);
        }
        // auto searchContentJob = UKUI::IDM::FileInfoJob::fromInfos(searchTask->contentResultInfos,false);
        // searchContentJob->setAutoDelete(true);
        // searchContentJob->setConditions(this->searchConditions);
        // connect(this, &SearchResultModel::aboutToUpdate, searchContentJob, &UKUI::IDM::FileInfoJob::cancel);
        // connect(searchContentJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
        //         textContentMatchFiles = searchContentJob->getInfos();
        //         searchTextContentFinish = true;
        //         QStringList list;
        //         if(searchContentFinish){
        //             list = getContentResult();
        //             ModelController::globalInstance()->SearchContentLoad(list);
        //         }
        //     }, Qt::BlockingQueuedConnection);
        // searchContentJob->start();
    });
}

SearchResultModel::~SearchResultModel()
{
    currentContentMatchFiles.clear();
    nameMatchFiles.clear();
    contentMatchFiles.clear();
    textContentMatchFiles.clear();
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
    Q_EMIT searchInSpaceChanged();
}

const QString &SearchResultModel::getSearchKeyword() const
{
    return searchKeyword;
}

void SearchResultModel::setSearchKeyword(const QString &newSearchKeyword)
{
    if (searchKeyword == newSearchKeyword)
        return;
    isFirstbatch = true;
    searchKeyword = newSearchKeyword;    
    reversedConditions.clear();

    auto condition = std::shared_ptr<Condition>(new Condition(Condition::ConditionMatchRule));
    condition->setConditionMatchRule(Condition::NameOrContent);
    condition->setIsAndCondition(false);

    auto condition1 = std::shared_ptr<Condition>(new Condition(Condition::FileName));
    condition1->setFileNameCondition(true, QStringList() << newSearchKeyword);
    condition1->setIsAndCondition(false);

    auto condition2 = std::shared_ptr<Condition>(new Condition(Condition::FileContent));
    condition2->setFileContentCondition(newSearchKeyword);
    condition2->setIsAndCondition(false);

    reversedConditions << condition;
    reversedConditions << condition1;
    if(aiIndexenable && isAiSubsystemavaliable)
        reversedConditions << condition2;
    Q_EMIT searchKeywordChanged();
    currentlabels.clear();
}

const QList<std::shared_ptr<Condition>> &SearchResultModel::getSearchConditions() const
{
    return searchConditions;
}

void SearchResultModel::setSearchConditions(const QList<std::shared_ptr<Condition>> &newSearchConditions)
{
    if (searchConditions == newSearchConditions)
        return;

    searchConditions.clear();

    searchConditions = newSearchConditions;
    Q_EMIT searchConditionsChanged();
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
    Q_EMIT currentSpaceIdChanged();
}

const QList<std::shared_ptr<Condition>> &SearchResultModel::getReversedConditions() const
{
    return reversedConditions;
}

void SearchResultModel::delayUpdate()
{
    aiConditionDescription = QJsonDocument();
    setLoading(true);
    Q_EMIT aboutToUpdate();
    delayer->start();
}

void SearchResultModel::doUpdateInternal()
{
    //this->beginResetModel();
    nameMatchFiles.clear();
    contentMatchFiles.clear();
    setHasResult(false);
    //this->endResetModel();
    if (searchKeyword.isEmpty()) {
        return;
    }   
    Q_EMIT ModelController::globalInstance()->searchKeywordChanged(searchKeyword);
    setLoading(true);

    //qDebug() << "do update internal" << searchInSpace << currentSpaceId << searchKeyword << reversedConditions << searchConditions;

    QList<std::shared_ptr<Condition>> conditions;
    conditions << reversedConditions;
    conditions << searchConditions;
    auto conditionsDescription = Condition::pharseConditions(conditions, labels);

    // fixme: 调用ai接口获取搜索数据，并且获取语义配置的json文件
    qDebug() << "start ai search";
    auto thread = new QThread;
    auto task = new AISearchTask(searchKeyword);
    // 将输入的关键词解析成条件再进行搜索
    task->setPharseConditions(true);
    task->moveToThread(thread);
    connect(thread, &QThread::started, task, &AISearchTask::doQuery);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(this, &SearchResultModel::aboutToUpdate, task, &AISearchTask::cancel);
    connect(ModelController::globalInstance(), &ModelController::cancelSearchRequest, task, &AISearchTask::cancel);
    connect(task, &AISearchTask::finisheds, this, [=](bool successed, const QList<QString> &results, const QJsonDocument &conditions, const QString &errorMsg){
        /*thread->quit();*/}, Qt::QueuedConnection
        );
    connect(task, &AISearchTask::finished, this, [=](bool successed, const QList<QString> &results, const QJsonDocument &conditions, const QString &errorMsg){
        thread->quit();
        if (!successed) {
            QStringList list;
            if(searchTextContentFinish){
                list = getContentResult();
                ModelController::globalInstance()->SearchContentLoad(list);
            }
            searchContentFinish = true;
            return;
        }
        aiConditionDescription = conditions;
        auto job = UKUI::IDM::FileInfoJob::fromPaths(results);
        QList<std::shared_ptr<Condition>> realConditions = searchConditions;
        QString currentSearhPath = ModelController::globalInstance()->getCurrentSearchpath();
        if(!currentSearhPath.isEmpty()){
            auto condition = std::shared_ptr<Condition>(new Condition(Condition::SearchLocation));
            condition->setConditionMatchRule(Condition::NameOrContent);
            condition->setSearchLocationCondition(true,currentSearhPath);
            condition->setIsAndCondition(false);
            realConditions.append(condition);
        }
        job->setConditions(realConditions);
        job->setAutoDelete(true);
        connect(this, &SearchResultModel::aboutToUpdate, job, &UKUI::IDM::FileInfoJob::cancel);
        connect(ModelController::globalInstance(), &ModelController::cancelSearchRequest, job, &UKUI::IDM::FileInfoJob::cancel);
        connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            contentMatchFiles = job->getInfos();
            QStringList list;
            searchContentFinish = true;
            if(searchTextContentFinish){
                list = getContentResult();
                ModelController::globalInstance()->SearchContentLoad(list);
            }
            QSet<QString> uris;
            if (searchInSpace) {
                for (auto path : searchInSpacePaths) {
                   QUrl url = QUrl::fromLocalFile(path);
                   uris << url.toEncoded();
               }
               QList<QSharedPointer<UKUI::IDM::FileInfo>> spacesMatchedInfos;
               for (auto info : contentMatchFiles) {
                   if (uris.contains(info->uri())) {
                       spacesMatchedInfos << info;
                   }
               }
               contentMatchFiles = spacesMatchedInfos;
            }
            if (contentMatchFiles.count() > 0) {
                setHasResult(true);
            }
            setLoading(false);
        }, Qt::BlockingQueuedConnection);
        job->start();
    });
    if(ModelController::globalInstance()->getAinotAvailabletype() == ModelController::allFit)
    {
        thread->start();
    }else{
        searchContentFinish = true;
    }

//    if (!aiSearch) {
        if (!searchInSpace) {
            searchTask->cancel();
            searchTask->setConditionsDescription(conditionsDescription);
            searchTask->start();
        } else {
            searchTask->setConditionsDescription(conditionsDescription);
            searchTask->setNeedSearchFile(false);
            searchTask->start();
            QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
            for (auto path:this->searchInSpacePaths) {
                auto info = UKUI::IDM::FileInfo::fromPath(path);
                infos<<info;
            }
            auto job = UKUI::IDM::FileInfoJob::fromInfos(infos);
            QList<std::shared_ptr<Condition>> allConditions;
            QList<std::shared_ptr<Condition>> conditions;
            allConditions << reversedConditions << searchConditions;
            // for (auto condition:allConditions) {
            //     if(condition->type() == Condition::FileName){
            //         conditions<<condition;
            //     }
            // }
            job->setConditions(allConditions);
            //job->setLabels(labels);
            job->setAutoDelete(true);
            connect(this, &SearchResultModel::aboutToUpdate, job, &UKUI::IDM::FileInfoJob::cancel);
            connect(ModelController::globalInstance(), &ModelController::cancelSearchRequest, job, &UKUI::IDM::FileInfoJob::cancel);
            connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                nameMatchFiles = job->getInfos();
                int i = 0;
                if(nameMatchFiles.isEmpty()){
                    Q_EMIT searchTask->searchLoadBatch(true);
                    return;
                }
                for (auto fileinfo:nameMatchFiles) {
                    spaceBatchFileinfoList<<fileinfo;
                    i++;
                    if(spaceBatchFileinfoList.size() % SEARCH_LOAD_BATCH == 0){
                        bool isfinish = (i == nameMatchFiles.size());
                        Q_EMIT searchTask->searchLoadBatch(isfinish);
                        spaceBatchFileinfoList.clear();
                    }else if(i == nameMatchFiles.size()){
                        Q_EMIT searchTask->searchLoadBatch(true);
                    }
                }
            },  Qt::BlockingQueuedConnection);
            job->start();
        }
/*    } else {
        // fixme: 调用ai接口获取搜索数据，并且获取语义配置的json文件
        qDebug() << "start ai search";
        auto thread = new QThread;
        auto task = new AISearchTask(searchKeyword);
        // 将输入的关键词解析成条件再进行搜索
        task->setPharseConditions(true);
        task->moveToThread(thread);
        connect(thread, &QThread::started, task, &AISearchTask::doQuery);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(this, &SearchResultModel::aboutToUpdate, task, &AISearchTask::cancel);
        connect(task, &AISearchTask::finished, this, [=](bool successed, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &results, const QJsonDocument &conditions, const QString &errorMsg){
            thread->quit();
            if (!successed) {
                return;
            }
            aiConditionDescription = conditions;
            auto job = UKUI::IDM::FileInfoJob::fromInfos(results);
            job->setAutoDelete(true);
            connect(this, &SearchResultModel::aboutToUpdate, job, &UKUI::IDM::FileInfoJob::cancel);
            connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                this->beginResetModel();
                contentMatchFiles = job->getInfos();
                if (contentMatchFiles.count() > 0) {
                    setHasResult(true);
                }
                this->endResetModel();
                setLoading(false);
            }, Qt::BlockingQueuedConnection);
            job->start();
        });
        thread->start();
    }*/
}

void SearchResultModel::saveSearchAsSpace(const QString &spaceName)
{
    // fixme: 当搜索结未完成时不应该保存
    QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
    infos << nameMatchFiles;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> nameMatcheFilesWithoutfilename;

    for(auto info:contentMatchFiles)
    {
        if(!nameMatchFiles.contains(info))
        {
            nameMatcheFilesWithoutfilename<<info;
        }
    }
    infos << nameMatcheFilesWithoutfilename;
    QList<std::shared_ptr<Condition>> conditions;
    conditions << reversedConditions;
    conditions << searchConditions;
    auto conditonsDescription = Condition::pharseConditions(conditions).toJsonObject();
    QJsonDocument doc(conditonsDescription);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);
    QStringList paths;
    for (auto info:infos) {
        paths<<info->path();
    }
    ModelController::globalInstance()->onSpaceCreateRequestWithFiles(spaceName, jsonStr, true, paths);
}

bool SearchResultModel::getHasResult() const
{
    return hasResult;
}

void SearchResultModel::setHasResult(bool newHasResult)
{
    if (hasResult == newHasResult)
        return;
    hasResult = newHasResult;
    Q_EMIT hasResultChanged();
}

QStringList SearchResultModel::getContentResult()
{
    //取智能内容识别和文本内容并集
    QStringList list;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> result;
    result<<contentMatchFiles<<textContentMatchFiles;
    for(auto fileinfo:result){
        auto path = fileinfo->path();
        if(searchInSpace && !this->searchInSpacePaths.contains(path)){//search in space
            continue;
        }
        list<<fileinfo->uri();
    }
    QSet<QString> set = QSet<QString>::fromList(list);
    return set.values();
}

const QStringList &SearchResultModel::getLabels() const
{
    return labels;
}

void SearchResultModel::setLabels(const QStringList &newLabels)
{
    if (labels == newLabels)
        return;
    labels = newLabels;
    Q_EMIT labelsChanged();
}

const QStringList &SearchResultModel::getcurrentLabels() const
{
    return currentlabels;
}

void SearchResultModel::setcurrentLabels(const QStringList &newLabels)
{
    if (currentlabels == newLabels)
        return;
    currentlabels = newLabels;
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
    Q_EMIT loadingChanged();
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
    Q_EMIT aiSearchChanged();
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
