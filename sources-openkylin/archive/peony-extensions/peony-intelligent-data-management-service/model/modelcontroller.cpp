/*
 * Peony Intelligent Spaces Service
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
 * Authors: He JinQin <hejinqin@kylinos.cn>
 *
 */

#include "modelcontroller.h"
#include "spacesfilesfetcher.h"
#include "database/databasemanager.h"

#ifdef COMPLIE_AS_PROJECT
#include "condition/condition.h"
#include "search/searchtask.h"
#include "file/fileinfojob.h"
#include "ai/aisearchtask.h"
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
#include <QCheckBox>
#include <QDebug>
#include <QProcess>
#include"kylin-ai/common/common.h"
#include <kysdk/kysdk-system/libkydate.h>
#include <UkuiSearchTask>
#include <unistd.h>
#include<systemd/sd-journal.h>
#include<condition/condition.h>
#include "file/fileinfomanager.h"
#include "file/fileinfo.h"
#include <QtDBus/QDBusMetaType>
#include <QGSettings>
#include"kylin-ai/common/common.h"
#include "ai/aipreviewhelper.h"
#include "global-settings.h"
#include  <QDateTime>
static ModelController *instance = nullptr;
static bool modelInited = false;

ModelController *ModelController::globalInstance()
{
    if (!instance) {
        instance = new ModelController;
    }
    return instance;
}

ModelController::ModelController(const QString &dbName, const QString &filedbname ,QObject *parent) : QObject(parent)
{
    fetched = false;
    fetcher = new SpacesFilesFetcher(dbName,filedbname);
    thread = new QThread;
    fetcher->moveToThread(thread);
    connect(thread, &QThread::started, fetcher, &SpacesFilesFetcher::fetch);
    // 单测内主线程若在构造期间尚未进入事件循环，BlockingQueuedConnection 会与 worker 发射
    // fetched 形成死锁；extensions-test 定义 IDM_TEST_RELAX_MODEL_INIT 时改为队列连接。
#ifdef IDM_TEST_RELAX_MODEL_INIT
    connect(fetcher, &SpacesFilesFetcher::fetched, this, &ModelController::initModels, Qt::QueuedConnection);
#else
    connect(fetcher, &SpacesFilesFetcher::fetched, this, &ModelController::initModels, Qt::BlockingQueuedConnection);
#endif
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService("com.peony.idm.service")) {       
        qCritical() << "QDbus register service failed reason:" << sessionBus.lastError();
    }
    if(!sessionBus.registerObject("/org/peony/idm/space/service",this,QDBusConnection::ExportAllSignals|QDBusConnection::ExportAllSlots)) {
        qCritical() << "peony-idm-transformer dbus register object failed reason:" << sessionBus.lastError();
    }
    qRegisterMetaType<QList<QStringList>>("QList<QStringList>");
    qDBusRegisterMetaType<QList<QStringList>>();
    qRegisterMetaType<QSharedPointer<UKUI::IDM::FileInfo>>("QSharedPointer<UKUI::IDM::FileInfo>");
    qRegisterMetaType<QList<QSharedPointer<UKUI::IDM::FileInfo>>>("QList<QSharedPointer<UKUI::IDM::FileInfo>>");
    qRegisterMetaType<QMap<QString,QStringList>>("QMap<QString,QStringList>");
    qDBusRegisterMetaType<QMap<QString,QStringList>>();
    startLoadAllSpaces();
    auto infoManager = UKUI::IDM::FileInfoManager::globalInstance();
    auto kbmanager = SemanticKBManager::globalInstance();
    connect(infoManager, &UKUI::IDM::FileInfoManager::fileChanged, this, &ModelController::handleFileEvent, Qt::UniqueConnection);
}

