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

#include "intelligent-vfs-info-plugin.h"
#include "file-vfs-info.h"
#include "file-utils.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-enumerator.h"
#include "global-settings.h"
#include "gio/gio.h"
#include <QDateTime>
#include <QDebug>
#include <QUrl>

IntelligentVFSInfoPlugin::IntelligentVFSInfoPlugin()
{
    m_idmDBusManager = IDMDBusManager::getInstance();
}

QString IntelligentVFSInfoPlugin::pathScheme()
{
    return QString("idm");
}

QString IntelligentVFSInfoPlugin::displayName()
{
    return QString(QObject::tr("Intelligent Space"));
}

QString IntelligentVFSInfoPlugin::displayIcon()
{
    return QString("folder-space");
}

QStringList IntelligentVFSInfoPlugin::fileEnumerator(const QString &path)
{
    QStringList lists;
    if (path == "/") {
        lists = m_idmDBusManager->getAllSpacesName();
        //qDebug() << "===========" << __func__ << lists;
    } else if (path.count('/') == 1) {
        QString realPath = FileUtils::urlDecode(path);
        QString spacesName = realPath.mid(realPath.lastIndexOf('/') + 1);
        QStringList values = m_idmDBusManager->getFilesInSpaceByName(spacesName);
        qDebug() << values.count() << spacesName;
        for (QString value : values) {
            QString tmpUri = "/" + spacesName + "/" + QString(QUrl::toPercentEncoding(value));
            lists.append(tmpUri);
        }
    } else if (path.count('/') == 2) {
        QString spacesName = path.left(path.lastIndexOf('/') + 1);
        QString result = path.split('/').last();
        QString dir = QUrl::fromPercentEncoding(result.toLatin1());
        FileEnumerator e;
        e.setEnumerateDirectory("file://" + dir);
        e.enumerateSync();
        QStringList uris = e.getChildrenUris();
        for (auto uri : uris) {
            QString tmp = FileUtils::urlDecode(uri);
            tmp.remove("file://");
            tmp = spacesName + QString(QUrl::toPercentEncoding(tmp));
            lists.append(tmp);
        }
    }
    return lists;
}

