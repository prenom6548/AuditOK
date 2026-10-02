/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-operation-utils.h"
#include "file-operation-manager.h"
#include "manager-control.h"

#include "file-move-operation.h"
#include "file-copy-operation.h"
#include "file-trash-operation.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "file-delete-operation.h"
#include "file-link-operation.h"

#include "file-untrash-operation.h"
#include "file-count-operation.h"
#include "file-properties-operation.h"

#include "file-info-job.h"
#include "file-info.h"
#include "file-enumerator.h"
#include "audio-play-manager.h"

#include "file-operation-internal-dialog.h"
#include "sound-effect.h"
#include "global-settings.h"

#include <QUrl>
#include <QFileInfo>
#include <gio/gio.h>
#include <gudev/gudev.h>

#include <QMessageBox>
#include <QApplication>

#include <QStandardPaths>
#include <QSet>

using namespace Peony;

static FileOperation *trashInternal(const QStringList &uris, bool addHistory, bool isFromSearchTab, bool forceShowDialog)
{
    FileOperation *op = nullptr;
    bool canNotTrash = false;
    bool isBigFile = false;
    bool skipDialog = false;

    QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString newBoxPath;
    QString oldBoxPath;

    //1.0保护箱路径为“/box/用户名/保护箱名称”,2.1保护箱路径为“/home/用户名/.box/保护箱名称”，只要包含这些路径就是保护箱下文件，是不可以删除到回收站的；
    //root下没有home；
    if(userPath.startsWith("/home")){
        newBoxPath = userPath+"/.box";

        QString user = userPath.remove("/home/");

        oldBoxPath = "/box/"+user;

    } else {
        newBoxPath = userPath+"/.box";
        oldBoxPath = "/box/root";
    }

    if (!uris.isEmpty()) {
        auto uri = uris.first();
        if (!uri.startsWith("file:/") || (uri.contains(newBoxPath)) || (uri.contains(oldBoxPath))) {
            canNotTrash = true;
        }
        if (uri.contains("kydrive") && uri.startsWith("file:///media")) {
            canNotTrash = true;
        }
    }

    // 只读文件系统场景需要跳过确认弹框，直接走文件操作异常报错弹框流程
    // fixme: 这里的判断条件不充分，只是必要条件
    if (uris.count() > 0) {
        auto uri = uris.first();
        auto parentUri = FileUtils::getParentUri(uri);
        auto info = FileInfo::fromUri(parentUri);
        if (info->isEmptyInfo()) {
            FileInfoJob j(info);
            j.querySync();
        }
        if (!info->canWrite()) {
            skipDialog = true;
            qDebug() << "[trashInternal] Parent directory canWrite=false, skipDialog=true, parentUri:" << parentUri;
        }
    }

    if (!canNotTrash) {
        quint64 total_size = 0;
        const quint64 ONE_GIB_SIZE = 1024*1024*1024;
        // Repeating the mount and udev checks for every URI blocks the UI
        // before the trash operation can even be started. The result is the
        // same for entries from one directory, so inspect each parent once.
        const bool trashSettings = GlobalSettings::getInstance()->getValue(TRASH_MOBILE_FILES).toBool();
        QSet<QString> checkedParentUris;
        for (const auto &uri : uris) {
            //fix iso symbolLink delete prompt can not trash issue
            //FIXME: replace BLOCKING api in ui thread. and no type.
            auto info = FileInfo::fromUri(uri);
            if (info->isSymbolLink())
                continue;

            const int pathSeparator = uri.lastIndexOf('/');
            const QString parentUri = pathSeparator > 0 ? uri.left(pathSeparator) : uri;
            if (checkedParentUris.contains(parentUri))
                continue;
            checkedParentUris.insert(parentUri);

            //folder need recursive calculate file size
            //屏蔽本部分代码，文件和文件夹走一样的异常处理流程，关联bug#92483
//            if(! info->isDir()){
//                total_size += file.size();
//            }

//            //file total size more than 10G, not trash but delete, task#56444,  bug#88871, bug#88894
//            if (total_size/10 > ONE_GIB_SIZE) {
//                canNotTrash = true;
//                isBigFile = true;
//                break;
//            }
//            qDebug() <<"total_size:" <<total_size<<ONE_GIB_SIZE<<canNotTrash<<isBigFile;

            //task#147972 【删除回收站】选项需求
            //如果是长城的机器并且带了9215控制器，不再区分移动设备，统一右键删除到回收站
            bool isUsbOrOptical = false;
            QString unixDevice = FileUtils::getUnixDeviceForUri(uri);
            qDebug() << "[trashInternal] Checking USB device for uri:" << uri << "unixDevice:" << unixDevice;
            if (!unixDevice.isEmpty()) {
                GUdevClient *client = g_udev_client_new(nullptr);
                if (client) {
                    GUdevDevice *device = g_udev_client_query_by_device_file(client, unixDevice.toUtf8().constData());
                    if (device) {
                        const char *id_bus = g_udev_device_get_property(device, "ID_BUS");
                        if (id_bus && g_strcmp0(id_bus, "usb") == 0) {
                            isUsbOrOptical = true;
                            qDebug() << "[trashInternal] USB device detected, ID_BUS=usb, device:" << unixDevice;
                        }

                        if (!isUsbOrOptical && g_udev_device_get_property_as_boolean(device, "ID_CDROM")) {
                            isUsbOrOptical = true;
                            qDebug() << "[trashInternal] Optical device detected, ID_CDROM=true, device:" << unixDevice;
                        }
                        g_object_unref(device);
                    }
                    g_object_unref(client);
                }
            }
            
            if (isUsbOrOptical) {
                canNotTrash = true;
                qDebug() << "[trashInternal] USB/Optical device confirmed, canNotTrash=true";
                break;
            }
            
            if (trashSettings)
                break;

            //file total size more than 10G, not trash but delete, task#56444
            //FIXME 判断是否是移动设备文件，可能不准确, 目前暂未找到好的判断方法
            bool isMobileDeviece = FileUtils::isMobileDeviceFile(uri);
            if(isMobileDeviece){
                canNotTrash = true;
                break;
            }
        }
    }

    if (!canNotTrash && !MANAGER_CONTROL->checkDeleteToTrashPermission()) {
        canNotTrash = true;
        forceShowDialog = true;
    }

    // comment for fixing #82054
//    if (!canNotTrash) {
//        FileEnumerator e;
//        e.setEnumerateDirectory("trash:///");
//        e.enumerateSync();
//        if (e.getChildrenUris().count() > 1000) {
//            canNotTrash = true;
//        }
//    }

    if (canNotTrash && !skipDialog) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        //task #155670,155671 improve delete file permanently message
        QString message;
        if (isBigFile){
            message = QObject::tr("The file is too large to be moved to the recycle bin. "
                                  "Do you want to permanently delete it?");

            if (uris.length() > 1)
                message = QObject::tr("These files are too large to be moved to the recycle bin. "
                                      "Do you want to permanently delete these %1 files?").arg(uris.length());
        }
        else if (uris.length() == 1){
            message = QObject::tr("Are you sure you want to permanently delete this file?");
        }else{
            message = QObject::tr("Are you sure you want to permanently delete these %1 files?"
                                  ).arg(uris.length());
        }
        QString message2 = QObject::tr(" Once deletion begins, "
                              "the file will not be recoverable.");
        //auto result = QMessageBox::question(nullptr, "", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        QWidget *parent = FileOperationUtils::getMainWindow();
        QMessageBox msgBox(parent);
        msgBox.setText(message);
        msgBox.setInformativeText(message2);
        msgBox.setIcon(QMessageBox::Question);
        QPushButton *deleteButton = msgBox.addButton(QObject::tr("Delete"), QMessageBox::AcceptRole);
        deleteButton->setProperty("isImportant", true);
        msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
        deleteButton->setDefault(true);

        int result = msgBox.exec();
        if (msgBox.clickedButton() == deleteButton) {
            op = FileOperationUtils::remove(uris);
        }
        return op;
    }

    auto fileOpMgr = FileOperationManager::getInstance();
    auto trashOp = new FileTrashOperation(uris);
    if (isFromSearchTab) {
        trashOp->setSearchOperation(isFromSearchTab);
    }
    fileOpMgr->startOperation(trashOp, addHistory, forceShowDialog);

    return trashOp;
}

