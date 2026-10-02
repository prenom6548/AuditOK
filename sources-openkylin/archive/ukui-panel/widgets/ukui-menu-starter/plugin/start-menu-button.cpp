
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

#include "start-menu-button.h"

#include <QDBusConnection>
#include <QDebug>
#include <QAction>
#include <QDBusReply>
#include <window-manager.h>
#include <QGSettings>
#include <app-launcher.h>
#include <dbus-connector.h>
#include <function-control.h>
#include <window-helper.h>
#include <QJsonObject>
#include <QMenu>
#include <QGuiApplication>
#include <QQuickWindow>

StartMenuButton &StartMenuButton::self()
{
    static StartMenuButton s_self;
    return s_self;
}

StartMenuButton::StartMenuButton(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<QAction*>>();
    connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen *screen) {
        if (m_currentScreen == screen) {
            clearCurrentScreen();
        }
    });

    connectGlobalInterface();
    connectMenuInterface();
    initFunctionControl();

    m_applicationInfo = new UkuiSearch::ApplicationInfo(this);
    initAppLocalNames();
}

StartMenuButton::~StartMenuButton()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-menu-starter-v11"));
}

void StartMenuButton::openStartMenu()
{
    QDBusInterface interface("org.ukui.menu", "/org/ukui/menu", "org.ukui.menu", QDBusConnection::sessionBus());
    interface.asyncCall(QStringLiteral("WinKeyResponse"));
}

void StartMenuButton::showDesktop()
{
    UkuiQuick::WindowManager::setShowingDesktop(!UkuiQuick::WindowManager::showingDesktop());
}

void StartMenuButton::execSessionAction(const QString &action)
{
    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());
    sessionManager.asyncCall(action);
}

bool StartMenuButton::loadUserAction(QMenu *menu)
{
    // sessionManager
    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());

    QDBusInterface interface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                             "org.freedesktop.Accounts", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> getReply = interface.call(QStringLiteral("ListCachedUsers"));
    bool hasMultipleUsers = getReply.isValid() && getReply.value().toVector().size() > 1;
    QDBusReply<bool> reply = sessionManager.call(QStringLiteral("canSwitch"));
    if (hasMultipleUsers && reply.isValid() && reply.value()) {
        QAction *switchUserAct = new QAction(QIcon::fromTheme("system-users-symbolic"), tr("Switch User"), menu);
        connect(switchUserAct, &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("switchUser"));
        });
        menu->addAction(switchUserAct);
    }

    reply = sessionManager.call(QStringLiteral("canLogout"));
    if (reply.isValid() && reply.value()) {
        QAction *logoutAct = new QAction(QIcon::fromTheme("system-log-out-symbolic"), tr("Log Out"), menu);
        connect(logoutAct, &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("logout"));
        });
        menu->addAction(logoutAct);
    }

    return !menu->actions().isEmpty();
}