std::shared_ptr<FileVFSInfo> IntelligentVFSInfoPlugin::queryFile(const QString &path)
{
    std::shared_ptr<FileVFSInfo> file = std::make_shared<FileVFSInfo>();
    if (!path.contains("/") || path.count('/') == 1) {
        file->setDir(true);
        file->setIconName("folder-space");
        QString spaceName = path;
        if (spaceName.startsWith('/')) {
            spaceName.remove('/');
        }
        if (spaceName == "New Intelligent Space") {
            spaceName = QObject::tr("New Intelligent Space");
        }

        qint64 createTime = IDMDBusManager::getInstance()->getSpaceCreateTime(spaceName);
        qint64 modifyTime = IDMDBusManager::getInstance()->getSpaceModifyTime(spaceName);
        QDateTime createDate = QDateTime::fromSecsSinceEpoch(createTime);
        QDateTime modifyDate = QDateTime::fromSecsSinceEpoch(modifyTime);
        QString ctime = createDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
        QString mtime = modifyDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
        file->setCreateTime(ctime);
        file->setModifiedTime(mtime);
        file->setDisplayName(spaceName);
    } else if (path.count('/') >= 2) {
        QString tmpPath = QUrl::fromPercentEncoding(path.toUtf8());
        QString lastPath = path.split('/').last();
        QString spaceName = tmpPath.split('/', QString::SkipEmptyParts).first();
        QString realUri = QUrl::fromPercentEncoding(lastPath.toLatin1());

        //qDebug() << __func__ << lastPath << spaceName << realUri << "tmpPath:" << tmpPath;
        // QSet<QString> fileSet = QSet<QString>::fromList(m_idmDBusManager->getFilesInSpaceByName(spaceName));
        // if (!fileSet.contains(realUri)) {
        //     qDebug() << __func__ << "is null" << lastPath << spaceName;
        //     return nullptr;
        // }

        if (realUri.contains('/')) {
            realUri = "file://" + realUri;
            realUri = FileUtils::urlEncode(realUri);
            g_autoptr(GFile) tmp = g_file_new_for_uri(realUri.toUtf8().constData());
            g_autoptr(GFileInfo) fileInfo = nullptr;
            if (tmp) {
                fileInfo = g_file_query_info(tmp, "*", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
                if (fileInfo) {
                    GFileType fileType = g_file_info_get_file_type(fileInfo);
                    QString displayName = g_file_info_get_display_name(fileInfo);
                    QString contentType = g_file_info_get_content_type(fileInfo);
                    guint64 modifiedTime = g_file_info_get_attribute_uint64(fileInfo, G_FILE_ATTRIBUTE_TIME_MODIFIED);
                    guint64 accessTime = g_file_info_get_attribute_uint64(fileInfo, G_FILE_ATTRIBUTE_TIME_ACCESS);
                    guint64 createTime = g_file_info_get_attribute_uint64(fileInfo, G_FILE_ATTRIBUTE_TIME_CREATED);
                    guint64 changeTime = g_file_info_get_attribute_uint64(fileInfo, G_FILE_ATTRIBUTE_TIME_CHANGED);
                    QDateTime modifiedDate = QDateTime::fromMSecsSinceEpoch(modifiedTime*1000);
                    QDateTime accessDate = QDateTime::fromMSecsSinceEpoch(accessTime*1000);
                    QDateTime createDate = QDateTime::fromMSecsSinceEpoch(createTime*1000);
                    QDateTime changeDate = QDateTime::fromMSecsSinceEpoch(changeTime*1000);
                    QString mtime = modifiedDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
                    QString atime = accessDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
                    QString ctime = createDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
                    QString ctime2 = changeDate.toString(GlobalSettings::getInstance()->getSystemTimeFormat());
                    QString iconName;
                    GIcon *g_icon = g_file_info_get_icon(fileInfo);
                    if (G_IS_THEMED_ICON(g_icon)) {
                        const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
                        if (icon_names) {
                            iconName = QString(*icon_names);
                            // auto p = icon_names;
                            // while (*p) {
                            //     QIcon icon = QIcon::fromTheme(*p);
                            //     if (!icon.isNull()) {
                            //         iconName = QString (*p);
                            //         break;
                            //     } else {
                            //         p++;
                            //     }
                            // }
                        }
                    }
                    bool has_unix_mode = g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_UNIX_MODE);
                    guint32 mode = 0;
                    if (has_unix_mode)
                        mode = g_file_info_get_attribute_uint32(fileInfo, G_FILE_ATTRIBUTE_UNIX_MODE);
                    bool isWrite = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
                    bool isRename = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME);
                    bool isExecute = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
                    bool isSymbolLink = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK);
                    QString user = g_file_info_get_attribute_string(fileInfo, G_FILE_ATTRIBUTE_OWNER_USER);
                    QString groupName = g_file_info_get_attribute_string(fileInfo, G_FILE_ATTRIBUTE_OWNER_GROUP);
                    QString colorsString = g_file_info_get_attribute_string(fileInfo, "metadata::peony-file-label-ids");
                    bool hasOpenedCount = g_file_info_has_attribute(fileInfo, "metadata::peony-opened-count");
                    if (hasOpenedCount) {
                        QString openedCount = g_file_info_get_attribute_string(fileInfo, "metadata::peony-opened-count");
                        file->addExtendInfo("metadata::peony-opened-count", QVariant(openedCount));
                    }

                    //设置文件信息
                    if (fileType == G_FILE_TYPE_DIRECTORY || contentType == "inode/directory") {
                        file->setDir(true);
                    }

                    file->setDisplayName(displayName);
                    file->setTargetUri(realUri);
                    file->setIconName(iconName);
                    file->setModifiedTime(mtime);
                    file->setAccessTime(atime);
                    file->setCreateTime(ctime);
                    file->setChangeTime(ctime2);
                    file->addExtendInfo(G_FILE_ATTRIBUTE_UNIX_MODE, QVariant(mode));
                    file->addExtendInfo(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, QVariant(isWrite));
                    file->addExtendInfo(G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME, QVariant(isRename));
                    file->addExtendInfo(G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, QVariant(isExecute));
                    file->addExtendInfo(G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK, QVariant(isSymbolLink));
                    file->addExtendInfo(G_FILE_ATTRIBUTE_OWNER_USER, QVariant(user));
                    file->addExtendInfo(G_FILE_ATTRIBUTE_OWNER_GROUP, QVariant(groupName));
                    file->addExtendInfo("metadata::peony-file-label-ids", QVariant(colorsString));
                    file->setSize(g_file_info_get_attribute_uint64(fileInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE));
                    file->setContentType(contentType);
                } else {
                    qDebug() << __func__ << "is null" << realUri;
                    return nullptr;
                }
            }
        } else {
            qDebug() << __func__ << "is null";
            return nullptr;
        }
    }
    return file;
}

