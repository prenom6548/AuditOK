/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "task-manager-item.h"

#include <XdgDesktopFile>
#include <XdgIcon>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6xdg/XdgDesktopFile>
#include <qt6xdg/XdgIcon>
#else
#include <qt5xdg/XdgDesktopFile>
#include <qt5xdg/XdgIcon>
#endif

#include <QDebug>
#include <QMap>
#include <QScreen>
#include <QGuiApplication>
#include <QDBusPendingCallWatcher>
#include <QDBusConnection>
#include <QFile>
#include <QFileInfo>

#include "window-manager.h"
#include "ukui-task-manager.h"
#include "utils.h"
#include "widget-delegate.h"
#include "recent-files-service.h"

#ifdef signals
#undef signals
#endif
#define signals qt_signals
#include <gio/gio.h>

using namespace TaskManager;
using namespace UkuiQuick;

class TaskManager::TaskManagerItemPrivate
{
public:
    ~TaskManagerItemPrivate();
    QString m_ID;
    QString m_desktopFile;
    QString m_name;
    QString m_genericName;
    QString m_iconName;
    QIcon m_icon;
    QStringList m_winIDs;
    QStringList m_winIDsOnCurrentDesktop;
    QString m_groupLeader;
    QString m_group;
    bool m_hasLauncher = false;
    QString m_display;
    Actions m_actions;
    XdgDesktopFile m_xdf;
    uint m_unreadMessagesNum = 0;
    QStringList m_demandsAttentionWinIDs;
    QMap<QString, QRect> m_widToGeometry;
    QMap<QString, QVariant> m_widToTitle;
    QMap<QString, QVariant> m_widToIcon;
    QMap<QString, QVariant> m_widToIconName;
    bool m_hasActiveWindow = false;
    QString m_quickWidget;
    RecentFilesService *m_recentFilesService = nullptr;
    int m_recentFileActionCount = 0;
};

TaskManagerItemPrivate::~TaskManagerItemPrivate()
{
    qDeleteAll(m_actions);
}

TaskManagerItem::TaskManagerItem(QObject *parent) : QObject(parent), d(new TaskManagerItemPrivate)
{
}

TaskManagerItem::TaskManagerItem(const QString &desktoFile, QObject *parent): QObject(parent), d(new TaskManagerItemPrivate)
{
    d->m_desktopFile = desktoFile;
    QFileInfo info(d->m_desktopFile);
    auto fileName = info.fileName();
    if (fileName.endsWith(QStringLiteral(".desktop"))) {
        d->m_ID = fileName.left(fileName.size() - 8);
    }
    d->m_xdf.load(d->m_desktopFile);
}

TaskManagerItem::TaskManagerItem(const QStringList &winIDs, QObject *parent) : QObject(parent), d(new TaskManagerItemPrivate)
{
    d->m_winIDs = winIDs;
}

TaskManagerItem::TaskManagerItem(const QString &desktoFile, RecentFilesService *recentFilesService, QObject *parent)
    : TaskManagerItem(desktoFile, parent)
{
    d->m_recentFilesService = recentFilesService;
}

TaskManagerItem::TaskManagerItem(const QStringList &winIDs, RecentFilesService *recentFilesService, QObject *parent)
    : TaskManagerItem(winIDs, parent)
{
    d->m_recentFilesService = recentFilesService;
}

TaskManagerItem::~TaskManagerItem()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

const QString &TaskManagerItem::ID() const
{
    return d->m_ID;
}

void TaskManagerItem::setID(const QString &appID)
{
    d->m_ID = appID;
}

const QString& TaskManagerItem::desktopFile() const
{
    return d->m_desktopFile;
}

void TaskManagerItem::setDesktopFile(const QString&desktopFile)
{
    d->m_desktopFile = desktopFile;
    d->m_xdf.load(desktopFile);
}

