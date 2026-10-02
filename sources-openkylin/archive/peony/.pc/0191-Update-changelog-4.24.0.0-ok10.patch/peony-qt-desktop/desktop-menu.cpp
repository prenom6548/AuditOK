/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "desktop-menu.h"

#include "file-delete-operation.h"
#include "sound-effect.h"
#include "directory-view-plugin-iface.h"
#include "file-info-job.h"
#include "file-info.h"
#include "file-utils.h"
#include "file-launch-action.h"
#include "file-launch-manager.h"
#include "file-lauch-dialog.h"

#include "clipboard-utils.h"
#include "file-operation-utils.h"
#include "file-enumerator.h"

#include "desktop-menu-plugin-manager.h"
#include "manager-control.h"
#include "menu-plugin-manager.h"

#include "global-settings.h"
#include "audio-play-manager.h"
#include "image-mount-manager.h"
#include "manager-control.h"

#include <QProcess>
#include <QStandardPaths>
#include <QMessageBox>
#include <QtConcurrent>
#include <algorithm>
#include <gio/gdesktopappinfo.h>
#include <file-launch-manager.h>
#include "desktop-window-manager.h"
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include"directory-view-menu.h"
#include "file-action-controler.h"
#include "file-meta-info.h"
#include "desktop-widgets-manager.h"
#include "widget-manager-dialog.h"
#include "platformwindow.h"
using namespace Peony;

DesktopMenu::DesktopMenu(DirectoryViewIface *view, QWidget *parent) : QMenu(parent)
{
    //comment old highlight effect
//    setProperty("useIconHighlightEffect", true);
//    setProperty("iconHighlightEffectMode", 1);
    setProperty("fillIconSymbolicColor", true);

    m_view = view;
    m_directory = view->getDirectoryUri();
    m_selections = view->getSelections();

    fillActions();
}

DesktopMenu::DesktopMenu(QString currentDirectory, QStringList selections, QWidget *parent) : QMenu(parent)
{
    //comment old highlight effect
    setProperty("fillIconSymbolicColor", true);

    m_directory = currentDirectory;
    m_selections = selections;
    connect(this, &DesktopMenu::setSortType,this,[=](int i){
       Q_EMIT DesktopWindowManager::getInstance()->setSortType(i);
    });
    connect(this,&DesktopMenu::setSortOrder,this,[=](int i){
       Q_EMIT DesktopWindowManager::getInstance()->setSortOrder(i);
    });

    connect(this,&DesktopMenu::markFilePos, DesktopWindowManager::getInstance(), &DesktopWindowManager::markCreateFilePos);
    fillActions();
}

QAction *DesktopMenu::exec(const QPoint &pos)
{
    m_execPos = pos;
    return QMenu::exec(pos);
}

void DesktopMenu::fillActions()
{
    //add open actions
    auto openActions = constructOpenOpActions();
    if (!openActions.isEmpty())
        addSeparator();

    //add view actions
    auto viewActions = constructViewOpActions();
    if (!viewActions.isEmpty())
        addSeparator();

    auto createTemplateActions = constructCreateTemplateActions();
    if (!createTemplateActions.isEmpty())
        addSeparator();

    //add operation actions
    auto fileOpActions = constructFileOpActions();
    if (!fileOpActions.isEmpty())
        addSeparator();

    //add propertries actions
    //if this is a background menu, then it should not add propertries actions
    bool isBackgroundMenu = m_selections.isEmpty();
    if(isBackgroundMenu) {
        auto widgetMgmtActions = constructWidgetManagementActions();
        if (!widgetMgmtActions.isEmpty())
            addSeparator();
        constructBackgroundMenuActions();
        addSeparator();
    }
    //add plugin actions
    auto pluginActions = constructMenuPluginActions();
    if (!pluginActions.isEmpty())
        addSeparator();
    constructFilePropertiesActions();
}

