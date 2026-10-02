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

#include "spacesfilesfetcher.h"

#include "../database/databasemanager.h"
#include "../condition/condition.h"
#include "../ai/semantickbmanager.h"


#ifdef COMPLIE_AS_PROJECT
#include "file/fileinfojob.h"
#include "file/fileinfomanager.h"
#include "modelcontroller.h"
#endif

#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QReadLocker>
#include <QWriteLocker>
#include <QTimer>

SpacesFilesFetcher::SpacesFilesFetcher(const QString &dbName,const QString &filedbname, QObject *parent) : QObject(parent), dbName(dbName),filedbName(filedbname)
{
    delayer = new QTimer(this);
    delayer->setInterval(300);
    delayer->setSingleShot(true);

    connect(delayer, &QTimer::timeout, this, &SpacesFilesFetcher::handleFilesCreatedInternal);
}

QStringList SpacesFilesFetcher::getAllSpacesName()
{
    QReadLocker r(&rwLock);
    return spaces.values();
}

int SpacesFilesFetcher::getSpaceIdByName(const QString &spaceName)
{
    QReadLocker r(&rwLock);
    return spaces.key(spaceName, -1);
}

QString SpacesFilesFetcher::getSpaceNameById(int spaceId)
{
    QReadLocker r(&rwLock);
    return spaces.value(spaceId);
}


QStringList SpacesFilesFetcher::getFilesInSpace(const QString &spaceName, bool isUserAdded)
{
    int spaceId = getSpaceIdByName(spaceName);
    return getFilesInSpaceById(spaceId, isUserAdded);
}

QStringList SpacesFilesFetcher::getFilesInSpaceById(int spaceId, bool isUserAdded)
{
    QReadLocker r(&rwLock);
    return isUserAdded? userAddedInfos.value(spaceId): infos.value(spaceId);
}
QList<std::shared_ptr<Condition>> SpacesFilesFetcher::getSpaceconditon(QString spaceName){

    QReadLocker r(&rwLock);
    int spaceId = getSpaceIdByName(spaceName);
    return spacesConditions.value(spaceId);
}

QStringList SpacesFilesFetcher::getSpacelabels(QString spaceName){
    QReadLocker r(&rwLock);
    int spaceId = getSpaceIdByName(spaceName);
    return spacesLabels.value(spaceId);
}



void SpacesFilesFetcher::DeleteTargetFiles(QList<QString> files)
{
    QStringList allSpaceName = getAllSpacesName();
    QMap<QString,QStringList> m_delete;
    for(auto spaceName:allSpaceName){
        QStringList allFiles;
        QStringList nowSpaceFiles;
        int spaceId = getSpaceIdByName(spaceName);
        auto useraddfiles = getFilesInSpace(spaceName,false);
        auto autoaddfiles = getFilesInSpace(spaceName,true);
        allFiles<<useraddfiles<<autoaddfiles;
        for(auto file:files){
            if(allFiles.contains(file)){
                nowSpaceFiles.append(file);
                if (infos.contains(spaceId)) {
                    infos[spaceId].removeAll(file);
                }
                if (userAddedInfos.contains(spaceId)) {
                    userAddedInfos[spaceId].removeAll(file);
                }
            }
        }
        if(!nowSpaceFiles.isEmpty()){
            m_delete.insert(spaceName,nowSpaceFiles);
        }
    }
    if(!m_delete.isEmpty())
        Q_EMIT FileDeleted(m_delete);
}

void SpacesFilesFetcher::updateSpaceFiles(QString SpaceName, QString file,bool isUserRemove,SpacesFilesFetcher::OperationType type)
{
    int spaceId = getSpaceIdByName(SpaceName);
    if(type == Add){
        if(isUserRemove){
              if(spaces.contains(spaceId)){
                  userAddedInfos[spaceId].append(file);
              }
        }else{
            if(spaces.contains(spaceId)){
                infos[spaceId].append(file);
            }
        }
    }else{
          if (spaces.contains(spaceId)) {
              userAddedInfos[spaceId].removeAll(file);
          }
        if (spaces.contains(spaceId)) {
            infos[spaceId].removeAll(file);
        }
    }
}