bool StartMenuButton::loadPowerAction(QMenu *menu)
{
    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());
    //锁屏
    QDBusReply<bool> reply = sessionManager.call(QStringLiteral("canLockscreen"));
    if (reply.isValid() && reply.value()) {
        QAction *lockScreenAct = new QAction(QIcon::fromTheme("system-lock-screen-symbolic"), tr("Lock Screen"), menu);
        connect(lockScreenAct, &QAction::triggered, this, [this] {
            UkuiQuick::AppLauncher::instance()->runCommand("ukui-screensaver-command -l");
        });
        menu->addAction(lockScreenAct);
    }
    //睡眠
    reply = sessionManager.call(QStringLiteral("canSuspend"));
    if (reply.isValid() && reply.value()) {
        QAction *suspendAct = new QAction(QIcon::fromTheme("ukui-sleep-symbolic"), tr("Suspend"), menu);
        connect(suspendAct, &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("suspend"));
        });
        menu->addAction(suspendAct);
    }
    //休眠
    reply = sessionManager.call(QStringLiteral("canHibernate"));
    if (reply.isValid() && reply.value()) {
        QAction *hibernateAct = new QAction(QIcon::fromTheme("ukui-hibernate-symbolic"), tr("Hibernate"), menu);
        connect(hibernateAct, &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("hibernate"));
        });
        menu->addAction(hibernateAct);
    }
    //定时关机
    bool isUpgradable = canUpgrade();
    //存在更新时是否隐藏关机和重启
    bool isNeedToHide = checkHideUpdate();
    reply = sessionManager.call(QStringLiteral("canPowerOff"));
    const QByteArray id("org.ukui.time-shutdown.settings");
    if (QGSettings::isSchemaInstalled(id) && reply.isValid() && reply.value() && !(isUpgradable && isNeedToHide)) {
        QAction *timeShutdownAct = new QAction(QIcon::fromTheme("timer-symbolic"), tr("Time Shutdown"), menu);
        connect(timeShutdownAct, &QAction::triggered, this, [this] {
            UkuiQuick::AppLauncher::instance()->runCommand("time-shutdown");
        });
        menu->addAction(timeShutdownAct);
    }

    //更新并关机
    if (isUpgradable) {
        QAction *upgradeShutdownAct = new QAction(QIcon::fromTheme("system-upd-shutdown-symbolic"), tr("Upgrade and Shut Down"), menu);
        upgradeShutdownAct->setToolTip(minutesToHour(m_estimatedLeftMinutes));
        connect(upgradeShutdownAct, &QAction::triggered, this, [this] {
            execUpgradeAction("shutdown");
        });
        menu->addAction(upgradeShutdownAct);
    }

    //关机

    if (reply.isValid() && reply.value() && !(isUpgradable && isNeedToHide)) {
        QAction *shutdownAct = new QAction(QIcon::fromTheme("system-shutdown-symbolic"), tr("Shut Down"), menu);
        connect(shutdownAct, &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("powerOff"));
        });
        menu->addAction(shutdownAct);
    }

    //更新并重启
    if (isUpgradable) {
        QAction *upgradeRebootAct = new QAction(QIcon::fromTheme("system-upd-reboot-symbolic"), tr("Upgrade and Restart"), menu);
        upgradeRebootAct->setToolTip(minutesToHour(m_estimatedLeftMinutes));
        connect(upgradeRebootAct, &QAction::triggered, this, [this] {
            execUpgradeAction("reboot");
        });
        menu->addAction(upgradeRebootAct);
    }

    //重启
    reply = sessionManager.call(QStringLiteral("canReboot"));
    if (reply.isValid() && reply.value() && !(isUpgradable && isNeedToHide)) {
        QAction *rebootAct = new QAction(QIcon::fromTheme("system-reboot-symbolic"), tr("Restart"), menu);
        connect(rebootAct, &QAction::triggered, this, [this] {
            execSessionAction(QStringLiteral("reboot"));
        });
        menu->addAction(rebootAct);
    }

    return !menu->actions().isEmpty();
}

void StartMenuButton::openGlobalSearch() {
    if (m_searchInterface) {
        m_searchInterface->asyncCall(QStringLiteral("mainWindowSwitch"));
    } else {
        qDebug() << "Can't connect to ukui-search DBus!";
        UkuiQuick::AppLauncher::instance()->launchApp("/usr/share/applications/ukui-search-menu.desktop");
    }
}

QString StartMenuButton::getAppLocalName(QString desktop)
{
    return m_appLocalNames.value(desktop);
}