VFSError *IntelligentVFSInfoPlugin::handerMakeDirectory(const QString &uri, bool &ret)
{
    Q_UNUSED(uri);
    Q_UNUSED(ret);
    return nullptr;
}

VFSError *IntelligentVFSInfoPlugin::handerFileCreate(const QString &uri, bool &ret)
{
    Q_UNUSED(uri);
    Q_UNUSED(ret);
    return nullptr;
}

VFSError *IntelligentVFSInfoPlugin::handerFileDelete(const QString &uri, bool &ret)
{
    Q_UNUSED(uri);
    Q_UNUSED(ret);
    return nullptr;
}

VFSError *IntelligentVFSInfoPlugin::handerFileCopy(const QString &sourceUri, const QString &destUri, bool &ret)
{
    QString strSourceTrueUri = Peony::FileUtils::urlDecode(sourceUri);
    QString strDestFile = Peony::FileUtils::urlDecode(destUri);
    struct VFSError* err = nullptr;
    if (sourceUri.startsWith("file://")) {
        QString destName = QUrl::fromPercentEncoding(strSourceTrueUri.remove("file://").toUtf8());
        QString spaceName = strDestFile.remove("idm://").split('/', QString::SkipEmptyParts).first();
        spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
        qDebug() << __func__ << destName << spaceName << sourceUri << strDestFile;
        QSet<QString> fileSet = m_idmDBusManager->getFilesInSpaceByName(spaceName).toSet();
        if (fileSet.contains(destName)) {
            ret = false;
            err = new VFSError;
            err->code = ErrorType(ERROR_NOT_SUPPORTED);
            err->message = QString(QObject::tr("File %1 is existed in space, please not add file repeatly.").arg(destName));
            return err;
        }

        ret = IDMDBusManager::getInstance()->userAddFilesToSpace(QStringList() << destName, spaceName);
    }

    if (!ret) {
        err = new VFSError;
        err->code = ErrorType(ERROR_NOT_SUPPORTED);
        err->message = QString(QObject::tr("Operation not supported"));
    }

    return err;
}

VFSError *IntelligentVFSInfoPlugin::handerFileMove(const QString &sourceUri, const QString &destUri, bool &ret)
{
    Q_UNUSED(sourceUri);
    Q_UNUSED(destUri)
    Q_UNUSED(ret);
    return nullptr;
}

VFSError *IntelligentVFSInfoPlugin::handerFileRename(const QString &sourceUri, const QString &newName)
{
    Q_UNUSED(sourceUri);
    Q_UNUSED(newName)
    return nullptr;
}

