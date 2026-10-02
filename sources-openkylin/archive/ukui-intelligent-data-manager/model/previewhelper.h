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

#ifndef PREVIEWHELPER_H
#define PREVIEWHELPER_H

#include <QObject>

#include "../file/fileinfo.h"

class PreviewHelper : public QObject
{
    Q_OBJECT
public:
    static PreviewHelper *globalInstance();
    explicit PreviewHelper(QObject *parent = nullptr);

    Q_INVOKABLE void setPreviewFile(const QString &path);
    Q_INVOKABLE void setPreviewSpaceId(int spaceId);

    const QString &getDisplayName() const;
    void setDisplayName(const QString &newDisplayName);

    const QString &getAccessTime() const;
    void setAccessTime(const QString &newAccessTime);

    const QString &getModificationTime() const;
    void setModificationTime(const QString &newModificationTime);

    const QString &getFileSize() const;
    void setFileSize(const QString &newFileSize);

    const QString &getFilePath() const;
    void setFilePath(const QString &newFilePath);

    bool getIsFolder() const;
    void setIsFolder(bool isFolder);

signals:
    void displayNameChanged();

    void accessTimeChanged();

    void modificationTimeChanged();

    void fileSizeChanged();

    void filePathChanged();

    void isFolderChanged();

private:
    bool isFileMode = true;

    QString filePath;
    QString displayName;
    QString accessTime;
    QString modificationTime;
    QString fileSize;
    bool isFolder;
    Q_PROPERTY(QString displayName READ getDisplayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString accessTime READ getAccessTime WRITE setAccessTime NOTIFY accessTimeChanged)
    Q_PROPERTY(QString modificationTime READ getModificationTime WRITE setModificationTime NOTIFY modificationTimeChanged)
    Q_PROPERTY(QString fileSize READ getFileSize WRITE setFileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(bool isFolder READ getIsFolder NOTIFY isFolderChanged)

    QSharedPointer<UKUI::IDM::FileInfo> fileInfo;
    Q_PROPERTY(QString filePath READ getFilePath WRITE setFilePath NOTIFY filePathChanged)
};

#endif // PREVIEWHELPER_H