bool SpacesFilesFetcher::getSpaceStatus(int spaceId)
{
    return spaceStatus.value(spaceId);
}

void SpacesFilesFetcher::updateSpaceStatus(int spaceId, bool statu)
{
    spaceStatus[spaceId] = statu;
}

bool SpacesFilesFetcher::isUserAddFile(const QString &spaceName, const QString &FileName)
{
    int spaceId = getSpaceIdByName(spaceName);
    QStringList filelist =  userAddedInfos.value(spaceId);
    return filelist.contains(FileName);
}

void SpacesFilesFetcher::updateSpaceConditions(int spaceId, const QString &spaceName, const QVariant &conditionsDescription)
{
#ifdef COMPLIE_AS_PROJECT
    SemanticKBManager::globalInstance()->setSpaceConditionChanged(QString::number(spaceId), true);
    int id = getSpaceIdByName(spaceName);
    auto infosToBeRemoved = getFilesInSpaceById(id, true);
    auto autoAddedInfos = getFilesInSpaceById(id, false);
    infosToBeRemoved << autoAddedInfos;
    auto conditions = Condition::generateContitionsFromDescription(conditionsDescription);
    auto labels = Condition::getLabelsFromDescription(conditionsDescription);
    if (conditions.isEmpty() && labels.isEmpty()) {
        {
            QWriteLocker w(&rwLock);
            spacesConditions.take(spaceId);
            spacesLabels.take(spaceId);
        }

//        // 清空空间内的文件
//        DatabaseManager dbManager;
//        for (auto info : infosToBeRemoved) {
//            dbManager.removeFileFromSpace(info, spaceId);
//        }
        Q_EMIT ModelController::globalInstance()->spaceRemovedWithFilesRequest(spaceId,infosToBeRemoved);
        // 清空视图文件
       //Q_EMIT this->requestClearSpace(spaceId);
        Q_EMIT this->requestClearSpace(spaceId);
        return;
    }

    // todo: 和旧的条件做比较，尝试尽量减少不必要的遍历和匹配
    bool needClearAndResearch = true;

    {
        QWriteLocker w(&rwLock);
        spacesConditions.insert(spaceId, conditions);
        spacesLabels.insert(spaceId, labels);
        Q_EMIT handleSpaceConditionsChangedDone(spaceId);
    }

    if (needClearAndResearch) {
        // 清空空间内的文件，并且发送请求重新搜索的信号
//        DatabaseManager dbManager;
//        for (auto info : infosToBeRemoved) {
//            dbManager.removeFileFromSpace(info, spaceId);
//        }
         Q_EMIT ModelController::globalInstance()->spaceRemovedWithFilesRequest(spaceId,infosToBeRemoved);
        {
            QWriteLocker w(&rwLock);
            this->infos[spaceId] = QStringList();
        }
        Q_EMIT this->requestRescanSpaceWithConditionsDescritpion(spaceId, conditionsDescription);
    } else {
        // 对当前的信息做过滤，得出需要移除的文件，请求更新
    }
#endif
}

void SpacesFilesFetcher::handleFileCreated(const QString &newFile)
{
    if (movedFiles.contains(newFile)) {
        // 搜索索引服务发出的信号会和peony发出的信号有冲突，这里为了解决冲突加上此流程
        movedFiles.remove(newFile);
        return;
    }

    if (deletedFilesToHandle.contains(newFile)) {
        deletedFilesToHandle.remove(newFile);
    } else {
        createdFilesToHandle.insert(newFile);
    }
    if (!delayer->isActive()) {
        delayer->start();
    }
}

void SpacesFilesFetcher::handleFileIndexCreated(const QString &newFile)
{
    needChecklabel = true;
    if (movedFiles.contains(newFile)) {
        // 搜索索引服务发出的信号会和peony发出的信号有冲突，这里为了解决冲突加上此流程
        movedFiles.remove(newFile);
        return;
    }

    if (deletedFilesToHandle.contains(newFile)) {
        deletedFilesToHandle.remove(newFile);
    } else {
        createdFilesToHandle.insert(newFile);
    }
    if (!delayer->isActive()) {
        delayer->start();
    }
}