void IntelligentVFSInfoPlugin::handerMonitorDirectory(HanderTransfer *transfer, const QString &uri)
{
    auto instance = IDMDBusManager::getInstance();
    QString rootUri = "idm:///";
    QObject::connect(instance, &IDMDBusManager::spaceAdded, [=](const QString &spaceName){
        qDebug() << __func__ << "spaceAdded" << spaceName << uri;
        if (uri == rootUri) {
            QString trueUri = "/" + QString(QUrl::toPercentEncoding(spaceName));
            Q_EMIT transfer->fileCreate(trueUri);
        }
    });
    QObject::connect(instance, &IDMDBusManager::spaceRemoved, [=](const QString &spaceName){
        qDebug() << __func__ << "spaceRemoved" << spaceName;
        if (uri == rootUri) {
            QString trueUri = "/" + QString(QUrl::toPercentEncoding(spaceName));
            Q_EMIT transfer->fileDelete(trueUri);
        }
    });
    QObject::connect(instance, &IDMDBusManager::spaceRenamed, [=](const QString &oldName, const QString &newName){
        qDebug() << __func__ << "spaceRenamed" << oldName;
        if (uri == rootUri) {
            QString oldUri = "/" + QString(QUrl::toPercentEncoding(oldName));
            QString newUri = "/" + QString(QUrl::toPercentEncoding(newName));
            qDebug() << "spaceRenamed" << oldUri << newUri;
            Q_EMIT transfer->fileRenamed(oldUri, newUri);
        }
    });
    QObject::connect(instance, &IDMDBusManager::spacesFilesAutoInserted, [=](const QString &spaceName, const QStringList &files){
        qDebug() << __func__ << "space File Add" << uri;
        QString tempUri = FileUtils::urlDecode(uri);
        QString currentSpaceName = tempUri.remove("idm:///");
        if (!currentSpaceName.isEmpty() && currentSpaceName == spaceName) {
            for (auto file : files) {
                QString encodeName = QString(QUrl::toPercentEncoding(file));
                QString trueUri = "/" + QString(QUrl::toPercentEncoding(spaceName)) + "/" + encodeName;
                Q_EMIT transfer->fileCreate(trueUri);
            }
        }
    });
    QObject::connect(instance, &IDMDBusManager::spacesFilesAutoDeleted, [=](const QMap<QString, QStringList> &deleteMap){
        qDebug() << __func__ << "space File spacesFilesAutoDeleted" << uri;
        QString tempUri = FileUtils::urlDecode(uri);
        QString currentSpaceName = tempUri.remove("idm:///");
        QStringList files = deleteMap.value(currentSpaceName);
        if (!files.isEmpty()) {
            for (auto file : files) {
                QString encodeName = QString(QUrl::toPercentEncoding(file));
                QString trueUri = "/" + QString(QUrl::toPercentEncoding(currentSpaceName)) + "/" + encodeName;
                Q_EMIT transfer->fileDelete(trueUri);
            }
        }
    });
}

VFSError *IntelligentVFSInfoPlugin::handleSetAttribute(const QString &uri, const QString attribute, AttributeType type, QueryFlags flags, void *value, bool &ret)
{
    QString strUri = uri;
    struct VFSError* err = nullptr;
    QString path = strUri.split('/', QString::SkipEmptyParts).last();
    QString realPath = QUrl::fromPercentEncoding(path.toLatin1());
    GError *error = nullptr;
    if (realPath.contains('/')) {
        QString realUri = "file://" + realPath;
        GFileQueryInfoFlags gFlags = static_cast<GFileQueryInfoFlags>(static_cast<int>(flags));
        g_autoptr(GFile) tfile = g_file_new_for_uri(realUri.toUtf8().constData());
        ret = g_file_set_attribute(tfile, attribute.toUtf8().constData(), GFileAttributeType(type), value, gFlags, nullptr, &error);
    }

    if (error) {
        err = new VFSError;
        err->code = Peony::ErrorType(error->code);
        err->message = QString(error->message);
    } else {
        ret = true;
    }
    return err;
}
