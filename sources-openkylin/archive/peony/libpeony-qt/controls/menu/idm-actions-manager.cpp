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

#include "idm-actions-manager.h"
#include "side-bar-abstract-item.h"
#include "directory-view-widget.h"
#include "idm-dbus-manager.h"
#include "file-launch-manager.h"
#include "file-launch-action.h"
#include "file-lauch-dialog.h"
#include "fm-window.h"
#include "plugin-manager.h"
#include "clipboard-utils.h"
#include "kysdk/applications/kmessagebox.h"
#include "properties-window-factory-plugin-manager.h"
#include "file-utils.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <menu-plugin-iface.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <gio/gdesktopappinfo.h>
#include "menu-plugin-manager.h"
#include "directory-view-factory-manager.h"
#include "view-factory-sort-filter-model.h"
#include "directory-view-plugin-iface2.h"
#include "intelligent-space-dialog.h"
using namespace Peony;

static IDMActionsManager *globalInstance = nullptr;


IDMActionsManager *IDMActionsManager::getInstance()
{
    if (!globalInstance) {
        globalInstance = new IDMActionsManager;
    }
    return globalInstance;
}

void IDMActionsManager::showFileReferencedMessage(const QString &spaceName, QWidget *parent)
{
    if (spaceName.isEmpty() || qApp->property("skipAskAddFile").toBool()) {
        return;
    }

    QWidget *dialogParent = parent;
    if (!dialogParent) {
        dialogParent = QApplication::topLevelAt(QCursor::pos());
    }
    if (!dialogParent) {
        dialogParent = QApplication::activeWindow();
    }
    kdk::KMessageBox messageBox(dialogParent);
    messageBox.setIcon(kdk::KMessageBox::Icon::Warning);

    auto okButton = new QPushButton(tr("OK"));
    messageBox.addButton(okButton, kdk::KMessageBox::AcceptRole);
    okButton->setProperty("isImportant", true);
    okButton->setDefault(true);
    okButton->setFocus();

    QString title = tr("The file has been referenced in the \"%1\" space.")
                            .arg(spaceName.toHtmlEscaped());
    QString description = tr("<font color='gray'>This operation does not move or copy the file itself. The file remains in its original directory. Manually added files are not affected by the filtering conditions in the space.</font>");
    messageBox.setText(title + "<br><br>" + description);

    auto checkbox = new QCheckBox;
    checkbox->setText(tr("Do not ask again"));
    messageBox.setCheckBox(checkbox);
    messageBox.exec();

    if (messageBox.clickedButton() == okButton && checkbox->isChecked()) {
        qApp->setProperty("skipAskAddFile", true);
    }
}

bool IDMActionsManager::addClipboardFilesToSpace(const QString &spaceName, QWidget *parent)
{
    if (spaceName.isEmpty()) {
        return false;
    }

    QStringList clipboardUris = ClipboardUtils::getClipboardFilesUris();
    if (clipboardUris.isEmpty()) {
        return false;
    }

    QSet<QString> existingFiles = IDMDBusManager::getInstance()->getFilesInSpaceByName(spaceName).toSet();
    QStringList filePaths;
    QString duplicatePath;
    for (const QString &uri : clipboardUris) {
        QUrl url = QUrl::fromEncoded(uri.toUtf8());
        QString filePath = url.isLocalFile() ? url.toLocalFile() : QString();
        if (filePath.isEmpty() && uri.startsWith('/')) {
            filePath = FileUtils::urlDecode(uri);
        }
        if (filePath.isEmpty()) {
            continue;
        }
        if (existingFiles.contains(filePath)) {
            if (duplicatePath.isEmpty()) {
                duplicatePath = filePath;
            }
            continue;
        }
        existingFiles.insert(filePath);
        filePaths.append(filePath);
    }

    if (filePaths.isEmpty()) {
        if (!duplicatePath.isEmpty()) {
            QWidget *dialogParent = parent ? parent : QApplication::topLevelAt(QCursor::pos());
            QMessageBox::warning(dialogParent, nullptr,
                                 tr("File %1 is existed in space, please not add file repeatly.").arg(duplicatePath));
        }
        return false;
    }

    bool isSuccess = IDMDBusManager::getInstance()->userAddFilesToSpace(filePaths, spaceName);
    if (isSuccess) {
        if (ClipboardUtils::isClipboardFilesBeCut()) {
            ClipboardUtils::clearClipboard();
        }
        showFileReferencedMessage(spaceName, parent);
    }
    return isSuccess;
}

bool IDMActionsManager::deleteSpaceWithConfirmation(const QString &spaceName, QWidget *parent)
{
    if (spaceName.isEmpty()
            || IDMDBusManager::getInstance()->getSpaceIdByName(spaceName) == 1) {
        return false;
    }

    QWidget *dialogParent = parent;
    if (!dialogParent) {
        dialogParent = QApplication::topLevelAt(QCursor::pos());
    }
    if (!dialogParent) {
        dialogParent = QApplication::activeWindow();
    }

    QString title = tr("Are you sure you want to delete the \"%1\" space?")
                            .arg(spaceName.toHtmlEscaped());
    QString description = tr("<font color='gray'>Deleting the space will also delete its file references. The source files are not affected and remain in their original directories.</font>");

    QMessageBox questionBox(dialogParent);
    questionBox.setIcon(QMessageBox::Icon::Warning);
    questionBox.setText(title + "<br><br>" + description);
    QPushButton *deleteButton = questionBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
    questionBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    questionBox.setDefaultButton(deleteButton);
    deleteButton->setChecked(true);
    questionBox.exec();

    if (questionBox.clickedButton() != deleteButton) {
        return false;
    }
    return IDMDBusManager::getInstance()->removeSpace(spaceName);
}