void StartMenuButton::openContextMenu(QQuickItem *item)
{
    QMenu *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);
    //电源管理
    QAction *powerManager = new QAction(QIcon::fromTheme("battery-symbolic"), tr("Power Manager"), menu);
    connect(powerManager, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->runCommand("ukui-control-center -m power");
    });
    menu->addAction(powerManager);
    //关于本机
    QAction *aboutThisComputer = new QAction(QIcon::fromTheme("help-about-symbolic"), tr("About This Computer"), menu);
    connect(aboutThisComputer, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->runCommand("ukui-control-center -m about");
    });
    menu->addAction(aboutThisComputer);

    //网络设置
    QAction *networkSettings = new QAction(QIcon::fromTheme("network-wired-symbolic"), tr("Network Settings"), menu);
    connect(networkSettings, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->runCommand("ukui-control-center -m netconnect");
    });
    menu->addAction(networkSettings);

    //分隔符
    menu->addSeparator();

    //系统监视器
    QAction *systemMonitor = new QAction(QIcon::fromTheme("ukui-system-monitor"),
                                         getAppLocalName("ukui-system-monitor"), menu);
    connect(systemMonitor, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->launchApp("/usr/share/applications/ukui-system-monitor.desktop");
    });
    menu->addAction(systemMonitor);

    //设置
    QAction *controlCenter = new QAction(QIcon::fromTheme("ukui-control-center"),
                                         getAppLocalName("ukui-control-center"), menu);
    connect(controlCenter, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->launchApp("/usr/share/applications/ukui-control-center.desktop");
    });
    menu->addAction(controlCenter);

    //文件管理器
    QAction *fileManager = new QAction(QIcon::fromTheme("system-file-manager"),
                                       getAppLocalName("peony"), menu);
    connect(fileManager, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->launchApp("/usr/share/applications/peony.desktop");
    });
    menu->addAction(fileManager);

    //搜索
    QAction *search = new QAction(QIcon::fromTheme("kylin-search"), tr("Search"), menu);
    connect(search, &QAction::triggered, this, &StartMenuButton::openGlobalSearch);
    menu->addAction(search);

    //终端
    QAction *terminal = new QAction(QIcon::fromTheme("utilities-terminal"),
                                    getAppLocalName("mate-terminal"), menu);
    connect(terminal, &QAction::triggered, []() {
        UkuiQuick::AppLauncher::instance()->runCommand("/usr/bin/x-terminal-emulator");
    });
    menu->addAction(terminal);

    //分隔符
    menu->addSeparator();

    //切换用户或注销子菜单
    QMenu *userSubMenu = new QMenu(tr("Switch User or Log Out"), menu);
    if (loadUserAction(userSubMenu)) {
        QAction *userMenuAction = menu->addMenu(userSubMenu);
        userMenuAction->setIcon(QIcon::fromTheme("user-available-symbolic"));
    } else {
        delete userSubMenu;
    }

    //电源子菜单
    QMenu *powerSubMenu = new QMenu(tr("Power"), menu);
    powerSubMenu->setToolTipsVisible(true);
    if (loadPowerAction(powerSubMenu)) {
        QAction *powerMenuAction = menu->addMenu(powerSubMenu);
        powerMenuAction->setIcon(QIcon::fromTheme("system-shutdown-symbolic"));
    } else {
        delete powerSubMenu;
    }

    //显示桌面
    QAction *showDesktop = new QAction(QIcon::fromTheme("desktop-symbolic"), tr("Show Desktop"), menu);
    connect(showDesktop, &QAction::triggered, this, &StartMenuButton::showDesktop);
    menu->addAction(showDesktop);

    //分隔符
    menu->addSeparator();

    //所有应用
    QAction *allApps = new QAction(QIcon::fromTheme("preferences-desktop-apps-symbolic"), tr("All Applications"), menu);
    connect(allApps, &QAction::triggered, this, &StartMenuButton::openStartMenu);
    menu->addAction(allApps);
    //设置父窗口
    menu->winId();
    if(menu->windowHandle()) {
        menu->windowHandle()->setTransientParent(item->window());
    }

    getCurrentScreen();
    connect(menu, &QMenu::aboutToShow, this, [this] {
        if (!m_contextMenuVisible) {
            m_contextMenuVisible = true;
            Q_EMIT contextMenuVisibleChanged();
        }
    });
    connect(menu, &QMenu::aboutToHide, this, [this] {
        if (m_contextMenuVisible) {
            m_contextMenuVisible = false;
            Q_EMIT contextMenuVisibleChanged();
        }
    });

    menu->popup(QCursor::pos());
}

bool StartMenuButton::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

bool StartMenuButton::disableMouseRightButton() const
{
    return m_disableMouseRightButton;
}

bool StartMenuButton::isCurrentScreen(QScreen *screen) const
{
    return !m_currentScreen.isNull() && m_currentScreen == screen;
}

bool StartMenuButton::currentScreenValid() const
{
    return !m_currentScreen.isNull();
}

bool StartMenuButton::menuWindowVisible() const
{
    return m_menuWindowVisible;
}

bool StartMenuButton::contextMenuVisible() const
{
    return m_contextMenuVisible;
}