void ModelController::startLoadAllSpaces()
{
    if (!thread->isRunning())
        thread->start();
    else {
        while (!fetched) {
            qApp->processEvents();
        }
    }
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

void ModelController::initModels()
{
    fetched = true;

//    auto spaces = fetcher->getAllSpacesName();
//    for (auto space : spaces) {
//        int id = fetcher->getSpaceIdByName(space);
//        auto infos = fetcher->getFilesInSpaceById(id, true);
//        QStringList userAddedInfos = infos;
//        auto autoAddedInfos = fetcher->getFilesInSpaceById(id, false);
//        infos << autoAddedInfos;
//    }

    // 从ukui-search获取文件创建事件

    // 定义 Unix 域套接字地址
   const std::string serverUnixPath =
       "unix:abstract=/tmp/.kylin-ai-business-unix/" +
       std::to_string(getuid()) + "/DataManagement.sock";

   // 连接到 Unix 域套接字
   QDBusConnection connection = QDBusConnection::connectToPeer(
       QString::fromStdString(serverUnixPath), "FileIndexFinished");

   // 检查连接是否成功
   if (!connection.isConnected()) {
       qDebug() << "Failed to connect to Unix domain socket:" << connection.lastError().message();
   }
   // 连接到 D-Bus 信号
   connection.connect("", "/com/kylin/AiBusiness/DataManagement", "com.kylin.AiBusiness.DataManagement", "FileIndexFinished", this->fetcher, SLOT(handleFileIndexCreated(QString)));
   connect(UKUI::IDM::FileInfoManager::globalInstance(), &UKUI::IDM::FileInfoManager::remoteFileEvent, this, [=](int eventType, const QString &arg1, const QString &arg2){
        switch (eventType) {
        case 1:
        case 101:
            Q_EMIT this->fileCreateRequest(arg1);
            break;
        case 102:{
            UKUI::IDM::FileInfoManager::globalInstance()->fileChanged(UKUI::IDM::FileInfoManager::FileDeleted, arg1, nullptr);
            Q_EMIT this->fileDeletedRequest(arg1);
            break;
            }
        case 103:
            UKUI::IDM::FileInfoManager::globalInstance()->fileChanged(UKUI::IDM::FileInfoManager::FileMoved, arg1, arg2);
            Q_EMIT this->fileMovedRequest(arg1, arg2);
            break;
        case 104:
            UKUI::IDM::FileInfoManager::globalInstance()->fileChanged(UKUI::IDM::FileInfoManager::FileAttrChanged, arg1, nullptr);
            break;
        default:
            break;
        }
    });
    // 在线程中处理新增文件和空间条件改变
   connect(this, &ModelController::spaceRemovedWithFilesRequest, this->fetcher, &SpacesFilesFetcher::handleSpaceRemoved,Qt::QueuedConnection);
   connect(this, &ModelController::spaceConditionsChanged, this->fetcher, &SpacesFilesFetcher::updateSpaceConditions);
   connect(this, &ModelController::fileCreateRequest, this->fetcher, &SpacesFilesFetcher::handleFileCreated);
   connect(this, &ModelController::filesReloaded, this->fetcher, &SpacesFilesFetcher::handleAfterFilesReloaded);
   connect(this, &ModelController::fileDeletedRequest, this->fetcher, &SpacesFilesFetcher::handleFileDeleted);
   connect(this, &ModelController::fileMovedRequest, this->fetcher, &SpacesFilesFetcher::handleFileMoved);
   connect(this, &ModelController::filesReloadedByCreate, this->fetcher, &SpacesFilesFetcher::handleAfterFilesReloadedBycreate);
   connect(this->fetcher, &SpacesFilesFetcher::updateSpacefileinfo, this, &ModelController::spaceUpdatefinish);
   connect(this->fetcher, &SpacesFilesFetcher::requestRescanSpaceWithConditionsDescritpion, this, &ModelController::handleRescanSpaceWithCondition);
   connect(this->fetcher,&SpacesFilesFetcher::FileDeleted,this,&ModelController::fileDeleted);
   connect(this->fetcher,&SpacesFilesFetcher::spaceCreateFinish,this,&ModelController::spaceCreateFinish);
   // 线程结果返回信号
   connect(this->fetcher, &SpacesFilesFetcher::handleFilesCreatedProgressed, this, &ModelController::filesCreated);
}

QStringList ModelController::getAllSpacesName(){
    QStringList spaces = fetcher->getAllSpacesName();
    return spaces;
}

int ModelController::getSpaceIdByName(const QString &spaceName){
    int spaceid = fetcher->getSpaceIdByName(spaceName);
    return spaceid;
}

bool ModelController::onSpaceCreateRequest(const QString &spaceName, const QString &conditionDescription, bool isCreatedFromSearch)
{
    return this->onSpaceCreateRequestWithFiles(spaceName, conditionDescription, isCreatedFromSearch, QStringList());
}
bool ModelController::onSpaceCreateRequestWithFiles(const QString &spaceName, const QString &conditionDescription, bool isCreatedFromSearch, QStringList infos)
{
    DatabaseManager manager;
    QString errMsg;
    QString realSpaceName;
    bool hasInitFiles = !infos.isEmpty();
    if (manager.createSpace(spaceName, QStringList(), &errMsg, &realSpaceName)) {
        int spaceId = manager.getSpaceIdByName(realSpaceName);
        QJsonDocument doc = QJsonDocument::fromJson(conditionDescription.toUtf8());
        auto jsonobj = doc.object();
        QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(spaceId) + ".json";
        QFile file(jsonPath);
        if (file.open(QFile::WriteOnly|QFile::Text)) {
            file.write(doc.toJson());
            fetcher->rwLock.lockForWrite();
            fetcher->spaces.insert(spaceId, realSpaceName);
#ifdef COMPLIE_AS_PROJECT
            auto conditions = Condition::generateContitionsFromDescription(jsonobj);
            fetcher->spacesConditions.insert(spaceId, conditions);
            auto labels = Condition::getLabelsFromDescription(jsonobj);
            fetcher->spacesLabels.insert(spaceId, labels);
#endif
            fetcher->rwLock.unlock();
            qDebug() << "space created" << spaceId << realSpaceName;

            Q_EMIT filesReloadedByCreate(infos, spaceId);

            if (!isCreatedFromSearch) {
                this->handleRescanSpaceWithCondition(spaceId, jsonobj);
            }
        }        
        return true;
    } else {
//         kdk::KMessageBox questionBox;
//         questionBox.setIconPixmap(QPixmap (5,5));
//         questionBox.setIcon(kdk::KMessageBox::Icon::Warning);
//         auto okButton = new QPushButton(tr("OK"));
//         questionBox.addButton(okButton, kdk::KMessageBox::AcceptRole);
//         questionBox.setText(tr("Failed to create space %1,%1 is existed").arg(spaceName));
//         okButton->setProperty("isImportant", true);
//         questionBox.createWinId();
//         questionBox.windowHandle()->setTransientParent(rootwindow);
//         questionBox.exec();
           qDebug() << "create space failed" << realSpaceName<<" "<<errMsg;
           return false;
    }
}
bool ModelController::renameSpace(int spaceId, const QString &spaceName)
{
    DatabaseManager manager;
    QString errorMsg;
    QString oldSpaceName = fetcher->getSpaceNameById(spaceId);
    if (manager.renameSpace(oldSpaceName, spaceName, &errorMsg)) {
        fetcher->rwLock.lockForWrite();
        fetcher->spaces[spaceId] = spaceName;
        fetcher->rwLock.unlock();
        SemanticKBManager::globalInstance()->setSpaceConditionChanged(QString::number(spaceId), true);
//        Q_EMIT spaceNameUpdated(spaceId, spaceName);
        return true;
    } else {
//        kdk::KMessageBox questionBox;
//        questionBox.setIconPixmap(QPixmap (5,5));
//        questionBox.setIcon(kdk::KMessageBox::Icon::Warning);
//        auto okButton = new QPushButton(tr("OK"));
//        questionBox.addButton(okButton, kdk::KMessageBox::AcceptRole);
//        questionBox.setText(tr("Failed to rename space %1 to %2, %2 is existed.").arg(oldSpaceName).arg(spaceName));
//        okButton->setProperty("isImportant", true);
//        questionBox.createWinId();
//        questionBox.windowHandle()->setTransientParent(rootwindow);
//        questionBox.exec();
        qDebug() <<tr("Failed to rename space %1 to %2, %2 is existed.").arg(oldSpaceName).arg(spaceName);
        return false;
    }
}

void ModelController::editSpaceCondition(QString spacename,QString conditons)
{
    int spaceid = getSpaceIdByName(spacename);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(conditons.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();
    QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(spaceid) + ".json";
    QJsonDocument doc(jsonObj);
    QFile jsonFile(jsonPath);
    QFile file(jsonPath);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(doc.toJson());
        file.close();
        }
    Q_EMIT spaceConditionsChanged(spaceid,spacename, jsonObj);
    Q_EMIT cancelSpaceSearch(spaceid);
}

bool ModelController::removeSpace(int spaceId){
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
        // 删除对应语义库
        SemanticKBManager::globalInstance()->deleteKBForSpace(QString::number(spaceId));
        Q_EMIT spaceRemoved(spaceId);
        return true;
    } else {
        qCritical()<<tr("Failed to remove space %1, error message: %2").arg(spaceName).arg(errorMsg);
    }
    return false;
}
void ModelController::spaceRemovedWithFiles(int spaceId, const QStringList &filesToBeRemoved){
    Q_EMIT spaceRemovedWithFilesRequest(spaceId,filesToBeRemoved);
}
void ModelController::handleRescanSpaceWithCondition(int spaceId,const QVariant &conditionsDescription){
#ifdef COMPLIE_AS_PROJECT
    // 如果包含文件内容的非或类型条件，则走AISearch
    contentFileinfo.clear();
    Aisearchfinish = true;
    searchfinish = false;
    bool isAndCondition = false;
    bool needcontent = true;
    QJsonObject aijsonObj = conditionsDescription.toJsonObject();
    QStringList keywords;

    int count = aijsonObj["conditions count"].toInt();
    for (int i = 0; i < count; i++) {
        auto fileContentObj = aijsonObj[QString::number(i)].toObject();
        auto condition = Condition::fromJsonObject(fileContentObj);
        if (condition->type() == Condition::ConditionMatchRule) {
            needcontent = condition->getConditionMatchRule() == 0?true:false;
        }
        if(condition->type() == Condition::FileName){
          keywords =  condition->getFileNameKeywords();
          bool iscontain = condition->getFileNameIsContains();//如果不包含则不走文件内容搜索流程
          if(!iscontain){
              needcontent = false;
          }
          fileContentObj["condition type"] = int(Condition::FileContent);
          fileContentObj["content"] = keywords.isEmpty()?"":keywords.first();
          fileContentObj["orCondition"] = false;
          aijsonObj[QString::number(i)] = fileContentObj;
        }
    }
    if (needcontent && !keywords.isEmpty()) {
        if (true) {
//            if(!condition->isAndCondition())
//                isAndCondition = false;
            auto aiSearchTask = new AISearchTask(keywords.first());
            aiSearchTask->setFilterConditionDescription(aijsonObj);
            aiSearchTask->setSpaceId(spaceId);
            auto aiSearchThread = new QThread;
            aiSearchTask->moveToThread(aiSearchThread);

            connect(aiSearchThread, &QThread::started, aiSearchTask, &AISearchTask::doQuery);
            connect(aiSearchThread, &QThread::finished, aiSearchThread, &QThread::deleteLater);
            connect(aiSearchTask, &SearchTask::destroyed, aiSearchThread, &QThread::quit);
            connect(aiSearchTask, &AISearchTask::finisheds, this, [=](bool successed, const QList<QString> &results, const QJsonDocument &conditions, const QString &errorMsg){
                /*thread->quit();*/}, Qt::QueuedConnection
                );
            connect(aiSearchTask, &AISearchTask::finished, this, [=,this](bool successed, const QList<QString> &result){
                if (successed) {
                    QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
                    for (auto path:result) {
                        auto info = UKUI::IDM::FileInfo::fromPath(path);
                        infos<<info;
                    }
                    auto job = UKUI::IDM::FileInfoJob::fromInfos(infos);
                    job->setSpaceId(spaceId);
                    job->setAutoDelete(true);
                    connect(this, &ModelController::cancelSpaceSearch, job, &UKUI::IDM::FileInfoJob::handleCancelWithSpaceId);
                    connect(this, &ModelController::spaceRemoved, job, &UKUI::IDM::FileInfoJob::handleCancelWithSpaceId);
                    connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=,this](){

                        auto infos = job->getInfos();
                         QList<QString> realResult;
                         for (QSharedPointer<UKUI::IDM::FileInfo> info:infos) {
                               auto path = info->path();
                               realResult<<path;
                         }
                        //或条件当搜索结束后与普通条件搜索结果取并集
                        if(searchfinish){
                            realResult<<contentFileinfo;
                            QSet<QString> set = QSet<QString>::fromList(realResult);
                            Q_EMIT this->filesReloaded(set.values(), spaceId);
                        }else{
                            contentFileinfo = realResult;
                            //或条件当AI搜索结束后 普通搜索还没结束暂不更新UI
                            //Q_EMIT this->filesReloaded(realResult, spaceId);
                        }
                        Aisearchfinish = true;
                    }, Qt::BlockingQueuedConnection);
                    job->start();
                }else{
                     Aisearchfinish = true;
                }
                // 安全删除
                aiSearchThread->quit();
                aiSearchTask->deleteLater();;
            });

            // 如果此空间的条件被改变或者被编辑则取消此次search
            connect(this, &ModelController::cancelSpaceSearch, aiSearchTask, &AISearchTask::cancelWithSpaceId);
            connect(this, &ModelController::spaceRemoved, aiSearchTask, &AISearchTask::cancelWithSpaceId);
            if(getAinotAvailabletype() == allFit){
                Aisearchfinish = false;
                aiSearchThread->start();
            }
//            if(isAndCondition)
//                return;
        }
    }

    // fixme: 操作完成后释放资源
    auto searchTask = new SearchTask;
    searchTask->setSpaceId(spaceId);
    searchTask->setConditionsDescription(conditionsDescription);
    QThread *searchThread = new QThread;
    searchTask->moveToThread(searchThread);

    connect(searchThread, &QThread::started, searchTask, &SearchTask::start);
    connect(searchThread, &QThread::finished, searchThread, &QThread::deleteLater);
    connect(searchTask, &SearchTask::destroyed, searchThread, &QThread::quit);

    connect(searchTask, &SearchTask::finished, this, [=,this](){
        auto result = searchTask->resultInfos;
        result << searchTask->contentResultInfos;
        auto job = UKUI::IDM::FileInfoJob::fromInfos(result);
        job->setSpaceId(spaceId);
        job->setAutoDelete(true);
        connect(this, &ModelController::cancelSpaceSearch, job, &UKUI::IDM::FileInfoJob::handleCancelWithSpaceId);
        connect(this, &ModelController::spaceRemoved, job, &UKUI::IDM::FileInfoJob::handleCancelWithSpaceId);
        connect(job, &UKUI::IDM::FileInfoJob::finished, this, [=,this](){
            searchfinish = true;
            auto infos = job->getInfos();
             QList<QString> realResult;
             for (QSharedPointer<UKUI::IDM::FileInfo> info:infos) {
                   auto path = info->path();
                   realResult<<path;
             }
            if(Aisearchfinish&&needcontent){
                realResult<<contentFileinfo;
                QSet<QString> set = QSet<QString>::fromList(realResult);
                Q_EMIT this->filesReloaded(set.values(), spaceId);
            }else{
                contentFileinfo = realResult;
                if(!needcontent){
                    Q_EMIT this->filesReloaded(contentFileinfo, spaceId);
                }
                //Q_EMIT this->filesReloaded(realResult, spaceId);
            }
            searchfinish = true;
        }, Qt::BlockingQueuedConnection);
        job->start();
        searchThread->quit();
        searchTask->deleteLater();;
    });
     //如果此空间的条件被改变或者被编辑则取消此次search
    connect(this, &ModelController::cancelSpaceSearch, searchTask, &SearchTask::handleCancelWithSpaceId);
    connect(this, &ModelController::spaceRemoved, searchTask, &SearchTask::handleCancelWithSpaceId);
    searchThread->start();
