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

#include <QDBusReply>
#include <QDBusMetaType>
#include <QDebug>
#include "idm-dbus-manager.h"
#include "idm-actions-manager.h"

using namespace Peony;

static IDMDBusManager* globalInstance = nullptr;

IDMDBusManager *IDMDBusManager::getInstance()
{
    if (!globalInstance) {
        globalInstance = new IDMDBusManager;
    }
    return globalInstance;
}

void IDMDBusManager::editSpaceCondition(const int spaceId)
{
    Q_UNUSED(spaceId);
}

QStringList IDMDBusManager::getAllSpacesName() const
{
    QStringList spaceNames;
    QDBusReply<QStringList> reply = m_idmInterface->call("getAllSpacesName");
    if (reply.isValid()) {
        spaceNames = reply.value();
    }
    return spaceNames;
}

int IDMDBusManager::getSpaceIdByName(const QString &spaceName) const
{
    int id = -1;
    QDBusReply<int> reply = m_idmInterface->call("getSpaceIdByName", spaceName);
    if (reply.isValid()) {
        id = reply.value();
    }
    return id;
}

QStringList IDMDBusManager::getFilesInSpaceByName(const QString &spaceName, bool isUserAdded) const
{
    QStringList files;
    QDBusReply<QStringList> reply = m_idmInterface->call("getFilesInSpaceByName", spaceName, isUserAdded);
    if (reply.isValid()) {
        files = reply.value();
    }
    return files;
}

QString IDMDBusManager::getConditionsBySpaces(const QString &spaceName) const
{
    QString conditions;
    QDBusReply<QString> reply = m_idmInterface->call("getConditionsBySpaces", spaceName);
    if (reply.isValid()) {
        conditions = reply.value();
    }
    return conditions;
}

bool IDMDBusManager::getSpacestatus(const QString &spaceName) const
{
    bool isDynamic = false;
    QDBusReply<bool> reply = m_idmInterface->call("getSpacestatus", spaceName);
    if (reply.isValid()) {
        isDynamic = reply.value();
    }
    return isDynamic;
}

bool IDMDBusManager::onSpaceCreateRequest(const QString &spaceName, const QString &conditionDescription, bool isCreatedFromSearch) const
{
    bool isSuccess = false;
    QDBusReply<bool> reply = m_idmInterface->call("onSpaceCreateRequest", spaceName, conditionDescription, isCreatedFromSearch);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    if (isSuccess && !spaceName.isEmpty()) {
        Q_EMIT spaceAdded(spaceName);
    }
    return isSuccess;
}

bool IDMDBusManager::onSpaceCreateRequestWithFiles(const QString &spaceName, const QString &conditionDescription, const bool &isCreatedFromSearch, const QStringList &infos) const
{
    bool isSuccess = false;
    QDBusReply<bool> reply = m_idmInterface->call("onSpaceCreateRequestWithFiles", spaceName, conditionDescription, isCreatedFromSearch, infos);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    if (isSuccess && !spaceName.isEmpty()) {
        Q_EMIT spaceAdded(spaceName);
        Q_EMIT createSpaceSignal("idm:///" + spaceName);
    }
    return isSuccess;
}

bool IDMDBusManager::removeSpace(const QString &spaceName) const
{
    bool isSuccess = false;
    int spaceId = this->getSpaceIdByName(spaceName);
    QDBusReply<bool> reply = m_idmInterface->call("removeSpace", spaceId);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    if (isSuccess) {
        Q_EMIT spaceRemoved(spaceName);
        Q_EMIT deleteSpaceSignal();
    }
    return isSuccess;
}

bool IDMDBusManager::renameSpace(const QString &oldName, const QString &newName) const
{
    bool isSuccess = false;
    int spaceId = this->getSpaceIdByName(oldName);
    QDBusReply<bool> reply = m_idmInterface->call("renameSpace", spaceId, newName);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    if (isSuccess) {
        Q_EMIT spaceRenamed(oldName, newName);
    }
    return isSuccess;
}

