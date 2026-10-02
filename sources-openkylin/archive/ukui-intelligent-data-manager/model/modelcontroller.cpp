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

#include "modelcontroller.h"
#include "spacesfilesfetcher.h"
#include "fileitemmodel.h"
#include "databasemanager.h"
#include "fileitemproxysortfiltermodel.h"

#ifdef COMPLIE_AS_PROJECT
#include "conditioneditor.h"
#include "condition.h"
#include "search/searchtask.h"
#include "fileinfojob.h"
#endif

#include <QDesktopServices>
#include <QMessageBox>

#include <QStandardPaths>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

#include <QMenu>

#include <QThread>

#include <QFileDialog>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QApplication>
#include <QTimer>

#include <QWindow>

#include <QDebug>

static ModelController *instance = nullptr;
static bool modelInited = false;

ModelController *ModelController::globalInstance()
{
    if (!instance) {
        instance = new ModelController;
    }
    return instance;
}

ModelController::ModelController(const QString &dbName, QObject *parent) : QObject(parent)
{
    fetched = false;
    fetcher = new SpacesFilesFetcher(dbName);
    thread = new QThread;
    fetcher->moveToThread(thread);
    connect(thread, &QThread::started, fetcher, &SpacesFilesFetcher::fetch);
    connect(fetcher, &SpacesFilesFetcher::fetched, this, &ModelController::initModels, Qt::BlockingQueuedConnection);
    connect(this, &ModelController::spaceRemoved, this, [=](int spaceId){
        auto proxyModel = getProxyModelBySpaceId(spaceId);
        auto sourceModel = qobject_cast<FileItemModel *>(proxyModel->sourceModel());
        Q_EMIT this->spaceRemovedWithFilesRequest(spaceId, sourceModel->infos);
        proxyModels.remove(spaceId);
    });

    connect(this, &ModelController::modelCreated, this, [=](FileItemProxySortFilterModel *proxyModel){
        if (proxyModels.values().contains(proxyModel))
            return;
        proxyModels.insert(proxyModel->getSpaceId(), proxyModel);
        auto model = qobject_cast<FileItemModel *>(proxyModel->sourceModel());
        connect(model, &FileItemModel::reportError, this, [=](const QString &errorMsg){
            warningWithDialog(nullptr, errorMsg);
        });
        connect(this, &ModelController::filesCreated, model, &FileItemModel::handleFilesAdded);
        connect(this, &ModelController::spaceFilesChangedForConditionsChanged, model, &FileItemModel::handleFilesChangedForConditionsChanged);
        connect(this, &ModelController::filesReloaded, model, &FileItemModel::handleFilesReloaded);
        //connect(this, &ModelController::spaceRemoved, proxyModel, &FileItemProxySortFilterModel::handleSpaceRemoved);
    });
}

ModelController::~ModelController()
{
    disconnect(thread);
    disconnect(fetcher);
    disconnect(this);
    thread->quit();
    // fixme: 在线程运行中退出会导致崩溃
    thread->wait();
    delete fetcher;
    delete thread;
}

void ModelController::startLoadModel()
{
    if (!thread->isRunning())
        thread->start();
    else {
        while (!fetched) {
            qApp->processEvents();
        }
        for (auto proxyModel : proxyModels) {
            Q_EMIT modelCreated(proxyModel);
        }
        Q_EMIT allModelCreated();
    }
}

void ModelController::startLoadRecentModel()
{
    auto model = new FileItemModel(this);
    auto proxyModel = new FileItemProxySortFilterModel(model, this);
    proxyModel->setIsRecent(true);
    model->startLoadRecentFiles();
    Q_EMIT recentModelCreated(proxyModel);
}

void ModelController::openFile(const QString &path)
{
    if (path.isEmpty())
        return;
    QUrl url = QUrl::fromLocalFile(path);
    Q_EMIT openFileRequest(url);
}