const QList<QAction *> DesktopMenu::constructOpenOpActions()
{
    QList<QAction *> l;

    bool isBackgroundMenu = m_selections.isEmpty();
    bool inpartition = FileActionController::getInstance()->getIsOperationByPartition();
    if(inpartition){
        FileActionController::getInstance()->setIsOperationByPartition(false);
    }
    if (isBackgroundMenu) {
        l<<addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open Desktop in Window"));
        connect(l.last(), &QAction::triggered, this, [=]() {
            this->openWindow(m_directory);
        });
    } else {
        auto fileinfo = FileInfo::fromUri(m_selections.first());
        auto mimeType = fileinfo->mimeType();
        QIcon  appicon;
        
        // 对于镜像文件，使用文件管理器图标，因为实际操作是挂载镜像而非用归档管理器打开
        if (IMAGE_MOUNT_MANAGER->isImageFile(m_selections.first())) {
            appicon = QIcon::fromTheme("system-file-manager");
        } else {
            GAppInfo *m_app_info = nullptr;
            if(fileinfo->isDesktopFile()){
                GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(fileinfo->filePath().toUtf8().constData());
                m_app_info  = G_APP_INFO(appInfo);
            }else{
                GAppInfo *appinfo = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
                auto app_info = G_APP_INFO(appinfo);
                m_app_info = static_cast<GAppInfo*>(g_object_ref(app_info));
            }
            GIcon *icon = g_app_info_get_icon(m_app_info);
            auto iconName = FileUtils::getIconStringFromGIcon(icon);
            if (iconName.startsWith("/")) {
                appicon.addFile(iconName);
            } else {
                appicon = QIcon::fromTheme(iconName);
            }
            appicon = appicon.isNull() ? QIcon::fromTheme("document-open-symbolic") : appicon;
        }
        if (m_selections.count() == 1
                && !m_selections.first().startsWith("trash://")
                && !m_selections.first().startsWith("computer://")
                && (m_selections.first() != ("file://" + QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()))) {
            auto info = FileInfo::fromUri(m_selections.first());
            auto displayName = info->displayName();
            //FIXME: replace BLOCKING api in ui thread.
            if (displayName.isEmpty())
                displayName = FileUtils::getFileDisplayName(info->uri());
            //when name is too long, show elideText
            //qDebug() << "charWidth:" <<charWidth;
            if (displayName.length() > ELIDE_TEXT_LENGTH)
            {
                int  charWidth = fontMetrics().averageCharWidth();
                displayName = fontMetrics().elidedText(displayName, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
            }
            if (info->isDir()) {
                l<<addAction(appicon, tr("Open"));
                l.last()->setShortcut(QKeySequence(Qt::Key_Enter));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    //this->openWindow(m_selections);
                    FileLaunchManager::openAsync(m_selections.first());
                });

                auto recommendActions = FileLaunchManager::getRecommendActions(m_selections.first());
                if (recommendActions.count() > 1)
                {
                    // fix: move default app to the first position in "Open with..." menu
                    auto defaultAction = FileLaunchManager::getDefaultAction(m_selections.first());
                    if (defaultAction) {
                        QString defaultAppName = defaultAction->getAppInfoName();
                        if (!defaultAppName.isEmpty()) {
                            for (int i = 1; i < recommendActions.count(); i++) {
                                if (recommendActions[i]->getAppInfoName() == defaultAppName) {
                                    recommendActions.move(i, 0);
                                    break;
                                }
                            }
                        }
                        defaultAction->deleteLater();
                    }

                    auto openWithAction = addAction(QIcon::fromTheme("openwith-symbolic"),tr("Open with..."));
                    QMenu *openWithMenu = new QMenu(this);
                    for (auto action : recommendActions) {
                        action->setParent(openWithMenu);
                        openWithMenu->addAction(static_cast<QAction*>(action));
                    }
                    auto fallbackActions = FileLaunchManager::getFallbackActions(m_selections.first());
                    for (auto action : fallbackActions) {
                        action->setParent(openWithMenu);
                        openWithMenu->addAction(static_cast<QAction*>(action));
                    }
                    openWithMenu->addSeparator();
                    openWithMenu->addAction(tr("More applications..."), this, [=]() {
                        FileLauchDialog d(m_selections.first());
                        d.exec();
                    });
                    openWithAction->setMenu(openWithMenu);
                }
            } else if (!info->isVolume()) {
                l<<addAction(appicon, tr("Open"));
                l.last()->setShortcut(QKeySequence(Qt::Key_Enter));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto uri = m_selections.first();
                    if (IMAGE_MOUNT_MANAGER->isImageFile(uri)) {
                        IMAGE_MOUNT_MANAGER->mountImage(uri);
                        return;
                    }
                    FileLaunchManager::openAsync(uri);
                });
                bool containsWhitelist = MANAGER_CONTROL->containsWhitelistApp(m_selections);
                //FIXME: add sub menu for open with action.
                QMenu *openWithMenu = new QMenu(this);
                auto recommendActions = FileLaunchManager::getRecommendActions(m_selections.first());
                auto fallbackActions = FileLaunchManager::getFallbackActions(m_selections.first());
                //fix has default open app but no recommend actions issue, link to bug#61365
                //fix open options has two same app issue, linkto bug#74480, 69977
                if (recommendActions.count() == 0 && fallbackActions.count() ==0)
                {
                    auto action = FileLaunchManager::getDefaultAction(m_selections.first());
                    if (action != NULL && action->getAppInfoDisplayName().length() > 0)
                        recommendActions.append(action);
                }

                // fix: move default app to the first position in "Open with..." menu
                if (recommendActions.count() > 1) {
                    auto defaultAction = FileLaunchManager::getDefaultAction(m_selections.first());
                    if (defaultAction) {
                        QString defaultAppName = defaultAction->getAppInfoName();
                        if (!defaultAppName.isEmpty()) {
                            for (int i = 1; i < recommendActions.count(); i++) {
                                if (recommendActions[i]->getAppInfoName() == defaultAppName) {
                                    recommendActions.move(i, 0);
                                    break;
                                }
                            }
                        }
                        defaultAction->deleteLater();
                    }
                }

                if (IMAGE_MOUNT_MANAGER->isImageFile(m_selections.first())) {
                    QAction *fileManagerAction = new QAction(QIcon::fromTheme("system-file-manager"), tr("File Manager"), openWithMenu);
                    connect(fileManagerAction, &QAction::triggered, this, [=]() {
                        IMAGE_MOUNT_MANAGER->mountImage(m_selections.first());
                    });
                    openWithMenu->addAction(fileManagerAction);
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
                openWithMenu->addAction(tr("More applications..."), this, [=]() {
                    // FileLauchDialog d(m_selections.first());
                    // d.exec();
                    FileLauchDialog* d = new FileLauchDialog(m_selections.first());
                    d->setAttribute(Qt::WA_DeleteOnClose);
                    d->show();
                });
                if (!containsWhitelist) {
                    auto openWithAction = addAction(QIcon::fromTheme("openwith-symbolic"),tr("Open with..."));
                    openWithAction->setMenu(openWithMenu);
                }
            } else {
                l<<addAction(tr("Open"));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto uri = m_selections.first();
                    //FIXME:
                });
            }
        } else {
            if (m_selections.count() == 1)
            {
                auto info = FileInfo::fromUri(m_selections.first());
                auto displayName = info->displayName();
                l<<addAction(appicon, tr("Open"));
                l.last()->setShortcut(QKeySequence(Qt::Key_Enter));
            }
            else {
                bool containsWhitelist = MANAGER_CONTROL->containsWhitelistApp(m_selections);
                if (!containsWhitelist) {
                    l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open %1 selected files").arg(m_selections.count()));
                }
            }
            connect(l.last(), &QAction::triggered, this, [=]() {
                if (m_selections.count() == 1) {
                    FileLaunchManager::openAsync(m_selections.first());
                    return;
                }
                qDebug()<<"triggered:"<<m_selections.count();
                QStringList dirs;
                QStringList files;
                for (auto uri : m_selections) {
                    auto info = FileInfo::fromUri(uri);
                    if (info->isDir() || info->isVolume()) {
                        dirs<<uri;
                    } else {
                        files<<uri;
                    }
                }
                if (!dirs.isEmpty())
                    this->openWindow(dirs);

                if (!files.isEmpty()) {
                    Peony::FileLaunchManager::openFilesByDefaultApplications(files);
                }
            });
        }
        if(!inpartition){
            l<<addAction(QIcon::fromTheme("organize-symbolic"), tr("Store %1 selected files into a new partition").arg(m_selections.count()));
            connect(l.last(), &QAction::triggered, this, [=]() {
                DesktopOrganizationManager::instance()->CreatePartition(m_selections);
            });
        }
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructCreateTemplateActions()
{
    QList<QAction *> l;
    if (m_selections.isEmpty()) {
        auto createAction = new QAction(QIcon::fromTheme("new-symbolic"),tr("New"), this);
        l<<createAction;
        QMenu *subMenu = new QMenu(this);
        createAction->setMenu(subMenu);
        addAction(createAction);

        QList<QAction *> actions;
        auto createFolderActions = new QAction(QIcon::fromTheme("folder"), tr("Folder"), this);
        actions<<createFolderActions;
        connect(actions.last(), &QAction::triggered, this, [=]() {
            Q_EMIT markFilePos(m_execPos);
            //FileOperationUtils::create(m_directory, nullptr, CreateTemplateOperation::EmptyFolder);
            CreateTemplateOperation op(m_directory, CreateTemplateOperation::EmptyFolder, tr("New Folder"));
            op.run();
            auto targetUri = op.target();
            qDebug()<<"target:"<<targetUri;
            //clear old data,fix bug#164160, not enter edit new file issue
            m_uris_to_edit.clear();
            m_uris_to_edit<<targetUri;
        });
        auto createEmptyFileAction = new QAction(QIcon::fromTheme("text-plain"), tr("Text Document"), this);
        actions<<createEmptyFileAction;
        connect(actions.last(), &QAction::triggered, this, [=]() {
            Q_EMIT markFilePos(m_execPos);
            CreateTemplateOperation op(m_directory);
            op.run();
            auto targetUri = op.target();
            qDebug()<<"target:"<<targetUri;
            //clear old data,fix bug#164160, not enter edit new file issue
            m_uris_to_edit.clear();
            m_uris_to_edit<<targetUri;
        });
        subMenu->addActions(actions);

        //enumerate template dir
        QDir templateDir(g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES));
        QString templatePath = templateDir.path();
        qWarning()<<"tempalte Path is"<<templatePath;
        QStringList fileNames;
        if(!templatePath.isEmpty())
        {
            QDir templateDir(templatePath);
            auto templates = templateDir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
            if (!templates.isEmpty()) {
                for (auto t : templates) {
                    QFileInfo qinfo(templateDir, t);
                    qWarning()<<"template entry is"<<qinfo.filePath();
                    GFile *gtk_file = g_file_new_for_path(qinfo.filePath().toUtf8().constData());
                    char *uri_str = g_file_get_uri(gtk_file);
                    //FIXME: replace BLOCKING api in ui thread.
                    std::shared_ptr<FileInfo> info = FileInfo::fromUri(uri_str);
                    QIcon tmpIcon;
                    if (info->uri().endsWith(".desktop")) {
                        QUrl url = info->uri();
                        auto path = url.path();
                        auto key_file = g_key_file_new();
                        if (g_key_file_load_from_file(key_file, path.toUtf8().constData(), G_KEY_FILE_NONE, 0)) {
                            g_autofree gchar* icon_name = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, 0);
                            if (icon_name) {
                                QString iconName = icon_name;
                                tmpIcon = QIcon::fromTheme(iconName);
                            }
                            g_key_file_free(key_file);
                        }
                    } else {
                        QString iconName = FileUtils::updateFileIconName(info->uri(), true);
                        tmpIcon = QIcon::fromTheme(iconName);
                    }

//                    QString mimeType = info->mimeType();
//                    if (mimeType.isEmpty()) {
//                        FileInfoJob job(info);
//                        job.querySync();
//                        mimeType = info->mimeType();
//                    }

//                    QIcon tmpIcon;
//                    GList *app_infos = g_app_info_get_recommended_for_type(mimeType.toUtf8().constData());
//                    GList *l = app_infos;
//                    QList<FileLaunchAction *> actions;
//                    bool isOnlyUnref = false;
//                    while (l) {
//                        auto app_info = static_cast<GAppInfo*>(l->data);
//                        if (!isOnlyUnref) {
//                            GThemedIcon *icon = G_THEMED_ICON(g_app_info_get_icon(app_info));
//                            const char * const * icon_names = g_themed_icon_get_names(icon);
//                            if (icon_names)
//                                tmpIcon = QIcon::fromTheme(*icon_names);
//                            if(!tmpIcon.isNull())
//                                isOnlyUnref = true;
//                        }
//                        l = l->next;
//                    }
//                    g_list_free_full(app_infos, g_object_unref);

                    fileNames.append(qinfo.fileName());
                    QAction *action = new QAction(tmpIcon, qinfo.baseName(), this);
                    connect(action, &QAction::triggered, this, [=]() {
                        Q_EMIT markFilePos(m_execPos);
                        CreateTemplateOperation op(m_directory, CreateTemplateOperation::Template, t);
                        op.run();
                        auto target = op.target();
                        //clear old data,fix bug#164160, not enter edit new file issue
                        m_uris_to_edit.clear();
                        m_uris_to_edit<<target;
                    });
                    subMenu->addAction(action);
                    g_free(uri_str);
                    g_object_unref(gtk_file);
                }
                //subMenu->addSeparator();
            } else {
                qWarning()<<"template entries is empty";
            }
        } else {
            qWarning()<<"template path is empty";
        }

        templatePath = "/opt/kingsoft/wps-office/templates/";
        if(!templatePath.isEmpty())
        {
            QDir templateDir(templatePath);
            auto templates = templateDir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
            if (!templates.isEmpty()) {
                for (auto t : templates) {
                    QFileInfo qinfo(templateDir, t);
                    auto abspath = qinfo.absoluteFilePath();
                    if (fileNames.contains(qinfo.fileName())) {
                        continue;
                    }

                    qWarning()<<"template entry is"<<qinfo.filePath();
                    GFile *gtk_file = g_file_new_for_path(qinfo.filePath().toUtf8().constData());
                    char *uri_str = g_file_get_uri(gtk_file);
                    std::shared_ptr<FileInfo> info = FileInfo::fromUri(uri_str);

                    QIcon tmpIcon;
                    if (info->uri().endsWith(".desktop")) {
                        QUrl url = info->uri();
                        auto path = url.path();
                        auto key_file = g_key_file_new();
                        if (g_key_file_load_from_file(key_file, path.toUtf8().constData(), G_KEY_FILE_NONE, 0)) {
                            g_autofree gchar* icon_name = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, 0);
                            if (icon_name) {
                                QString iconName = icon_name;
                                tmpIcon = QIcon::fromTheme(iconName);
                            }
                            g_key_file_free(key_file);
                        }
                    } else {
                        QString iconName = FileUtils::updateFileIconName(info->uri(), true);
                        tmpIcon = QIcon::fromTheme(iconName);
                    }

                    QAction *action = new QAction(tmpIcon, qinfo.baseName(), this);
                    connect(action, &QAction::triggered, [=]() {
                        // automatically check for conficts
                        CreateTemplateOperation op(m_directory, CreateTemplateOperation::Template, abspath);
                        Peony::FileOperationErrorDialogWarning dlg;
                        connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
                        op.run();
                        auto target = op.target();
                        m_uris_to_edit<<target;
                    });
                    subMenu->addAction(action);
                    g_free(uri_str);
                    g_object_unref(gtk_file);
                }
//                subMenu->addSeparator();
            } else {
                qWarning()<<"template entries is empty";
            }
        } else {
            qWarning()<<"template path is empty";
        }
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructViewOpActions()
{
    QList<QAction *> l;

    if (m_selections.isEmpty()) {
        auto useAutoLayoutAction = addAction(QIcon::fromTheme("view-auto-sort-symbolic"),tr("Auto arrange"));
        useAutoLayoutAction->setCheckable(true);
        useAutoLayoutAction->setChecked(GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool());
        connect(useAutoLayoutAction, &QAction::triggered, [=](bool checked){
            useAutoLayoutAction->setChecked(checked);
            GlobalSettings::getInstance()->setGSettingValue(DESKTOP_USE_AUTO_LAYOUT, checked);
        });
        l<<useAutoLayoutAction;

        auto viewTypeAction = addAction(QIcon::fromTheme("view-grid-symbolic"),tr("View Type"));
        l<<viewTypeAction;
        QMenu *viewTypeSubMenu = new QMenu(this);

        auto zoomLevel = GlobalSettings::getInstance()->getValue(DEFAULT_DESKTOP_ZOOM_LEVEL).toInt();

        auto smallAction = viewTypeSubMenu->addAction(tr("Small"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(Small);
        });
        auto normalAction = viewTypeSubMenu->addAction(tr("Normal"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(Normal);
        });
        auto largeAction = viewTypeSubMenu->addAction(tr("Large"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(Large);
        });
        auto hugeAction = viewTypeSubMenu->addAction(tr("Huge"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(Huge);
        });

        switch (zoomLevel) {
        case Small:
            smallAction->setCheckable(true);
            smallAction->setChecked(true);
            break;
        case Normal:
            normalAction->setCheckable(true);
            normalAction->setChecked(true);
            break;
        case Large:
            largeAction->setCheckable(true);
            largeAction->setChecked(true);
            break;
        case Huge:
            hugeAction->setCheckable(true);
            hugeAction->setChecked(true);
            break;
        default:
            break;
        }

        viewTypeAction->setMenu(viewTypeSubMenu);

        //sort type
        auto sortTypeAction = addAction(QIcon::fromTheme("ukui-view-sort-letter-symbolic"),tr("Sort By"));
        l<<sortTypeAction;
        QMenu *sortTypeMenu = new QMenu(this);
        sortTypeMenu->setStyle(new MenuStyle(sortTypeMenu->style()));
        QList<QAction *> tmp;
        tmp<<sortTypeMenu->addAction(tr("Name"));
        tmp<<sortTypeMenu->addAction(tr("Time Modified"));
        tmp<<sortTypeMenu->addAction(tr("File Type"));
        tmp<<sortTypeMenu->addAction(tr("File Size"));

        int sortType = GlobalSettings::getInstance()->getValue(LAST_DESKTOP_SORT_ORDER).toInt();//m_view->getSortType();
        qDebug() << "sortType:" <<sortType <<tmp.count();
        if (sortType < 0 || sortType >= tmp.count()) {
            sortType = GlobalSettings::getInstance()->getValue(LAST_DESKTOP_SORT_ORDER).toInt();
        }
        tmp.at(sortType)->setCheckable(true);
        tmp.at(sortType)->setChecked(true);


        for (int i = 0; i < tmp.count(); i++) {
            connect(tmp.at(i), &QAction::triggered, this, [=]() {
                qDebug() << "setSortType in menu:" <<i;
                Q_EMIT setSortType(i);
                GlobalSettings::getInstance()->setValue(LAST_DESKTOP_SORT_ORDER, i);
            });
        }

        sortTypeAction->setMenu(sortTypeMenu);

        //sort order
//        auto sortOrderAction = addAction(tr("Sort Order"));
//        l<<sortOrderAction;
//        QMenu *sortOrderMenu = new QMenu(this);
        tmp.clear();
        sortTypeMenu->addSeparator();
        QStringList sortNames;
        sortNames.append(tr("Descending Order"));
        sortNames.append(tr("Ascending Order"));
        if (sortType == 1) {
            sortNames.clear();
            sortNames.append(tr("Newest to oldest"));
            sortNames.append(tr("Oldest to newest"));
        } else if (sortType == 3) {
            sortNames.clear();
            sortNames.append(tr("Files from large to small"));
            sortNames.append(tr("Files from small to large"));
        }


        sortTypeMenu->addSeparator();
        tmp<<sortTypeMenu->addAction(QIcon::fromTheme("view-sort-descending-symbolic"),sortNames.at(0));
        tmp<<sortTypeMenu->addAction(QIcon::fromTheme("view-sort-ascending-symbolic"),sortNames.at(1));
        int sortOrder = GlobalSettings::getInstance()->getValue(DESKTOP_SORT_ORDER).toInt();
        tmp.at(sortOrder)->setCheckable(true);
        tmp.at(sortOrder)->setChecked(true);
        for (int i = 0; i < tmp.count(); i++) {
            connect(tmp.at(i), &QAction::triggered, this, [=](){
                Q_EMIT setSortOrder(i);
                GlobalSettings::getInstance()->setValue(DESKTOP_SORT_ORDER, i);
            });
        }

        /*
        l<<addAction(QIcon::fromTheme("zoom-in-symbolic"), tr("Zoom &In"), this, [=](){
            auto desktopView = dynamic_cast<AdvancedDesktopIconView*>(m_view);
            desktopView->zoomIn();
        });
        l<<addAction(QIcon::fromTheme("zoom-out-symbolic"), tr("Zoom &Out"), this, [=](){
            auto desktopView = dynamic_cast<AdvancedDesktopIconView*>(m_view);
            desktopView->zoomOut();
        });
        */
        auto useOriganizationAction = addAction(QIcon::fromTheme("organize-symbolic"), tr("Desktop Origanization"));
        useOriganizationAction->setCheckable(true);
        useOriganizationAction->setChecked(GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool());
        connect(useOriganizationAction, &QAction::triggered, [=](bool checked){
            useOriganizationAction->setChecked(checked);
            GlobalSettings::getInstance()->setGSettingValue(DESKTOP_USE_ORIGANIZATION, checked);
            if(checked){
                Q_EMIT FileActionController::getInstance()->desktopOriganization();
            }else{
                Q_EMIT FileActionController::getInstance()->quitOriganization();
            }
        });
        l<<useOriganizationAction;
        auto newPartitionAction = addAction(QIcon::fromTheme("organize-symbolic"), tr("Create New Partiton"));
        connect(newPartitionAction, &QAction::triggered, [=](){
             DesktopOrganizationManager::instance()->CreatePartition(QStringList());
        });
        l<<useOriganizationAction;

        l<<addAction(QIcon::fromTheme("view-refresh-symbolic"), tr("Refresh"));
        connect(l.last(), &QAction::triggered, this, [=]() {
            Q_EMIT refresh();
        });
        l.last()->setShortcut(QKeySequence::Refresh);
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructFileOpActions()
{
    QList<QAction *> l;

    if (!m_selections.isEmpty()) {
        //don't show file operations when select home path
        QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        if (m_selections.count() == 1 && m_selections.first() == "trash:///") {
            l<<addAction(QIcon::fromTheme("edit-select-invert-symbolic"),tr("Reverse Select"));
            connect(l.last(), &QAction::triggered, this, [=]() {
                //qDebug() << "Reverse select";
                Q_EMIT invertSelections();
            });

            addSeparator();

            FileEnumerator e;
            e.setEnumerateDirectory("trash:///");
            e.enumerateSync();
            auto trashChildren = e.getChildrenUris();
            //comment to avoid uncorrect operation in desktop, only in trash can do it
//            l<<addAction(QIcon::fromTheme("view-refresh-symbolic"), tr("&Restore all"), [=]() {
//                FileOperationUtils::restore(trashChildren);
//            });
//            l.last()->setEnabled(!trashChildren.isEmpty());
            l<<addAction(QIcon::fromTheme("user-trash-symbolic"), tr("Clean the trash"), this, [=]() {
                auto removeop = Peony::FileOperationUtils::clearRecycleBinWithDialog(trashChildren);
                qApp->setProperty("clearTrash",true);
//                if(removeop){
//                    removeop->connect(removeop,&Peony::FileDeleteOperation::operationFinished, [=](){
//                        Peony::SoundEffect::getInstance()->recycleBinClearMusic();
//                    });
//                }
//                Peony::AudioPlayManager::getInstance()->playWarningAudio();
//                auto result = QMessageBox::question(nullptr, tr("Delete Permanently"), tr("Are you sure that you want to delete these files? "
//                                                    "Once you start a deletion, the files deleting will never be "
//                                                    "restored again."));
//                if (result == QMessageBox::Yes) {
//                    FileEnumerator e;
//                    FileOperationUtils::remove(trashChildren);
//                }
            });
            l.last()->setEnabled(!trashChildren.isEmpty());
        } else if (m_selections.count() == 1 && (m_selections.first() == "computer:///" || m_selections.first() == homeUri)) {
            l<<addAction(QIcon::fromTheme("edit-select-invert-symbolic"),tr("Reverse Select"));
            connect(l.last(), &QAction::triggered, this, [=]() {
                //qDebug() << "Reverse select";
                Q_EMIT invertSelections();
            });
        } else if (! m_selections.contains(homeUri)) {
            l<<addAction(QIcon::fromTheme("edit-cut-symbolic"), tr("Cut"));
            l.last()->setShortcut(QKeySequence::Cut);
            connect(l.last(), &QAction::triggered, this, [=]() {
                ClipboardUtils::setClipboardFiles(m_selections, true);
                Q_EMIT changeOpacity(0.5);
            });
            l<<addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"));
            l.last()->setShortcut(QKeySequence::Copy);
            connect(l.last(), &QAction::triggered, this, [=]() {
                ClipboardUtils::setClipboardFiles(m_selections, false);
                //auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
                //desktopView->viewport()->update();
            });
            if (m_selections.count() == 1){
                l<<addAction(QIcon::fromTheme("rename-symbolic"), tr("Rename"));
                l.last()->setShortcut(QKeySequence(Qt::Key_F2));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    Q_EMIT editUri(m_selections.first());
                });
            } else if (m_selections.count() > 1) {
                l<<addAction(QIcon::fromTheme("rename-symbolic"), tr("Rename"));
                l.last()->setShortcut(QKeySequence(Qt::Key_F2));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    Q_EMIT editUris(m_selections);
                });
            }
            if (!m_selections.contains("trash:///")) {
                bool canTrash = true;
                bool canDelete = true;
                bool containsWhitelist = MANAGER_CONTROL->containsWhitelistApp(m_selections);
                for (auto uri : m_selections) {
                    auto info = FileInfo::fromUri(uri);
                    if (! info->canTrash())
                        canTrash = false;

                    if (! info->canDelete())
                        canDelete = false;
                }

                if (canTrash && !MANAGER_CONTROL->checkDeleteToTrashPermission()) {
                    canTrash = false;
                }

                if (canTrash && !containsWhitelist)
                {
                    l<<addAction(QIcon::fromTheme("user-trash-symbolic"), tr("Delete to trash"));
                    l.last()->setShortcut(QKeySequence::Delete);
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        FileOperationUtils::trash(m_selections, true);
                    });
                }
                else if(canDelete && !containsWhitelist)
                {
                    //comment delete forever right menu option,reference to mac and Windows
                    //add delete forever option
                    l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete forever"));
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        FileOperationUtils::executeRemoveActionWithDialog(m_selections);
                    });
                }
            }
            l<<addAction(QIcon::fromTheme("edit-select-invert-symbolic"),tr("Reverse Select"));
            connect(l.last(), &QAction::triggered, this, [=]() {
                //qDebug() << "Reverse select";
                Q_EMIT invertSelections();
            });
        } else {
            l<<addAction(QIcon::fromTheme("edit-select-invert-symbolic"),tr("Reverse Select"));
            connect(l.last(), &QAction::triggered, this, [=]() {
                //qDebug() << "Reverse select";
                Q_EMIT invertSelections();
            });
        }
    } else {
        auto pasteAction = addAction(QIcon::fromTheme("edit-paste-symbolic"), tr("Paste"));
        l<<pasteAction;
        l.last()->setShortcut(QKeySequence::Paste);
        pasteAction->setEnabled(ClipboardUtils::isClipboardHasFiles());
        connect(l.last(), &QAction::triggered, this, [=]() {
            Q_EMIT paste(ClipboardUtils::getClipboardFilesUris());
        });

        addActions(FileOperationManager::getInstance()->getUndoRedoActions());

        l<<addAction(QIcon::fromTheme("edit-select-all-symbolic"),tr("Select All"));
        l.last()->setShortcut(QKeySequence::SelectAll);
        connect(l.last(), &QAction::triggered, this, [=]() {
            //qDebug() << "select all";
            Q_EMIT invertSelections();
        });
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructFilePropertiesActions()
{
    QList<QAction *> l;

    l<<addAction(QIcon::fromTheme("properties-symbolic"), tr("Properties"));
    l.last()->setShortcut(QKeySequence("Alt+Enter"));
    connect(l.last(), &QAction::triggered, this, [=]() {
        //FIXME:
        if (m_selections.isEmpty()) {
            this->showProperties(m_directory);
        } else {
            this->showProperties(m_selections);
        }
    });

    return l;
}

void DesktopMenu::openWindow(const QString &uri)
{
    QUrl url = uri;
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/peony");
    p.setArguments(QStringList()<<"--show-folders"<<url.toEncoded());
    p.startDetached();
#else
    p.startDetached("/usr/bin/peony", QStringList()<<"--show-folders"<<uri);
#endif
}

const QList<QAction *> DesktopMenu::constructMenuPluginActions()
{
    QList<QAction *> l;
    //FIXME:
    // bool needFileLabel = true;
    // if (!m_selections.isEmpty()) {
    //     for (auto uri : m_selections) {
    //         if(uri == "trash:///" || uri == "computer:///"){
    //             needFileLabel = false;
    //             break;
    //         }
    //     }
    // }
    // if(needFileLabel){
    //     Peony::FileLabelInternalMenuPlugin *labelplugin = new Peony::FileLabelInternalMenuPlugin(nullptr);
    //     auto lableactions = labelplugin->menuActions(MenuPluginInterface::DirectoryView, m_directory, m_selections);
    //     addActions(lableactions);
    //     addSeparator();
    // }

    auto mgr = DesktopMenuPluginManager::getInstance();
    if (mgr->isLoaded()) {
        //add debug log to check open-terminal not show in first time issue
        qDebug() << "DesktopMenu PluginManager is loaded";
        // Sort plugins by their class name to ensure consistent ordering
        // across different languages, instead of using translated plugin names
        auto plugins = mgr->getPlugins();
        std::sort(plugins.begin(), plugins.end(), [](MenuPluginInterface *a, MenuPluginInterface *b) {
            QObject *objA = dynamic_cast<QObject*>(a);
            QObject *objB = dynamic_cast<QObject*>(b);
            QString nameA = objA ? objA->metaObject()->className() : "Unknown";
            QString nameB = objB ? objB->metaObject()->className() : "Unknown";
            return nameA < nameB;
        });

        for (auto plugin : plugins) {
            QObject *obj = dynamic_cast<QObject*>(plugin);
            if (obj) {
                qDebug() << "desktop plugin class:" << obj->metaObject()->className();
            }
            
            QString pluginId = plugin->name();
            QString pluginFileName = mgr->getPluginName(pluginId);
            
            // Check SMB share permission; if it's disabled, skip the share plugin
            if (!MANAGER_CONTROL->checkSmbSharePermission() && 
                pluginFileName.contains("libpeony-share-menu-plugin.so")) {
                qDebug() << "SMB share plugin disabled, skipping desktop plugin:" << pluginFileName;
                continue;
            }
            
            auto actions = plugin->menuActions(MenuPluginInterface::DesktopWindow,
                                               m_directory,
                                               m_selections);
            for (auto action : actions) {
                action->setParent(this);
                if(action->icon().isNull()){
                    auto pluginicon = plugin->icon();
                    if(!pluginicon.isNull()){
                        action->setIcon(pluginicon);
                    }
                }
            }
            l<<actions;
        }
    }
    addActions(l);

    auto isoActions = constructIsoMountActions();
    if (!isoActions.isEmpty()) {
        l << isoActions;
    }
    return l;
}

const QList<QAction *> DesktopMenu::constructIsoMountActions()
{
    QList<QAction *> l;

    if (m_selections.count() == 1) {
        QString uri = m_selections.first();
        if (!IMAGE_MOUNT_MANAGER->isImageFile(uri)) {
            return l;
        }

        auto mountAction = addAction(QIcon::fromTheme("drive-multidisk-symbolic"), tr("Mount"));
        l << mountAction;
        connect(mountAction, &QAction::triggered, this, [=]() {
            IMAGE_MOUNT_MANAGER->mountImage(uri);
        });
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructBackgroundMenuActions()
{
    QList<QAction *> l;
    QAction *action = addAction(QIcon::fromTheme("desktop-background-symbolic"),QObject::tr("Background Settings"));
    l << action;
    connect(action, &QAction::triggered, [=]() {
        //go to control center set background
        gotoSetBackground();
    });
    QAction *settingsAction = addAction(QIcon::fromTheme("display-settings-symbolic"),QObject::tr("Display Settings"));
    l << settingsAction;
    connect(settingsAction, &QAction::triggered, [=]() {
        //go to control center set resolution ratio
        gotoSetResolution();
    });

    return l;
}

void DesktopMenu::openWindow(const QStringList &uris)
{
    QStringList args;
    for (auto arg : uris) {
        QUrl url = arg;
        args<<QString(url.toEncoded());
    }
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/peony");
    p.setArguments(QStringList()<<"--show-folders"<<args);
    p.startDetached();
#else
    p.startDetached("/usr/bin/peony", QStringList()<<"--show-folders"<<args);
#endif
}

void DesktopMenu::showProperties(const QString &uri)
{
    //qDebug() << "showProperties uri:" <<uri;
    if (uri == "computer:///" || uri == "//")
    {
        gotoAboutComputer();
        return;
    }

    QUrl url = uri;
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/peony");
    p.setArguments(QStringList()<<"--show-properties"<<url.toEncoded());
    p.startDetached();
#else
    p.startDetached("/usr/bin/peony", QStringList()<<"--show-properties"<<url.toEncoded());
#endif
}

void DesktopMenu::showProperties(const QStringList &uris)
{
    QStringList args;
    for (auto arg : uris) {
        QUrl url = arg;
        args << url.toEncoded();
    }

    args.removeDuplicates();

    //注释掉这里以修复BUG#17809
    if (uris.contains("computer:///")) {
        args.removeAt(args.indexOf("computer:///"));
        QtConcurrent::run([=]() {
            gotoAboutComputer();
        });
    }

    if (uris.contains("trash:///") && uris.count() > 1) {
        args.removeAt(args.indexOf("trash:///"));
        QtConcurrent::run([=]() {
            QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.setProgram("/usr/bin/peony");
            p.setArguments(QStringList() << "--show-properties" << "trash:///");
            p.startDetached();
#else
            p.startDetached("/usr/bin/peony", QStringList()<<"--show-properties"<<args);
#endif
        });
    }

    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/peony");
    p.setArguments(QStringList() << "--show-properties" << args);
    p.startDetached();
#else
    p.startDetached("/usr/bin/peony", QStringList()<<"--show-properties"<<args);
#endif
}

void DesktopMenu::gotoAboutComputer()
{
    QProcess p;
    p.setProgram("/usr/bin/ukui-control-center");
    //-m About para to show about computer infos, related to bug#88258
    p.setArguments(QStringList()<<"-m" << "About");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/ukui-control-center", QStringList()<<"-m" << "About");
#endif
    p.waitForFinished(-1);
}

void DesktopMenu::gotoSetBackground()
{
    QProcess p;
    p.setProgram("/usr/bin/ukui-control-center");
    //old version use -b, new version use -m Wallpaper as para
    p.setArguments(QStringList()<<"-m" <<"Wallpaper");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/ukui-control-center", QStringList()<<"-m" <<"Wallpaper");
#endif
    p.waitForFinished(-1);
}

void DesktopMenu::gotoSetResolution()
{
    QProcess p;
    p.setProgram("/usr/bin/ukui-control-center");
    p.setArguments(QStringList()<<"-m" <<"display");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/ukui-control-center", QStringList()<<"-m" <<"display");
#endif
    p.waitForFinished(-1);

}

const QList<QAction *> DesktopMenu::constructWidgetManagementActions()
{
    QList<QAction *> l;

    FileActionController *ctrl = FileActionController::getInstance();
    const int focusedId = ctrl->focusedDesktopWidgetInstanceId();
    if (focusedId >= 0) {
        PlatformWindow *pw = nullptr;
        for (PlatformWindow *w : PlatformWindow::allWindows()) {
            if (w->id() == ctrl->clickedScreenId()) {
                pw = w;
                break;
            }
        }
        bool showRemove = true;
        if (pw && pw->mainView() && pw->mainView()->config()) {
            auto *wc = pw->mainView()->config()->child(QStringLiteral("widgets"), focusedId);
            if (wc && wc->getValue(QStringLiteral("id")).toString() == QStringLiteral("desktop-organization-tool")) {
                showRemove = false;
            }
        }
        if (showRemove) {
            QAction *removeWidgetAct = addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Remove Desktop Widget"));
            l << removeWidgetAct;
            connect(removeWidgetAct, &QAction::triggered, this, [ctrl]() {
                const int instanceId = ctrl->focusedDesktopWidgetInstanceId();
                if (instanceId < 0) {
                    return;
                }
                PlatformWindow *target = nullptr;
                for (PlatformWindow *w : PlatformWindow::allWindows()) {
                    if (w->id() == ctrl->clickedScreenId()) {
                        target = w;
                        break;
                    }
                }
                DesktopWidgetsManager::getInstance()->removeWidget(instanceId, target);
                ctrl->setFocusedDesktopWidgetInstanceId(-1);
                for (PlatformWindow *w : PlatformWindow::allWindows()) {
                    w->updateDesktopWidgetsUsedRegion();
                }
            });
        }
    }

    auto action = addAction(QIcon::fromTheme(QStringLiteral("list-add-symbolic")), tr("Add Plugin"));
    l << action;
    connect(action, &QAction::triggered, this, []() {
        auto dlg = WidgetManagerDialog::instance();
        dlg->show();
        dlg->raise();
        dlg->activateWindow();
    });

    return l;
}