void SpacesFilesFetcher::handleFileDeleted(const QString &deletedFile)
{
    // fixme: 同步数据库
    if (createdFilesToHandle.contains(deletedFile)) {
        createdFilesToHandle.remove(deletedFile);
    } else {
        deletedFilesToHandle.insert(deletedFile);
    }
    if (!delayer->isActive()) {
        delayer->start();
    }
}

void SpacesFilesFetcher::handleFileMoved(const QString &oldFile, const QString &newFile)
{
    movedFiles.remove(oldFile);
    movedFiles.insert(newFile);

    createdFilesToHandle.insert(newFile);
    deletedFilesToHandle.insert(oldFile);

    if (!delayer->isActive()) {
        delayer->start();
    }
}

void SpacesFilesFetcher::handleAfterFilesReloaded(const QStringList &infos, int spaceId)
{
    this->infos.remove(spaceId);
    QStringList userAddAllFiles = getFilesInSpaceById(spaceId,true);
    QStringList updateFiles = infos;
    for (QString &filePath : userAddAllFiles) {
        updateFiles.removeAll(filePath);
    }

    // 移除文件夹路径，只保留文件
    QStringList filesOnly;
    for (const QString &path : updateFiles) {
        if (!QFileInfo(path).isDir()) {
            filesOnly.append(path);
        }
    }
    updateFiles = filesOnly;

    this->infos.insert(spaceId, updateFiles);
    QString spacename =  getSpaceNameById(spaceId);
    Q_EMIT updateSpacefileinfo(spacename);
    DatabaseManager dbManager;
    for (auto info : updateFiles) {
        if (info.isEmpty()) {
            bool ok = dbManager.addFileToSpace(info, spaceId);
            if(!ok){
                return;
            }

        } else {
            dbManager.addFileToSpace(info, spaceId);
        }
    }
}


void SpacesFilesFetcher::handleAfterFilesReloadedBycreate(const QStringList &infos, int spaceId){
    QString spacename =  getSpaceNameById(spaceId);
    // 移除文件夹路径，只保留文件
    QStringList filesOnly;
    for (const QString &path : infos) {
        if (!QFileInfo(path).isDir()) {
            filesOnly.append(path);
        }
    }
    this->infos.insert(spaceId, filesOnly);
    Q_EMIT spaceCreateFinish(spacename,filesOnly);
    DatabaseManager dbManager;
    for (auto info : filesOnly) {
        if (info.isEmpty()) {
            bool ok = dbManager.addFileToSpace(info, spaceId);
            if(!ok){
                return;
            }

        } else {
            dbManager.addFileToSpace(info, spaceId);
        }
    }   
}

void SpacesFilesFetcher::handleSpaceRemoved(int spaceId, const QStringList &infosToBeRemoved)
{
    DatabaseManager dbManager;
    for (auto info : infosToBeRemoved) {
        dbManager.removeFileFromSpace(info, spaceId);
    }
}