FileOperationUtils::FileOperationUtils()
{

}

FileOperation *FileOperationUtils::move(const QStringList &srcUris, const QString &destUri, bool addHistory, bool copyMove)
{
    FileOperation *op;
    QString destDir = nullptr;
    auto fileOpMgr = FileOperationManager::getInstance();
    if (destUri != "trash:///") {
        if (true == destUri.startsWith("computer:///")) {
            destDir = FileUtils::getTargetUri(destUri);
            if (nullptr == destDir){
                qWarning()<<"get target uri failed, from uri:"
                          <<destUri;
                destDir = destUri;
            }
        }
        else {
            destDir = destUri;
        }

        auto moveOp = new FileMoveOperation(srcUris, destDir);
        moveOp->setAction(copyMove? Qt::MoveAction: Qt::TargetMoveAction);
        //moveOp->setCopyMove(copyMove);
        op = moveOp;
        fileOpMgr->startOperation(moveOp, addHistory);
    } else {
        op = FileOperationUtils::trash(srcUris, true);
    }
    return op;
}

FileOperation *FileOperationUtils::copy(const QStringList &srcUris, const QString &destUri, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto copyOp = new FileCopyOperation(srcUris, destUri);
    fileOpMgr->startOperation(copyOp, addHistory);
    return copyOp;
}