bool TaskManagerItem::hasLauncher() const
{
    return d->m_hasLauncher;
}

void TaskManagerItem::setHasLauncher(bool hasLauncher)
{
    if(d->m_hasLauncher && hasLauncher) {
        return;
    }
    if(!hasLauncher && d->m_hasLauncher) {
        for(Action *action : d->m_actions) {
            if(action->type() == Action::Type::RemoveQuickLauncher) {
                action->setEnabled(false);
            }
            if(action->type() == Action::Type::AddQuickLauncher) {
                action->setEnabled(true);
            }
        }
    } else if(hasLauncher && !d->m_hasLauncher) {
        for(Action *action : d->m_actions) {
            if(action->type() == Action::Type::RemoveQuickLauncher) {
                action->setEnabled(true);
            }
            if(action->type() == Action::Type::AddQuickLauncher) {
                action->setEnabled(false);
            }
        }
    }
    d->m_hasLauncher = hasLauncher;
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::HasLauncher, UkuiTaskManager::Actions});
}

const QString &TaskManagerItem::display() const
{
    return d->m_display;
}

QString TaskManagerItem::quickWidget() const
{
    return d->m_quickWidget;
}

const QString &TaskManagerItem::name() const
{
    return d->m_name;
}

const QIcon &TaskManagerItem::icon() const
{
    return d->m_icon;
}

const QString &TaskManagerItem::genericName() const
{
    return d->m_genericName;
}

const QString &TaskManagerItem::iconName() const
{
    return d->m_iconName;
}

const QStringList &TaskManagerItem::winIDs() const
{
    return d->m_winIDs;
}

void TaskManagerItem::addWinID(const QString &winId)
{
    if(d->m_winIDs.contains(winId)) {
        return;
    }
    d->m_winIDs.append(winId);
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WinIdList});
    if(WindowManager::isOnCurrentDesktop(winId) || WindowManager::isOnAllDesktops(winId)) {
        d->m_winIDsOnCurrentDesktop.append(winId);
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::CurrentDesktopWinIdList, UkuiTaskManager::CurrentWinIdList});
    }
    geometryChanged(winId);
    QString title = WindowManager::windowTitle(winId);
    if(title.isEmpty()) {
        title = d->m_name;
    }
    d->m_widToTitle.insert(winId, title);
    QIcon icon = WindowManager::windowIcon(winId);
    if(icon.isNull()) {
        icon = d->m_icon;
    }
    d->m_widToIcon.insert(winId, icon.isNull() ? QVariant("application-x-desktop") : icon);
    d->m_widToIconName.insert(winId, icon.name());
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WindowTitles, UkuiTaskManager::WindowIcons, UkuiTaskManager::WindowIconsName});
}

void TaskManagerItem::removeWinID(const QString &winID)
{
    bool updateIconName = d->m_winIDs.isEmpty()? false : d->m_winIDs.first() == winID;
    if(!d->m_winIDs.isEmpty()) {
        d->m_winIDs.removeAll(winID);
        d->m_widToTitle.remove(winID);
        d->m_widToIcon.remove(winID);
        d->m_widToIconName.remove(winID);
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WinIdList, UkuiTaskManager::WindowTitles, UkuiTaskManager::WindowIcons, UkuiTaskManager::WindowIconsName});
        if(!hasLauncher() && d->m_xdf.name().isEmpty() && !d->m_winIDs.isEmpty() && updateIconName) {
            d->m_name = WindowManager::windowTitle(d->m_winIDs.first());
            d->m_icon = WindowManager::windowIcon(d->m_winIDs.first());
            Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::Name, UkuiTaskManager::Icon});
        }
    }

    if(d->m_winIDsOnCurrentDesktop.removeAll(winID)) {
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::CurrentDesktopWinIdList, UkuiTaskManager::CurrentWinIdList});
    }
    if(d->m_widToGeometry.remove(winID)) {
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WindowGeometries, UkuiTaskManager::CurrentWinIdList});
    }
    if(d->m_demandsAttentionWinIDs.removeAll(winID)) {
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::DemandsAttentionWinIdList});
    }
}