#endif
}

void ModelController::handleFileEvent(UKUI::IDM::FileInfoManager::FileEventType type, const QString &arg1, const QString arg2)
{
    using namespace UKUI::IDM;
    QStringList filePaths;
    filePaths.append(arg1);
    QString path = arg1;
    QStringList spaceNames = getAllSpacesName();
    switch (type) {
    // fixme: 使用pluma等文本编辑器修改文件后无法监听到后续事件
    case FileInfoManager::FileEventType::FileIgnoredAutomatic: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        auto info = UKUI::IDM::FileInfo::fromPath(path, false);
        for (auto name : spaceNames) {
            QStringList infos = getFilesInSpaceByName(name);
            if (infos.contains(path)) {
                auto infoJob = FileInfoJob::fromPaths(QStringList()<<path, false);
                connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
                    delete infoJob;
                });
                connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                    auto infos = infoJob->getInfos();
                    for (auto info : infos) {
                        FileInfoManager::globalInstance()->addFileToWatcher(info->uri());
                    }
                    delete infoJob;
                });
                if(!QFile::exists(path)){
                    removeFileFromSpace(name,filePaths);
                }
                connect(infoJob, &FileInfoJob::errored, this, [=]{
                    // 删除文件
                    //removeFile(path);
                    removeFileFromSpace(name,filePaths);
                });
                infoJob->start();
            }
        }
        break;
    }
    case FileInfoManager::FileEventType::UserAddFile: {
        auto info = FileInfo::fromPath(path);
        auto infoJob = FileInfoJob::fromPaths(QStringList()<<path);
        connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
            delete infoJob;
        });
        connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            delete infoJob;
        });
        infoJob->start();
        break;
    }
    case FileInfoManager::FileEventType::FileDeleted: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        QStringList spaceNames = getAllSpacesName();
        for (auto name : spaceNames) {
            QStringList infos = getFilesInSpaceByName(name);
            if (infos.contains(path)) {
                removeFileFromSpace(name,filePaths);
            }
        }
        break;
    }
    case FileInfoManager::FileEventType::FileMoved: {
        QStringList spaceNames = getAllSpacesName();
        QStringList newFilePathList;
        newFilePathList.append(arg2);
        for (auto name : spaceNames) {
            QStringList filelist =  this->fetcher->getFilesInSpace(name,true);
            QStringList infos = getFilesInSpaceByName(name);
            bool isUserRemove = filelist.contains(path);
            if (infos.contains(path)) {
                removeFileFromSpace(name,filePaths);
                if(isUserRemove){
                    userAddFilesToSpace(newFilePathList,name);
                }
            }
        }
        break;
    }
    case FileInfoManager::FileEventType::FileAttrChanged:
    case FileInfoManager::FileEventType::FileModified: {
        if (!arg2.isEmpty()) {
            // 文件夹改变事件，跳过
            return;
        }
        // 更新文件信息
        auto info = UKUI::IDM::FileInfo::fromPath(path, false);
        for (auto name : spaceNames) {
            QStringList infos = getFilesInSpaceByName(name);
            if (infos.contains(path)) {
                auto infoJob = FileInfoJob::fromPaths(QStringList()<<path);
                connect(infoJob, &UKUI::IDM::FileInfoJob::cancelled, [=]{
                    delete infoJob;
                });
                connect(infoJob, &UKUI::IDM::FileInfoJob::finished, this, [=]{
                    auto infos = infoJob->getInfos();
                    Q_EMIT spaceDataChanged(name,filePaths);
                    delete infoJob;
                });
                infoJob->start();
            }
        }
        break;
    }
    default:
        break;
    }
}

