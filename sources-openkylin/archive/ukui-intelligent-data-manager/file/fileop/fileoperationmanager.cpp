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

#include "fileoperationmanager.h"

#include "modelcontroller.h"
#include "fileinfo.h"
#include "searchresultmodel.h"

//#include <peony-qt/file-operation-manager.h>
//#include <peony-qt/file-operation-utils.h>
//#include <peony-qt/file-utils.h>

//#include <QMutex>

#include <peony-qt/file-info-job.h>
#include <peony-qt/file-info.h>
#include <peony-qt/file-launch-manager.h>
#include <peony-qt/file-launch-action.h>
#include <peony-qt/file-operation-utils.h>
#include <peony-qt/clipboard-utils.h>
#include <peony-qt/file-label-model.h>
#include <peony-qt/file-lauch-dialog.h>

#include <kinputdialog.h>

#include <QMenu>
#include <QUrl>
#include <QFileDialog>
#include <QApplication>
#include <QWindow>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QInputDialog>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QFontMetrics>

static FileOperationManager *instance = nullptr;

FileOperationManager *FileOperationManager::globalInstance()
{
    if (!instance) {
        instance = new FileOperationManager;
    }
    return instance;
}

FileOperationManager::FileOperationManager(QObject *parent) : QObject(parent)
{

}

QString FileOperationManager::getNewSpaceName(const QString &currentSpaceName, QWindow *parentWindow, bool *ok)
{
    qDebug() << parentWindow;
    kdk::KInputDialog d;
    d.setWindowFlag(Qt::Dialog);
    d.setInputMode(kdk::KInputDialog::TextInput);
    d.setLabelText(tr("Space Name"));
    d.setPlaceholderText(tr("Please input space name"));
    d.setTextValue(currentSpaceName);
    d.setOkButtonText(tr("OK"));
    d.setCancelButtonText(tr("Cancel"));
    d.createWinId();
    d.windowHandle()->setTransientParent(parentWindow);

    if (d.exec()) {
        if (ok) {
            *ok = true;
        }
        return d.textValue();
    }
    return nullptr;
}

