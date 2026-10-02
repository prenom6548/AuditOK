/*
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_PANEL_START_MENU_BUTTON_H
#define UKUI_PANEL_START_MENU_BUTTON_H

#include <QObject>
#include <QPointer>
#include <QScreen>
#include <QDBusInterface>
#include <QQuickItem>
#include <QMenu>
#include <application-info.h>

#define ACCOUNTS_SERVICE_NAME       "org.freedesktop.Accounts"
#define ACCOUNTS_SERVICE_PATH      "/org/freedesktop/Accounts"
#define ACCOUNTS_SERVICE_INTERFACE  "org.freedesktop.Accounts"

//系统更新
#define SYSTEMUPGRADE_SERVICE       "com.kylin.systemupgrade"
#define SYSTEMUPGRADE_PATH          "/com/kylin/systemupgrade"
#define SYSTEMUPGRADE_INTERFACE     "com.kylin.systemupgrade.interface"
#define CHECK_INSTALL_REQUIRED      "CheckInstallRequired"
#define CHECK_HIDE_UPDATE           "CheckHideUpdate"
#define TRIGGER_INSTALL_ON_SHUTDOWN "TriggerInstallOnShutdown"

class QAction;

class StartMenuButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)
    Q_PROPERTY(bool disableMouseRightButton READ disableMouseRightButton NOTIFY disableMouseRightButtonChanged)
    Q_PROPERTY(bool currentScreenValid READ currentScreenValid NOTIFY currentScreenChanged)
    Q_PROPERTY(bool menuWindowVisible READ menuWindowVisible NOTIFY menuWindowVisibleChanged)
    Q_PROPERTY(bool contextMenuVisible READ contextMenuVisible NOTIFY contextMenuVisibleChanged)

public:
    explicit StartMenuButton(QObject *parent=nullptr);
    static StartMenuButton &self();
    ~StartMenuButton() override;

    Q_INVOKABLE void openStartMenu();
    Q_INVOKABLE void showDesktop();
    Q_INVOKABLE void openGlobalSearch();
    Q_INVOKABLE void openContextMenu(QQuickItem *item);
    Q_INVOKABLE bool isCurrentScreen(QScreen *screen) const;

    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;
    bool currentScreenValid() const;
    bool menuWindowVisible() const;
    bool contextMenuVisible() const;

Q_SIGNALS:
    void disableMouseLeftButtonChanged();
    void disableMouseRightButtonChanged();
    void currentScreenChanged();
    void menuWindowVisibleChanged();
    void contextMenuVisibleChanged();

public Q_SLOTS:
    void execSessionAction(const QString &action);
    void onFunctionControlChanged(const QString& name, bool exit = false);
    void execUpgradeAction(const QString &action);
    void onMenuPropertiesChanged(QString interface, QVariantMap property, QStringList list);

    // TODO: Reload Config
    // void reloadActions();

private:
    bool loadUserAction(QMenu *menu);
    bool loadPowerAction(QMenu *menu);
    void connectGlobalInterface();
    void connectMenuInterface();
    void initFunctionControl();
    //检测是否需要更新
    bool canUpgrade();
    QString minutesToHour(int estmateTime);
    //检测有更新时是否需要隐藏关机和重启选项
    bool checkHideUpdate();
    void setCurrentScreen(QScreen *screen);
    void clearCurrentScreen();
    void getCurrentScreen();
    void getMenuVisible();

    void initAppLocalNames();
    QString getAppLocalName(QString desktop);

private:
    QDBusInterface *m_searchInterface {nullptr};
    UkuiSearch::ApplicationInfo *m_applicationInfo{nullptr};
    QDBusInterface *m_menuInterface {nullptr};

    bool m_disableMouseLeftButton = false;
    bool m_disableMouseRightButton = false;
    QPointer<QScreen> m_currentScreen;
    QMetaObject::Connection m_currentScreenDestroyedConnection;
    bool m_menuWindowVisible = false;
    bool m_contextMenuVisible = false;
    //检测更新并重启和更新并关机预计剩余时间(单位：分钟)
    int m_estimatedLeftMinutes = 0;

    QHash<QString, QString> m_appLocalNames;
};

#endif //UKUI_PANEL_START_MENU_BUTTON_H
