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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#ifndef MODELCONTROLLER_H
#define MODELCONTROLLER_H

#include <QObject>
#include <QUrl>

#include <atomic>

#include<QWindow>
#include "model/searchresultmodel.h"
#include "file/fileinfo.h"
#include "ai/ailabelhelper.h"
#include "ai/semantickbmanager.h"
#include "spacessettings.h"
#include "file/fileinfomanager.h"
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)|| defined(_M_ARM64)
    #define ARCH_AVAILABLE_AI
#endif
class SpacesFilesFetcher;
class Condition;
class ModelController : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.peony.idm.modelcontroller")
public:
    enum AIConditionType{
        allFit=0,
        aiIndexNotFit,
        aiSubsystemNotFit
    };
    static ModelController *globalInstance();
    explicit ModelController(const QString &dbName = "peony-idm.db", const QString &filedbname = "peony-idm-file.db",QObject *parent = nullptr);
    ~ModelController();
    void spaceCreateFinish(QString spaceName, QStringList fileinfos);
public Q_SLOTS:
    //加载所有空间
    void startLoadAllSpaces();
    //获取空间名字
    QStringList getAllSpacesName();
    //根据空间名获取spaceid
    int getSpaceIdByName(const QString &spaceName);
    //创建空白空间请求
    bool onSpaceCreateRequest(const QString &spaceName, const QString &conditionDescription, bool isCreatedFromSearch);
    //创建带有文件的空间 从搜索
    bool onSpaceCreateRequestWithFiles(const QString &spaceName, const QString &conditionDescription, bool isCreatedFromSearch, QStringList infos);
    bool renameSpace(int spaceId, const QString &spaceName);
    //条件变化时点击保存 空间内容重新开始搜素
    void editSpaceCondition(QString spacename,QString conditons);
    bool removeSpace(int spaceId);
    //删除空间需要 告知删除空间的文件
    void spaceRemovedWithFiles(int spaceId, const QStringList &filesToBeRemoved);
    //根据空间名获取对应空间中的文件
    QStringList getFilesInSpaceByName(QString spacename, bool isUserAdded = false);
    //手动添加文件到空间中
    bool userAddFilesToSpace(QStringList paths,QString Spacename);
    //开始搜索关键字
    void startsearch(QString keyword,QString SpaceName,QString ClientId);
    //搜索完成通知插件更新文件内容搜索文件信息
    void SearchContentLoad(QStringList infolist);
    void fileTextContentLoad(QList<QStringList> infolist);
    //搜索界面条件更新 通知插件根据条件筛选文件
    void updateSearchCondition(QString conditions);
    //获取空间条件
    QString getConditionsBySpaces(QString spacename);
    //更新空间自动更新状态
    bool updateSpacestatus(QString spacename,bool status);
    //获取空间自动更新状态
    bool getSpacestatus(QString spacename);
    //根据搜素关键字获取相似标签 没有关键字则获取top50
    QStringList getIntelligentLabel(QString keyword="");
    QString getFileSummery(QString filepath);
    //通知空间重新更新文件视图界面
    void spaceUpdatefinish(QString spacename);
    //有符合要求的文件 通知空间自动更新 包括新增和删除
    void filesCreated(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &newFiles, int spaceId);
    void fileDeleted(QMap<QString,QStringList> deleteSpacesFile);
    //移除文件从空间中
    bool removeFileFromSpace(const QString &SpacesName, const QStringList &filepaths);
    //取消搜索 通知搜索线程 取消搜索流程
    void cancelSearch();
    //根据标签重新搜索
    void reSearchWithLabels(QString keyWord, QStringList labels, QString SpaceName);
    //检测AI环境接口
    void requestShowAISubsystemDialog();
    bool isAisubsysteminited();
    int  getAinotAvailabletype();
    bool isUserAddFile(const QString &spaceName,const QString &FileName);
    void setCurrentSearchPath(const QString &path);
    QString getCurrentSearchpath();
    QStringList getFileLabels(QString filepath);
    qint64 getSpaceAccesstime(QString spaceName);
    qint64 getSpaceModifytime(QString spaceName);
    void setSpacesSortConfig(int spaceId, int sortType, int sortOrder, int userAddedFirst);
    QVariantHash getSpacesSortConfig(int spaceId);
    void userAddFilesToSpaceBySpaceplugin(const QStringList &filePath,const QString &spaceName,bool isUserAdd);
    void editSpaceConditionBySpaceplugin(const QString &uri);
     //搜索完成通知插件更新文件名文件信息
    void fileReloadByBatch(QStringList infolist,bool finish = false);
    void unmountFilesInSpace(QString unmountPath);
    // 语义知识库相关接口
    QString semanticKBCreate(const QString &spaceName);
    //bool semanticKBAddDocument(const QString &spaceName, const QString &docPath);
    bool semanticKBAddDocumentsForSpace(const QString &spaceName/* const QStringList &docPaths, const QStringList &docIds, QList<int> *errorCodes = nullptr*/);
    //bool semanticKBDeleteDocument(const QString &spaceName, const QString &docPath);
    bool semanticKBDeleteDocuments(const QString &spaceName, const QStringList &docPaths/*QList<int> *errorCodes = nullptr*/);
    QStringList semanticKBListDocuments(const QString &spaceName);
    int semanticKBGetDocumentStatus(const QString &spaceName, const QString &docPath);
    QString semanticKBChat(const QString &spaceName, const QString &question, const QString &model = QString());
    bool semanticKBDeleteSpaceKB(const QString &spaceName);
    QString generateDocId(const QString &spaceName, const QString &path);
    bool semanticKBDeleteAllDocuments(const QString &spaceName);
    bool semanticKBDeleteFileFromAllSpaces(const QString &filePath);
