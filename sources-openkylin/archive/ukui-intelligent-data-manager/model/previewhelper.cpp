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

#include "previewhelper.h"

#include "modelcontroller.h"
#include "spacesfilesfetcher.h"
#include "fileinfojob.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDateTime>

#include <global-settings.h>

PreviewHelper *instance = nullptr;

PreviewHelper *PreviewHelper::globalInstance()
{
    if (!instance) {
        instance = new PreviewHelper;
    }
    return instance;
}

PreviewHelper::PreviewHelper(QObject *parent)
    : QObject{parent}
{

}

void PreviewHelper::setPreviewFile(const QString &path)
{
    fileInfo = UKUI::IDM::FileInfo::fromPath(path, false);

    setFilePath(path);
    if (!fileInfo->isValid()) {
        UKUI::IDM::FileInfoJob::queryInfoSyncWithConditions(fileInfo, QList<QSharedPointer<Condition>>(), QList<QSharedPointer<Condition>>());
    }
    setDisplayName(fileInfo->displayName());
    setAccessTime(fileInfo->openDateString());
    setModificationTime(fileInfo->modifyDateString());
    setFileSize(fileInfo->sizeFormat());
    bool isFolder = fileInfo->mimeType() == "inode/directory";
    setIsFolder(isFolder);
}

void PreviewHelper::setPreviewSpaceId(int spaceId)
{
    fileInfo = nullptr;

    auto spaceName = ModelController::globalInstance()->fetcher->getSpaceNameById(spaceId);

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString jsonPath = QString("%1/spaces/%2.json").arg(configPath).arg(spaceId);
    QFileInfo spaceConfigFile(jsonPath);
    auto createTime = spaceConfigFile.birthTime();
    auto modifiedTime = spaceConfigFile.lastModified();

    QString createTimeString = Peony::GlobalSettings::getInstance()->transToSystemTimeFormat(createTime.toSecsSinceEpoch());
    QString modifiedTimeString = Peony::GlobalSettings::getInstance()->transToSystemTimeFormat(modifiedTime.toSecsSinceEpoch());

    setFilePath(nullptr);
    setDisplayName(spaceName);

    setAccessTime(createTimeString);
    setModificationTime(modifiedTimeString);
    setFileSize(nullptr);
    setIsFolder(false);
}

const QString &PreviewHelper::getDisplayName() const
{
    return displayName;
}

void PreviewHelper::setDisplayName(const QString &newDisplayName)
{
    if (displayName == newDisplayName)
        return;
    displayName = newDisplayName;
    Q_EMIT displayNameChanged();
}

const QString &PreviewHelper::getAccessTime() const
{
    return accessTime;
}

void PreviewHelper::setAccessTime(const QString &newAccessTime)
{
    if (accessTime == newAccessTime)
        return;
    accessTime = newAccessTime;
    Q_EMIT accessTimeChanged();
}

const QString &PreviewHelper::getModificationTime() const
{
    return modificationTime;
}

void PreviewHelper::setModificationTime(const QString &newModificationTime)
{
    if (modificationTime == newModificationTime)
        return;
    modificationTime = newModificationTime;
    Q_EMIT modificationTimeChanged();
}

const QString &PreviewHelper::getFileSize() const
{
    return fileSize;
}

void PreviewHelper::setFileSize(const QString &newFileSize)
{
    if (fileSize == newFileSize)
        return;
    fileSize = newFileSize;
    Q_EMIT fileSizeChanged();
}

const QString &PreviewHelper::getFilePath() const
{
    return filePath;
}

void PreviewHelper::setFilePath(const QString &newFilePath)
{
    if (filePath == newFilePath)
        return;
    filePath = newFilePath;
    Q_EMIT filePathChanged();
}

bool PreviewHelper::getIsFolder() const
{
    return isFolder;
}

void PreviewHelper::setIsFolder(bool isFolder)
{
    if (isFolder == this->isFolder)
        return;
    this->isFolder = isFolder;
    Q_EMIT isFolderChanged();
}