QStringList ModelController::getFilesInSpaceByName(QString spacename, bool isUserAdded){
    int id = fetcher->getSpaceIdByName(spacename);
    auto infos = fetcher->getFilesInSpaceById(id, true);
    auto autoAddedInfos = fetcher->getFilesInSpaceById(id, false);
    infos << autoAddedInfos;
    return infos;
}

bool ModelController::userAddFilesToSpace(QStringList paths,QString SpaceName)
{
    // fixme: run in thread
    DatabaseManager manager;
    int spaceId = getSpaceIdByName(SpaceName);
    bool ok = true;
    for (auto path : paths) {
        if (QFileInfo(path).isDir()) {
            continue;
        }
        QString errMsg;
        ok = manager.addFileToSpace(path, spaceId, true, &errMsg);
        if(ok){
            handleFileEvent(UKUI::IDM::FileInfoManager::UserAddFile, path, nullptr);
            this->fetcher->updateSpaceFiles(SpaceName,path,true,SpacesFilesFetcher::OperationType::Add);
        }
    }
    SemanticKBManager::globalInstance()->setSpaceConditionChanged(QString::number(spaceId), true);
    return ok;
}

QString ModelController::getConditionsBySpaces(QString spacename){
    int spaceId = getSpaceIdByName(spacename);
//    QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/spaces/" + QString::number(spaceId) + ".json";
//    QJsonDocument doc;
//    QFile jsonFile(jsonPath);
//    QString conditionobj;
//    if (jsonFile.open(QFile::ReadOnly | QFile::Text)) {
//        doc = QJsonDocument::fromJson(jsonFile.readAll());
//        conditionobj = doc.toJson(QJsonDocument::Indented);
//        jsonFile.close();
//    }
    QList<std::shared_ptr<Condition>> conditions = this->fetcher->getSpaceconditon(spacename);
    QStringList  labels = this->fetcher->getSpacelabels(spacename);
    auto obj = Condition::pharseConditions(conditions,labels);
    QJsonDocument doc(obj.toJsonObject());
    QString conditionsobj = doc.toJson();
    return conditionsobj;
}

