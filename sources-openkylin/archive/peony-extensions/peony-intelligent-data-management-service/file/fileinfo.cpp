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

#include "fileinfo.h"
#include "fileinfo_p.h"
#include "fileinfomanager.h"

#include <QMutex>
#include <QMutexLocker>

#include <QString>
#include <QUrl>

using namespace UKUI::IDM;

QSharedPointer<FileInfo> FileInfo::fromPath(const QString &filePath, bool addWatch)
{
    QUrl url = QUrl::fromLocalFile(filePath);
    return fromUri(url.toEncoded(), addWatch);
}

QSharedPointer<FileInfo> FileInfo::fromUri(const QString &uri, bool addWatch)
{
    QSharedPointer<FileInfo> info = FileInfoManager::globalInstance()->getInfoByUri(uri);
    if (info.isNull()) {
        info = QSharedPointer<FileInfo>(new FileInfo);
        info->priv->uri = uri;
        FileInfoManager::globalInstance()->insertInfo(info);
        if (addWatch)
            FileInfoManager::globalInstance()->addFileToWatcher(uri);
    }

    return info;
}

QSharedPointer<FileInfo> FileInfo::fromUrl(const QUrl &url, bool addWatch)
{
    return fromUri(url.toEncoded(), addWatch);
}

bool FileInfo::isValid() const
{
    QMutexLocker(&priv->mutex);
    bool isValid = priv->state == FileInfoPrivate::State::Valid;
    return isValid;
}

QString FileInfo::uri() const
{
    QMutexLocker(&priv->mutex);
    QString uri = priv->uri;
    return uri;
}

QString FileInfo::path() const
{
    QMutexLocker(&priv->mutex);
    QString path = priv->path;
    return path;
}

QString FileInfo::parentPath() const
{
    QMutexLocker(&priv->mutex);
    QString path = priv->parentPath;
    return path;
}

QString FileInfo::displayName() const
{
    QMutexLocker(&priv->mutex);
    QString name = priv->name;
    return name;
}

QString FileInfo::iconName() const
{
    QMutexLocker(&priv->mutex);
    QString iconName = priv->iconName;
    return iconName;
}

QString FileInfo::createDateString() const
{
    QMutexLocker(&priv->mutex);
    QString string = priv->createDateString;
    return string;
}

QString FileInfo::modifyDateString() const
{
    QMutexLocker(&priv->mutex);
    QString string = priv->modifyDateString;
    return string;
}

QString FileInfo::openDateString() const
{
    QMutexLocker(&priv->mutex);
    QString string = priv->openDateString;
    return string;
}

QString FileInfo::dateString() const
{
    QMutexLocker(&priv->mutex);
    QString string = priv->dateString;
    return string;
}

QString FileInfo::version() const
{
    QMutexLocker(&priv->mutex);
    QString version = priv->version;
    return version;
}

QString FileInfo::mimeType() const
{
    QMutexLocker(&priv->mutex);
    QString mimeType = priv->mimeType;
    return mimeType;
}

QString FileInfo::fileType() const
{
    QMutexLocker(&priv->mutex);
    QString fileType = priv->fileType;
    return fileType;
}

quint64 FileInfo::timeModifiedMsec() const
{
    QMutexLocker(&priv->mutex);
    quint64 timeModifiedMsec = priv->timeModifiedMsec;
    return timeModifiedMsec;
}

quint64 FileInfo::timeAccessMsec() const
{
    QMutexLocker(&priv->mutex);
    quint64 timeAccessMsec = priv->timeAccessMsec;
    return timeAccessMsec;
}

qint64 FileInfo::timeRecentModified() const
{
    QMutexLocker(&priv->mutex);
    qint64 timeRecentModified = priv->timeRecentModified;
    return timeRecentModified;
}

quint64 FileInfo::size() const
{
    QMutexLocker(&priv->mutex);
    quint64 size = priv->size;
    return size;
}

QString FileInfo::sizeFormat() const
{
    QMutexLocker(&priv->mutex);
    QString sizeFormat = priv->sizeFormat;
    return sizeFormat;
}

quint64 FileInfo::openCount() const
{
    QMutexLocker(&priv->mutex);
    quint64 count = priv->openCount;
    return count;
}

QList<QVariant> FileInfo::labelColors() const
{
    QList<QVariant> colors;
    QMutexLocker(&priv->mutex);
    colors = priv->labelColors;
    return colors;
}
void FileInfo::setChecked(bool flag)
{
     QMutexLocker(&priv->mutex);
     priv->ischecked = flag;
}
bool FileInfo::isChecked() const
{
    QMutexLocker(&priv->mutex);
    bool ischecked = priv->ischecked;
    return ischecked;
}
FileInfo::FileInfo()
{
    priv = new FileInfoPrivate;
}

FileInfo::~FileInfo()
{
    // TODO: 批量异步更新数据库
    FileInfoManager::globalInstance()->removeFileFromWatcher(this->path());
    FileInfoManager::globalInstance()->removeInfoPtrByUri(priv->uri);
    delete priv;
}