void IDMDBusManager::startLoadAllSpaces() const
{
    QDBusReply<void> reply = m_idmInterface->call("startLoadAllSpaces");
    if (reply.isValid()) {
        qDebug() << "startLoadAllSpaces";
    } else {
        qWarning() << "DBus call failed:" << reply.error().message();
    }
}

bool IDMDBusManager::updateSpacestatus(const QString &spaceName, const bool &status) const
{
    bool isSuccess = false;
    QDBusReply<bool> reply = m_idmInterface->call("updateSpacestatus", spaceName, status);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    return isSuccess;
}

bool IDMDBusManager::spaceRemovedWithFiles(const int &spaceId, const QStringList &files) const
{
    bool isSuccess = false;
    QDBusReply<bool> reply = m_idmInterface->call("spaceRemovedWithFiles", spaceId, files);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    return isSuccess;
}

bool IDMDBusManager::userAddFilesToSpace(const QStringList &paths, const QString &spaceName, const bool &isSpaceAdded) const
{
    bool isSuccess = false;
    QDBusReply<bool> reply = m_idmInterface->call("userAddFilesToSpace", paths, spaceName);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    if (isSuccess && !isSpaceAdded) {
        Q_EMIT spacesFilesAutoInserted(spaceName, paths);
    }
    if (isSuccess && isSpaceAdded) {
        Q_EMIT spaceAddFilesToSpaced(spaceName);
        IDMActionsManager::showFileReferencedMessage(spaceName);
    }
    return isSuccess;
}

void IDMDBusManager::updateSearchCondition(const QString &conditions) const
{
    m_idmInterface->call("updateSearchCondition", conditions);
}

QStringList IDMDBusManager::getIntelligentLabel(const QString &keyword) const
{
    QDBusReply<QStringList> reply = keyword.isEmpty() ?
                m_idmInterface->call("getIntelligentLabel") :
                m_idmInterface->call("getIntelligentLabel", keyword);

    return reply.isValid() ? reply.value() : QStringList{};
}

void IDMDBusManager::editSpaceCondition(const QString &spaceName, const QString &conditions) const
{
    m_idmInterface->call("editSpaceCondition", spaceName, conditions);
}

void IDMDBusManager::reSearchWithLabels(const QStringList &labels, const QString &spaceName, const QString &keyWord) const
{
    m_idmInterface->call("reSearchWithLabels", keyWord, labels, spaceName);
}

bool IDMDBusManager::removeFilesFromSpace(const QString &spaceName, const QStringList &paths) const
{
    bool isSuccess = false;
    QDBusReply<bool> reply = m_idmInterface->call("removeFileFromSpace", spaceName, paths);
    if (reply.isValid()) {
        isSuccess = reply.value();
    }
    return isSuccess;
}

bool IDMDBusManager::isUserAddFile(const QString &spaceName, const QString path) const
{
    bool isUserAdded = false;
    QDBusReply<bool> reply = m_idmInterface->call("isUserAddFile", spaceName, path);
    if (reply.isValid()) {
        isUserAdded = reply.value();
    }
    return isUserAdded;
}

bool IDMDBusManager::isAiAvailable()
{
    bool isavailable = false;
    QDBusReply<bool> reply = m_idmInterface->call("isAisubsysteminited");
    if (reply.isValid()) {
        isavailable = reply.value();
    }
    return isavailable;
}

QString IDMDBusManager::getFileSummery(const QString &filePath)
{
    QString summery;
    QDBusReply<QString> reply = m_idmInterface->call("getFileSummery",filePath);
    if (reply.isValid()) {
        summery = reply.value();
    }
    return summery;
}

QStringList IDMDBusManager::getFileLabels(const QString &filePath)
{
    QStringList fileLabels;
    QDBusReply<QStringList> reply = m_idmInterface->call("getFileLabels",filePath);
    if (reply.isValid()) {
        fileLabels = reply.value();
    }
    return fileLabels;
}

qint64 IDMDBusManager::getSpaceCreateTime(const QString &spaceName) const
{
    qint64 createTime = 0;
    QDBusReply<qint64> reply = m_idmInterface->call("getSpaceAccesstime", spaceName);
    if (reply.isValid()) {
        createTime = reply.value();
    }
    return createTime;
}

