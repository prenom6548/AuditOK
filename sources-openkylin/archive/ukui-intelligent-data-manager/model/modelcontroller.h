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

#ifndef MODELCONTROLLER_H
#define MODELCONTROLLER_H

#include <QObject>
#include <QUrl>

#include <atomic>

#include "fileitemproxysortfiltermodel.h"

class SpacesFilesFetcher;
class Condition;
class FileItemProxySortFilterModel;
class QWindow;

class ModelController : public QObject
{
    friend class PreviewHelper;
    friend class ConditionEditor;
    Q_OBJECT
public:
    static ModelController *globalInstance();

    explicit ModelController(const QString &dbName = "ukui-idm.db", QObject *parent = nullptr);
    ~ModelController();

    Q_INVOKABLE void startLoadModel();
    Q_INVOKABLE void startLoadRecentModel();
    Q_INVOKABLE void openFile(const QString &path);
    Q_INVOKABLE void openFileInDirectory(const QString &path);
    Q_INVOKABLE void userAddFilesToSpace(const QList<QUrl> &urls, FileItemProxySortFilterModel *model, bool sendSignal = true);
    Q_INVOKABLE void removeFileFromSpace(const QString &path, FileItemProxySortFilterModel *model, bool sendSignal = true);
    Q_INVOKABLE void removeFileFromAllSpace(const QString &path);
    Q_INVOKABLE void editSpaceCondition(int spaceId);
    Q_INVOKABLE void createSpace();
    Q_INVOKABLE void removeSpace(int spaceId);
    Q_INVOKABLE void requestRenameSpace(int spaceId);
    Q_INVOKABLE void requestShowMenu(FileItemProxySortFilterModel *model, int index, const QPoint &globalPos);
    Q_INVOKABLE void editSearch();
    Q_INVOKABLE void saveSearchResultAsSpace();
    Q_INVOKABLE void requestAddUserAddedFileToSapce(FileItemProxySortFilterModel *model);

    void renameSpace(int spaceId, const QString &spaceName);
    void moveUserAddedFileFromSpaceTo(const QString &path, FileItemProxySortFilterModel *currentModel, FileItemProxySortFilterModel *targetModel);

    QMap<int, QString> getSpaceIdsWithSpace();

    QList<QSharedPointer<UKUI::IDM::FileInfo>> getUserAddedFilesFromSpaceById(int spaceId);

    FileItemProxySortFilterModel *getProxyModelBySpaceId(int spaceId);

    bool isModelInited();

    void setRootWindow(QWindow *newRootWindow);

public slots:
    void onSpaceConditionChanged(int spaceId, const QString &spaceName, const QVariant conditionsDescription);
    void onSpaceCreateRequest(const QString &spaceName, const QVariant &conditionDescription, bool isCreatedFromSearch);
    void onSpaceCreateRequestWithFiles(const QString &spaceName, const QVariant &conditionDescription, bool isCreatedFromSearch, QList<QSharedPointer<UKUI::IDM::FileInfo>> infos);

signals:
    void modelCreated(FileItemProxySortFilterModel *model);
    void recentModelCreated(FileItemProxySortFilterModel *model);
    void allModelCreated();

    void setSpaceConditionRequest(int spaceId, const QString &spaceName);
    void createSpaceRequest();

    void spaceNameUpdated(int spaceId, const QString &spaceName);
    void spaceCreated(int spaceId, const QString &spaceName);
    void spaceRemoved(int spaceId);
    void spaceConditionsChanged(int spaceId, const QString &spaceName, const QVariant &conditionsDescription);
    void showMenuRequested(FileItemProxySortFilterModel *model, int index, const QPoint &globalPos);
    void editSearchRequested();
    void searchSaveRequested();
    void renameSpaceRequested(int spaceId);

    // 用于和条件过滤线程交互
    void fileDeletedRequest(const QString &deletedFile);
    void fileCreateRequest(const QString &newFile);
    void fileMovedRequest(const QString &oldFile, const QString &newFile);
    void spaceConditionChangedRequest(int spaceId, const QList<QSharedPointer<Condition>> &conditions);
    void spaceRemovedWithFilesRequest(int spaceId, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &filesToBeRemoved);

    // 从线程返回的结果
    void filesCreated(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, int spaceId);
    void spaceFilesChangedForConditionsChanged(bool needReload, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &resultFiles, int spaceId);

    // 空间条件改变返回的结果
    void filesReloaded(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, int spaceId);

    // 打开文件请求
    void openFileRequest(const QUrl &url);

private slots:
    void initModels();
    void handleRescanSpaceWithCondition(int spaceId, const QVariant &conditionsDescription);

    void warningWithDialog(const QString &title, const QString &errorMsg);
    void criticalWithDialog(const QString &title, const QString &errorMsg);

private:
    std::atomic_bool fetched;
    SpacesFilesFetcher *fetcher;
    QThread *thread;
    QMap<int, FileItemProxySortFilterModel *> proxyModels;

    QWindow *rootWindow = nullptr;
};

#endif // MODELCONTROLLER_H