const QList<QAction *> IDMActionsManager::constructIDMActions(SideBarAbstractItem *item, FMWindowIface *window, DirectoryViewWidget *view, IDMActionsManager::MenuType type)
{
    QList<QAction *> l;
    m_realselectUris.clear();
    QStringList selectUris;
    for (auto selectUri : getSelections(view, item)) {
       selectUris.append(FileUtils::urlDecode(selectUri));
    }
    QString directoryUri = FileUtils::urlDecode(getDirectoryUri(view, item));
    bool isFileSelected = !selectUris.isEmpty();
    bool ismultiselected = selectUris.count() > 1 ? true : false;
    bool isRootDirectory = directoryUri == "idm:///" ? true : false;
    bool isDynamic = false;

    QString uri = selectUris.isEmpty() ? "" : selectUris.first();
    QString realUri = getRealUriFromUri(uri);
    QString spaceName;
    if (MenuType::SideBar == type) {
        spaceName = directoryUri.remove("idm:///");
    } else if (MenuType::DirectoryView == type){
        if (isFileSelected && isRootDirectory) {
            spaceName = selectUris.first().remove("idm:///");
        } else if (!isRootDirectory) {
            spaceName = directoryUri.remove("idm:///");
        }
    }
    m_realselectUris = selectUris;
    auto info = FileInfo::fromUri(uri);
    auto displayName = info->displayName();
    auto mimeType = info->mimeType();
    GAppInfo *m_app_info = nullptr;
    if(info->isDesktopFile()){
        GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(info->filePath().toUtf8().constData());
        m_app_info  = G_APP_INFO(appInfo);
    }else{
        GAppInfo *appinfo = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
        auto app_info = G_APP_INFO(appinfo);
        m_app_info = static_cast<GAppInfo*>(g_object_ref(app_info));
    }
    GIcon *icon = g_app_info_get_icon(m_app_info);
    QIcon  appicon;
    auto iconName = FileUtils::getIconStringFromGIcon(icon);
    if (iconName.startsWith("/")) {
        appicon.addFile(iconName);
    } else {
        appicon = QIcon::fromTheme(iconName);
    }
    appicon = appicon.isNull() ? QIcon::fromTheme("document-open-symbolic") : appicon;

    qDebug() << __func__ << spaceName << realUri;
    if (!spaceName.isEmpty()) {
        isDynamic = IDMDBusManager::getInstance()->getSpacestatus(spaceName);
    }

    auto renamAction = new QAction(QIcon::fromTheme("rename-symbolic"),tr("Rename Spaces"));
    connect(renamAction, &QAction::triggered, this, [=](){
        QWidget *widget = QApplication::topLevelAt(QCursor::pos());
        IntelligentSpaceRenameDialog d(spaceName, widget);
        if (d.exec() == QDialog::Accepted) {
            QString text = d.getNewName();
            if (!text.isNull() && !text.isEmpty()) {
                bool isSuccess = IDMDBusManager::getInstance()->renameSpace(spaceName, text);
                if (!isSuccess){
                     QMessageBox::warning(widget,"", tr("Failed to rename space %1 to %2, %2 is existed.").arg(spaceName).arg(text), QMessageBox::Ok);
                }
            }
        }
    });

    auto editCondition = new QAction(QIcon::fromTheme("edit-symbolic"),tr("Edit Conditiones"));
    editCondition->setObjectName(EDIT_SPACE_ACTION);
    connect(editCondition, &QAction::triggered, this, [=](){
        QString uri = "idm:///" + QString(QUrl::toPercentEncoding(spaceName));
        Q_EMIT IDMDBusManager::getInstance()->editSpaceConditionSignal(uri);
    });

    auto refreshAction = new QAction(QIcon::fromTheme("view-wrapped-symbolic"),tr("Close dynamically"));
    if (!isDynamic) {
        refreshAction->setText(tr("Open dynamically"));
    }
    //更新状态  
    connect(refreshAction, &QAction::triggered, this, [=](){
        IDMDBusManager::getInstance()->updateSpacestatus(spaceName, !isDynamic);
    });

    auto deleteAction = new QAction(QIcon::fromTheme("user-trash-symbolic"),tr("Delete Spaces"));
    connect(deleteAction, &QAction::triggered, this, [=](){
        deleteSpaceWithConfirmation(spaceName);
    });
    int spaceId = IDMDBusManager::getInstance()->getSpaceIdByName(spaceName);
    bool isDeFaultSpace  = spaceId == 1 ? true:false;
    deleteAction->setDisabled(isDeFaultSpace);
    auto openAction = new QAction(appicon,tr("Open"));
    openAction->setShortcut(QKeySequence(Qt::Key_Enter));
    connect(openAction, &QAction::triggered, this, [=](){
        if (window) {
            auto info = FileInfo::fromUri(uri);
            if (info->isDir()) {
                window->goToUri(uri, true);
            } else {
                FileLaunchManager::openAsync(uri, false, false);
            }
        }
    });

    auto openWithAction = new QAction(QIcon::fromTheme("openwith-symbolic"),tr("Open With"));
    QMenu *openWithMenu = new QMenu;
    openWithMenu->setProperty("skipHighlightIconEffect", true);
    auto recommendActions = Peony::FileLaunchManager::getRecommendActions(realUri);
    auto fallbackActions = Peony::FileLaunchManager::getFallbackActions(realUri);
    if (recommendActions.count() == 0 && fallbackActions.count() == 0)
    {
        auto action = Peony::FileLaunchManager::getDefaultAction(realUri);
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
        Peony::FileLauchDialog d(realUri);
        d.exec();
    });
    openWithAction->setMenu(openWithMenu);

    auto openLocationAction = new QAction(QIcon::fromTheme("folder-visiting-symbolic"),tr("Open The Folder Location"));
    openLocationAction->setEnabled(!ismultiselected);
    QString realPath;
    if (!realUri.isEmpty()) {
        realPath = QString(realUri).remove("file://");
    }
    connect(openLocationAction, &QAction::triggered, this, [=](){
        if (!realUri.isEmpty()) {
            QFileInfo info(realPath);
            QString directoryPath = "file://" + info.absolutePath();
            if (window) {
                auto windowIface = window->create(directoryPath);
                auto newWindow = dynamic_cast<QWidget *>(windowIface);
                newWindow->setAttribute(Qt::WA_DeleteOnClose);
                newWindow->show();
            }
        }
    });
    openLocationAction->setEnabled(selectUris.count() == 1);

    auto separator = new QAction;
    separator->setSeparator(true);

    auto separator1 = new QAction;
    separator1->setSeparator(true);

    auto separator2 = new QAction;
    separator2->setSeparator(true);

    auto separator3 = new QAction;
    separator3->setSeparator(true);


    auto separator4 = new QAction;
    separator4->setSeparator(true);


    auto separator5 = new QAction;
    separator5->setSeparator(true);

    auto separator6 = new QAction;
    separator6->setSeparator(true);

    auto separator7 = new QAction;
    separator7->setSeparator(true);

    auto separator8 = new QAction;
    separator8->setSeparator(true);
    QDir DocumentDir(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));
    auto addFileToSpaceAction = new QAction(QIcon::fromTheme("list-add-symbolic"),tr("Add File"));
    connect(addFileToSpaceAction, &QAction::triggered, this, [=]{
        QWidget *widget = QApplication::topLevelAt(QCursor::pos());
        QFileDialog fileDialog(widget);
        // fixme: 居中窗口
        fileDialog.setWindowTitle(tr("Choose file to add into space"));
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::AnyFile);
        fileDialog.setDirectory(DocumentDir);
        if (fileDialog.exec()) {
            auto urls = fileDialog.selectedUrls();
            if (!urls.isEmpty()) {
                //userAddFilesToSpace
                QStringList filePaths;
                for (auto url : urls) {
                    filePaths.append(url.path());
                }
                qDebug() << "urls: " << urls << "filePaths:" << filePaths << spaceName;

                QSet<QString> allFiles = IDMDBusManager::getInstance()->getFilesInSpaceByName(spaceName).toSet();
                if (allFiles.contains(filePaths.first())) {
                    QMessageBox::warning(widget, nullptr, tr("File %1 is existed in space, please not add file repeatly.").arg(filePaths.first()));
                    return;
                }

                bool isSuccess = IDMDBusManager::getInstance()->userAddFilesToSpace(filePaths, spaceName);
                if (isSuccess) {
                    showFileReferencedMessage(spaceName, widget);
                }
            }
        }
    });

    // auto searchRelatedAction = new QAction(tr("Search Related Files"));
    // connect(searchRelatedAction, &QAction::triggered, this, [=]{

    // });

    bool isUserAddedFile = false;
    auto moveToSpaceAction = new QAction(QIcon::fromTheme("app-group-movein-symbolic"),tr("Move to space"));
    auto moveToSpaceMenu = new QMenu("");
    auto allSpacesNames = IDMDBusManager::getInstance()->getAllSpacesName();
    moveToSpaceMenu->setToolTipsVisible(true);
    for (auto currentSpaceName : allSpacesNames) {
        if(spaceName == currentSpaceName){
            continue;
        }
        QFontMetrics fm(qApp->font());
        QString elidedText = fm.elidedText(currentSpaceName, Qt::ElideRight, 300);
        auto action = moveToSpaceMenu->addAction(elidedText);
        if(currentSpaceName != elidedText){
            //QFont spaceNameFont = moveToSpaceMenu->font();
            //auto realDisplayName = Peony::FileUtils::wrapTextForTooltip(currentSpaceName, QCursor::pos(), spaceNameFont);
            action->setToolTip(currentSpaceName);
        }
        action->setIcon(QIcon::fromTheme("folder-space-symbolic"));
        connect(action, &QAction::triggered, this, [=]{
            if (currentSpaceName != spaceName) {
                if (!selectUris.isEmpty()) {
                    for (auto uri : selectUris) {
                        QStringList filePaths;
                        QString tempUri = getRealUriFromUri(uri);
                        QString realPath = tempUri.remove("file://");
                        filePaths.append(realPath);
                        QSet<QString> allFiles = IDMDBusManager::getInstance()->getFilesInSpaceByName(currentSpaceName).toSet();
                        if (allFiles.contains(realPath)) {
                            QMessageBox::warning(nullptr, nullptr, tr("File %1 is existed in space, please not add file repeatly.").arg(realPath));
                            continue;
                        }
                        bool ok = IDMDBusManager::getInstance()->removeFilesFromSpace(spaceName, filePaths);
                        if(ok){
                            IDMDBusManager::getInstance()->userAddFilesToSpace(filePaths,currentSpaceName);
                        }
                    }
                }

            }
        });
    }
    moveToSpaceAction->setMenu(moveToSpaceMenu);

    auto removeFromSpaceAction = new QAction(QIcon::fromTheme("user-trash-symbolic"),tr("Remove From Space"));
    connect(removeFromSpaceAction, &QAction::triggered, this, [=]{

        if (!qApp->property("skipAskRemoveFile").toBool()) {
            QWidget *widget = QApplication::topLevelAt(QCursor::pos());
            kdk::KMessageBox questionBox(widget);
            questionBox.setIcon(kdk::KMessageBox::Icon::Warning);
            auto cancelButton = new QPushButton(tr("Cancel"));
            auto okButton = new QPushButton(tr("Remove"));
            questionBox.addButton(cancelButton, kdk::KMessageBox::RejectRole);
            questionBox.addButton(okButton, kdk::KMessageBox::AcceptRole);
            okButton->setProperty("isImportant", true);
            okButton->setDefault(true);
            okButton->setFocus();
            QString text = tr("<font color='gray'>Qualified removed files will still be automatically aggregated in the next update.</font>");
            QString filetext = tr("<font color='gray'>Qualified removed files will still be automatically aggregated in the next update.</font>");

            if(ismultiselected)
                questionBox.setText(tr("Do you want to remove selected %1 item(s)?<br>").arg(selectUris.size())+"<br>"+text);
            else
                questionBox.setText(tr("Do you want to remove this file?<br>")+"<br>"+"<br>"+filetext);
            auto checkbox = new QCheckBox;
            checkbox->setText(tr("Do not ask again"));
            questionBox.setCheckBox(checkbox);
            questionBox.exec();
            auto resultButton = questionBox.clickedButton();

            if (resultButton != okButton)
                return;
            if (checkbox->isChecked()) {
                qApp->setProperty("skipAskRemoveFile", true);
            }
        }

        if (!selectUris.isEmpty()) {
            QStringList paths;
            for (auto selectUri : selectUris) {
                QString tempUri = getRealUriFromUri(selectUri);
                QString realPath = tempUri.remove("file://");
                paths.append(realPath);
            }
            IDMDBusManager::getInstance()->removeFilesFromSpace(spaceName, paths);
        }
    });
    removeFromSpaceAction->setVisible(isFileSelected);


    bool fileremoveble = !((realPath == "/usr/share/peony-intelligent-data-management-service/智能空间使用手册.pdf") && spaceId == 1);
    removeFromSpaceAction->setEnabled(fileremoveble);
    moveToSpaceAction->setEnabled(fileremoveble);


    auto pluginManager = Peony::PluginManager::getInstance();
    QString pluginName = "libpeony-menu-plugin-kylin-aiassistant.so";
    auto plugininterface = pluginManager->getPluginByFileName(pluginName);