bool ModelController::updateSpacestatus(QString spacename,bool status){
    DatabaseManager manager;
    QString errorMsg;
    QString realSpaceName = spacename;
    if (spacename.isEmpty()) {//适配创建默认无名字的空间时
        auto currentSpaceNames = getAllSpacesName();
        int count = 1;
        while (true) {
            auto currentSpaceName = tr("New Space %1").arg(count);
            realSpaceName = tr("New Space %1").arg(count-1);
            if (currentSpaceNames.contains(currentSpaceName)) {
                count++;
            } else {
                break;
            }
        }
    }
    int spaceId = getSpaceIdByName(realSpaceName);
    if (manager.updateSpacestatus(realSpaceName, status)) {
        this->fetcher->updateSpaceStatus(spaceId,status);
        return true;
    } else {
        qCritical()<<tr("Failed to updatestatus space %1, error message: %2").arg(spacename).arg(errorMsg);
    }
    return false;
}
bool ModelController::getSpacestatus(QString spacename){
    int spaceId = getSpaceIdByName(spacename);
    return this->fetcher->getSpaceStatus(spaceId);
}

bool ModelController::removeFileFromSpace(const QString &SpacesName, const QStringList &filepaths)
{
    DatabaseManager manager;
    QString errorMsg;
    QStringList filelist =  this->fetcher->getFilesInSpace(SpacesName,true);
    int spaceId = getSpaceIdByName(SpacesName);
    for (auto path : filepaths) {
        this->fetcher->updateSpaceFiles(SpacesName,path,true,SpacesFilesFetcher::OperationType::Remove);
        bool isUserRemove = filelist.contains(path);
        bool ok = manager.removeFileFromSpace(path,spaceId,isUserRemove,&errorMsg);
        if(!ok)
            return false;
    }
    Q_EMIT this->spacefileloadfinish(SpacesName);
    return true;
}

