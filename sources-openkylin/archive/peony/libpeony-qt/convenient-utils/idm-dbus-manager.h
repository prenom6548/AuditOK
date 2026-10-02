/*
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef IDMDBUSMANAGER_H
#define IDMDBUSMANAGER_H

#include <QObject>
#include <QDBusInterface>
#include "peony-core_global.h"
static const QString IDM_SERVICE = "com.peony.idm.service";
static const QString IDM_SERVICE_PATH = "/org/peony/idm/space/service";
static const QString IDM_CONTROLLER_INTERFACE = "org.peony.idm.modelcontroller";

namespace Peony {

class PEONYCORESHARED_EXPORT IDMDBusManager : public QObject
{
    Q_OBJECT
public:
    static IDMDBusManager* getInstance();
    void editSpaceCondition(const int spaceId);
    QStringList getAllSpacesName() const;
    int getSpaceIdByName(const QString &spaceName) const;
    QStringList getFilesInSpaceByName(const QString &spaceName, bool isUserAdded = false) const;
    QString getConditionsBySpaces(const QString &spaceName) const;
    bool getSpacestatus(const QString &spaceName) const;
    bool onSpaceCreateRequest(const QString &spaceName = "", const QString &conditionDescription = "", bool isCreatedFromSearch = true) const;
    bool onSpaceCreateRequestWithFiles(const QString &spaceName, const QString &conditionDescription, const bool &isCreatedFromSearch, const QStringList &infos) const;
    bool removeSpace(const QString &spaceName) const;
    bool renameSpace(const QString &oldName, const QString &newName) const;
    void startLoadAllSpaces() const;
    bool updateSpacestatus(const QString &spaceName, const bool &status) const;
    bool spaceRemovedWithFiles(const int &spaceId, const QStringList &files) const;
    void updateSearchCondition(const QString &conditions) const;
    QStringList getIntelligentLabel(const QString &keyword = "") const;
    void editSpaceCondition(const QString &spaceName, const QString &conditions) const;
    void reSearchWithLabels(const QStringList &labels, const QString &spaceName = "", const QString &keyWord = "") const;
    bool removeFilesFromSpace(const QString &spaceName, const QStringList &paths) const;
    bool isUserAddFile(const QString &spaceName, const QString path) const;
    bool isAiAvailable();
    QString getFileSummery(const QString &filePath);
    QStringList getFileLabels(const QString &filepath);
    qint64 getSpaceCreateTime(const QString &spaceName) const;
    qint64 getSpaceModifyTime(const QString &spaceName) const;
    QVariantHash getSpacesSortConfig(const QString &spaceName) const;
    void setSpacesSortConfig(const QString &spaceName, const int sortType, const int sortOrder);
    int getSpacesSortType(const QString &spaceName) const;
    int getSpacesSortOrder(const QString &spaceName) const;
    void setActiveWindow(QWidget *widget);
    void unmountFilesInSpace(QString unmountPath);
    QWidget* getActiveWindow();
    // Semantic Knowledge Base methods
    QString semanticKBCreate(const QString &spaceName);
    bool semanticKBAddDocumentsForSpace(const QString &spaceName);
    bool semanticKBAddDocuments(const QString &spaceName, const QStringList &docPaths);
    bool semanticKBDeleteDocuments(const QString &spaceName, const QStringList &docPaths);
    bool semanticKBDeleteAllDocuments(const QString &spaceName);
    QStringList semanticKBListDocuments(const QString &spaceName);
    int semanticKBGetDocumentStatus(const QString &spaceName, const QString &docPath);
    QString semanticKBChat(const QString &spaceName, const QString &question, const QString &model = QString());
    bool semanticKBDeleteSpaceKB(const QString &spaceName);
private:
    explicit IDMDBusManager(QObject *parent = nullptr);
    ~IDMDBusManager();

public Q_SLOTS:
    void spaceFileLoadFinish(const QString &spaceName) const;
    void spacesFilesAutoInsert(const QString &spaceName, const QStringList &files) const;
    void spaceFilesAutoDelete(const QMap<QString, QStringList> &deleteMap) const;
    void spaceCreateFinish(const QString &spaceName, const QStringList &files) const;
    bool userAddFilesToSpace(const QStringList &paths, const QString &spaceName, const bool &isSpaceAdded = false) const;
    void editSpaceConditionBySpaceplugin(const QString &uri);
    void spaceDataChanged(const QString &spaceName, const QStringList &filePaths) const;

Q_SIGNALS:
    void spaceAdded(const QString &spaceName) const;
    void spaceRemoved(const QString &spaceName) const;
    void spaceRenamed(const QString &oldName, const QString &newName) const;
    void spaceFileLoadFinished(const QString &spaceName) const;
    void spacesFilesAutoInserted(const QString &spaceName, const QStringList &files, const bool isGotoUri = false) const;
    void spacesFilesAutoDeleted(const QMap<QString, QStringList> &deleteMap) const;
    void editSpaceConditionSignal(const QString &uri = "") const;
    void createSpaceSignal(const QString &uri) const;
    void deleteSpaceSignal() const;
    void spaceCreateFinished(const QString &spaceName, const QStringList &files) const;
    void spaceAddFilesToSpaced(const QString &spaceName) const;
    void spaceFileDataChanged(const QString &spaceName, const QStringList &filePaths) const;

private:
    QDBusInterface *m_idmInterface = nullptr;
    QWidget*  m_activeWindow = nullptr;
};

}

#endif // IDMDBUSMANAGER_H