Q_SIGNALS:
    void allModelCreated();
    void spaceRemoved(int spaceId);
    // 空间条件更新 返回结果给前端
    void filesReloaded(const QStringList &infos, int spaceId);
    void filesReloadedByCreate(const QStringList &infos, int spaceId);
    void spaceConditionsChanged(int spaceId, const QString &spaceName, const QVariant &conditionsDescription);
    void cancelSpaceSearch(int spaceId);
    void spaceRemovedWithFilesRequest(int spaceId, const QStringList &filesToBeRemoved);
    void setSpaceConditionRequest(int spaceId, const QString &spaceName);
    void SpaceCreatefinished(const QString &spaceName,const QStringList &filelist);
    void fileDeletedRequest(const QString &deletedFile);
    void fileCreateRequest(const QString &newFile);
    void fileMovedRequest(const QString &oldFile, const QString &newFile);
    void searchfileReload(QList<QStringList> infos,QString clientId);
    void searchKeywordChanged(QString keyword);
    void searchContentReload(QStringList infolist,QString clientId);
    void searchTextContentReload(QList<QStringList> infolist,QString clientId);
    void searchfileReloadByBatch(QStringList infolist,QString clientId,bool finish = false);
    void searchConditionChanged(QString conditions);
    void spacefileloadfinish(QString Spacename);
    void spacesFilesAutoInsert(QString Spacename,QStringList infos,bool needGotoUri = false);
    void spacesFilesAutoDelete(QMap<QString,QStringList> deleteSpacesFile);
    void cancelSearchRequest();
    void userAddFilesToSpaceBySpacepluginSignal(const QStringList &filePath,const QString &spaceName,bool isUserAdd);
    void editSpaceConditionBySpacepluginSignal(const QString &uri);
    void spaceDataChanged(const QString &spaceName,const QStringList &filelist);
private Q_SLOTS:
    void initModels();
    void handleRescanSpaceWithCondition(int spaceId,const QVariant &conditionsDescription);
    void handleFileEvent(UKUI::IDM::FileInfoManager::FileEventType type, const QString &arg1, const QString arg2);
private:
    std::atomic_bool fetched;
    SpacesFilesFetcher *fetcher;
    QThread *thread;
    QList<QString> contentFileinfo;
    bool Aisearchfinish;
    bool searchfinish;
    QString m_clientId;
    QString m_currentPath;
    SearchResultModel* m_searchmodel = nullptr;
};


#endif // MODELCONTROLLER_H