qint64 IDMDBusManager::getSpaceModifyTime(const QString &spaceName) const
{
    qint64 modifyTime = 0;
    QDBusReply<qint64> reply = m_idmInterface->call("getSpaceModifytime", spaceName);
    if (reply.isValid()) {
        modifyTime = reply.value();
    }
    return modifyTime;
}

QVariantHash IDMDBusManager::getSpacesSortConfig(const QString &spaceName) const
{
    int spaceId = this->getSpaceIdByName(spaceName);
    QVariantHash config;
    QDBusReply<QVariantHash> reply = m_idmInterface->call("getSpacesSortConfig", spaceId);
    if (reply.isValid()) {
        config = reply.value();
    }
    return config;
}

void IDMDBusManager::setSpacesSortConfig(const QString &spaceName, const int sortType, const int sortOrder)
{
    int spaceId = this->getSpaceIdByName(spaceName);
    int userAddedtype = 0;
    m_idmInterface->call("setSpacesSortConfig", spaceId, sortType, sortOrder, userAddedtype);
}

int IDMDBusManager::getSpacesSortType(const QString &spaceName) const
{
    QVariantHash config = getSpacesSortConfig(spaceName);
    if (config.isEmpty()) {
        return 0;
    }
    return config.value("sortType").toInt();
}

int IDMDBusManager::getSpacesSortOrder(const QString &spaceName) const
{
    QVariantHash config = getSpacesSortConfig(spaceName);
    if (config.isEmpty()) {
        return 0;
    }
    return config.value("sortOrder").toInt();
}

void IDMDBusManager::setActiveWindow(QWidget *widget)
{
    m_activeWindow = widget;
}

void IDMDBusManager::unmountFilesInSpace(QString unmountPath)
{
     m_idmInterface->call("unmountFilesInSpace",unmountPath);
}

QWidget *IDMDBusManager::getActiveWindow()
{
    return m_activeWindow;
}

void IDMDBusManager::editSpaceConditionBySpaceplugin(const QString &uri)
{
    Q_EMIT editSpaceConditionSignal(uri);
}

void IDMDBusManager::spaceDataChanged(const QString &spaceName, const QStringList &filePaths) const
{
    Q_EMIT spaceFileDataChanged(spaceName, filePaths);
}

IDMDBusManager::IDMDBusManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QMap<QString, QStringList>>("QMap<QString, QStringList> &");
    qDBusRegisterMetaType<QMap<QString,QStringList>>();
    m_idmInterface = new QDBusInterface(IDM_SERVICE,
                                        IDM_SERVICE_PATH,
                                        IDM_CONTROLLER_INTERFACE,
                                        QDBusConnection::sessionBus());
    if (m_idmInterface->isValid()) {
        //startLoadAllSpaces();
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "spacefileloadfinish",
                                              this,
                                              SLOT(spaceFileLoadFinish(QString)));
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "spacesFilesAutoInsert",
                                              this,
                                              SLOT(spacesFilesAutoInsert(QString, QStringList)));
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "spacesFilesAutoDelete",
                                              this,
                                              SLOT(spaceFilesAutoDelete(QMap<QString, QStringList>)));
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "SpaceCreatefinished",
                                              this,
                                              SLOT(spaceCreateFinish(QString, QStringList)));
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "userAddFilesToSpaceBySpacepluginSignal",
                                              this,
                                              SLOT(userAddFilesToSpace(QStringList, QString,bool)));
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "editSpaceConditionBySpacepluginSignal",
                                              this,SLOT(editSpaceConditionBySpaceplugin(QString)));
        QDBusConnection::sessionBus().connect(IDM_SERVICE,
                                              IDM_SERVICE_PATH,
                                              IDM_CONTROLLER_INTERFACE,
                                              "spaceDataChanged",
                                              this,
                                              SLOT(spaceDataChanged(QString,QStringList)));

    }
}

IDMDBusManager::~IDMDBusManager()
{
    delete m_idmInterface;
    m_idmInterface = NULL;
}

