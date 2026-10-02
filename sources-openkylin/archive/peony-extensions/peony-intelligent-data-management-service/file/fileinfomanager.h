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

#ifndef FILEINFOMANAGER_H
#define FILEINFOMANAGER_H

#include <QObject>
#include <file/fileinfo.h>
namespace UKUI {
namespace IDM {

class FileInfoManagerPrivate;

class FileInfoManager : public QObject
{
    Q_OBJECT
public:
    enum FileEventType {
        Unknown,
        FileCreated,
        FileModified,
        FileDeleted,
        FileMoveOut,
        FileMoveIn,
        FileMoved,
        FileAccessed,
        FileOpened,
        FileClosed,
        FileAttrChanged,
        FileIgnoredAutomatic,
        FileUnmount,
        UserAddFile,
        UserRemoveFile,
        Other
    };
    Q_ENUM(FileEventType)

    static FileInfoManager *globalInstance();

    void addFileToWatcher(const QString &url, bool *ok = nullptr);
    QStringList addFilesToWatcher(const QStringList &urls);
    void removeFileFromWatcher(const QString &url, bool *ok = nullptr);
    QStringList removeFilesFromWatcher(const QStringList &urls);

Q_SIGNALS:
    void fileChanged(FileEventType type, const QString &arg1, const QString arg2);
    void remoteFileEvent(int eventType, const QString &arg1, const QString arg2);

private:
    FileInfoManagerPrivate *priv;
    explicit FileInfoManager(QObject *parent = nullptr);
    ~FileInfoManager();

    friend class FileInfo;
    QSharedPointer<FileInfo> getInfoByUri(const QString &uri);
    void insertInfo(QSharedPointer<FileInfo> info);
    void removeInfoPtrByUri(const QString &uri);
};

}
}

#endif // FILEINFOMANAGER_H