void TaskManagerItem::setHasActiveWindow(bool has)
{
    if(d->m_hasActiveWindow != has) {
        d->m_hasActiveWindow = has;
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::HasActiveWindow});
    }
}

bool TaskManagerItem::hasActiveWindow()
{
    return d->m_hasActiveWindow;
}

const QString &TaskManagerItem::group() const
{
    return d->m_group;
}

void TaskManagerItem::setGroup(const QString &group)
{
    d->m_group = group;
}

void TaskManagerItem::init()
{
    connect(WindowManager::self(), &WindowManager::currentDesktopChanged, this, &TaskManagerItem::refreshWinIdsOnCurrentDesktop);
    connect(WindowManager::self(), &WindowManager::onAllDesktopsChanged, this, &TaskManagerItem::desktopChanged);
    connect(WindowManager::self(), &WindowManager::windowDesktopChanged, this, &TaskManagerItem::desktopChanged);
    connect(WindowManager::self(), &WindowManager::titleChanged, this, &TaskManagerItem::titleChanged);
    connect(WindowManager::self(), &WindowManager::iconChanged, this, &TaskManagerItem::iconChanged);
    connect(WindowManager::self(), &WindowManager::demandsAttentionChanged, this, &TaskManagerItem::demandsAttentionChanged);
    connect(WindowManager::self(), &WindowManager::geometryChanged, this, &TaskManagerItem::geometryChanged);
    connect(WidgetInfo::self(), &WidgetInfo::widgetInfoUpdate, this, &TaskManagerItem::onWidgetInfoUpdate);

    d->m_quickWidget = WidgetInfo::self()->widgetFor(d->m_ID);
    d->m_name = d->m_xdf.localizedValue("Name").toString();

    d->m_iconName = d->m_xdf.iconName();
    if(d->m_iconName.startsWith("/")) {
        if(QFile::exists(d->m_iconName)) {
            d->m_icon = QIcon(d->m_iconName);
        }
    } else {
        d->m_icon = QIcon::fromTheme(d->m_iconName);
        if (d->m_icon.isNull()) {
            d->m_icon = XdgIcon::fromTheme(d->m_iconName);
        }
    }

    if(!winIDs().isEmpty()) {
        //TODO： 获取groupLeader窗口的图标？
        if (winIDs().contains(WindowManager::currentActiveWindow())) {
            this->setHasActiveWindow(true);
        }
        if(d->m_icon.isNull()) {
            d->m_icon = WindowManager::windowIcon(winIDs().at(0));
        }
        if(d->m_name.isEmpty()) {
            d->m_name = WindowManager::windowTitle(winIDs().at(0));
        }
        d->m_group =  WindowManager::windowGroup(winIDs().at(0));
        refreshWinIdsOnCurrentDesktop();
        refreshWinIdsOnScreens();
        refreshWindowTitlesAndIcons();
    }

    if (d->m_recentFilesService) {
        connect(d->m_recentFilesService, &RecentFilesService::appRecentFilesChanged, this,
                [this](const QString &appId) {
                    if (appId == d->m_ID) {
                        reloadRecentFileActions();
                    }
                });
    }
    reloadRecentFileActions();
    //desktop中的action
    for(const QString &action : d->m_xdf.actions()) {
        auto a = new Action(action, d->m_xdf.actionName(action), d->m_xdf.actionIcon(action), Action::Type::DesktopAction, action);
        connect(a, &Action::actionTriggered, this, [&](const QVariant& param){
            desktopActionActive(param.toString());
        });
        d->m_actions.append(a);
    }
    //打开应用
    if(d->m_xdf.isValid()) {
        d->m_actions.append(new Action(d->m_xdf.name(), d->m_xdf.name(), d->m_icon, Action::Type::NewInstance));
        connect(d->m_actions.last(), &Action::actionTriggered, this, &TaskManagerItem::newInstanceActionActive);

    }
    //添加到任务栏和从任务栏移除
    auto removeLauncher = new Action("Remove launcher", tr("Remove launcher from panel"),
                                         QIcon::fromTheme("ukui-unfixed-symbolic"), Action::Type::RemoveQuickLauncher);
    connect(removeLauncher, &Action::actionTriggered, this, &TaskManagerItem::removeQuickLauncherActionActive);
    removeLauncher->setEnabled(hasLauncher());
    d->m_actions.append(removeLauncher);
    //NoDisplay为true时无添加到任务栏选项
    if (!d->m_xdf.value("NoDisplay").toBool()) {
        auto addLauncher = new Action("Add launcher", tr("Add launcher to panel"),
                                      QIcon::fromTheme("ukui-fixed-symbolic"), Action::Type::AddQuickLauncher);
        connect(addLauncher, &Action::actionTriggered, this, [&](const QVariant &param) {
            addQuickLauncherActionActive(param.toInt());
        });
        addLauncher->setEnabled(!hasLauncher() && !d->m_ID.isEmpty());

        d->m_actions.append(addLauncher);
    }

    //关闭
    auto exit = new Action("Close", tr("Close"), QIcon::fromTheme("application-exit-symbolic"), Action::Type::Exit);
    connect(exit, &Action::actionTriggered, this, [&](const QVariant& param){
        exitActionActive(param.toStringList());
    });
    d->m_actions.append(exit);

    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::Actions,
                                    UkuiTaskManager::Id,
                                    UkuiTaskManager::Icon,
                                    UkuiTaskManager::Name});
}