void ModelController::openFileInDirectory(const QString &path)
{
    QStringList uris;
    QUrl url = QUrl::fromLocalFile(path);
    uris << url.toEncoded();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.FileManager1",
                                                      "/org/freedesktop/FileManager1",
                                                      "org.freedesktop.FileManager1",
                                                      "ShowItems");
    msg << uris;
    msg << qAppName();
    QDBusConnection::sessionBus().asyncCall(msg);
}

void ModelController::userAddFilesToSpace(const QList<QUrl> &urls, FileItemProxySortFilterModel *model, bool sendSignal)
{
    // fixme: run in thread
    DatabaseManager manager;
    int spaceId = model->getSpaceId();
    auto sourceModel = qobject_cast<FileItemModel *>(model->sourceModel());
    for (auto url : urls) {
        QString errMsg;
        bool ok = true;
        QString path = url.toLocalFile();
        auto info = UKUI::IDM::FileInfo::fromUrl(url);

        if (sourceModel->infos.contains(info)) {
            ok = false;
            errMsg = QObject::tr("File %1 is existed in space, please not add file repeatly.").arg(path);
        } else {
            ok = manager.addFileToSpace(path, spaceId, true, &errMsg);
        }

        if (ok) {
            if (sendSignal) {
                model->handleFileEvent(UKUI::IDM::FileInfoManager::UserAddFile, path, nullptr);
            }
        } else {
            QTimer::singleShot(0, qApp, [=]{
                warningWithDialog(nullptr, errMsg);
            });
        }
    }
}

void ModelController::removeFileFromSpace(const QString &path, FileItemProxySortFilterModel *model, bool sendSignal)
{
    // fixme: run in thread, and batch process
    DatabaseManager manager;
    QString errMsg1, errMsg2;
    manager.removeFileFromSpace(path, model->getSpaceId(), true, &errMsg1);
    manager.removeFileFromSpace(path, model->getSpaceId(), false, &errMsg2);
    if (errMsg1.isEmpty() || errMsg2.isEmpty() && sendSignal) {
        model->handleFileEvent(UKUI::IDM::FileInfoManager::UserRemoveFile, path, nullptr);
    }
}

void ModelController::removeFileFromAllSpace(const QString &path)
{
    // fixme: run in thread, and batch process
    DatabaseManager manager;
    QString errMsg;
    manager.removeFileFromAllSpace(path, &errMsg);
}

void ModelController::editSpaceCondition(int spaceId)
{
    QString spaceName = fetcher->getSpaceNameById(spaceId);
    qDebug() << "edit space condition" << spaceName;
    Q_EMIT setSpaceConditionRequest(spaceId, spaceName);
}

void ModelController::createSpace()
{
    Q_EMIT createSpaceRequest();
}

void ModelController::removeSpace(int spaceId)
{
    DatabaseManager manager;
    QString errorMsg;
    QString spaceName = fetcher->getSpaceNameById(spaceId);
    if (manager.removeSpace(spaceName, &errorMsg)) {
        fetcher->rwLock.lockForWrite();
        fetcher->spaces.remove(spaceId);
        fetcher->rwLock.unlock();
        qDebug() << "space removed" << spaceId;
        // 同时删除配置文件
        QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(spaceId) + ".json";
        QFile file(jsonPath);
        file.remove();
        Q_EMIT spaceRemoved(spaceId);
    } else {
        warningWithDialog(nullptr, tr("Failed to remove space %1, error message: %2").arg(spaceName).arg(errorMsg));
    }
}

void ModelController::requestRenameSpace(int spaceId)
{
    Q_EMIT renameSpaceRequested(spaceId);
}

void ModelController::requestShowMenu(FileItemProxySortFilterModel *model, int index, const QPoint &globalPos)
{
    qDebug() << model->spaceName() << index << globalPos;
    Q_EMIT showMenuRequested(model, index, globalPos);
}

void ModelController::editSearch()
{
    Q_EMIT editSearchRequested();
}

void ModelController::saveSearchResultAsSpace()
{
    Q_EMIT searchSaveRequested();
}