FileOperation *FileOperationUtils::copyDesktopFile(const QStringList &srcUris, const QString &destUri, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto copyOp = new FileCopyOperation(srcUris, destUri);
    copyOp->setProperty("copyDesktopFile", true);
    fileOpMgr->startOperation(copyOp, false);
    return copyOp;
}

FileOperation *FileOperationUtils::trash(const QStringList &uris, bool addHistory)
{
    return trash(uris, addHistory, false);
}

FileOperation *FileOperationUtils::trash(const QStringList &uris, bool addHistory, bool isFromSearchTab)
{
    return trashInternal(uris, addHistory, isFromSearchTab, false);
}

FileOperation *FileOperationUtils::trashWithDialog(const QStringList &uris, bool addHistory)
{
    return trashInternal(uris, addHistory, false, true);
}

FileOperation *FileOperationUtils::rename(const QString &uri, const QString &newName, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto renameOp = new FileRenameOperation(uri, newName);
    fileOpMgr->startOperation(renameOp, addHistory);
    return renameOp;
}

FileOperation *FileOperationUtils::remove(const QStringList &uris)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto removeOp = new FileDeleteOperation(uris);
    fileOpMgr->startOperation(removeOp);
    return removeOp;
}

FileOperation *FileOperationUtils::link(const QString &srcUri, const QString &destUri, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto linkOp = new FileLinkOperation(srcUri, destUri);
    fileOpMgr->startOperation(linkOp, addHistory);
    return linkOp;
}

std::shared_ptr<FileInfo> FileOperationUtils::queryFileInfo(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    auto job = new FileInfoJob(info);
    job->querySync();
    job->deleteLater();
    return info;
}

FileOperation *FileOperationUtils::moveWithAction(const QStringList &srcUris, const QString &destUri, bool addHistory, Qt::DropAction action)
{
    return moveWithAction(srcUris, destUri, addHistory, action, false);
}

