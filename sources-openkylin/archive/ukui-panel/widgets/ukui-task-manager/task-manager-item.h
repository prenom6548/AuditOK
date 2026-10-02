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

#ifndef UKUI_TASK_MANAGER_TASK_MANAGER_ITEM_H
#define UKUI_TASK_MANAGER_TASK_MANAGER_ITEM_H

#include <QObject>
#include <QIcon>
#include <QVariant>
#include "actions.h"
class QQuickItem;

namespace TaskManager {
class RecentFilesService;
class TaskManagerItemPrivate;
class TaskManagerItem : public QObject
{
    Q_OBJECT
public:
    explicit TaskManagerItem(QObject *parent = nullptr);
    explicit TaskManagerItem(const QString &desktoFile, QObject *parent = nullptr);
    explicit TaskManagerItem(const QStringList &winIDs, QObject *parent = nullptr);
    explicit TaskManagerItem(const QString &desktoFile, RecentFilesService *recentFilesService, QObject *parent = nullptr);
    explicit TaskManagerItem(const QStringList &winIDs, RecentFilesService *recentFilesService, QObject *parent = nullptr);
    ~TaskManagerItem();

    const QString &ID() const;
    void setID(const QString &appID);
    const QString &desktopFile() const;
    void setDesktopFile(const QString &desktopFile);
    const QString &name() const;
    const QString &genericName() const;
    const QString &iconName() const;
    const QIcon &icon() const;
    const QStringList &winIDs() const;
    const QStringList &currentDesktopWinIDs();
    const QMap<QString,QRect> &WindowGeometries() const;
    const QMap<QString, QVariant> &windowTitles() const;
    const QMap<QString, QVariant> &windowIcons() const;
    const QMap<QString, QVariant> &windowIconsName() const;
    void addWinID(const QString &winId);
    void removeWinID(const QString &winID);
    void iconChanged(const QString& windowId);
    void titleChanged(const QString& windowId);
    void desktopChanged(const QString& windowId);
    void demandsAttentionChanged(const QString& windowId);
    void geometryChanged(const QString& windowId);

    void setHasActiveWindow(bool has);
    bool hasActiveWindow();
    const QString &group() const;
    void setGroup(const QString &group);
    bool hasLauncher() const;
    void setHasLauncher(bool hasLauncher);
    Actions actions() const;
    void prepareActionsForMenu();
    QStringList demandsAttentionWinIDs() const;
    const QString &display() const;
    QString quickWidget() const;
    uint unReadMessagesNum();
    void init();
    void updateIconStatus();

public Q_SLOTS:
    void updateUnreadMessagesNum(const QString &desktopFile, uint num);
    void newInstanceActionActive(const QVariant& param);
    void launch(const QQuickItem *item = nullptr);

private Q_SLOTS:
    void reloadRecentFileActions();
    void refreshWinIdsOnCurrentDesktop();
    void refreshWinIdsOnScreens();
    void refreshWindowTitlesAndIcons();
    void desktopActionActive(const QString &name);
    void removeQuickLauncherActionActive();
    void addQuickLauncherActionActive(int index);
    void exitActionActive(const QStringList &winIds);
    void onWidgetInfoUpdate(const QString &appid, const QString &widgetId);

Q_SIGNALS:
    void dataUpdated(QVector<int> roles);
private:
    TaskManagerItemPrivate *d = nullptr;
};

}
#endif //UKUI_TASK_MANAGER_TASK_MANAGER_ITEM_H