void ModelController::requestAddUserAddedFileToSapce(FileItemProxySortFilterModel *model)
{
    int spaceId = model->getSpaceId();

    if (spaceId >= 0) {
        auto urls = QFileDialog::getOpenFileUrls(nullptr, tr("Choose File to Add"));
        if (!urls.isEmpty()) {
            this->userAddFilesToSpace(urls, model, true);
        }
    } else {
        qWarning() << "request add file to space with invalid spaceId" << spaceId;
    }
}

void ModelController::renameSpace(int spaceId, const QString &spaceName)
{
    DatabaseManager manager;
    QString errorMsg;
    QString oldSpaceName = fetcher->getSpaceNameById(spaceId);
    if (manager.renameSpace(oldSpaceName, spaceName, &errorMsg)) {
        fetcher->rwLock.lockForWrite();
        fetcher->spaces[spaceId] = spaceName;
        fetcher->rwLock.unlock();
        Q_EMIT spaceNameUpdated(spaceId, spaceName);
    } else {
        warningWithDialog(nullptr, tr("Failed to rename space %1 to %2, %2 is existed.").arg(oldSpaceName).arg(spaceName));
    }
}

void ModelController::moveUserAddedFileFromSpaceTo(const QString &path, FileItemProxySortFilterModel *currentModel, FileItemProxySortFilterModel *targetModel)
{
    auto info = UKUI::IDM::FileInfo::fromPath(path);
    int targetIndex = targetModel->m_sourceModel->infos.indexOf(info);
    if (targetIndex >= 0 && targetIndex < targetModel->m_sourceModel->userAddedInfos.count()) {
        warningWithDialog(tr("Moving Failed"), tr("Can not move file to %1, file has arealy existed.").arg(targetModel->getSpaceName()));
    } else {
        removeFileFromSpace(path, currentModel);
        QList<QUrl> urls;
        urls << QUrl::fromLocalFile(path);
        userAddFilesToSpace(urls, targetModel);
    }
}

QMap<int, QString> ModelController::getSpaceIdsWithSpace()
{
    QMap<int, QString> result;
    fetcher->rwLock.lockForRead();
    result = fetcher->spaces;
    fetcher->rwLock.unlock();
    return result;
}

QList<QSharedPointer<UKUI::IDM::FileInfo> > ModelController::getUserAddedFilesFromSpaceById(int spaceId)
{
    return fetcher->getFilesInSpaceById(spaceId, true);
}

FileItemProxySortFilterModel *ModelController::getProxyModelBySpaceId(int spaceId)
{
    return proxyModels.value(spaceId, nullptr);
}

bool ModelController::isModelInited()
{
    return fetched;
}

void ModelController::onSpaceConditionChanged(int spaceId, const QString &spaceName, const QVariant conditionsDescription)
{

}

void ModelController::onSpaceCreateRequest(const QString &spaceName, const QVariant &conditionDescription, bool isCreatedFromSearch)
{
    this->onSpaceCreateRequestWithFiles(spaceName, conditionDescription, isCreatedFromSearch, QList<QSharedPointer<UKUI::IDM::FileInfo>>());
}