void ModelController::startsearch(QString keyword,QString SpaceName,QString ClientId){
     m_clientId = ClientId;
    if (m_searchmodel) {
         m_searchmodel->deleteLater();
    }
     m_searchmodel = new SearchResultModel;
     if(!SpaceName.isEmpty()){
        auto paths = getFilesInSpaceByName(SpaceName);
        m_searchmodel->setSearchInSpace(true);
        m_searchmodel->setSearchInSpacePaths(paths);
     }else{
         m_searchmodel->setSearchInSpace(false);
         m_searchmodel->setSearchInSpacePaths(QStringList());
     }
     m_searchmodel->setSearchKeyword(keyword);
}

void ModelController::reSearchWithLabels(QString keyword,QStringList Labels,QString SpaceName){
    cancelSearch();
    auto searchmodel = new SearchResultModel;
    if(!SpaceName.isEmpty()){
       auto paths = getFilesInSpaceByName(SpaceName);
       searchmodel->setSearchInSpace(true);
       searchmodel->setSearchInSpacePaths(paths);
    }else{
        searchmodel->setSearchInSpace(false);
        searchmodel->setSearchInSpacePaths(QStringList());
    }
    searchmodel->setLabels(Labels);
    searchmodel->setSearchKeyword(keyword);
}

void ModelController::SearchContentLoad(QStringList infolist){
    Q_EMIT searchContentReload(infolist,m_clientId);
}

void ModelController::fileReloadByBatch(QStringList infolist,bool finish){
    Q_EMIT searchfileReloadByBatch(infolist,m_clientId,finish);
}

void ModelController::unmountFilesInSpace(QString unmountPath)
{
    DatabaseManager manager;
    QString errorMsg;
    auto spaceNames = getAllSpacesName();
    for (auto SpacesName : spaceNames) {
        QStringList filelist =  this->fetcher->getFilesInSpace(SpacesName,true);
        int spaceId = getSpaceIdByName(SpacesName);
        auto filepaths = getFilesInSpaceByName(SpacesName);
        for (auto path : filepaths) {
            if(path.contains(unmountPath)){
            this->fetcher->updateSpaceFiles(SpacesName,path,true,SpacesFilesFetcher::OperationType::Remove);
            bool isUserRemove = filelist.contains(path);
            bool ok = manager.removeFileFromSpace(path,spaceId,isUserRemove,&errorMsg);
            }
        }
        Q_EMIT this->spacefileloadfinish(SpacesName);
    }
    return ;
}

