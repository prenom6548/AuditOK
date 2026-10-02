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

#include "app-menu-plugin.h"
#include "settings.h"
#include "app-manager.h"
#include "../context-menu-extension.h"
#include "basic-app-filter-model.h"
#include "user-config.h"
#include "recently-installed-model.h"
#include "security-function-control.h"

#include <QStringLiteral>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>
#include <QFileInfo>
#include <QProcess>

namespace UkuiMenu {

class AppContentMenu : public ContextMenuExtension
{
public:
    int index() const override;
    QList<QAction *> actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location,
                             const QString &locationId) override;

private:
    static void addToTop(QObject *parent, const QString &appId, const int &appTop, QList<QAction *> &list);
    static void addToPanelAction(QObject *parent, const QString &appId, QList<QAction *> &list);
    static void addToDesktopAction(QObject *parent, const QString &appId, QList<QAction *> &list);
    static void addUninstall(QObject *parent, const QString &appId, QList<QAction *> &list);
    static void addRemoveFromList(QObject *parent, const QString &appId, const bool &isRecentInstall, QList<QAction *> &list);
};

int AppContentMenu::index() const
{
    return ContextMenuExtension::index();
}

QList<QAction *> AppContentMenu::actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location,
                                         const QString &locationId)
{
    if (!parent || (data.type() != DataType::Normal)) {
        return {};
    }

    QList<QAction *> list;
    QAction* actionSeparator = nullptr;
    QString appId = data.id();
    int appTop = data.top();

    switch (location) {
        case MenuInfo::AppList: {
            //置顶
            if (locationId == "all") {
                addToTop(parent, appId, appTop, list);
            }
        }
        case MenuInfo::Extension:
        case MenuInfo::FolderPage:
        case MenuInfo::FullScreen:
        case MenuInfo::Folder:
            // 添加到任务栏
            addToPanelAction(parent, appId, list);
            //添加到桌面快捷方式
            addToDesktopAction(parent, appId, list);
            //添加分割线
            actionSeparator = new QAction;
            actionSeparator->setSeparator(true);
            list << actionSeparator;
            //添加从当前列表移除
            addRemoveFromList(parent, appId, RecentlyInstalledModel::isRecentInstall(data), list);
            // 卸载
            if (!SecurityFunctionControl::instance()->disableUninstallAction()
                    && !appId.isEmpty()
                    && !GlobalSetting::instance()->isSystemApp(appId)) {
                addUninstall(parent, appId, list);
            }
            break;
        default:
            break;
    }

    if (!actionSeparator) {
        actionSeparator->setVisible(list.last() != actionSeparator);
    }
    return list;
}

void AppContentMenu::addToTop(QObject *parent, const QString &appId, const int &appTop, QList<QAction *> &list)
{
    QString actionName = (appTop == 0) ? QObject::tr("Fixed to all applications") : QObject::tr("Unfixed from all applications");
    list << new QAction(actionName, parent);
    QObject::connect(list.last(), &QAction::triggered, parent, [appId, appTop] {
        BasicAppFilterModel::instance()->databaseInterface()->fixAppToTop(appId, appTop);
    });
}

void AppContentMenu::addToPanelAction(QObject *parent, const QString &appId, QList<QAction *> &list)
{
    QDBusInterface iface("org.ukui.taskManager", "/taskManager", "org.ukui.taskManager", QDBusConnection::sessionBus());

    if (!iface.isValid()) {
        qWarning() << "Ukui taskManager dbusinterface error";
        return;
    }

    iface.setTimeout(1000);
    QDBusReply<bool> isFixedOnTaskBar = iface.call("checkQuickLauncher", appId);

    if (!isFixedOnTaskBar.isValid()) {
        qWarning() << "Ukui taskManager dbusinterface call checkQuickLauncher timeout";
        return;
    }

    QString actionName = isFixedOnTaskBar ? QObject::tr("Remove from taskbar") : QObject::tr("Add to taskbar");
    QString functionName = isFixedOnTaskBar ? "removeQuickLauncher" : "addQuickLauncher";
    QIcon icon = isFixedOnTaskBar ? QIcon::fromTheme("ukui-unfixed-symbolic") : QIcon::fromTheme("view-pin-symbolic");
    list << new QAction(icon, actionName, parent);

    QObject::connect(list.last(), &QAction::triggered, parent, [appId, functionName] {
        QDBusInterface iface("org.ukui.taskManager", "/taskManager", "org.ukui.taskManager", QDBusConnection::sessionBus());

        if (!iface.isValid()) {
            qWarning() << "Ukui taskManager dbusinterface error";
            return;
        }

        iface.setTimeout(1);
        QDBusReply<bool> ret = iface.call(functionName, appId);

        if (!ret.isValid() || !ret) {
            qWarning() << "Add/Remove from taskbar error";
        }
    });
}

void AppContentMenu::addToDesktopAction(QObject *parent, const QString &appId, QList<QAction *> &list)
{
    list << new QAction(QIcon::fromTheme("emblem-link2-symbolic"), QObject::tr("Send to desktop shortcuts"), parent);

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString path = QString(desktopPath + "/" + QFileInfo(appId).fileName());

    if (QFile::exists(path)) {
        list.last()->setEnabled(false);
        return;
    }

    QObject::connect(list.last(), &QAction::triggered, parent, [appId, path] {
        if (QFile::copy(appId, path)) {
            // 设置权限755
            // QFile::Permissions permissions(0x0755); // 这也是可以的
            QFile::Permissions permissions = {
                QFile::ReadUser  | QFile::WriteUser | QFile::ExeUser |
                QFile::ReadGroup | QFile::ExeGroup  |
                QFile::ReadOther | QFile::ExeOther
            };

            if (!QFile::setPermissions(path, permissions)) {
                qWarning() << "set file permissions failed, file:" << path;
            }
        }
    });
}

void AppContentMenu::addUninstall(QObject *parent, const QString &appId, QList<QAction *> &list)
{
    list << new QAction(QIcon::fromTheme("user-trash-symbolic"), QObject::tr("Uninstall"), parent);
    QObject::connect(list.last(), &QAction::triggered, parent, [appId] {
        AppManager::instance()->launchBinaryApp("kylin-uninstaller", appId);
    });
}

void AppContentMenu::addRemoveFromList(QObject *parent, const QString &appId, const bool &isRecentInstall, QList<QAction *> &list)
{
    if (isRecentInstall) {
        list << new QAction(QIcon::fromTheme("edit-delete-symbolic"), QObject::tr("Clear recent installation mark"), parent);
        QObject::connect(list.last(), &QAction::triggered, parent, [appId] {
            BasicAppFilterModel::instance()->databaseInterface()->updateApLaunchedState(appId, true);
        });
    }
}

// ====== AppMenuPlugin ====== //
QString AppMenuPlugin::id()
{
    return QStringLiteral("app-menu");
}

WidgetExtension *AppMenuPlugin::createWidgetExtension()
{
    return nullptr;
}

ContextMenuExtension *AppMenuPlugin::createContextMenuExtension()
{
    return new AppContentMenu;
}

} // UkuiMenu