void ModelController::onSpaceCreateRequestWithFiles(const QString &spaceName, const QVariant &conditionDescription, bool isCreatedFromSearch, QList<QSharedPointer<UKUI::IDM::FileInfo> > infos)
{
    DatabaseManager manager;
    QString errMsg;
    QString realSpaceName;
    bool hasInitFiles = !infos.isEmpty();
    if (manager.createSpace(spaceName, QStringList(), &errMsg, &realSpaceName)) {
        int spaceId = manager.getSpaceIdByName(realSpaceName);
        QJsonObject configObj = conditionDescription.toJsonObject();
        QJsonDocument doc(configObj);
        QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(spaceId) + ".json";
        QFile file(jsonPath);
        if (file.open(QFile::WriteOnly|QFile::Text)) {
            file.write(doc.toJson());
            fetcher->rwLock.lockForWrite();
            fetcher->spaces.insert(spaceId, realSpaceName);
            fetcher->rwLock.unlock();
            qDebug() << "space created" << spaceId << realSpaceName;
            spaceCreated(spaceId, realSpaceName);
            auto model = new FileItemModel(spaceId, realSpaceName, QList<QSharedPointer<UKUI::IDM::FileInfo>>());
            auto proxyModel = new FileItemProxySortFilterModel(model, this);
            modelCreated(proxyModel);

            Q_EMIT filesReloaded(infos, spaceId);

#ifdef COMPLIE_AS_PROJECT
            if (!isCreatedFromSearch) {
                // 如果此处的调用非搜索保存请求且存在条件，则需要进行搜索和匹配
                auto conditions = ConditionEditor::generateContitionsFromDescription(conditionDescription);
                if (!conditions.isEmpty()) {
                    // 对空间进行按条件进行初始化
                    Q_EMIT this->spaceConditionsChanged(spaceId, realSpaceName, conditionDescription);
                }
            }
#endif
        }
    } else {
        warningWithDialog(nullptr, tr("Failed to create space %1, is there a same name space created?").arg(spaceName));
    }
}

void ModelController::initModels()
{
    fetched = true;

    auto spaces = fetcher->getAllSpacesName();
    for (auto space : spaces) {
        int id = fetcher->getSpaceIdByName(space);
        auto infos = fetcher->getFilesInSpaceById(id, true);
        QList<QSharedPointer<UKUI::IDM::FileInfo>> userAddedInfos = infos;
        auto autoAddedInfos = fetcher->getFilesInSpaceById(id, false);
        infos << autoAddedInfos;
        auto model = new FileItemModel(id, space, infos, this);
        model->userAddedInfos = userAddedInfos;
        auto proxyModel = new FileItemProxySortFilterModel(model, this);

        Q_EMIT modelCreated(proxyModel);
    }

    // 从ukui-search获取文件创建事件
    connect(UKUI::IDM::FileInfoManager::globalInstance(), &UKUI::IDM::FileInfoManager::remoteFileEvent, this, [=](int eventType, const QString &arg1, const QString &arg2){
        switch (eventType) {
        case 1:
        case 101:
            Q_EMIT this->fileCreateRequest(arg1);
            break;
        case 102:
            UKUI::IDM::FileInfoManager::globalInstance()->fileChanged(UKUI::IDM::FileInfoManager::FileDeleted, arg1, nullptr);
            Q_EMIT this->fileDeletedRequest(arg1);
            break;
        case 103:
            UKUI::IDM::FileInfoManager::globalInstance()->fileChanged(UKUI::IDM::FileInfoManager::FileMoved, arg1, arg2);
            Q_EMIT this->fileMovedRequest(arg1, arg2);
            break;
        default:
            break;
        }
    });

    // 在线程中处理新增文件和空间条件改变
    connect(this, &ModelController::spaceConditionsChanged, this->fetcher, &SpacesFilesFetcher::updateSpaceConditions);
    connect(this, &ModelController::fileCreateRequest, this->fetcher, &SpacesFilesFetcher::handleFileCreated);
    connect(this, &ModelController::fileDeletedRequest, this->fetcher, &SpacesFilesFetcher::handleFileDeleted);
    connect(this, &ModelController::fileMovedRequest, this->fetcher, &SpacesFilesFetcher::handleFileMoved);
    connect(this, &ModelController::filesReloaded, this->fetcher, &SpacesFilesFetcher::handleAfterFilesReloaded);
    //connect(this, &ModelController::spaceRemoved, this->fetcher, &SpacesFilesFetcher::handleSpaceRemoved);

    // 线程结果返回信号
    connect(this->fetcher, &SpacesFilesFetcher::handleFilesCreatedProgressed, this, &ModelController::filesCreated);
    connect(this->fetcher, &SpacesFilesFetcher::handleSpaceConditionsProgressed, this, &ModelController::spaceFilesChangedForConditionsChanged);

    // 处理fetcher的扫描文件请求
    connect(this->fetcher, &SpacesFilesFetcher::requestRescanSpaceWithConditionsDescritpion, this, &ModelController::handleRescanSpaceWithCondition);
    connect(this->fetcher, &SpacesFilesFetcher::requestClearSpace, this, [=](int spaceId){
        Q_EMIT this->filesReloaded(QList<QSharedPointer<UKUI::IDM::FileInfo>>(), spaceId);
    });
    connect(this, &ModelController::spaceRemovedWithFilesRequest, this->fetcher, &SpacesFilesFetcher::handleSpaceRemoved);
    connect(this->fetcher, &SpacesFilesFetcher::requestFilesInSpaceById, this, [=](int spaceId){
        QList<QSharedPointer<UKUI::IDM::FileInfo>> infosToBeRemoved;
        auto proxyModel = getProxyModelBySpaceId(spaceId);
        if (proxyModel) {
            auto sourceModel = qobject_cast<FileItemModel *>(proxyModel->sourceModel());
            infosToBeRemoved = sourceModel->infos;
        }
        return infosToBeRemoved;
    }, Qt::BlockingQueuedConnection);

    Q_EMIT allModelCreated();
}