void FileOperationManager::showMenu(FileItemProxySortFilterModel *model, int index, const QPoint &globalPos)
{
    QMenu menu;
    QString path = model->index(index, 0).data(Qt::UserRole + 1).toString();
    QUrl url = QUrl::fromLocalFile(path);
    QString uri = url.toEncoded();
    bool isFileSelected = !path.isEmpty();
    bool isSpaceMenu = model->getSpaceId() >= 0;
    if (!isSpaceMenu && !isFileSelected) {
        return;
    }

    bool isUserAddedFile = model->isUserAddedFile(path);

    auto openAction = new QAction(tr("Open"));
    openAction->setVisible(isFileSelected);
    connect(openAction, &QAction::triggered, model, [=]{
        Peony::FileLaunchManager::openAsync(uri);
    });

    auto openWithAction = new QAction(tr("Open With"));
    openWithAction->setVisible(isFileSelected);
    QMenu *openWithMenu = new QMenu;
    // do not highlight application icons.
    openWithMenu->setProperty("skipHighlightIconEffect", true);

    //use origin uri instead of target uri, fix recommand menu not same with desktop issue
    //link to bug#80207
    auto recommendActions = Peony::FileLaunchManager::getRecommendActions(uri);
    auto fallbackActions = Peony::FileLaunchManager::getFallbackActions(uri);
    //fix has default open app but no recommend actions issue, link to bug#61365
    //fix open options has two same app issue, linkto bug#74480
    if (recommendActions.count() == 0 && fallbackActions.count() == 0)
    {
        auto action = Peony::FileLaunchManager::getDefaultAction(uri);
        if (action != NULL && action->getAppInfoDisplayName().length() > 0)
            recommendActions.append(action);
    }
    for (auto action : recommendActions) {
        action->setParent(openWithMenu);
        openWithMenu->addAction(static_cast<QAction*>(action));
    }

    for (auto action : fallbackActions) {
        action->setParent(openWithMenu);
        openWithMenu->addAction(static_cast<QAction*>(action));
    }
    openWithMenu->addSeparator();
    openWithMenu->addAction(tr("More applications"), this, [=]() {
        Peony::FileLauchDialog d(uri);
        d.createWinId();
        d.windowHandle()->setTransientParent(rootWindow);
        d.exec();
    });
    openWithAction->setMenu(openWithMenu);

    auto helpAction = new QAction(tr("Help Documnet"));
    helpAction->setVisible(isSpaceMenu);
    menu.addAction(openAction);
    menu.addAction(openWithAction);
    menu.addAction(helpAction);

    auto separator1 = new QAction;
    separator1->setSeparator(true);
    menu.addAction(separator1);

    auto addFileToSpaceAction = new QAction(tr("Add File"));
    addFileToSpaceAction->setVisible(!isFileSelected && isSpaceMenu);
    connect(addFileToSpaceAction, &QAction::triggered, model, [=]{
        QFileDialog fileDialog;
        fileDialog.createWinId();
        fileDialog.windowHandle()->setTransientParent(rootWindow);
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::AnyFile);
        if (fileDialog.exec()) {
            auto urls = fileDialog.selectedUrls();
            if (!urls.isEmpty())
                ModelController::globalInstance()->userAddFilesToSpace(urls, model);
        }
    });

    auto searchRelatedAction = new QAction(tr("Search Related Files"));
    searchRelatedAction->setVisible(isFileSelected && isSpaceMenu);
    connect(searchRelatedAction, &QAction::triggered, model, [=]{
        auto info = UKUI::IDM::FileInfo::fromPath(path);
        // 使用信号告知qml界面设置搜索关键字
        SearchResultProxyModel::globalInstance()->setSearchKeywordReqeust(info->displayName());
    });

    auto newAction = new QAction(tr("New"));
    newAction->setVisible(!isFileSelected && isSpaceMenu);
    auto newActionMenu = new QMenu;
    QDir templateDir(g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES));
    auto templates = templateDir.entryInfoList(QDir::Files, QDir::Name);
    if (!templates.isEmpty()) {
        QMimeDatabase mimeDB;
        for (auto templateInfo : templates) {
            auto mimeType = mimeDB.mimeTypeForFile(templateInfo);
            auto action = new QAction(QIcon::fromTheme(mimeType.iconName()), templateInfo.baseName());
            connect(action, &QAction::triggered, model, [=]{
                QFileDialog fileDialog;
                fileDialog.createWinId();
                fileDialog.windowHandle()->setTransientParent(rootWindow);
                fileDialog.setWindowTitle(tr("Choose directory to create file"));
                fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
                fileDialog.setFileMode(QFileDialog::DirectoryOnly);
                if (fileDialog.exec()) {
                    QString directoryPath = fileDialog.directory().absolutePath();
                    QUrl directoryUrl = QUrl::fromLocalFile(directoryPath);
                    if (!directoryUrl.isEmpty()) {
                        auto createOp = Peony::FileOperationUtils::create(directoryUrl.toEncoded(), templateInfo.fileName(), Peony::CreateTemplateOperation::Template);
                        connect(createOp, &Peony::FileOperation::operationFinished, model, [=]{
                            if (!createOp->hasError()) {
                                QList<QUrl> urls;
                                QString targetUri = createOp->property("targetUri").toString();
                                if (!targetUri.isEmpty()) {
                                    QUrl url(targetUri);
                                    urls << url;
                                    ModelController::globalInstance()->userAddFilesToSpace(urls, model);
                                }
                            }
                        }, Qt::BlockingQueuedConnection);
                    }
                }
            });
            newActionMenu->addAction(action);
        }

        newActionMenu->addSeparator();
    }

    auto createEmptyFileAction = new QAction(QIcon::fromTheme("document-new-symbolic"), tr("New File"));
    connect(createEmptyFileAction, &QAction::triggered, model, [=]{
        QFileDialog fileDialog;
        fileDialog.createWinId();
        fileDialog.windowHandle()->setTransientParent(rootWindow);
        fileDialog.setWindowTitle(tr("Choose directory to create file"));
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::DirectoryOnly);
        if (fileDialog.exec()) {
            QString directoryPath = fileDialog.directory().absolutePath();
            QUrl directoryUrl = QUrl::fromLocalFile(directoryPath);
            if (!directoryUrl.isEmpty()) {
                auto createOp = Peony::FileOperationUtils::create(directoryUrl.toEncoded());
                connect(createOp, &Peony::FileOperation::operationFinished, model, [=]{
                    if (!createOp->hasError()) {
                        QList<QUrl> urls;
                        QString targetUri = createOp->property("targetUri").toString();
                        if (!targetUri.isEmpty()) {
                            QUrl url(targetUri);
                            urls << url;
                            ModelController::globalInstance()->userAddFilesToSpace(urls, model);
                        }
                    }
                }, Qt::BlockingQueuedConnection);
            }
        }
    });
    newActionMenu->addAction(createEmptyFileAction);

    auto createEmptyFolderAction = new QAction(QIcon::fromTheme("folder-new-symbolic"), tr("New Folder"));
    connect(createEmptyFolderAction, &QAction::triggered, model, [=]{
        QFileDialog fileDialog;
        fileDialog.createWinId();
        fileDialog.windowHandle()->setTransientParent(rootWindow);
        fileDialog.setWindowTitle(tr("Choose directory to create file"));
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::DirectoryOnly);
        if (fileDialog.exec()) {
            QString directoryPath = fileDialog.directory().absolutePath();
            QUrl directoryUrl = QUrl::fromLocalFile(directoryPath);
            if (!directoryUrl.isEmpty()) {
                auto createOp = Peony::FileOperationUtils::create(directoryUrl.toEncoded(), nullptr, Peony::CreateTemplateOperation::EmptyFolder);
                connect(createOp, &Peony::FileOperation::operationFinished, model, [=]{
                    if (!createOp->hasError()) {
                        QList<QUrl> urls;
                        QString targetUri = createOp->property("targetUri").toString();
                        if (!targetUri.isEmpty()) {
                            QUrl url(targetUri);
                            urls << url;
                            ModelController::globalInstance()->userAddFilesToSpace(urls, model);
                        }
                    }
                }, Qt::BlockingQueuedConnection);
            }
        }
    });
    newActionMenu->addAction(createEmptyFolderAction);

    newAction->setMenu(newActionMenu);

    auto moveToSpaceAction = new QAction(tr("Move to space"));
    moveToSpaceAction->setVisible(isFileSelected && isSpaceMenu);
    auto moveToSpaceMenu = new QMenu;
    auto idsAndspaces = ModelController::globalInstance()->getSpaceIdsWithSpace();
    for (auto spaceId : idsAndspaces.keys()) {
        auto text = idsAndspaces.value(spaceId);
        QFontMetrics fm(qApp->font());
        QString elidedText = fm.elidedText(text, Qt::ElideRight, 300);
        auto action = moveToSpaceMenu->addAction(elidedText);
        action->setToolTip(text);
        connect(action, &QAction::triggered, this, [=]{
            auto targetProxyModel = ModelController::globalInstance()->getProxyModelBySpaceId(spaceId);
            if (targetProxyModel) {
                ModelController::globalInstance()->moveUserAddedFileFromSpaceTo(path, model, targetProxyModel);
            }
        });
    }
    moveToSpaceAction->setMenu(moveToSpaceMenu);
    moveToSpaceAction->setEnabled(isUserAddedFile);

    auto removeFromSpaceAction = new QAction(tr("Remove From Space"));
    connect(removeFromSpaceAction, &QAction::triggered, model, [=]{
        ModelController::globalInstance()->removeFileFromSpace(path, model);
    });
    removeFromSpaceAction->setVisible(isFileSelected && isSpaceMenu);
    removeFromSpaceAction->setEnabled(isUserAddedFile);

    menu.addAction(addFileToSpaceAction);
    menu.addAction(searchRelatedAction);
    menu.addAction(newAction);
    menu.addAction(moveToSpaceAction);
    menu.addAction(removeFromSpaceAction);

    auto separator2 = new QAction;
    separator2->setSeparator(true);
    separator2->setVisible(isFileSelected && isSpaceMenu);
    menu.addAction(separator2);

    auto copyAction = new QAction(tr("Copy"));
    connect(copyAction, &QAction::triggered, model, [=]{
        QStringList uris;
        QUrl url = QUrl::fromLocalFile(path);
        uris << url.toEncoded();
        Peony::ClipboardUtils::getInstance()->setClipboardFiles(uris, false, false);
    });
    copyAction->setVisible(isFileSelected);

    auto renameAction = new QAction(tr("Rename"));
    connect(renameAction, &QAction::triggered, model, [=]{
        auto info = UKUI::IDM::FileInfo::fromPath(path);
        QInputDialog inputDialog;
        inputDialog.setWindowTitle(tr("Rename"));
        inputDialog.setLabelText(tr("Input New File Name"));
        inputDialog.setTextValue(info->displayName());
        inputDialog.createWinId();
        inputDialog.windowHandle()->setTransientParent(rootWindow);
        QString newName;
        if (inputDialog.exec()) {
            newName = inputDialog.textValue();
        }
        if (!newName.isEmpty()) {
            // 重命名操作调用了Peony::FileInfo的displayName()方法获取当前文件名称，这里用peony的接口查询一次文件信息，避免重命名操作异常
            Peony::FileInfoJob j(info->uri());
            j.querySync();
            auto renameOp = Peony::FileOperationUtils::rename(info->uri(), newName, false);
            Q_UNUSED(renameOp)
        }
    });
    renameAction->setVisible(isFileSelected && isSpaceMenu);

    auto deleteAction = new QAction(tr("Delete Original File"));
    connect(deleteAction, &QAction::triggered, model, [=]{
        QStringList uris;
        QUrl url = QUrl::fromLocalFile(path);
        uris << url.toEncoded();
        // fixme: 指定父窗口
        Peony::FileOperationUtils::trashWithDialog(uris, false);
    });
    deleteAction->setVisible(isFileSelected && isSpaceMenu);
    menu.addAction(copyAction);
    menu.addAction(renameAction);
    menu.addAction(deleteAction);

    auto separator3 = new QAction;
    separator3->setSeparator(true);
    separator3->setVisible(isFileSelected);
    menu.addAction(separator3);

    auto labelAction = new QAction(tr("Label"));
    labelAction->setVisible(isFileSelected && isSpaceMenu);
    if (labelAction->isVisible()) {
        auto labelActionMenu = new QMenu;
        auto labelModel = FileLabelModel::getGlobalModel();
        auto labels = labelModel->getAllFileLabelItems();
        // ugly
        Peony::FileInfoJob j(uri);
        j.querySync();
        auto fileLabelIds = labelModel->getFileLabelIds(uri);
        for (auto label : labels) {
            auto labelAction = labelActionMenu->addAction(label->name());
            labelAction->setCheckable(true);
            if (fileLabelIds.contains(label->id())) {
                labelAction->setChecked(true);
            }
            connect(labelAction, &QAction::triggered, model, [=](bool checked){
                // ugly
                Peony::FileInfoJob j(uri);
                j.querySync();
                if (checked) {
                    labelModel->addLabelToFile(uri, label->id());
                } else {
                    labelModel->removeFileLabel(uri, label->id());
                }

                QFileInfo fileInfo(path);
                UKUI::IDM::FileInfoManager::globalInstance()->fileChanged(UKUI::IDM::FileInfoManager::FileAttrChanged, path, nullptr);
            });
        }
        labelAction->setMenu(labelActionMenu);
    }
    menu.addAction(labelAction);

    auto separator4 = new QAction;
    separator4->setSeparator(true);
    separator4->setVisible(isFileSelected && isSpaceMenu);
    menu.addAction(separator4);

    auto propertiesAction = new QAction(tr("Properties"));
    connect(propertiesAction, &QAction::triggered, model, [=]{
        QStringList uris;
        QUrl url = QUrl::fromLocalFile(path);
        uris << url.toEncoded();
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.FileManager1",
                                                          "/org/freedesktop/FileManager1",
                                                          "org.freedesktop.FileManager1",
                                                          "ShowItemProperties");
        msg << uris;
        msg << qAppName();
        QDBusConnection::sessionBus().asyncCall(msg);
    });
    propertiesAction->setVisible(isFileSelected);
    menu.addAction(propertiesAction);

    auto resultAction = menu.exec(globalPos);
}

void FileOperationManager::openFile(const QUrl &url)
{
    Peony::FileLaunchManager::openAsync(url.toEncoded());
}

void FileOperationManager::setRootWindow(QWindow *newRootWindow)
{
    rootWindow = newRootWindow;
}