Actions TaskManagerItem::actions() const
{
    return d->m_actions;
}

void TaskManagerItem::prepareActionsForMenu()
{
    if (!d->m_recentFilesService || d->m_ID.isEmpty()) {
        return;
    }

    if (d->m_recentFilesService->pruneMissingLocalFilesForApp(d->m_ID)) {
        reloadRecentFileActions();
    }
}

void TaskManagerItem::reloadRecentFileActions()
{
    // Recent-file actions are maintained as a prefix of m_actions so they can be
    // replaced in place without disturbing the persistent desktop/window actions.
    const int removeCount = std::min(d->m_recentFileActionCount, d->m_actions.size());
    for (int i = 0; i < removeCount; ++i) {
        Q_ASSERT(d->m_actions.at(0)->type() == Action::Type::RecentFile);
        auto *action = d->m_actions.takeAt(0);
        action->deleteLater();
    }

    Actions recentActions;
    if (d->m_recentFilesService && !d->m_ID.isEmpty()) {
        const auto recentFiles = d->m_recentFilesService->recentFilesForApp(d->m_ID);
        for (const auto &entry : recentFiles) {
            auto *fileAction = new Action("RecentFile-" + entry.bookmark.href(),
                                          d->m_recentFilesService->recentFileDisplayName(entry),
                                          d->m_recentFilesService->recentFileIcon(entry),
                                          Action::Type::RecentFile);
            connect(fileAction, &Action::actionTriggered, this, [this, entry](const QVariant&) {
                d->m_recentFilesService->openRecentFile(entry);
            });
            recentActions.append(fileAction);
        }
    }

    d->m_recentFileActionCount = recentActions.size();
    if (!recentActions.isEmpty()) {
        recentActions += d->m_actions;
        d->m_actions = recentActions;
    }

    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::Actions});
}

QStringList TaskManagerItem::demandsAttentionWinIDs() const
{
    return d->m_demandsAttentionWinIDs;
}

