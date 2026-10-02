/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#include "searchtask.h"
#include "condition/condition.h"
#include "file/fileinfojob.h"
#include "model/modelcontroller.h"
#include "ai/ailabelhelper.h"

#include <QWriteLocker>
#include <QReadLocker>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileInfo>

#include <QDebug>

SearchTask::SearchTask(QObject *parent)
    : QObject{parent}
{
    m_interface = new QDBusInterface("com.ukui.search.fileindex.service",
                                        "/org/ukui/search/privateDirWatcher",
                                        "org.ukui.search.fileindex",
                                        QDBusConnection::sessionBus(),
                                        this);

    // 检查服务是否可用
    if (!m_interface->isValid()) {
        qWarning() << "DBus interface not valid:" << QDBusConnection::sessionBus().lastError().message();
    }
    ukuiSearchTask = new UkuiSearch::UkuiSearchTask;
    ukuiSearchTask->setInformNum(10);
    ukuiSearchTask->setMaxResultNum();
    ukuiSearchTask->initSearchPlugin(UkuiSearch::SearchProperty::SearchType::File);
    queue = ukuiSearchTask->init();

    connect(ukuiSearchTask, &UkuiSearch::UkuiSearchTask::searchError, this, [=](){
        Q_EMIT searchFinished(false);
    });
    connect(ukuiSearchTask, &UkuiSearch::UkuiSearchTask::reachInformNum, this, &SearchTask::handleSearchResult);
    connect(ukuiSearchTask, &UkuiSearch::UkuiSearchTask::searchFinished, this, [=](){
        qDebug() <<"finish search keyword:=="<<m_searchKeyword<<"==";
        handleSearchResult();
        searchDone = true;
        m_finalTimer->stop();
        Q_EMIT searchFinished(true);
        if(searchContentDone || !needSearchFileContent){
            if (!cancelled)
                Q_EMIT finished();
        }
    });

   ukuiSearchContentTask = new UkuiSearch::UkuiSearchTask;
   ukuiSearchContentTask->setInformNum(10);
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

    m_finalTimer = new QTimer(this);
    m_finalTimer->setSingleShot(true);
    connect(m_finalTimer, &QTimer::timeout, this, &SearchTask::forceEmitResults);
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
    needSearchFileContent = true;
    hasContentSearchAndCondition = false;

    conditionsDescription = newConditionsDescription;

    auto newConditions = Condition::generateContitionsFromDescription(newConditionsDescription);

    auto newLabels = Condition::getLabelsFromDescription(newConditionsDescription);

    rwlock.lockForWrite();

    andConditions.clear();
    orConditions.clear();
    conditions = newConditions;
    labels = newLabels;

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
    updateSearchDirs();
    int FileNameIndex = -1;
    int i = 0;
    bool hasFilemameNotContain = false;
    for (auto condition : conditions) {
        bool shouldAddCondition = true;

        switch (condition->type()) {
        case Condition::FileName: {
            auto keywords = condition->getFileNameKeywords();
            FileNameIndex = i;
            if (keywords.isEmpty()) {
                shouldAddCondition = false;
                break;
            }else{
                m_searchKeyword = keywords.at(0);
            }
            if (condition->getFileNameIsContains()) {
                needSearchFile = true;
                for (auto keyword : keywords) {
                    ukuiSearchTask->addKeyword(keyword);
                }
            }
            else{
                needSearchFile = true;
                needSearchFileContent = false;//如果不包含则不走文件内容搜索流程
                hasFilemameNotContain = true;
                #ifdef UKUI_SEARCH_HAS_MATCH_ALL
                            ukuiSearchTask->setMatchAllIfNoKeyword(true);
                #endif
            }
            break;
        }
        case Condition::FileSize:
        case Condition::FileType: {
            needSearchFile = true;
#ifdef UKUI_SEARCH_HAS_MATCH_ALL
            ukuiSearchTask->setMatchAllIfNoKeyword(true);
#endif
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
#ifdef UKUI_SEARCH_HAS_MATCH_ALL
            ukuiSearchTask->setMatchAllIfNoKeyword(true);
#endif
            hasDateRangeCondition = true;
            break;
        }
        case Condition::ConditionMatchRule: {
            needSearchFile = true;
            needSearchFileContent =  hasFilemameNotContain ? false:(condition->getConditionMatchRule()==0?true:false);
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
        i++;
    }
    if(needSearchFileContent && FileNameIndex !=-1){
        auto nameCondition = conditions[FileNameIndex];
        auto keywords = nameCondition->getFileNameKeywords();
        if(!keywords.isEmpty()){
            ukuiSearchContentTask->addKeyword(keywords.first());
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


    ukuiSearchTask->setRecurse(true);
    ukuiSearchContentTask->setRecurse(true);

    // 清空旧条件的结果
    queue->clear();
    contentQueue->clear();
    resultInfos.clear();
    contentResultInfos.clear();

    Q_EMIT conditionsDescriptionChanged();
}

QStringList SearchTask::getLabels()
{
    return labels;
}

void SearchTask::start()
{
    cancelled = false;
    filesMatchedWithLabels.clear();
    // 如果条件不完全，比如文件名条件未设置文件名，搜索路径未设置路径，则不需要执行搜索
    if (!needSearchFile && !needSearchFileContent && labels.isEmpty()) {
        if (!cancelled)
            Q_EMIT finished();
        return;
    }

     m_finalTimer->start(10000); // 10秒后强制返回结果

    if (!labels.isEmpty()) {
        bool basicConditionsOnlyHasSearchLocation = true;
        for (auto condition : andConditions) {
            if (condition->type() != Condition::SearchLocation) {
                basicConditionsOnlyHasSearchLocation = false;
            }
        }
        for (auto condition : orConditions) {
            if (condition->type() != Condition::SearchLocation) {
                basicConditionsOnlyHasSearchLocation = false;
            }
        }
        // fixme标签搜索异步实现，并且和文件以及内容搜索取交集
        AILabelHelper helper;
        QStringList files = helper.getFilesByLabels(labels);
        filesMatchedWithLabels = QSet<QString>::fromList(files);
        if (!needSearchFile && !needSearchFileContent || basicConditionsOnlyHasSearchLocation) {
            for (auto path : filesMatchedWithLabels) {
                auto info = UKUI::IDM::FileInfo::fromPath(path);
                contentResultInfos << info;
            }
            Q_EMIT finished();
        }
    }
    qDebug() <<"start search keyword:=="<<m_searchKeyword<<"==";
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
    m_finalTimer->stop();
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
        if(cancelled){
            return;
        }
        auto item = queue->dequeue();
        QString path = item.getValue(UkuiSearch::SearchProperty::FilePath).toString();
        if (!labels.isEmpty()) {
            if (!filesMatchedWithLabels.contains(path))
                continue;
        }
        QVariant timeVar = item.getValue(UkuiSearch::SearchProperty::ModifiedTime);
        if (timeVar.isValid()) {
            QDateTime modificationTimeDate = timeVar.toDateTime();
            // fixme: 判断日期相关条件
        }
        // fixme: 判断大小相关条件
        // fixme: 判断类型相关条件

        if (spaceId >= 0) {
            QFileInfo fileinfo(path);
            auto info = UKUI::IDM::FileInfo::fromPath(path);
            if(fileinfo.suffix() == "ogg"){
                if(UKUI::IDM::FileInfoJob::queryInfoSyncWithConditions(info, andConditions, orConditions)){
                    resultInfos << info;
                }
            }else{
                if (UKUI::IDM::FileInfoJob::infoMatchCondtions(fileinfo, andConditions, orConditions)) {
                    auto info = UKUI::IDM::FileInfo::fromPath(path);
                    //ModelController::globalInstance()->fileReload(info->uri(),info->modifyDateString(),info->fileType(),info->sizeFormat());
                    resultInfos << info;
                }
            }
        } else {
            auto info = UKUI::IDM::FileInfo::fromPath(path);
            resultInfos << info;
            if(resultInfos.size() % SEARCH_LOAD_BATCH == 0){
                if (m_finalTimer->isActive()) {
                    m_finalTimer->stop();
                }
                m_finalTimer->start();
                Q_EMIT searchLoadBatch(false);
            }
        }
    }

   while (!contentQueue->isEmpty()) {
        if(cancelled){
            return;
        }
       auto item = contentQueue->dequeue();
       QString path = item.getValue(UkuiSearch::SearchProperty::FilePath).toString();
       if (!labels.isEmpty()) {
           if (!filesMatchedWithLabels.contains(path))
               continue;
       }
       QVariant timeVar = item.getValue(UkuiSearch::SearchProperty::ModifiedTime);
       if (timeVar.isValid()) {
           QDateTime modificationTimeDate = timeVar.toDateTime();
           // fixme: 判断日期相关条件
       }
       // fixme: 判断大小相关条件
       // fixme: 判断类型相关条件

       if (spaceId >= 0) {
           QFileInfo fileinfo(path);
           //file content result don't need check filename condition
           QList<std::shared_ptr<Condition>> realandConditions;
           QList<std::shared_ptr<Condition>> realOrConditions;
           for (auto condition:andConditions) {
               if(condition->type()!= Condition::FileName){
                   realandConditions<<condition;
               }
           }
           for (auto condition:orConditions) {
               if(condition->type()!= Condition::FileName){
                   realOrConditions<<condition;
               }
           }
           if (UKUI::IDM::FileInfoJob::infoMatchCondtions(fileinfo, realandConditions, realOrConditions)) {
               auto info = UKUI::IDM::FileInfo::fromPath(path);
               contentResultInfos << info;
           }
       } else {
           auto info = UKUI::IDM::FileInfo::fromPath(path);
           contentResultInfos << info;
       }
   }
}
void SearchTask::updateSearchDirs(){
    ukuiSearchTask->clearSearchDir();
    //ukuiSearchContentTask->clearSearchDir();
    QDBusReply<QStringList> indexDirsRpl = m_interface->call("currentSearchDirs");
    if (indexDirsRpl.isValid()) {
        indexDirs = indexDirsRpl.value();
    }
    if(currentSearchpath.isEmpty()){
        ukuiSearchTask->addSearchDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        ukuiSearchContentTask->addSearchDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    }
    else{
        ukuiSearchTask->addSearchDir(currentSearchpath);
        ukuiSearchContentTask->addSearchDir(currentSearchpath);
    }
//    for (auto dir:indexDirs) {
//        ukuiSearchTask->addSearchDir(dir);
//    }
}

void SearchTask::setSearchPath(const QString path)
{
    currentSearchpath = path;
}

void SearchTask::setNeedSearchFile(bool flag)
{
    needSearchFile = flag;
}

void SearchTask::forceEmitResults() {
    if (cancelled)
        return;
    qDebug() << "10秒定时器触发，强制返回当前搜索结果";
    while (!queue->isEmpty()) {
        if(cancelled){
            return;
        }
        auto item = queue->dequeue();
        QString path = item.getValue(UkuiSearch::SearchProperty::FilePath).toString();
        if (!labels.isEmpty()) {
            if (!filesMatchedWithLabels.contains(path))
                continue;
        }
        if (spaceId >= 0) {
            QFileInfo fileinfo(path);
            if (UKUI::IDM::FileInfoJob::infoMatchCondtions(fileinfo, andConditions, orConditions)) {
                auto info = UKUI::IDM::FileInfo::fromPath(path);
                //ModelController::globalInstance()->fileReload(info->uri(),info->modifyDateString(),info->fileType(),info->sizeFormat());
                resultInfos << info;
            }
        } else {
            auto info = UKUI::IDM::FileInfo::fromPath(path);
            resultInfos << info;
        }
    }
    // 强制发送当前所有结果
    if (!resultInfos.isEmpty()) {
        Q_EMIT searchLoadBatch(false,true);
    }
}
