/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "backuppositionselectdialog.h"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include "filefilterproxymodelforbackup.h"
#include "../../common/utils.h"
#include "../messageboxutils.h"

BackupPositionSelectDialog::BackupPositionSelectDialog(QWidget* parent) :
    QFileDialog(parent)
{
    setWindowTitle(tr("Please select a path"));
    setViewMode(QFileDialog::Detail);
    setFileMode(QFileDialog::DirectoryOnly);
    // 主题和文管重构了QFileDialog，下面代码已经失效
//    setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
//    QList<QUrl> siderUrls;
//    siderUrls << QUrl::fromLocalFile(Utils::getSysRootPath());
//    siderUrls << QUrl::fromLocalFile(Utils::getSysRootPath() + "/data");
//    QList<QString> localDisks = Utils::getLocalDisks();
//    for (const QString& path: localDisks) {
//        siderUrls << QUrl::fromLocalFile(path);
//    }
//    setSidebarUrls(siderUrls);
//    FileFilterProxyModeForBackup *proxy = new FileFilterProxyModeForBackup;
//    setProxyModel(proxy);
}

BackupPositionSelectDialog::~BackupPositionSelectDialog()
{}

bool BackupPositionSelectDialog::checkSelectedFiles()
{
    QStringList selectFiles = selectedFiles();
    for (const QString & fileName : selectFiles) {
        // 防命令注入
        // 1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
        // 2、形如：$()的文件夹名称
        // 3、形如：${}的文件夹名称
        // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
        if (    fileName.contains(QRegularExpression(".*`.*`.*"))
             || fileName.contains(QRegularExpression(".*\\$\\(.*\\).*"))
             || fileName.contains(QRegularExpression(".*\\$\\{.*\\}.*"))
             || fileName.contains(QRegularExpression("[;&|]+"))) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), QObject::tr("Path can not include symbols that such as : ``,$(),${},;,&,|,etc."), QObject::tr("OK"));
            return false;
        }

        // 主题重新实现了QFileDialog后很多功能已经丢失(如：文件夹过滤等)，需要再次进行过滤，真难用
        QString backupPath = Utils::getSysRootPath() + BACKUP_PATH;
        backupPath.replace("//", "/");
        if (fileName == backupPath || fileName.contains(BACKUP_SNAPSHOTS_PATH)) {
            // 不能嵌套备份，请选择其它目录
            MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Information"),
                                                  QObject::tr("Cannot nest backups, please select another directory."),
                                                  QObject::tr("OK"));
            return false;
        } else if (fileName.endsWith(BACKUP_PATH)) {
            QString subfile = fileName + "/snapshots";
            if (Utils::isDirExist(subfile)) {
                // 不能嵌套备份，请选择其它目录
                MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Information"),
                                                      QObject::tr("Cannot nest backups, please select another directory."),
                                                      QObject::tr("OK"));
                return false;
            }
        }
    }

    return true;
}

void BackupPositionSelectDialog::goAccept()
{
    if (checkSelectedFiles())
        accept();
}

// 主题和文管重构了QFileDialog，需要使用下面代码绑定accept信号
void BackupPositionSelectDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)

    if (this->isVisible()) {
        for (QWidget* widget : this->findChildren<QWidget*>()) {
            if (widget->objectName() == "acceptButton") {
                QPushButton * btn = qobject_cast<QPushButton*>(widget);
                disconnect(btn, &QPushButton::clicked, 0, 0);
                connect(btn, &QPushButton::clicked, [=](bool checked){
                    Q_UNUSED(checked)
                    this->goAccept();
                });
            }
        }
    }
}