void TaskManagerItem::iconChanged(const QString &windowId)
{
    if(d->m_winIDs.contains(windowId)) {
        QIcon icon = WindowManager::windowIcon(windowId);
        if(icon.isNull()) {
            return;
        }
        d->m_widToIcon.insert(windowId, icon);
        d->m_widToIconName.insert(windowId, icon.name());
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WindowIcons, UkuiTaskManager::WindowIconsName});
        if(d->m_icon.isNull() && windowId == winIDs().first()) {
            d->m_icon = WindowManager::windowIcon(winIDs().at(0));
            Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::Icon});
        }
    }
}

void TaskManagerItem::titleChanged(const QString &windowId)
{
    if(d->m_winIDs.contains(windowId)) {
        d->m_widToTitle.insert(windowId, WindowManager::windowTitle(windowId));
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WindowTitles});
        if (d->m_name.isEmpty() && windowId == winIDs().first()) {
            d->m_name = WindowManager::windowTitle(winIDs().at(0));
            Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::Name});
        }
    }
}

void TaskManagerItem::desktopChanged(const QString &windowId)
{
    if(currentDesktopWinIDs().contains(windowId)) {
        if(!WindowManager::isOnAllDesktops(windowId) && !WindowManager::isOnCurrentDesktop(windowId)) {
            d->m_winIDsOnCurrentDesktop.removeAll(windowId);
            Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::CurrentDesktopWinIdList, UkuiTaskManager::CurrentWinIdList});
        }
    } else if (d->m_winIDs.contains(windowId)){
        if(WindowManager::isOnAllDesktops(windowId) || WindowManager::isOnCurrentDesktop(windowId)) {
            d->m_winIDsOnCurrentDesktop.append(windowId);
            Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::CurrentDesktopWinIdList, UkuiTaskManager::CurrentWinIdList});
        }
    }
}

void TaskManagerItem::demandsAttentionChanged(const QString &windowId)
{
    if(d->m_winIDs.contains(windowId)) {
        if(WindowManager::isDemandsAttention(windowId)) {
            if(!d->m_demandsAttentionWinIDs.contains(windowId)) {
                d->m_demandsAttentionWinIDs.append(windowId);
                Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::DemandsAttentionWinIdList});
            }
        } else {
            if(d->m_demandsAttentionWinIDs.contains(windowId)) {
                d->m_demandsAttentionWinIDs.removeAll(windowId);
                Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::DemandsAttentionWinIdList});
            }
        }
    }
}

void TaskManagerItem::geometryChanged(const QString &windowId)
{
    if(!d->m_winIDs.contains(windowId)) {
        return;
    }
    d->m_widToGeometry.insert(windowId, WindowManager::geometry(windowId));
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WindowGeometries, UkuiTaskManager::CurrentWinIdList});
}

void TaskManagerItem::refreshWinIdsOnCurrentDesktop()
{
    d->m_winIDsOnCurrentDesktop.clear();
    for(const QString &wid : d->m_winIDs) {
        if(WindowManager::isOnCurrentDesktop(wid) || WindowManager::isOnAllDesktops(wid)) {
            d->m_winIDsOnCurrentDesktop.append(wid);
        }
    }
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::CurrentDesktopWinIdList, UkuiTaskManager::CurrentWinIdList});
}

void TaskManagerItem::refreshWinIdsOnScreens()
{
    d->m_widToGeometry.clear();
    for (const QString &wid: d->m_winIDs) {
        d->m_widToGeometry.insert(wid, WindowManager::geometry(wid));
    }
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::WindowGeometries, UkuiTaskManager::CurrentWinIdList});
}

void TaskManagerItem::refreshWindowTitlesAndIcons()
{
    d->m_widToTitle.clear();
    for(const QString &wid : d->m_winIDs) {
        QString title = WindowManager::windowTitle(wid);
        if(title.isEmpty()) {
            title = d->m_name;
        }
        d->m_widToTitle.insert(wid, title);
        QIcon icon = WindowManager::windowIcon(wid);
        if(icon.isNull()) {
            icon = d->m_icon;
        }
        d->m_widToIcon.insert(wid, icon.isNull() ? QVariant("application-x-desktop") : icon);
        d->m_widToIconName.insert(wid, icon.name());
    }
}