void StartMenuButton::connectGlobalInterface() {
    UkuiQuick::DbusConnector *dbusCon = new UkuiQuick::DbusConnector("com.ukui.search.service", "/",
                                                                     "org.ukui.search.service",
                                                                     QDBusConnection::sessionBus());
    connect(dbusCon, &UkuiQuick::DbusConnector::ready, this, [this](QDBusInterface *dbus) {
        m_searchInterface = dbus;
    });
    dbusCon->start();
}

void StartMenuButton::connectMenuInterface()
{
    UkuiQuick::DbusConnector *dbusCon = new UkuiQuick::DbusConnector("org.ukui.menu",
                                                                     "/org/ukui/menu",
                                                                     "org.freedesktop.DBus.Properties",
                                                                     QDBusConnection::sessionBus());
    connect(dbusCon, &UkuiQuick::DbusConnector::ready, this, [this](QDBusInterface *interface) {
        if (!interface || !interface->isValid()) return;
        m_menuInterface = interface;
        getMenuVisible();

        if (!QDBusConnection::sessionBus().connect("org.ukui.menu",
                                                   "/org/ukui/menu",
                                                   "org.freedesktop.DBus.Properties",
                                                   "PropertiesChanged", this, SLOT(onMenuPropertiesChanged(QString, QVariantMap, QStringList)))) {
            qWarning() << "MenuStarter: QDBusConnection::sessionBus() connect onPropertiesChanged failed";
        }
    });
    dbusCon->start();
}

void StartMenuButton::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-menu-starter-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-menu-starter-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-menu-starter-v11"), true);
    });
}

void StartMenuButton::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-menu-starter-v11")) {
        return;
    }

    auto controlConfig = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "StartMenuButton function control config:" << controlConfig;
    //是否禁用左键点击
    if (controlConfig.contains(QStringLiteral("disableMouseLeftButton"))) {
        bool disable = controlConfig.value(QStringLiteral("disableMouseLeftButton")).toBool();
        if (disable != m_disableMouseLeftButton) {
            m_disableMouseLeftButton = disable;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    } else {
        m_disableMouseLeftButton = false;
        Q_EMIT disableMouseLeftButtonChanged();
    }
    //是否禁用右键点击
    if (controlConfig.contains(QStringLiteral("disableMouseRightButton"))) {
        bool disable = controlConfig.value(QStringLiteral("disableMouseRightButton")).toBool();
        if (disable != m_disableMouseRightButton) {
            m_disableMouseRightButton = disable;
            Q_EMIT disableMouseRightButtonChanged();
        }
    } else {
        m_disableMouseRightButton = false;
        Q_EMIT disableMouseRightButtonChanged();
    }
}

bool StartMenuButton::canUpgrade()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(SYSTEMUPGRADE_SERVICE,
                                                      SYSTEMUPGRADE_PATH,
                                                      SYSTEMUPGRADE_INTERFACE,
                                                      CHECK_INSTALL_REQUIRED);
    QDBusMessage res = QDBusConnection::systemBus().call(msg);
    if (res.type() == QDBusMessage::ReplyMessage && res.arguments().count() > 1) {
        m_estimatedLeftMinutes = res.arguments().at(1).toInt();
        return res.arguments().at(0).toBool();
    }
    return false;
}

QString StartMenuButton::minutesToHour(int estmateTime)
{
    if (estmateTime > 60) {
        int hour = estmateTime / 60;
        int min = estmateTime % 60;
        return tr("Estimated %1 hour %2 minutes").arg(hour).arg(min);
    } else if (estmateTime > 0) {
        return tr("Estimated %1 minutes").arg(estmateTime);
    }
    return {};
}

bool StartMenuButton::checkHideUpdate()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(SYSTEMUPGRADE_SERVICE,
                                                      SYSTEMUPGRADE_PATH,
                                                      SYSTEMUPGRADE_INTERFACE,
                                                      CHECK_HIDE_UPDATE);
    QDBusMessage res = QDBusConnection::systemBus().call(msg);
    if (res.type() == QDBusMessage::ReplyMessage) {
        return res.arguments().at(0).toBool();
    }
    qWarning() << "Call 'com.kylin.systemupgrade' interface method 'CheckHideUpdate' failed" << res.errorMessage();
    return false;
}