void ModelController::fileTextContentLoad(QList<QStringList> infolist)
{
    Q_EMIT searchTextContentReload(infolist,m_clientId);
}

void ModelController::updateSearchCondition(QString conditions){
    Q_EMIT searchConditionChanged(conditions);
}

void  ModelController::spaceUpdatefinish(QString spacename){
    Q_EMIT spacefileloadfinish(spacename);
}

void ModelController::filesCreated(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, int spaceId){

    QString spaceName = this->fetcher->getSpaceNameById(spaceId);
    QStringList spacefiles =  getFilesInSpaceByName(spaceName,false);
    bool needGotoUri = spacefiles.isEmpty();
    QStringList filelist;
    for (auto info:infos) {
        filelist<<info->path();
    }
    Q_EMIT spacesFilesAutoInsert(spaceName,filelist,needGotoUri);
}

void ModelController::fileDeleted(QMap<QString,QStringList> deleteSpacesFile){
     Q_EMIT spacesFilesAutoDelete(deleteSpacesFile);
}

void ModelController::cancelSearch(){
    Q_EMIT cancelSearchRequest();
}

void ModelController::spaceCreateFinish(QString spaceName,QStringList fileinfos){
    Q_EMIT SpaceCreatefinished(spaceName,fileinfos);
}

void ModelController::requestShowAISubsystemDialog()
{
    int type = getAinotAvailabletype();
    if(type==1)
        QProcess::startDetached("ukui-control-center", {"-m", "search"});
    else
        QProcess::startDetached("ukui-control-center", {"-m", "aisubsystem"});
}

bool ModelController::isAisubsysteminited()
{
    return is_ai_subsystem_inited();
}

int ModelController::getAinotAvailabletype()
{
    bool flage = false;
    const QByteArray id("org.ukui.search.settings");
    if (QGSettings::isSchemaInstalled(id)) {
        auto settings = new QGSettings(id, QByteArray(), nullptr);
        QStringList keys = settings->keys();
        if (keys.contains("aiIndexEnable")) {
            flage = settings->get("aiIndexEnable").toBool();
        }
        delete settings;
    }
    if(!isAisubsysteminited())
        return aiSubsystemNotFit;
    if(!flage)
        return aiIndexNotFit;
    return allFit;
}

bool ModelController::isUserAddFile(const QString &spaceName, const QString &FileName)
{
    return this->fetcher->isUserAddFile(spaceName,FileName);
}

void ModelController::setCurrentSearchPath(const QString &path)
{
    m_currentPath = path;
}

QString ModelController::getCurrentSearchpath()
{
    return m_currentPath;
}

QStringList ModelController::getIntelligentLabel(QString keyword){
    QStringList labels;
    bool isAiavailable = getAinotAvailabletype()==allFit ?true:false;
    if(!isAiavailable){
        return labels;
    }
    AILabelHelper labelHelper;
    if (keyword.isEmpty()) {
        labels = labelHelper.getTopLabels(50);
    } else {
        labels = labelHelper.getSimilarLabels(keyword);
    }
    return labels;
}

QString ModelController::getFileSummery(QString filepath)
{
    QString fileSummery;
    bool isAiavailable = getAinotAvailabletype()==allFit ?true:false;
    if(!isAiavailable){
        return fileSummery;
    }
    AIPreviewHelper aiPreviewHelper;
    if (!filepath.isEmpty()) {
        fileSummery = aiPreviewHelper.getFileSummary(filepath);
    }
    return fileSummery;
}

QStringList ModelController::getFileLabels(QString filepath){
    QStringList fileLabelList;
    bool isAiavailable = getAinotAvailabletype()==allFit ?true:false;
    if(!isAiavailable){
        return fileLabelList;
    }
    AIPreviewHelper aiPreviewHelper;
    if (!filepath.isEmpty()) {
        fileLabelList = aiPreviewHelper.getFileLabels(filepath);
    }
    return fileLabelList;
}

qint64 ModelController::getSpaceAccesstime(QString spaceName)
{
    int spaceId = fetcher->getSpaceIdByName(spaceName);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString jsonPath = QString("%1/spaces/%2.json").arg(configPath).arg(spaceId);
    QFileInfo spaceConfigFile(jsonPath);
    auto createTime = spaceConfigFile.birthTime();
    auto createSec = createTime.toSecsSinceEpoch();
    return createSec;
}