//    bool isAiassistantAavilable = false;
//    QProcess process;
//    QString packageName = "kylin-aiassistant";
//    process.start("dpkg", QStringList() << "-l" << packageName);
//    if (!process.waitForFinished()) {
//         isAiassistantAavilable = false;
//      }
//      QString output = process.readAllStandardOutput();
//      if (output.contains(packageName)) {
//          isAiassistantAavilable =  true;
//      } else {
//          isAiassistantAavilable =  false;
//      }
    QList<QAction*> assistantActions;
    bool showAiassistant = false;
    if (plugininterface) {
        auto plugin = static_cast<Peony::MenuPluginInterface *>(plugininterface);
        QStringList uris;
        for(auto selectUri : selectUris)
        {
            QString realUri = getRealUriFromUri(selectUri);
            uris << realUri;
        }
        assistantActions = plugin->menuActions(Peony::MenuPluginInterface::DirectoryView, nullptr, uris);
        showAiassistant = true;
        if(!assistantActions.isEmpty())
        {
            assistantActions.first()->setVisible(isFileSelected);
            connect(assistantActions.first(),&QAction::triggered,this,[=]{
                QProcess::startDetached("kylin-aiassistant",{"-l"});
            });
        }
    }

    auto copyAction = new QAction(QIcon::fromTheme("edit-copy-symbolic"),tr("Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setObjectName(COPY_ACTION);
    connect(copyAction, &QAction::triggered, this, [=]{
        QStringList uris;
        if(ismultiselected)
        {
            for(auto path : selectUris)
            {
                QString realPath = getRealUriFromUri(path);
                uris << realPath;
            }
        }
        else
        {
            if (!realUri.isEmpty()) {
                uris << realUri;
            }
        }
        ClipboardUtils::getInstance()->setClipboardFiles(uris, false, false);
    });
    copyAction->setVisible(isFileSelected);

    auto saveAsAction = new QAction(QIcon::fromTheme("document-save-as-symbolic"),tr("Save As"));
    connect(saveAsAction, &QAction::triggered, this, [=]{
        if (realPath.isEmpty())
            return;
        QString filesuffix = QFileInfo(realPath).suffix();
        QString filter = QString(tr("file")+" (*.%1);;"+tr("All File")+" (*)").arg(filesuffix);
        QFileDialog dialog(nullptr);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setOption(QFileDialog::DontConfirmOverwrite);
        dialog.setNameFilter(filter);
        if(dialog.exec() == QDialog::Accepted)
        {
            QString filePath = dialog.selectedFiles().first();
            QFileInfo fileinfo(filePath);
            QString DirectoryPath = fileinfo.absolutePath();
            bool iswriteble = QFileInfo(DirectoryPath).isWritable();
            if(iswriteble)
            {
                if(QFile::exists(filePath) && realPath != filePath)
                    QFile::remove(filePath);
                QFile::copy(realPath, filePath);
            }
            else
            {
                kdk::KMessageBox questionBox;
                questionBox.setIcon(kdk::KMessageBox::Icon::Warning);
                auto cancelButton = new QPushButton(tr("Cancel"));
                auto okButton = new QPushButton(tr("OK"));
                questionBox.addButton(cancelButton, kdk::KMessageBox::RejectRole);
                questionBox.addButton(okButton, kdk::KMessageBox::AcceptRole);
                okButton->setProperty("isImportant", true);
                okButton->setDefault(true);
                okButton->setFocus();
                questionBox.setText(tr("This location has no write permission.please change the storage location"));
                questionBox.exec();
            }
        }
    });
    saveAsAction->setVisible(isFileSelected);
    saveAsAction->setEnabled(!ismultiselected);


    auto pluginIds = MenuPluginManager::getInstance()->getPluginIds();
    //sort plugiins by name, so the menu option orders is relatively fixed
    std::sort(pluginIds.begin(), pluginIds.end());

//    auto labelAction = new QAction(tr("Label"));
//    if (labelAction->isVisible()) {
//        auto labelActionMenu = new QMenu;
//        auto labelModel = FileLabelModel::getGlobalModel();
//        auto labels = labelModel->getAllFileLabelItems();
//        Peony::FileInfoJob j(uri);
//        j.querySync();
//        auto fileLabelIds = labelModel->getFileLabelIds(uri);
//        for (auto label : labels) {
//            auto labelAction = labelActionMenu->addAction(label->name());
//            labelAction->setCheckable(true);
//            if (fileLabelIds.contains(label->id())) {
//                labelAction->setChecked(true);
//            }
//            connect(labelAction, &QAction::triggered, this, [=](bool checked){
//                Peony::FileInfoJob j(uri);
//                j.querySync();
//                if (checked) {
//                    labelModel->addLabelToFile(uri, label->id());
//                } else {
//                    labelModel->removeFileLabel(uri, label->id());
//                }
//            });
//        }
//        labelAction->setMenu(labelActionMenu);
//    }

    auto propertiesAction = new QAction(QIcon::fromTheme("properties-symbolic"),tr("Properties"));
    propertiesAction->setShortcut(QKeySequence("Alt+Enter"));
    connect(propertiesAction, &QAction::triggered, this, [=](){
        auto w = PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<selectUris);
        PropertiesWindowFactoryPluginManager::getInstance()->show();
    });

    switch (type) {
    case MenuType::SideBar: {
        l.append(renamAction);
        l.append(deleteAction);
        l.append(separator6);
        l.append(editCondition);
        l.append(refreshAction);       
        break;
    }
    case MenuType::DirectoryView: {
        if ((isRootDirectory && !selectUris.isEmpty() && !ismultiselected) ||
                (!isRootDirectory && selectUris.isEmpty())) {

            if (isRootDirectory && !selectUris.isEmpty() && !ismultiselected) {
                if (!qApp->property("tabletMode").toBool()) {
                    auto openInNewTabAction = new QAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open in New Tab"), this);
                    openInNewTabAction->setObjectName("open-in-new-tab-action");
                    openInNewTabAction->setShortcut(QKeySequence("Ctrl+t"));
                    connect(openInNewTabAction, &QAction::triggered, this, [=]() {
                        if (window) {
                            QStringList uris;
                            uris << selectUris.first();
                            window->addNewTabs(uris);
                        }
                    });
                    l.append(openInNewTabAction);
                }

                auto openInNewWindowAction = new QAction(QIcon::fromTheme("window-new-symbolic"), tr("Open in New Window"), this);
                openInNewWindowAction->setObjectName("open-in-new-window-action");
                openInNewWindowAction->setShortcut(QKeySequence("Ctrl+n"));
                connect(openInNewWindowAction, &QAction::triggered, this, [=]() {
                    if (window) {
                        auto windowIface = window->create(selectUris.first());
                        auto newWindow = dynamic_cast<QWidget *>(windowIface);
                        newWindow->setAttribute(Qt::WA_DeleteOnClose);
                        newWindow->show();
                    }
                });
                l.append(openInNewWindowAction);
            }

            if (!isRootDirectory && selectUris.isEmpty()) {
                if (!qApp->property("tabletMode").toBool()) {
                    auto openInNewTabAction2 = new QAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open in New Tab"), this);
                    openInNewTabAction2->setObjectName("open-in-new-tab-action");
                    openInNewTabAction2->setShortcut(QKeySequence("Ctrl+t"));
                    connect(openInNewTabAction2, &QAction::triggered, this, [=]() {
                        if (window) {
                            QStringList uris;
                            uris << directoryUri;
                            window->addNewTabs(uris);
                        }
                    });
                    l.append(openInNewTabAction2);
                }

                auto openInNewWindowAction2 = new QAction(QIcon::fromTheme("window-new-symbolic"), tr("Open in New Window"), this);
                openInNewWindowAction2->setObjectName("open-in-new-window-action");
                openInNewWindowAction2->setShortcut(QKeySequence("Ctrl+n"));
                connect(openInNewWindowAction2, &QAction::triggered, this, [=]() {
                    if (window) {
                        auto windowIface = window->create(directoryUri);
                        auto newWindow = dynamic_cast<QWidget *>(windowIface);
                        newWindow->setAttribute(Qt::WA_DeleteOnClose);
                        newWindow->show();
                    }
                });
                l.append(openInNewWindowAction2);
                auto separatortmp = new QAction;
                separatortmp->setSeparator(true);
                l.append(separatortmp);
                auto viewTypeAction = new QAction(QIcon::fromTheme("view-grid-symbolic"), tr("View Type"), this);
                viewTypeAction->setObjectName("view-type-action");
                QMenu *viewTypeMenu = new QMenu();
                {
                    Peony::ViewFactorySortFilterModel2 model;
                    model.setDirectoryUri(directoryUri);
                    auto viewNames = model.supportViewIds();
                    viewNames.removeAll("Space View");
                    auto viewFactoryManager = Peony::DirectoryViewFactoryManager2::getInstance();
                    for (auto viewId : viewNames) {
                        auto factory = viewFactoryManager->getFactory(viewId);
                        if (factory) {
                            auto viewAction = viewTypeMenu->addAction(factory->viewIcon(), factory->viewName());
                            viewAction->setData(viewId);
                            if (view && view->viewId() == viewId) {
                                viewAction->setCheckable(true);
                                viewAction->setChecked(true);
                            } else {
                                connect(viewAction, &QAction::triggered, this, [=]() {
                                    window->beginSwitchView(viewId);
                                });
                            }
                        }
                    }
                }
                viewTypeAction->setMenu(viewTypeMenu);
                l.append(viewTypeAction);

                auto sortTypeAction = new QAction(QIcon::fromTheme("ukui-view-sort-letter-symbolic"), tr("Sort By"), this);
                sortTypeAction->setObjectName("sort-type-action");
                QMenu *sortTypeMenu = new QMenu();
                QList<QAction *> sortActions;
                QStringList sortNames;
                int sortType = view->getSortType();
                sortNames << tr("Descending Order") << tr("Ascending Order");
                if (sortType == 1) {
                    sortNames.clear();
                    sortNames << tr("Newest to oldest") << tr("Oldest to newest");
                } else if (sortType == 3) {
                    sortNames.clear();
                    sortNames << tr("Files from large to small") << tr("Files from small to large");
                }
                sortActions << sortTypeMenu->addAction(tr("File Name"));
                sortActions << sortTypeMenu->addAction(tr("Time Modified"));
                sortActions << sortTypeMenu->addAction(tr("File Type"));
                sortActions << sortTypeMenu->addAction(tr("File Size"));
                sortActions << sortTypeMenu->addSeparator();
                sortActions << sortTypeMenu->addAction(sortNames[0]);
                sortActions << sortTypeMenu->addAction(sortNames[1]);
                int sortOrder = view->getSortOrder();
                sortActions.at(sortOrder+5)->setCheckable(true);
                sortActions.at(sortOrder+5)->setChecked(true);
                if (view) {
                    if (sortType >= 0 && sortType < sortActions.count()) {
                        sortActions.at(sortType)->setCheckable(true);
                        sortActions.at(sortType)->setChecked(true);
                    }
                    for (int i = 0; i < sortActions.count(); i++) {
                        connect(sortActions.at(i), &QAction::triggered, this, [=]() {
                            if(i<4){
                                window->setCurrentSortColumn(i);
                            }else{
                                window->setCurrentSortOrder(Qt::SortOrder(i-5));
                            }
                        });
                    }
                }
                sortTypeAction->setMenu(sortTypeMenu);
                l.append(sortTypeAction);

                auto sortPreferencesAction = new QAction(QIcon::fromTheme("ukui-view-sort-favorite-symbolic"), tr("Sort Preferences"), this);
                sortPreferencesAction->setObjectName("sort-preferences-action");
                QMenu *sortPreferencesMenu = new QMenu();
                auto folderFirst = sortPreferencesMenu->addAction(tr("Folder First"));
                folderFirst->setCheckable(true);
                if (window) {
                    folderFirst->setChecked(window->getWindowSortFolderFirst());
                }
                connect(folderFirst, &QAction::triggered, this, [=](bool checked) {
                    if (window) {
                        window->setSortFolderFirst(checked);
                    }
                });
                if (QLocale::system().name().contains("zh")) {
                    auto useDefaultNameSortOrder = sortPreferencesMenu->addAction(tr("Chinese First"));
                    useDefaultNameSortOrder->setCheckable(true);
                    if (window) {
                        useDefaultNameSortOrder->setChecked(window->getWindowUseDefaultNameSortOrder());
                    }
                    connect(useDefaultNameSortOrder, &QAction::triggered, this, [=](bool checked) {
                        if (window) {
                            window->setUseDefaultNameSortOrder(checked);
                        }
                    });
                }
                auto showHidden = sortPreferencesMenu->addAction(tr("Show Hidden"));
                showHidden->setCheckable(true);
                if (window) {
                    showHidden->setChecked(window->getWindowShowHidden());
                }
                connect(showHidden, &QAction::triggered, this, [=](bool checked) {
                    if (window) {
                        window->setShowHidden(checked);
                    }
                });
                sortPreferencesAction->setMenu(sortPreferencesMenu);
                l.append(sortPreferencesAction);

                auto refreshAction2 = new QAction(QIcon::fromTheme("view-refresh-symbolic"), tr("Refresh"), this);
                refreshAction2->setObjectName("refresh-action");
                refreshAction2->setShortcut(QKeySequence::Refresh);
                connect(refreshAction2, &QAction::triggered, this, [=]() {
                    if (window) {
                        window->refresh();
                    }
                });
                l.append(refreshAction2);

                auto spaceActionSeparator = new QAction;
                spaceActionSeparator->setSeparator(true);
                l.append(spaceActionSeparator);

                l.append(editCondition);
                l.append(addFileToSpaceAction);

                auto pasteAction = new QAction(QIcon::fromTheme("edit-paste-symbolic"), tr("Paste as Reference"), this);
                pasteAction->setObjectName(PASTE_ACTION);
                pasteAction->setShortcut(QKeySequence::Paste);
                pasteAction->setEnabled(ClipboardUtils::isClipboardHasFiles());
                connect(pasteAction, &QAction::triggered, this, [=]() {
                    addClipboardFilesToSpace(spaceName);
                });
                l.append(pasteAction);
            }
            if (isRootDirectory) {
                auto separatortmp1 = new QAction;
                separatortmp1->setSeparator(true);
                l.append(separatortmp1);
                l.append(renamAction);
                l.append(editCondition);
                l.append(refreshAction);
                l.append(deleteAction);
            }
        } else if (!isRootDirectory && !selectUris.isEmpty()) {
            l.append(openAction);
            l.append(openWithAction);
            l.append(openLocationAction);
            l.append(separator);
            // l.append(searchRelatedAction);          
            l.append(copyAction);
            l.append(saveAsAction);
            l.append(separator2);
            for (auto id : pluginIds) {
                FileLabelInternalMenuPlugin * lableplugin= new FileLabelInternalMenuPlugin(nullptr);
                if(id == lableplugin->name()){
                    auto plugin = MenuPluginManager::getInstance()->getPlugin(id);
                    auto actions = plugin->menuActions(MenuPluginInterface::DirectoryView, directoryUri, m_realselectUris);
                    for (auto action : actions) {
                        action->setObjectName(FILE_LABEL_ACTION);
                        l.append(action);
                    }
                    l.append(separator5);
                }
                lableplugin->deleteLater();
             }
            l.append(separator3);
            l.append(moveToSpaceAction);
            l.append(removeFromSpaceAction);
            l.append(separator4);
            if(showAiassistant){
                l.append(assistantActions);
            }
            l.append(separator1);
            auto pluginIdsForFile = MenuPluginManager::getInstance()->getPluginIds();
            std::sort(pluginIdsForFile.begin(), pluginIdsForFile.end());
            QStringList realSelectUrisForPlugin;
            for (auto uri : m_realselectUris) {
                realSelectUrisForPlugin.append(getRealUriFromUri(uri));
            }
            auto sendToAction = new QAction(QIcon::fromTheme("send-to-symbolic"),tr("send to"), this);
            QMenu *subMenu = new QMenu();
            sendToAction->setMenu(subMenu);
            l<<sendToAction;
            for (auto id : pluginIdsForFile) {
                auto plugin = MenuPluginManager::getInstance()->getPlugin(id);
                if (plugin->name() == "Peony-Qt Create Link Extension") {
                    auto actions = plugin->menuActions(MenuPluginInterface::DirectoryView, directoryUri, realSelectUrisForPlugin);
                    for (auto action : actions) {
                        action->setParent(this);
                        action->setObjectName(plugin->name());
                        if (action->icon().isNull()) {
                            auto pluginicon = plugin->icon();
                            if (!pluginicon.isNull()) {
                                action->setIcon(pluginicon);
                            }
                        }
                    }
                    subMenu->addActions(actions);
                }
            }
            l.append(separator1);
            l.append(propertiesAction);
        }else if(isRootDirectory && selectUris.isEmpty()){
            // 智能空间根目录未选中文件时，显示通用菜单项
            // Open in New Tab
            if (!qApp->property("tabletMode").toBool()) {
                auto openInNewTabAction = new QAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open in New Tab"), this);
                openInNewTabAction->setObjectName("open-in-new-tab-action");
                openInNewTabAction->setShortcut(QKeySequence("Ctrl+t"));
                connect(openInNewTabAction, &QAction::triggered, this, [=]() {
                    if (window) {
                        QStringList uris;
                        uris << directoryUri;
                        window->addNewTabs(uris);
                    }
                });
                l.append(openInNewTabAction);
            }

            // Open in New Window
            auto openInNewWindowAction = new QAction(QIcon::fromTheme("window-new-symbolic"), tr("Open in New Window"), this);
            openInNewWindowAction->setObjectName("open-in-new-window-action");
            openInNewWindowAction->setShortcut(QKeySequence("Ctrl+n"));
            connect(openInNewWindowAction, &QAction::triggered, this, [=]() {
                if (window) {
                    auto windowIface = window->create(directoryUri);
                    auto newWindow = dynamic_cast<QWidget *>(windowIface);
                    newWindow->setAttribute(Qt::WA_DeleteOnClose);
                    newWindow->show();
                }
            });
            l.append(openInNewWindowAction);

            l.append(separator);

            // View Type
            auto viewTypeAction = new QAction(QIcon::fromTheme("view-grid-symbolic"), tr("View Type"), this);
            viewTypeAction->setObjectName("view-type-action");
            QMenu *viewTypeMenu = new QMenu();
            {
                Peony::ViewFactorySortFilterModel2 model;
                model.setDirectoryUri(directoryUri);
                auto viewNames = model.supportViewIds();
                // Filter out Space View from the list
                viewNames.removeAll("Space View");
                auto viewFactoryManager = Peony::DirectoryViewFactoryManager2::getInstance();
                for (auto viewId : viewNames) {
                    auto factory = viewFactoryManager->getFactory(viewId);
                    if (factory) {
                        auto viewAction = viewTypeMenu->addAction(factory->viewIcon(), factory->viewName());
                        viewAction->setData(viewId);
                        if (view && view->viewId() == viewId) {
                            viewAction->setCheckable(true);
                            viewAction->setChecked(true);
                        } else {
                            connect(viewAction, &QAction::triggered, this, [=]() {
                                window->beginSwitchView(viewId);
                            });
                        }
                    }
                }
            }
            viewTypeAction->setMenu(viewTypeMenu);
            l.append(viewTypeAction);

            // Sort By
            auto sortTypeAction = new QAction(QIcon::fromTheme("ukui-view-sort-letter-symbolic"), tr("Sort By"), this);
            sortTypeAction->setObjectName("sort-type-action");
            QMenu *sortTypeMenu = new QMenu();
            QList<QAction *> sortActions;

            QStringList sortNames;
            int sortType = view->getSortType();
            sortNames << tr("Descending Order") << tr("Ascending Order");
            if (sortType == 1) {
                sortNames.clear();
                sortNames << tr("Newest to oldest") << tr("Oldest to newest");
            } else if (sortType == 3) {
                sortNames.clear();
                sortNames << tr("Files from large to small") << tr("Files from small to large");
            }

            sortActions << sortTypeMenu->addAction(tr("File Name"));
            sortActions << sortTypeMenu->addAction(tr("Time Modified"));
            sortActions << sortTypeMenu->addAction(tr("File Type"));
            sortActions << sortTypeMenu->addAction(tr("File Size"));
            sortActions << sortTypeMenu->addSeparator();
            sortActions << sortTypeMenu->addAction(sortNames[0]);
            sortActions << sortTypeMenu->addAction(sortNames[1]);
            int sortOrder = view->getSortOrder();
            sortActions.at(sortOrder+5)->setCheckable(true);
            sortActions.at(sortOrder+5)->setChecked(true);
            if (view) {              
                if (sortType >= 0 && sortType < sortActions.count()) {
                    sortActions.at(sortType)->setCheckable(true);
                    sortActions.at(sortType)->setChecked(true);
                }
                for (int i = 0; i < sortActions.count(); i++) {
                    connect(sortActions.at(i), &QAction::triggered, this, [=]() {
                        if(i<4){
                            window->setCurrentSortColumn(i);
                        }else{
                            window->setCurrentSortOrder(Qt::SortOrder(i-5));
                        }
                    });
                }
            }
            sortTypeAction->setMenu(sortTypeMenu);
            l.append(sortTypeAction);

            // Sort Preferences
            auto sortPreferencesAction = new QAction(QIcon::fromTheme("ukui-view-sort-favorite-symbolic"), tr("Sort Preferences"), this);
            sortPreferencesAction->setObjectName("sort-preferences-action");
            QMenu *sortPreferencesMenu = new QMenu();
            auto folderFirst = sortPreferencesMenu->addAction(tr("Folder First"));
            folderFirst->setCheckable(true);
            if (window) {
                folderFirst->setChecked(window->getWindowSortFolderFirst());
            }
            connect(folderFirst, &QAction::triggered, this, [=](bool checked) {
                if (window) {
                    window->setSortFolderFirst(checked);
                }
            });

            // Chinese First (only for Chinese locale)
            if (QLocale::system().name().contains("zh")) {
                auto useDefaultNameSortOrder = sortPreferencesMenu->addAction(tr("Chinese First"));
                useDefaultNameSortOrder->setCheckable(true);
                if (window) {
                    useDefaultNameSortOrder->setChecked(window->getWindowUseDefaultNameSortOrder());
                }
                connect(useDefaultNameSortOrder, &QAction::triggered, this, [=](bool checked) {
                    if (window) {
                        window->setUseDefaultNameSortOrder(checked);
                    }
                });
            }

            auto showHidden = sortPreferencesMenu->addAction(tr("Show Hidden"));
            showHidden->setCheckable(true);
            if (window) {
                showHidden->setChecked(window->getWindowShowHidden());
            }
            connect(showHidden, &QAction::triggered, this, [=](bool checked) {
                if (window) {
                    window->setShowHidden(checked);
                }
            });

            sortPreferencesAction->setMenu(sortPreferencesMenu);
            l.append(sortPreferencesAction);

            // New (submenu)

            // Refresh
            auto refreshAction = new QAction(QIcon::fromTheme("view-refresh-symbolic"), tr("Refresh"), this);
            refreshAction->setObjectName("refresh-action");
            refreshAction->setShortcut(QKeySequence::Refresh);
            connect(refreshAction, &QAction::triggered, this, [=]() {
                if (window) {
                    window->refresh();
                }
            });
            l.append(refreshAction);
            l.append(separator8);
            auto newAction = new QAction(QIcon::fromTheme("new-symbolic"), tr("New"), this);
            newAction->setObjectName("new-action");
            QMenu *newMenu = new QMenu();
            auto createIntelligentSpaceAction = new QAction(QIcon::fromTheme("folder-space-symbolic"), tr("Intelligent Space"), this);
            createIntelligentSpaceAction->setObjectName("create-intelligent-space-action");
            connect(createIntelligentSpaceAction, &QAction::triggered, this, [=]() {
                // Show create intelligent space dialog directly
                IntelligentSpaceCreateDialog d(nullptr);
                d.exec();
            });
            newMenu->addAction(createIntelligentSpaceAction);
            newAction->setMenu(newMenu);
            l.append(newAction);
        }
        break;
    }
    default:
        break;
    }

    return l;
}