void StartMenuButton::setCurrentScreen(QScreen *screen)
{
    if (m_currentScreen == screen) {
        return;
    }

    if (m_currentScreenDestroyedConnection) {
        disconnect(m_currentScreenDestroyedConnection);
        m_currentScreenDestroyedConnection = {};
    }

    m_currentScreen = screen;
    if (m_currentScreen) {
        m_currentScreenDestroyedConnection = connect(m_currentScreen.data(), &QObject::destroyed, this, [this]() {
            m_currentScreenDestroyedConnection = {};
            m_currentScreen.clear();
            Q_EMIT currentScreenChanged();
        });
    }

    Q_EMIT currentScreenChanged();
}

void StartMenuButton::clearCurrentScreen()
{
    if (m_currentScreenDestroyedConnection) {
        disconnect(m_currentScreenDestroyedConnection);
        m_currentScreenDestroyedConnection = {};
    }

    if (m_currentScreen.isNull()) {
        return;
    }

    m_currentScreen.clear();
    Q_EMIT currentScreenChanged();
}

void StartMenuButton::getCurrentScreen()
{
    setCurrentScreen(UkuiQuick::WindowProxy::currentScreen());
}

void StartMenuButton::getMenuVisible()
{
    bool menuWindowVisible = false;
    if (!m_menuInterface || !m_menuInterface->isValid()) {
        qWarning() << "MenuStarter: menu interface is not valid";
    } else {
        QDBusReply<QVariant> result = m_menuInterface->call("Get", "org.ukui.menu", "Visible");
        if (result.isValid()) {
            menuWindowVisible = result.value().toBool();
        } else {
            qWarning() << "MenuStarter: call get('Visible') failed: " << result.error().name() << result.error().message();
        }
    }

    if (m_menuWindowVisible != menuWindowVisible) {
        m_menuWindowVisible = menuWindowVisible;
        if (m_menuWindowVisible) {
            getCurrentScreen();
        }
        Q_EMIT menuWindowVisibleChanged();
    }
}

void StartMenuButton::initAppLocalNames()
{
    if (!m_applicationInfo)
        return;

    QHash<QString, QString> appNames = {
        {"ukui-system-monitor", tr("System Monitor")},
        {"ukui-control-center", tr("Control Center")},
        {"peony", tr("File Manager")},
        {"mate-terminal", tr("Open Terminal")}
    };

    for (const QString& appName : appNames.keys()) {
        QString fullPath;
        m_applicationInfo->desktopFilePathFromName(appName, fullPath);
        if (!fullPath.isEmpty()) {
            QVariant localName = m_applicationInfo->getInfo(fullPath, UkuiSearch::ApplicationProperty::Property::LocalName);
            m_appLocalNames.insert(appName, localName.toString());
        } else {
            qWarning() << "Can't get desktop file path from name:" << appName;
            m_appLocalNames.insert(appName, appNames.value(appName));
        }
    }
}

void StartMenuButton::execUpgradeAction(const QString &action)
{
    QDBusInterface interface(SYSTEMUPGRADE_SERVICE,
                             SYSTEMUPGRADE_PATH,
                             SYSTEMUPGRADE_INTERFACE,
                             QDBusConnection::systemBus());

    if (interface.isValid()) {
        interface.asyncCall(TRIGGER_INSTALL_ON_SHUTDOWN, action);
    } else {
        qCritical() << "'com.kylin.systemupgrade' interface is invalid!" <<  QDBusConnection::systemBus().lastError().message();
    }
}

void StartMenuButton::onMenuPropertiesChanged(QString interface, QVariantMap property, QStringList list)
{
    Q_UNUSED(list)
    if (interface != "org.ukui.menu") return;

    if (property.keys().contains("Visible")) {
        bool menuWindowVisible = property.value("Visible").toBool();

        if (m_menuWindowVisible != menuWindowVisible) {
            m_menuWindowVisible = menuWindowVisible;
            if (m_menuWindowVisible) {
                getCurrentScreen();
            }
            Q_EMIT menuWindowVisibleChanged();
        }
    }
}
