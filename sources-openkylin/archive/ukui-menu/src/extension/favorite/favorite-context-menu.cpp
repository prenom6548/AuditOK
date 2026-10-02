/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "favorite-context-menu.h"
#include "app-favorite-model.h"
#include "event-track.h"
#include "basic-app-model.h"
#include "favorite-folder-helper.h"

namespace UkuiMenu {
// GCOVR_EXCL_BR_START

int FavoriteContextMenu::index() const
{
    return 512;
}

QList<QAction *>
FavoriteContextMenu::actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location,
                             const QString &locationId)
{
    if (!parent) {
        return {};
    }

    QList<QAction *> list;

    if (data.type() == DataType::Folder) {
        list << new QAction(QIcon::fromTheme("app-group-disband-symbolic"), QObject::tr("Dissolve folder"), parent);
        QObject::connect(list.last(), &QAction::triggered, parent, [data] {
            FavoriteFolderHelper::instance()->deleteFolder(data.id().toInt());
        });
        return list;
    }

    switch (location) {
        case MenuInfo::AppList:
        case MenuInfo::FullScreen:
        case MenuInfo::FolderPage: {
            if (data.favorite() == 0) {
                list << new QAction(QIcon::fromTheme("non-starred-symbolic"), QObject::tr("Add to favorite"), parent);
                QObject::connect(list.last(), &QAction::triggered, parent, [data] {
                    AppFavoritesModel::instance().addAppToFavorites(data.id());
                    //BasicAppModel::instance()->databaseInterface()->fixAppToFavorite(data.id(), 1);
                    EventTrack::instance()->sendDefaultEvent("fix_to_favorite", "Right-click Menu");
                });

            } else if (data.favorite() > 0) {
                list << new QAction(QIcon::fromTheme("ukui-cancel-star-symbolic"), QObject::tr("Remove from favorite"), parent);
                QObject::connect(list.last(), &QAction::triggered, parent, [data] {
                    AppFavoritesModel::instance().removeAppFromFavorites(data.id());
                    BasicAppModel::instance()->databaseInterface()->fixAppToFavorite(data.id(), 0);
                    EventTrack::instance()->sendDefaultEvent("remove_from_favorite", "Right-click Menu");
                });
            }
            break;
        }
        case MenuInfo::Extension: {
            appendActionsForAppsOutsideFolder(parent, data.id(), list);
            QAction *action = new QAction(QIcon::fromTheme("ukui-cancel-star-symbolic"), QObject::tr("Remove from favorite"), parent);
            QObject::connect(action, &QAction::triggered, parent, [data] {
                AppFavoritesModel::instance().removeAppFromFavorites(data.id());
                EventTrack::instance()->sendDefaultEvent("remove_from_favorite", "Right-click Menu");
            });
            list << action;

            break;
        }
        case MenuInfo::Folder: {
            appendActionsForAppsInFolder(parent, data.id(), list);
            break;
        }
        default:
            break;
    }

    return list;
}

void FavoriteContextMenu::appendActionsForAppsInFolder(QObject *parent, const QString &appId, QList<QAction *> &list)
{
    FavoritesFolder folder;

    list << new QAction(QIcon::fromTheme("app-group-moveout-symbolic"), QObject::tr("Remove from folder"), parent);
    QObject::connect(list.last(), &QAction::triggered, parent, [appId] {
        FavoriteFolderHelper::instance()->removeAppFromFolder(appId);
    });
}

void FavoriteContextMenu::appendActionsForAppsOutsideFolder(QObject *parent, const QString &appId, QList<QAction *> &list)
{
    QAction *action = new QAction(QIcon::fromTheme("app-group-movein-symbolic"), QObject::tr("Move to folder"), parent);

    QMenu *subMenu = new QMenu();
    QAction *subAction = new QAction(QObject::tr("Create a new folder"), parent);
    QObject::connect(subAction, &QAction::triggered, parent, [appId] {
        AppFavoritesModel::instance().addAppToFolder(appId, "");
    });
    subMenu->addAction(subAction);

    for (auto folder : FavoriteFolderHelper::instance()->folderData()) {
        QString folderName = folder.getName();
        int folderId = folder.getId();
        QAction *subAction = new QAction(QObject::tr("Add to \"%1\"").arg(folderName), parent);
        QObject::connect(subAction, &QAction::triggered, parent, [appId, folderId] {
            AppFavoritesModel::instance().addAppToFolder(appId, QString::number(folderId));
        });
        subMenu->addAction(subAction);
    }

    action->setMenu(subMenu);
    list << action;
}

// GCOVR_EXCL_BR_STOP
} // UkuiMenu