qint64 ModelController::getSpaceModifytime(QString spaceName)
{
    int spaceId = fetcher->getSpaceIdByName(spaceName);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString jsonPath = QString("%1/spaces/%2.json").arg(configPath).arg(spaceId);
    QFileInfo spaceConfigFile(jsonPath);
    auto modifiedTime = spaceConfigFile.lastModified();
    auto modifiedSec = modifiedTime.toSecsSinceEpoch();
    return modifiedSec;
}

QVariantHash ModelController::getSpacesSortConfig(int spaceId)
{
    QVariantHash values;
    values = SpacesSettings::globalInstance()->getSpacesSortConfig(spaceId);
    return values;
}

void ModelController::userAddFilesToSpaceBySpaceplugin(const QStringList &filePath, const QString &spaceName, bool isUserAdd)
{
    Q_EMIT userAddFilesToSpaceBySpacepluginSignal(filePath,spaceName,isUserAdd);
}

void ModelController::editSpaceConditionBySpaceplugin(const QString &uri)
{
    Q_EMIT editSpaceConditionBySpacepluginSignal(uri);
}

void ModelController::setSpacesSortConfig(int spaceId, int sortType, int sortOrder, int userAddedtype){
    SpacesSettings::globalInstance()->setSpacesSortConfig(spaceId, sortType, sortOrder == Qt::AscendingOrder? 0: 1, userAddedtype);
}

// 语义知识库相关接口实现
QString ModelController::semanticKBCreate(const QString &spaceName)
{
    int spaceid = getSpaceIdByName(spaceName);
    return SemanticKBManager::globalInstance()->getOrCreateKBForSpace(spaceName, QString::number(spaceid));
}

QStringList ModelController::semanticKBListDocuments(const QString &spaceName)
{
    int spaceid = getSpaceIdByName(spaceName);
    return SemanticKBManager::globalInstance()->listDocumentsInSpace(QString::number(spaceid));
}

int ModelController::semanticKBGetDocumentStatus(const QString &spaceName, const QString &docPath)
{
    qDebug() << "semanticKBGetDocumentStatus called:" << spaceName << docPath;
    int spaceid = getSpaceIdByName(spaceName);
    qDebug() << "spaceid:" << spaceid;
    auto docId = generateDocId(spaceName,docPath);
    qDebug() << "docId:" << docId;
    int st = -1;
    int *status = &st;
    int result = SemanticKBManager::globalInstance()->getDocumentStatus(spaceName,QString::number(spaceid), docId, status);
    qDebug() << "getDocumentStatus result:" << result << "status:" << (status ? *status : -1);
    return result;
}

QString ModelController::semanticKBChat(const QString &spaceName, const QString &question, const QString &model)
{
    int spaceid = getSpaceIdByName(spaceName);
    return SemanticKBManager::globalInstance()->chatWithSpace(QString::number(spaceid), question, model);
}

bool ModelController::semanticKBDeleteSpaceKB(const QString &spaceName)
{
    int spaceid = getSpaceIdByName(spaceName);
    return SemanticKBManager::globalInstance()->deleteKBForSpace(QString::number(spaceid));
}

bool ModelController::semanticKBAddDocumentsForSpace(const QString &spaceName/* const QStringList &docPaths,
                                             const QStringList &docIds, QList<int> *errorCode*/)
{
    int spaceid = getSpaceIdByName(spaceName);
    QStringList docPaths = getFilesInSpaceByName(spaceName);
    QStringList docIds;
    for(auto path : docPaths){
        auto docid = generateDocId(spaceName,path);
        docIds.append(docid);
    }
    QList<int> errorCode;
    return SemanticKBManager::globalInstance()->addDocumentsToSpace(QString::number(spaceid), docPaths, docIds, &errorCode);
}

bool ModelController::semanticKBDeleteDocuments(const QString &spaceName,
                                                const QStringList &docPaths)
{
    int spaceid = getSpaceIdByName(spaceName);
    QStringList docIds;
    for(auto path : docPaths){
        auto docid = generateDocId(spaceName,path);
        docIds.append(docid);
    }
    QList<int> errorCode;
    return SemanticKBManager::globalInstance()->deleteDocumentsFromSpace(QString::number(spaceid), docIds, &errorCode);
}

QString ModelController::generateDocId(const QString &spaceName, const QString &path)
{
//    QFileInfo fileInfo(path);
//    QString fileName = fileInfo.fileName();
    int spaceid = getSpaceIdByName(spaceName);
    return QString("Space_%1_DocID_%2").arg(spaceid).arg(path);
}

bool ModelController::semanticKBDeleteAllDocuments(const QString &spaceName)
{
    int spaceid = getSpaceIdByName(spaceName);
    return SemanticKBManager::globalInstance()->deleteAllDocumentsInSpace(QString::number(spaceid));
}

bool ModelController::semanticKBDeleteFileFromAllSpaces(const QString &filePath)
{
    // 从所有语义库中删除该文件
    QStringList allSpaceNames = getAllSpacesName();
    for (const QString &spaceName : allSpaceNames) {
        QString docId = generateDocId(spaceName, filePath);
        SemanticKBManager::globalInstance()->deleteFileFromAllSpaces(filePath, docId);
    }
    return true;
}