void SpacesFilesFetcher::handleFilesCreatedInternal()
{
#ifdef COMPLIE_AS_PROJECT
    QSet<QString> targetFiles;
    QSet<QString> targetDeletedFiles;
    createdFilesToHandle.swap(targetFiles);
    deletedFilesToHandle.swap(targetDeletedFiles);

    {
        QReadLocker r(&rwLock);
        for (auto spaceId : spaces.keys()) {
            if(!getSpaceStatus(spaceId))//当自动更新关闭则跳过
                continue;
            auto conditions = spacesConditions.value(spaceId);
            auto labels = spacesLabels.value(spaceId);
            if (conditions.isEmpty() && labels.isEmpty()) {
                // 空白空间跳过匹配流程
                continue;
            }
            auto job = UKUI::IDM::FileInfoJob::fromPaths(targetFiles.toList());
            job->setConditions(conditions);
            job->setLabels(labels);
            // 需要处理文件内容条件
            job->setHandleContentCondition(true);
            job->setAutoDelete(true);
            connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                auto targetInfos = job->getInfos();
                if (!targetInfos.isEmpty()) {
                    Q_EMIT this->handleFilesCreatedProgressed(targetInfos, spaceId);
                    DatabaseManager dbManager;
                    QStringList allFiles = getFilesInSpaceById(spaceId,true);
                    auto spaceName = this->getSpaceNameById(spaceId);
                    for (auto info : targetInfos) {
                        auto path = info->path();
                        if(allFiles.contains(path)){
                            continue;
                        }
                        bool ok = dbManager.addFileToSpace(path, spaceName);
                        if(ok){
                            updateSpaceFiles(spaceName,path,false,Add);
                        }
                    }
                }
            }, Qt::BlockingQueuedConnection);
            // 如果在处理变更时空间条件发生改变，则取消此次处理，这里可能会丢失一部分数据，需要考虑结合其他场景优化
            connect(this, &SpacesFilesFetcher::handleSpaceConditionsChangedDone, job, [=](int changedSpaceId){
                if (changedSpaceId == spaceId) {
                    job->cancel();
                }
            });
            job->start();
        }
    }

    if (!targetDeletedFiles.isEmpty()) {
        DeleteTargetFiles(targetDeletedFiles.values());
        DatabaseManager dbManager;
        for (auto file : targetDeletedFiles) {
            dbManager.removeFileFromAllSpace(file);
            SemanticKBManager::globalInstance()->deleteFileFromAllSpaces(file, "");
        }
    }
#endif
}

void SpacesFilesFetcher::fetch()
{
    DatabaseManager manager(dbName);

    auto allSpaces = manager.getAllSpaceNames();
    for (auto space : allSpaces) {
        int id = manager.getSpaceIdByName(space);
        if(id == 1 && space == "Intelligent Data Manager Introduction"){
            space = tr("Intelligent Data Manager Introduction");
        }
        spaces.insert(id, space);
        // 获取条件配置
        // 加载标签和条件配置
        QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(id) + ".json";
        QFile file(jsonPath);
        file.open(QFile::ReadOnly|QFile::Text);
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        auto conditions = Condition::generateContitionsFromDescription(doc.object());
        auto labels = Condition::getLabelsFromDescription(doc.object());
        spacesConditions.insert(id, conditions);
        spacesLabels.insert(id, labels);
        bool status = manager.getSpacestatus(space);
        spaceStatus.insert(id,status);
    }

    QStringList filesNeedRemove;
    auto userAddedFiles = manager.getAllFilesFromSpacesBySpaceIds(true);
    for (int spaceId : userAddedFiles.keys()) {
        if (!this->spaces.keys().contains(spaceId)) {
            // fixme: 清除多余的数据
            continue;
        }
        QStringList files = userAddedFiles.value(spaceId);
        QList<QSharedPointer<UKUI::IDM::FileInfo>> fileinfoList;
        QStringList infoList;
        for (auto file : files) {
            infoList << file;
            auto info = UKUI::IDM::FileInfo::fromPath(file,true);
            fileinfoList << info;
        }
        userAddedFileInfos.insert(spaceId, fileinfoList);
        userAddedInfos.insert(spaceId, infoList);
    }

    auto autoAddedFiles = manager.getAllFilesFromSpacesBySpaceIds(false);
    for (int spaceId : autoAddedFiles.keys()) {
        if (!this->spaces.keys().contains(spaceId)) {
            // fixme: 清除多余的数据
            continue;
        }
        QStringList files = autoAddedFiles.value(spaceId);
        QStringList infoList;
        QList<QSharedPointer<UKUI::IDM::FileInfo>> fileinfoList;
        for (auto file : files) {
            infoList << file;
            auto info = UKUI::IDM::FileInfo::fromPath(file,true);
            fileinfoList << info;
        }
        infos.insert(spaceId, infoList);
        fileInfos.insert(spaceId,fileinfoList);
    }

    Q_EMIT fetched(true);

//    // 在fetched之后，此处的缓存不再使用，数据转到各个model中处理
//    userAddedInfos.clear();
//    infos.clear();
}