FileOperation *FileOperationUtils::moveWithAction(const QStringList &srcUris, const QString &destUri, bool addHistory, Qt::DropAction action, bool bMoveFromSearchTab)
{
    FileOperation *op;
    QString destDir = nullptr;
    auto fileOpMgr = FileOperationManager::getInstance();
    if (destUri != "trash:///") {
        if (true == destUri.startsWith("computer:///")) {
            destDir = FileUtils::getTargetUri(destUri);
            if (nullptr == destDir){
                qWarning()<<"get target uri failed, from uri:"
                          <<destUri;
                destDir = destUri;
            }
        }
        else {
            destDir = destUri;
        }

        if (action == Qt::CopyAction) {
            // fix 71411
            op = new FileCopyOperation(srcUris, destDir);
        } else {
            auto moveOp = new FileMoveOperation(srcUris, destDir);
            moveOp->setAction(action);
            moveOp->setSearchOperation(bMoveFromSearchTab);
            op = moveOp;
        }

        fileOpMgr->startOperation(op, addHistory);
    } else {
        op = FileOperationUtils::trash(srcUris, true, bMoveFromSearchTab);
    }
    return op;
}

FileOperation *FileOperationUtils::clearRecycleBinWithDialog(const QStringList &list)
{
    return clearRecycleBinWithDialog(list, nullptr);
}

FileOperation *FileOperationUtils::clearRecycleBinWithDialog(const QStringList &list, QWidget *parent)
{
    // fix bug #456223 参照新版弹窗设计稿修改
    QMessageBox questionbox(parent);
    questionbox.setIcon(QMessageBox::Question);
    questionbox.setText(QObject::tr("Are you sure you want to empty the Recycle Bin?"));
    questionbox.setInformativeText(QObject::tr("Files that are emptied cannot be recovered"));
    QAbstractButton *deleteButton = questionbox.addButton(QObject::tr("Clean"), QMessageBox::AcceptRole);
    deleteButton->setProperty("isImportant", true);
    questionbox.setDefaultButton(qobject_cast<QPushButton *>(deleteButton));
    questionbox.setStandardButtons(QMessageBox::Cancel);

    Peony::AudioPlayManager::getInstance()->playWarningAudio();
    questionbox.exec();
    QAbstractButton *clicked = questionbox.clickedButton();
    if (clicked == deleteButton) {
        FileOperation *operation = nullptr;
        if (!list.isEmpty()) {
            operation = FileOperationUtils::remove(list);
        } else {
            auto uris = FileUtils::getChildrenUris("trash:///");
            if (uris.isEmpty()) {
                return nullptr;
            }
            operation = FileOperationUtils::remove(uris);
        }
        SoundEffect::getInstance()->recycleBinDeleteMusic();
        return operation;
    } else {
        return nullptr;
    }
}

FileOperation *FileOperationUtils::restore(const QString &uriInTrash)
{
    QStringList uris;
    uris<<uriInTrash;
    auto fileOpMgr = FileOperationManager::getInstance();
    auto untrashOp = new FileUntrashOperation(uris);
    fileOpMgr->startOperation(untrashOp, true);
    return untrashOp;
}

FileOperation *FileOperationUtils::restore(const QStringList &urisInTrash)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto untrashOp = new FileUntrashOperation(urisInTrash);
    //FIXME: support undo?
    fileOpMgr->startOperation(untrashOp, false);
    return untrashOp;
}

FileOperation *FileOperationUtils::create(const QString &destDirUri, const QString &name, CreateTemplateOperation::Type type)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto createOp = new CreateTemplateOperation(destDirUri, type, name);
    fileOpMgr->startOperation(createOp, true);
    return createOp;
}

FileOperation *FileOperationUtils::setReadOnly(const QStringList &srcUris, bool readOnly, bool recursive)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    FilePropertiesOperation::Options options = FilePropertiesOperation::ChangeReadOnly;
    if (recursive) {
        options |= FilePropertiesOperation::ChangeRecursively;
    }
    auto readOnlyOp = new FilePropertiesOperation(srcUris, options, false, readOnly);
    fileOpMgr->startOperation(readOnlyOp, false);
    return readOnlyOp;
}

FileOperation *FileOperationUtils::setHidden(const QStringList &srcUris, bool hidden, bool recursive)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    FilePropertiesOperation::Options options = FilePropertiesOperation::ChangeHidden;
    if (recursive) {
        options |= FilePropertiesOperation::ChangeRecursively;
    }
    auto hiddenOp = new FilePropertiesOperation(srcUris, options, hidden, false);
    fileOpMgr->startOperation(hiddenOp, false);
    return hiddenOp;
}

