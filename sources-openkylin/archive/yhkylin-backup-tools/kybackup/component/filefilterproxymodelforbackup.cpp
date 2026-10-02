/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "filefilterproxymodelforbackup.h"
#include <QFileSystemModel>
#include <QDir>
#include "../../common/mydefine.h"
#include "../../common/utils.h"

FileFilterProxyModeForBackup::FileFilterProxyModeForBackup(QObject *parent) :
    QSortFilterProxyModel(parent)
{}

FileFilterProxyModeForBackup::~FileFilterProxyModeForBackup()
{}

bool FileFilterProxyModeForBackup::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());

    if (fileModel != nullptr && fileModel->isDir(index0)) {
        QString fileName = fileModel->fileName(index0);
        // 防命令注入
        // 1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
        if (fileName.contains(QRegularExpression(".*`.*`.*")))
            return false;
        // 2、形如：$()的文件夹名称
        if (fileName.contains(QRegularExpression(".*\\$\\(.*\\).*")))
            return false;
        // 3、形如：${}的文件夹名称
        if (fileName.contains(QRegularExpression(".*\\$\\{.*\\}.*")))
            return false;
        // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
        if (fileName.contains(QRegularExpression("[;&|]+")))
            return false;

        QString filePath = fileModel->filePath(index0);
        if (fileName == "backup") {
            if (Utils::isDirExist(filePath + "/snapshots"))
                return false;
        }

        return !(filePath.endsWith(BACKUP_SNAPSHOTS_PATH) || filePath == BACKUP_PATH);
    } else {
        return true;
    }
}