void ModelController::handleRescanSpaceWithCondition(int spaceId, const QVariant &conditionsDescription)
{
#ifdef COMPLIE_AS_PROJECT
    // fixme: 操作完成后释放资源
    auto searchTask = new SearchTask;
    searchTask->setSpaceId(spaceId);
    searchTask->setConditionsDescription(conditionsDescription);
    QThread *searchThread = new QThread;
    searchTask->moveToThread(searchThread);

    connect(searchThread, &QThread::started, searchTask, &SearchTask::start);
    connect(searchThread, &QThread::finished, searchThread, &QThread::deleteLater);
    connect(searchTask, &SearchTask::destroyed, searchThread, &QThread::quit);

    connect(searchTask, &SearchTask::finished, this, [=](){
        auto result = searchTask->resultInfos;
        result << searchTask->contentResultInfos;
        auto job = UKUI::IDM::FileInfoJob::fromInfos(result);
        job->setSpaceId(spaceId);
        job->setAutoDelete(true);
        connect(this, &ModelController::spaceConditionsChanged, job, &UKUI::IDM::FileInfoJob::handleCancelWithSpaceId);
        connect(this, &ModelController::spaceRemoved, job, &UKUI::IDM::FileInfoJob::handleCancelWithSpaceId);
        connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=](){
            auto realResult = job->getInfos();
            Q_EMIT this->filesReloaded(realResult, spaceId);
        }, Qt::BlockingQueuedConnection);
        job->start();

        delete searchTask;
    });

    // 如果此空间的条件被改变或者被编辑则取消此次search
    connect(this, &ModelController::spaceConditionsChanged, searchTask, &SearchTask::handleCancelWithSpaceId);
    connect(this, &ModelController::spaceRemoved, searchTask, &SearchTask::handleCancelWithSpaceId);

    searchThread->start();
#endif
}

void ModelController::warningWithDialog(const QString &title, const QString &errorMsg)
{
    QMessageBox box;
    box.setIcon(QMessageBox::Warning);
    box.createWinId();
    box.windowHandle()->setTransientParent(rootWindow);
    box.setWindowTitle(title);
    box.setText(errorMsg);
    box.exec();
}

void ModelController::criticalWithDialog(const QString &title, const QString &errorMsg)
{
    QMessageBox box;
    box.setIcon(QMessageBox::Critical);
    box.createWinId();
    box.windowHandle()->setTransientParent(rootWindow);
    box.setWindowTitle(title);
    box.setText(errorMsg);
    box.exec();
}

void ModelController::setRootWindow(QWindow *newRootWindow)
{
    rootWindow = newRootWindow;
}