FileOperation *FileOperationUtils::setReadOnlyAndHidden(const QStringList &srcUris, bool readOnly, bool hidden, bool recursive)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    FilePropertiesOperation::Options options = FilePropertiesOperation::Options(FilePropertiesOperation::ChangeReadOnly|FilePropertiesOperation::ChangeHidden);
    if (recursive) {
        options |= FilePropertiesOperation::ChangeRecursively;
    }
    auto propertiesOp = new FilePropertiesOperation(srcUris, options, hidden, readOnly);
    fileOpMgr->startOperation(propertiesOp);
    return propertiesOp;
}

void FileOperationUtils::executeRemoveActionWithDialog(const QStringList &uris)
{
    if (uris.isEmpty())
        return;

    Peony::AudioPlayManager::getInstance()->playWarningAudio();
    int result = 0;
    QString message = QObject::tr("Are you sure you want to permanently delete this file?");
    if (uris.count() > 1) {
        message = QObject::tr("Are you sure you want to permanently delete these %1 files?"
                              ).arg(uris.length());
    }
    QString message2 = QObject::tr(" Once deletion begins, "
                                  "the file will not be recoverable.");
    //result = QMessageBox::question(nullptr, "", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    QWidget *parent = FileOperationUtils::getMainWindow();
    QMessageBox msgBox(parent);
    msgBox.setText(message);
    msgBox.setInformativeText(message2);
    msgBox.setIcon(QMessageBox::Question);
    QPushButton *deleteButton = msgBox.addButton(QObject::tr("Delete"), QMessageBox::AcceptRole);
    deleteButton->setProperty("isImportant", true);
    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    deleteButton->setDefault(true);

    result = msgBox.exec();
    if (msgBox.clickedButton() == deleteButton) {
        FileOperationUtils::remove(uris);
    }
}

//not accurate of process, name has tr("duplicate") not processed
//do not use this function before you fixed it
bool FileOperationUtils::leftNameIsDuplicatedFileOfRightName(const QString &left, const QString &right)
{
    auto tmpl = left;
    auto tmpr = right;
    tmpl.remove(QRegExp("\\(\\d+\\)"));
    tmpr.remove(QRegExp("\\(\\d+\\)"));
    return tmpl == tmpr;
}

static int getNumOfFileName(const QString &name)
{
    QRegExp regExp("\\(\\d+\\)");
    int num = 0;

    if (name.contains(regExp)) {
        int pos = 0;
        QString tmp;
        while ((pos = regExp.indexIn(name, pos)) != -1) {
            tmp = regExp.cap(0).toUtf8();
            pos += regExp.matchedLength();
        }
        tmp.remove(0,1);
        tmp.chop(1);
        num = tmp.toInt();
    }
    return num;
}

bool FileOperationUtils::leftNameLesserThanRightName(const QString &left, const QString &right)
{
    auto tmpl = left;
    auto tmpr = right;
    int numl = getNumOfFileName(tmpl);
    int numr = getNumOfFileName(tmpr);
    //fix bug#97408,change indicator meanings
    return numl == numr? left > right: numl > numr;
}

QWidget *FileOperationUtils::getMainWindow()
{
    QWidget *parent = QApplication::activeModalWidget();
    if (!parent) {
        parent = QApplication::activeWindow();
    }
    if (!parent && QApplication::focusWidget()) {
        parent = QApplication::focusWidget()->window();
    }
    if (parent) {
        return parent->window();
    }

    QWidget *fallbackMainWindow = nullptr;
    const auto widgets = qApp->topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (widget->objectName() == "_peony_mainwindow") {
            if (widget->isActiveWindow()) {
                return widget;
            }
            if (!fallbackMainWindow) {
                fallbackMainWindow = widget;
            }
        }
    }
    return fallbackMainWindow;
}