IDMActionsManager::IDMActionsManager(QObject *parent) : QObject(parent)
{

}

const QStringList IDMActionsManager::getSelections(DirectoryViewWidget *view, SideBarAbstractItem *item)
{
    if (view)
         return view->getSelections();
    if (item)
        return QStringList() << item->uri();
    return {};
}

const QString IDMActionsManager::getDirectoryUri(DirectoryViewWidget *view, SideBarAbstractItem *item)
{
    if (view)
        return view->getDirectoryUri();
    if (item)
        return item->uri();
    return {};
}

const QString IDMActionsManager::getRealUriFromUri(const QString &uri)
{
    QString realUri = "";
    if (!uri.isEmpty()) {
        QString tempUri = uri.split('/', QString::SkipEmptyParts).last();
        realUri = "file://" + QUrl::fromPercentEncoding(tempUri.toLatin1());
    }
    return realUri;
}

QString truncateToByteLimit(const QString &text, int maxBytes) {
    QByteArray utf8Data = text.toUtf8();
    if (utf8Data.size() <= maxBytes) {
       return text; // 如果字节数未超限，返回原文本
    }
    // 从最大字节数位置向前查找安全的截断点
    int truncatePos = maxBytes;
    // 向前查找，直到找到一个安全的截断点（不是多字节字符的中间字节）
    while (truncatePos > 0 && (utf8Data.at(truncatePos) & 0xC0) == 0x80) {
       truncatePos--;
    }
    // 如果truncatePos为0，说明第一个字节就是多字节字符的一部分，返回空字符串
    if (truncatePos == 0) {
       return "";
    }
    // 截取安全的字节部分并转换回QString
    QByteArray safeData = utf8Data.left(truncatePos);
    return QString::fromUtf8(safeData);
}
