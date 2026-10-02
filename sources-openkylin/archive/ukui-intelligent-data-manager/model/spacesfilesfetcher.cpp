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
#include "../condition/conditioneditor.h"
#include "../file/fileinfo.h"

#ifdef COMPLIE_AS_PROJECT
#include "fileinfojob.h"
#include "fileinfomanager.h"
#include "modelcontroller.h"
#endif

#include <QStandardPaths>
#include <QFile>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QReadLocker>
#include <QWriteLocker>
#include <QTimer>

SpacesFilesFetcher::SpacesFilesFetcher(const QString &dbName, QObject *parent) : QObject(parent), dbName(dbName)
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

QList<QSharedPointer<UKUI::IDM::FileInfo> > SpacesFilesFetcher::getFilesInSpace(const QString &spaceName, bool isUserAdded)
{
    int spaceId = getSpaceIdByName(spaceName);
    return getFilesInSpaceById(spaceId, isUserAdded);
}

QList<QSharedPointer<UKUI::IDM::FileInfo> > SpacesFilesFetcher::getFilesInSpaceById(int spaceId, bool isUserAdded)
{
    QReadLocker r(&rwLock);
    return isUserAdded? userAddedInfos.value(spaceId): infos.value(spaceId);
}

void SpacesFilesFetcher::updateSpaceConditions(int spaceId, const QString &spaceName, const QVariant &conditionsDescription)
{
#ifdef COMPLIE_AS_PROJECT
    auto infosToBeRemoved = this->requestFilesInSpaceById(spaceId);

    auto conditions = ConditionEditor::generateContitionsFromDescription(conditionsDescription);
    if (conditions.isEmpty()) {
        // 清空空间内的文件
        DatabaseManager dbManager;
        for (auto info : infosToBeRemoved) {
            QUrl url = info->uri();
            dbManager.removeFileFromSpace(url.toLocalFile(), spaceId);
        }

        // 清空视图文件
        Q_EMIT this->requestClearSpace(spaceId);

        return;
    }

    // todo: 和旧的条件做比较，尝试尽量减少不必要的遍历和匹配
    bool needClearAndResearch = true;

    {
        QWriteLocker w(&rwLock);
        spacesConditions.insert(spaceId, conditions);
        Q_EMIT handleSpaceConditionsChangedDone(spaceId);
    }

    if (needClearAndResearch) {
        // 清空空间内的文件，并且发送请求重新搜索的信号
        DatabaseManager dbManager;
        for (auto info : infosToBeRemoved) {
            QUrl url = info->uri();
            dbManager.removeFileFromSpace(url.toLocalFile(), spaceId);
        }
        {
            QWriteLocker w(&rwLock);
            this->infos.clear();
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

void SpacesFilesFetcher::handleAfterFilesReloaded(const QList<QSharedPointer<UKUI::IDM::FileInfo> > &infos, int spaceId)
{
    DatabaseManager dbManager;

    for (auto info : infos) {
        QString path = info->path();
        if (path.isEmpty()) {
            QUrl url = info->uri();
            dbManager.addFileToSpace(url.toLocalFile(), spaceId);
        } else {
            dbManager.addFileToSpace(path, spaceId);
        }
    }
}

void SpacesFilesFetcher::handleSpaceRemoved(int spaceId, const QList<QSharedPointer<UKUI::IDM::FileInfo> > &infosToBeRemoved)
{
    DatabaseManager dbManager;
    for (auto info : infosToBeRemoved) {
        QUrl url = info->uri();
        dbManager.removeFileFromSpace(url.toLocalFile(), spaceId);
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
            auto conditions = spacesConditions.value(spaceId);
            if (conditions.isEmpty()) {
                // 空白空间跳过匹配流程
                continue;
            }
            auto job = UKUI::IDM::FileInfoJob::fromPaths(targetFiles.toList());
            job->setConditions(conditions);
            job->setAutoDelete(true);
            connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                auto targetInfos = job->getInfos();
                if (!targetInfos.isEmpty()) {
                    Q_EMIT this->handleFilesCreatedProgressed(targetInfos, spaceId);

                    DatabaseManager dbManager;
                    auto spaceName = this->getSpaceNameById(spaceId);
                    for (auto info : targetInfos) {
                        auto path = info->path();
                        dbManager.addFileToSpace(path, spaceName);
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
        DatabaseManager dbManager;
        for (auto file : targetDeletedFiles) {
            dbManager.removeFileFromAllSpace(file);
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
        spaces.insert(id, space);

#ifdef COMPLIE_AS_PROJECT
        // 获取条件配置
        // 加载标签和条件配置
        QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(id) + ".json";
        QFile file(jsonPath);
        file.open(QFile::ReadOnly|QFile::Text);
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        auto conditions = ConditionEditor::generateContitionsFromDescription(doc.object());
        spacesConditions.insert(id, conditions);
#endif
    }

    QStringList filesNeedRemove;
    auto userAddedFiles = manager.getAllFilesFromSpacesBySpaceIds(true);
    for (int spaceId : userAddedFiles.keys()) {
        if (!this->spaces.keys().contains(spaceId)) {
            // fixme: 清除多余的数据
            continue;
        }
        QStringList files = userAddedFiles.value(spaceId);
        QList<QSharedPointer<UKUI::IDM::FileInfo>> infoList;
        for (auto file : files) {
            auto info = UKUI::IDM::FileInfo::fromPath(file);
            infoList << info;
        }
        userAddedInfos.insert(spaceId, infoList);
    }

    auto autoAddedFiles = manager.getAllFilesFromSpacesBySpaceIds(false);
    for (int spaceId : autoAddedFiles.keys()) {
        if (!this->spaces.keys().contains(spaceId)) {
            // fixme: 清除多余的数据
            continue;
        }
        QStringList files = autoAddedFiles.value(spaceId);
        QList<QSharedPointer<UKUI::IDM::FileInfo>> infoList;
        for (auto file : files) {
            auto info = UKUI::IDM::FileInfo::fromPath(file);
            infoList << info;
        }
        infos.insert(spaceId, infoList);
    }

    Q_EMIT fetched(true);

    // 在fetched之后，此处的缓存不再使用，数据转到各个model中处理
    userAddedInfos.clear();
    infos.clear();
}