const QStringList &TaskManagerItem::currentDesktopWinIDs()
{
    return d->m_winIDsOnCurrentDesktop;
}

void TaskManagerItem::updateUnreadMessagesNum(const QString &desktopFile, uint num)
{
    QFileInfo fileInfo(desktopFile);
    QString desktopName;
    if (fileInfo.fileName().endsWith(QStringLiteral(".desktop"))) {
        desktopName = fileInfo.completeBaseName();
    } else {
        desktopName = fileInfo.fileName();
    }

    if(desktopName == d->m_ID) {
        qDebug() << "Update unread messagesNum" << desktopFile << num;
        d->m_unreadMessagesNum = num;
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::UnreadMessagesNum});
    }
}

uint TaskManagerItem::unReadMessagesNum()
{
    return d->m_unreadMessagesNum;
}

void TaskManagerItem::newInstanceActionActive(const QVariant& param)
{
    launch(param.value<QQuickItem *>());
}

void TaskManagerItem::launch(const QQuickItem* item)
{
    if(d->m_desktopFile.isEmpty()) {
        return;
    }
    QString method = QStringLiteral("LaunchApp");
    if(item) {
        method = QStringLiteral("LaunchAppWithGeometry");
    }
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("com.kylin.ProcessManager"),
                                                          QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                          QStringLiteral("com.kylin.ProcessManager.AppLauncher"),
                                                          method);
    message << d->m_desktopFile;
    if(item) {
        auto point = item->mapToGlobal(QPoint(0, 0));
        message << static_cast<int>(point.x()) << static_cast<int>(point.y()) << static_cast<uint>(item->width()) << static_cast<uint>(item->height());
    }

    qDebug() << "Taskmanager Calling:" << method << message;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)));
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [&] (QDBusPendingCallWatcher *self) {
	qDebug() << "Taskmanager Calling finished:" << self->isError();
        if (self->isError()) {
            qDebug() << "launch" << d->m_desktopFile << self->error();
            d->m_xdf.startDetached();
        }
        self->deleteLater();
    });
}

void TaskManagerItem::desktopActionActive(const QString &name)
{
    d->m_xdf.actionActivate(name, {});
}

void TaskManagerItem::removeQuickLauncherActionActive()
{
    UkuiTaskManager::self().removeQuickLauncher(d->m_desktopFile);
}

void TaskManagerItem::addQuickLauncherActionActive(int index)
{
    UkuiTaskManager::self().setQuickLauncher(d->m_desktopFile, index);
}

void TaskManagerItem::exitActionActive(const QStringList &winIds)
{
    for(const QString &wid : winIds) {
        WindowManager::closeWindow(wid);
    }
}

void TaskManagerItem::onWidgetInfoUpdate(const QString& appid, const QString& widgetId)
{
    if(d->m_ID == appid) {
        d->m_quickWidget = widgetId;
        Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::QuickWidgetId});
    }
}

const QMap<QString, QRect> &TaskManagerItem::WindowGeometries() const
{
    return d->m_widToGeometry;
}

const QMap<QString, QVariant> &TaskManagerItem::windowTitles() const
{
    return d->m_widToTitle;
}

const QMap<QString, QVariant> &TaskManagerItem::windowIcons() const
{
    return d->m_widToIcon;
}

const QMap<QString, QVariant> &TaskManagerItem::windowIconsName() const
{
    return d->m_widToIconName;
}

void TaskManagerItem::updateIconStatus()
{
    Q_EMIT dataUpdated(QVector<int>{UkuiTaskManager::HasActiveWindow, UkuiTaskManager::WindowGeometries, UkuiTaskManager::CurrentWinIdList});
}
