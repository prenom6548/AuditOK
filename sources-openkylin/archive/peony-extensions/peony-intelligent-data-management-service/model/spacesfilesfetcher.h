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

#ifndef SPACESFILESFETCHER_H
#define SPACESFILESFETCHER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QReadWriteLock>
#include "file/fileinfo.h"
#include <memory>

class Condition;
class QTimer;

class SpacesFilesFetcher : public QObject
{
    enum OperationType {
        Add=1,
        Remove,
    };
    friend class ModelController;
    friend class PreviewHelper;
    Q_OBJECT
public:
    explicit SpacesFilesFetcher(const QString &dbName = "peony-idm.db", const QString &filedbname = "peony-idm-file.db",QObject *parent = nullptr);

    QStringList getAllSpacesName();
    int getSpaceIdByName(const QString &spaceName);
    QString getSpaceNameById(int spaceId);

    // 仅在fetched阶段调用，fetched之后需要从FileItemModel中获取空间文件信息
    QStringList getFilesInSpace(const QString &spaceName, bool isUserAdded = false);
    QStringList getFilesInSpaceById(int spaceId, bool isUserAdded = false);
    QList<std::shared_ptr<Condition>>  getSpaceconditon(QString SpaceName);
    QStringList getSpacelabels(QString SpaceName);
    void DeleteTargetFiles(QList<QString>files);
    void updateSpaceFiles(QString SpaceName, QString file,bool isUserRemove ,SpacesFilesFetcher::OperationType type);
    bool getSpaceStatus(int spaceId);
    void updateSpaceStatus(int spaceId,bool statu);
    bool isUserAddFile(const QString &spaceName,const QString &FileName);
Q_SIGNALS:
    void fetched(bool successed);

    void handleSpaceConditionsChangedDone(int spaceId);
    void handleFilesCreatedProgressed(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &newFiles, int spaceId);
    void handleSpaceConditionsProgressed(bool needReload, const QList<std::shared_ptr<UKUI::IDM::FileInfo>> &resultFiles, int spaceId);

    void requestRescanSpaceWithConditionsDescritpion(int spaceId, const QVariant &conditionsDescription);
    void requestClearSpace(int spaceId);
    void updateSpacefileinfo(QString spacename);
    QList<std::shared_ptr<UKUI::IDM::FileInfo>> requestFilesInSpaceById(int spaceId);
    void FileDeleted(QMap<QString,QStringList> deleteSpacesFiles);
    void spaceCreateFinish(QString spaceName,const QStringList &infosToBeRemoved);
public Q_SLOTS:
    void fetch();
    void updateSpaceConditions(int spaceId, const QString &spaceName, const QVariant &conditionsDescription);
    void handleFileCreated(const QString &newFile);
    void handleFileIndexCreated(const QString &newFile);
    void handleFileDeleted(const QString &deletedFile);
    void handleFileMoved(const QString &oldFile, const QString &newFile);
    void handleAfterFilesReloaded(const QStringList &infos, int spaceId);
    void handleAfterFilesReloadedBycreate(const QStringList &infos, int spaceId);
    void handleSpaceRemoved(int spaceId, const QStringList &infosToBeRemoved);
protected Q_SLOTS:
    void handleFilesCreatedInternal();

private:
    QReadWriteLock rwLock;

    QTimer *delayer = nullptr;
    QSet<QString> createdFilesToHandle;
    QSet<QString> deletedFilesToHandle;
    QSet<QString> movedFiles;

    QMap<int, QString> spaces;
    QMap<int, QStringList> spacesLabels;
    QMap<int, QList<std::shared_ptr<Condition>>> spacesConditions;
    QMap<int, QStringList> infos;
    QMap<int, QStringList> userAddedInfos;
    QMap<int, QList<QSharedPointer<UKUI::IDM::FileInfo>>> fileInfos;
    QMap<int, QList<QSharedPointer<UKUI::IDM::FileInfo>>> userAddedFileInfos;
    QMap<int,bool> spaceStatus;
    QString dbName;
    QString filedbName;
    bool needChecklabel = false;
};

#endif // SPACESFILESFETCHER_H