void IDMDBusManager::spaceFileLoadFinish(const QString &spaceName) const
{
    qDebug() << "spaceFileLoadFinish" << spaceName;
    Q_EMIT spaceFileLoadFinished(spaceName);
}

void IDMDBusManager::spacesFilesAutoInsert(const QString &spaceName, const QStringList &files) const
{
    qDebug() << __func__ << spaceName << files;
    Q_EMIT spacesFilesAutoInserted(spaceName, files);
}

void IDMDBusManager::spaceFilesAutoDelete(const QMap<QString, QStringList> &deleteMap) const
{
    qDebug() << __func__ << deleteMap;
    Q_EMIT spacesFilesAutoDeleted(deleteMap);
}

void IDMDBusManager::spaceCreateFinish(const QString &spaceName, const QStringList &files) const
{
    qDebug() << __func__ << spaceName;
    Q_EMIT spaceAdded(spaceName);
    Q_EMIT spaceCreateFinished(spaceName, files);
}


QString IDMDBusManager::semanticKBCreate(const QString &spaceName)
{
    QDBusReply<QString> reply = m_idmInterface->call("semanticKBCreate", spaceName);
    return reply.isValid() ? reply.value() : QString();
}

bool IDMDBusManager::semanticKBAddDocuments(const QString &spaceName, const QStringList &docPaths)
{
    QDBusReply<bool> reply = m_idmInterface->call("semanticKBAddDocuments", spaceName,docPaths);
    return reply.isValid() ? reply.value() : false;
}

bool IDMDBusManager::semanticKBAddDocumentsForSpace(const QString &spaceName)
{
    QDBusReply<bool> reply = m_idmInterface->call("semanticKBAddDocumentsForSpace", spaceName);
    return reply.isValid() ? reply.value() : false;
}

bool IDMDBusManager::semanticKBDeleteDocuments(const QString &spaceName,const QStringList &docPaths)
{
    QDBusReply<bool> reply = m_idmInterface->call("semanticKBDeleteDocuments", spaceName, docPaths);
    return reply.isValid() ? reply.value() : false;
}

QStringList IDMDBusManager::semanticKBListDocuments(const QString &spaceName)
{
    QDBusReply<QStringList> reply = m_idmInterface->call("semanticKBListDocuments", spaceName);
    return reply.isValid() ? reply.value() : QStringList();
}

int IDMDBusManager::semanticKBGetDocumentStatus(const QString &spaceName, const QString &docPath)
{
    qDebug() << "IDMDBusManager::semanticKBGetDocumentStatus called:" << spaceName << docPath;
    // int st = -1;
    // int *status = &st;
    QDBusReply<int> reply = m_idmInterface->call("semanticKBGetDocumentStatus", spaceName, docPath);
    if (!reply.isValid()) {
        qDebug() << "D-Bus reply invalid, error:" << reply.error().message();
        return -1;
    }
    // Note: D-Bus can't return pointer, status is not available via this interface
    // if (status) {
    //     qDebug() << "status pointer provided but D-Bus can't return it, ignoring";
    // }
    qDebug() << "D-Bus reply value:" << reply.value();
    return reply.value();
}

QString IDMDBusManager::semanticKBChat(const QString &spaceName, const QString &question, const QString &model)
{
    QDBusReply<QString> reply = model.isEmpty() ?
                                    m_idmInterface->call("semanticKBChat", spaceName, question) :
                                    m_idmInterface->call("semanticKBChat", spaceName, question, model);
    return reply.isValid() ? reply.value() : QString();
}

bool IDMDBusManager::semanticKBDeleteSpaceKB(const QString &spaceName)
{
    QDBusReply<bool> reply = m_idmInterface->call("semanticKBDeleteSpaceKB", spaceName);
    return reply.isValid() ? reply.value() : false;
}

bool IDMDBusManager::semanticKBDeleteAllDocuments(const QString &spaceName)
{
    QDBusReply<bool> reply = m_idmInterface->call("semanticKBDeleteAllDocuments", spaceName);
    return reply.isValid() ? reply.value() : false;
}
