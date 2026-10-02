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

namespace UKUI {
namespace IDM {
class FileInfo;
}
}

class Condition;
class QTimer;

class SpacesFilesFetcher : public QObject
{
    friend class ModelController;
    friend class PreviewHelper;
    Q_OBJECT
public:
    explicit SpacesFilesFetcher(const QString &dbName = "ukui-idm.db", QObject *parent = nullptr);

    QStringList getAllSpacesName();
    int getSpaceIdByName(const QString &spaceName);
    QString getSpaceNameById(int spaceId);

    // 仅在fetched阶段调用，fetched之后需要从FileItemModel中获取空间文件信息
    QList<QSharedPointer<UKUI::IDM::FileInfo>> getFilesInSpace(const QString &spaceName, bool isUserAdded = false);
    QList<QSharedPointer<UKUI::IDM::FileInfo>> getFilesInSpaceById(int spaceId, bool isUserAdded = false);

signals:
    void fetched(bool successed);

    void handleSpaceConditionsChangedDone(int spaceId);
    void handleFilesCreatedProgressed(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &newFiles, int spaceId);
    void handleSpaceConditionsProgressed(bool needReload, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &resultFiles, int spaceId);

    void requestRescanSpaceWithConditionsDescritpion(int spaceId, const QVariant &conditionsDescription);
    void requestClearSpace(int spaceId);

    QList<QSharedPointer<UKUI::IDM::FileInfo>> requestFilesInSpaceById(int spaceId);

public slots:
    void fetch();
    void updateSpaceConditions(int spaceId, const QString &spaceName, const QVariant &conditionsDescription);
    void handleFileCreated(const QString &newFile);
    void handleFileDeleted(const QString &deletedFile);
    void handleFileMoved(const QString &oldFile, const QString &newFile);
    void handleAfterFilesReloaded(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, int spaceId);
    void handleSpaceRemoved(int spaceId, const QList<QSharedPointer<UKUI::IDM::FileInfo> > &infosToBeRemoved);

protected slots:
    void handleFilesCreatedInternal();

private:
    QReadWriteLock rwLock;

    QTimer *delayer = nullptr;
    QSet<QString> createdFilesToHandle;
    QSet<QString> deletedFilesToHandle;
    QSet<QString> movedFiles;

    QMap<int, QString> spaces;
    QMap<int, QList<QSharedPointer<Condition>>> spacesConditions;
    QMap<int, QList<QSharedPointer<UKUI::IDM::FileInfo>>> infos;
    QMap<int, QList<QSharedPointer<UKUI::IDM::FileInfo>>> userAddedInfos;

    QString dbName;
};

#endif // SPACESFILESFETCHER_H
