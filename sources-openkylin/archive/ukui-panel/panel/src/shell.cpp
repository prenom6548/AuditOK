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
 *          iaom <zhangpengfei@kylinos.cn>
 */

#include "shell.h"
#include "panel.h"
#include "panel-next.h"

#include <QScreen>
#include <QGuiApplication>
#include <config-loader.h>
#include <QDBusInterface>
#include <QDBusReply>
#include <dbus-connector.h>
#include <config-loader.h>
#include <function-control.h>
#include <unistd.h>

#include "panel-top-bar.h"
#include "island-drag-helper.h"
#include "panel-gsettings.h"
#include "paneladaptor.h"

using namespace UkuiPanel;
static Shell *s_shell = nullptr;

Shell *Shell::self()
{
    if(!s_shell) {
        s_shell = new Shell;
    }
    return s_shell;
}

Shell::Shell(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QScreen*>();
    qmlRegisterSingletonType<IslandDragHelper>("org.ukui.panel.shell", 1, 0, "IslandDragHelper", [](QQmlEngine*, QJSEngine*) -> QObject* {
    return IslandDragHelper::self();
});
    m_screenManager = new ScreensManager(this);
    initRemoteConfig();
    initDbus();
    m_config = UkuiQuick::ConfigLoader::getConfig(QStringLiteral("panel"), UkuiQuick::ConfigLoader::Local, QStringLiteral("ukui-panel"));
    if(m_config->getValue(QStringLiteral("currentPanelVersion")).isNull()) {
        m_config->setValue(QStringLiteral("currentPanelVersion"), Version4);
        m_currentVersion = Version4;
    } else {
        m_currentVersion = static_cast<PanelType>(m_config->getValue(QStringLiteral("currentPanelVersion")).toInt());
    }
    PanelGSettings::instance()->setPanelType(m_currentVersion);
    if(m_config->contains("showPanelOnAllScreens")) {
        m_showPanelOnAllScreens = m_config->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
    } else {
        m_showPanelOnAllScreens = true;
        m_config->setValue(QStringLiteral("showPanelOnAllScreens"), m_showPanelOnAllScreens);
    }
    connect(m_config, &UkuiQuick::Config::configChanged, this, [&](QString key){
        if(key == "currentPanelVersion") {
            if(m_currentVersion != static_cast<PanelType>(m_config->getValue(QStringLiteral("currentPanelVersion")).toInt())) {
                switchTo(static_cast<PanelType>(m_config->getValue(QStringLiteral("currentPanelVersion")).toInt()));
            }
        } else if(key == "showPanelOnAllScreens") {
            auto value = m_config->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
            if(value == m_showPanelOnAllScreens) {
                return;
            }
            m_showPanelOnAllScreens = value;
            if (!m_configDataChange) {
                m_configDataChange = true;
                auto ev = new QEvent(QEvent::UpdateRequest);
                QCoreApplication::postEvent(this, ev);
            }
        }
    });

    connect(m_screenManager, &ScreensManager::screenAdded, this, [&](Screen *screen){
        if(!m_showPanelOnAllScreens) {
            return;
        }

        if(!screen->isValid()) {
            return;
        }

        if(PanelType::Version4 == m_currentVersion) {
            if(!m_panelsVersion4.contains(screen)) {
                auto panel = createPanel(screen, QString("panel%1").arg(m_panelsVersion4.size()));
                connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
                m_panelsVersion4.insert(screen, panel);
            }
        } else {
            if(!m_panelsVersion5.contains(screen)) {
                m_panelsVersion5.insert(screen, createPanelNext(screen, QString("panel%1").arg(m_panelsVersion5.size())));
                m_panelTopBars.insert(screen, createPanelTopBar(screen, QString("panelTopBar%1").arg(m_panelTopBars.size())));
            }
        }
    });

    connect(m_screenManager, &ScreensManager::screenRemoved, this, [&](Screen *screen){
        if(m_panelsVersion4.contains(screen)) {
            delete m_panelsVersion4.take(screen);
        }
        if (m_panelsVersion5.contains(screen)) {
            delete m_panelsVersion5.take(screen);
        }
        if (m_panelTopBars.contains(screen)) {
            delete m_panelTopBars.take(screen);
        }
    });

    connect(m_screenManager, &ScreensManager::primaryScreenChanged, this, [&](Screen *screen){
        if(m_showPanelOnAllScreens) {
            return;
        }
        if(PanelType::Version4 == m_currentVersion) {
            auto panel = m_panelsVersion4.first();
            m_panelsVersion4.clear();
            panel->setPanelScreen(screen);
            m_panelsVersion4.insert(screen, panel);

        } else {
            auto panel = m_panelsVersion5.first();
            m_panelsVersion5.clear();
            panel->setPanelScreen(screen);
            m_panelsVersion5.insert(screen, panel);
            auto toolBar = m_panelTopBars.first();
            m_panelTopBars.clear();
            toolBar->setTopBarScreen(screen);
            m_panelTopBars.insert(screen, toolBar);
        }
    });

    new PanelAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/panel"), this);
    dbus.registerService(QStringLiteral("org.ukui.panel"));

    //功能管控
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), true);
    });
}

Shell::~Shell()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-v11"));
}

void Shell::start()
{
    switch (m_currentVersion) {
        case Version4:
            startVersion4panel();
            break;
        case Version5:
            startVersion5Panel();
            break;
    }
    keepAlive();
    m_started = true;
    m_remoteConfig->updateMainViewConfig();
}

void Shell::initRemoteConfig()
{
    m_remoteConfig = new RemoteConfig(this);
    QString display = QCoreApplication::instance()->property("display").toString();
    QUrl address(QStringLiteral("local:ukui-panel-config-")  + QString(qgetenv("USER")) + QStringLiteral("-") + QString::number(getuid()) + QStringLiteral("-") + display);
    m_qroHost.setHostUrl(address);
    qDebug() << "Init static remote config object:" << m_qroHost.enableRemoting<RemoteConfigSourceAPI>(m_remoteConfig);
    qDebug() << "Init dynamic remote config object:" << m_qroHost.enableRemoting(m_remoteConfig,"RemoteConfig");
}

void Shell::initDbus()
{
    //dbus
    const QString statusManagerService = "com.kylin.statusmanager.interface";
    QDBusInterface statusManagerDBus(statusManagerService, "/" , statusManagerService,
                                     QDBusConnection::sessionBus(), this);

    if (statusManagerDBus.isValid()) {
        QDBusReply<bool> message = statusManagerDBus.call("get_current_tabletmode");
        if (message.isValid()) {
            m_isTabletMode = message.value();
        }
    }

    onTabletModeChanged(m_isTabletMode);
    //平板模式切换信号
    QDBusConnection::sessionBus().connect(statusManagerService, "/", statusManagerService,
                                          "mode_change_signal", this, SLOT(onTabletModeChanged(bool)));

    //锁屏服务：锁屏时隐藏任务栏（带滑出动画），解锁时恢复
    const QString screenSaverService = QStringLiteral("org.ukui.ScreenSaver");
    QDBusConnection::sessionBus().connect(screenSaverService, "/", screenSaverService,
                                          "lock", this, SLOT(onScreenLocked()));
    QDBusConnection::sessionBus().connect(screenSaverService, "/", screenSaverService,
                                          "unlock", this, SLOT(onScreenUnlocked()));

    //初始锁屏状态同步：服务可能晚于 panel 启动，用 DbusConnector 后台等待服务就绪后再查询
    m_screenSaverConnector = new UkuiQuick::DbusConnector(screenSaverService, "/", screenSaverService,
                                                          QDBusConnection::sessionBus(), this);
    connect(m_screenSaverConnector, &UkuiQuick::DbusConnector::ready, this, [this](QDBusInterface *interface) {
        if (!interface) {
            return;
        }
        //ready 在子线程触发并经队列连接回主线程，此处在主线程操作 panel 窗口是安全的
        QDBusReply<bool> locked = interface->call("GetLockState");
        if (locked.isValid()) {
            locked.value() ? onScreenLocked() : onScreenUnlocked();
        }
        interface->deleteLater();
    });
    m_screenSaverConnector->start();
}

void Shell::onTabletModeChanged(bool isTabletMode)
{
    m_isTabletMode = isTabletMode;
    for (const auto &panel : m_panelsVersion4) {
        panel->setShellHidden(m_isTabletMode);
    }
    for (const auto &panel : m_panelsVersion5) {
        panel->setShellHidden(m_isTabletMode);
    }
}

void Shell::onScreenLocked()
{
    m_screenLocked = true;
    for (const auto &panel : m_panelsVersion4) {
        panel->setScreenLockHidden(true);
    }
    for (const auto &panel : m_panelsVersion5) {
        panel->setScreenLockHidden(true);
    }
    for (const auto &topBar : m_panelTopBars) {
        topBar->setScreenLockHidden(true);
    }
}

void Shell::onScreenUnlocked()
{
    m_screenLocked = false;
    for (const auto &panel : m_panelsVersion4) {
        panel->setScreenLockHidden(false);
    }
    for (const auto &panel : m_panelsVersion5) {
        panel->setScreenLockHidden(false);
    }
    for (const auto &topBar : m_panelTopBars) {
        topBar->setScreenLockHidden(false);
    }
}

void Shell::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-v11")) {
        return;
    }
    //管控中有值时，设置为管控值，否则设置为配置文件中的值
    auto config = exit? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();
    qDebug() << "Shell" << "function control config:" << config;

    //showPanelOnAllScreens
    if (config.contains(QStringLiteral("showPanelOnAllScreens"))) {
        bool value = config.value(QStringLiteral("showPanelOnAllScreens")).toBool();
        if(value == m_showPanelOnAllScreens) {
            return;
        }
        m_showPanelOnAllScreens = value;
    } else {
        auto value = m_config->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
        if(value == m_showPanelOnAllScreens) {
            return;
        }
        m_showPanelOnAllScreens = value;
    }
    if (!m_configDataChange) {
        m_configDataChange = true;
        auto ev = new QEvent(QEvent::UpdateRequest);
        QCoreApplication::postEvent(this, ev);
    }
}

void Shell::switchTo(Shell::PanelType type)
{
    if(type == m_currentVersion) {
        return;
    }
    if(m_started) {
        qDeleteAll(m_panelsVersion4);
        m_panelsVersion4.clear();
        qDeleteAll(m_panelsVersion5);
        m_panelsVersion5.clear();
        qDeleteAll(m_panelTopBars);
        m_panelTopBars.clear();
    }
    m_currentVersion = type;
    m_config->setValue(QStringLiteral("currentPanelVersion"), m_currentVersion);
    start();
    PanelGSettings::instance()->setPanelType(m_currentVersion);
}

void Shell::startVersion4panel()
{
    qDebug() << "startVersion4panel!";
    Screen *primary = m_screenManager->primaryScreen();
    auto panel = createPanel(primary, "panel0");
    connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
    m_panelsVersion4.insert(primary, panel);
    //如果设置为在所有屏幕上显示任务栏再创建副屏的任务栏，否则不创建
    if(m_showPanelOnAllScreens) {
        for(Screen * screen : m_screenManager->screens()) {
            if(!screen->isValid()) {
                return;
            }
            if(screen != primary) {
                panel = createPanel(screen, QString("panel%1").arg(m_panelsVersion4.size()));
                connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
                m_panelsVersion4.insert(screen, panel);
            }
        }
    }
}

Panel* Shell::createPanel(Screen *screen, const QString &id)
{
    auto panel = new Panel(screen, id);
    panel->setShellHidden(m_isTabletMode);
    panel->setScreenLockHidden(m_screenLocked);
    return panel;
}

void Shell::startVersion5Panel()
{
    qDebug() << "startVersion5panel!";
    Screen *primary = m_screenManager->primaryScreen();
    m_panelsVersion5.insert(primary, createPanelNext(primary, "panel0"));
    m_panelTopBars.insert(primary, createPanelTopBar(primary, "panelTopBar0"));

    if(m_showPanelOnAllScreens) {
        for(Screen * screen : m_screenManager->screens()) {
            if(screen != primary) {
                m_panelsVersion5.insert(screen, createPanelNext(screen, QString("panel%1").arg(m_panelsVersion5.size())));
                m_panelTopBars.insert(screen, createPanelTopBar(screen, QString("panelTopBar%1").arg(m_panelTopBars.size())));
            }
        }
    }
}

PanelNext* Shell::createPanelNext(Screen *screen, const QString& id)
{
    auto panel = new PanelNext(screen, id);
    //island从任务栏移除
    connect(panel, &PanelNext::islandRemoved, this, &Shell::islandRemovedFromPanel);
    //island从顶栏移除，增加到任务栏上
    connect(this, &Shell::islandRemovedFromTopBar, panel, &PanelNext::addIsland);
    //island从任务栏移除，同步给所有任务栏
    connect(this, &Shell::islandRemovedFromPanel, panel, &PanelNext::removeIsland);

    //增加自定义插件
    connect(panel, &PanelNext::customWidgetAdded, this, &Shell::customWidgetAdded);
    connect(this, &Shell::customWidgetAdded, panel, &PanelNext::addCustomWidget);

    //移除自定义插件
    connect(panel, &PanelNext::customWidgetRemoved, this, &Shell::customWidgetRemoved);
    connect(this, &Shell::customWidgetRemoved, panel, &PanelNext::removeCustomWidget);
    panel->setShellHidden(m_isTabletMode);
    panel->setScreenLockHidden(m_screenLocked);
    return panel;
}

PanelTopBar* Shell::createPanelTopBar(Screen* screen, const QString& id)
{
    auto panelTopBar = new PanelTopBar(screen, id);
    //island从顶移除
    connect(panelTopBar, &PanelTopBar::islandRemoved, this, &Shell::islandRemovedFromTopBar);
    //island从任务栏移除，添加到顶栏上
    connect(this, &Shell::islandRemovedFromPanel, panelTopBar, &PanelTopBar::addIsland);
    //island从顶栏移除，同步给所有顶栏
    connect(this, &Shell::islandRemovedFromTopBar, panelTopBar, &PanelTopBar::removeIsland);
    panelTopBar->setScreenLockHidden(m_screenLocked);
    return panelTopBar;
}

void Shell::keepAlive()
{
    if (!m_keepAliveClient) {
        m_keepAliveClient = new UkuiQuick::HeartBeat::KeepAliveClient(QCoreApplication::applicationName() ,this);
    }

    if (m_keepAliveClient->isStarted() && m_keepAliveClient->isRegistered()) {
        return;
    }
    bool success = m_keepAliveClient->start();
    if (!success) {
        qWarning() << "Failed to start keep-alive client";
    }
}

Shell::PanelType Shell::currentVersion()
{
    return m_currentVersion;
}

bool Shell::addCustomWidget(const QString& id)
{
    if(!m_panelsVersion5.isEmpty() && !id.isEmpty()) {
        return m_panelsVersion5.first()->addCustomWidget(id);
    }
    return false;
}

bool Shell::removeCustomWidget(const QString& id)
{
    if(!m_panelsVersion5.isEmpty() && !id.isEmpty()) {
        return m_panelsVersion5.first()->removeCustomWidget(id);
    }
    return false;
}

bool Shell::event(QEvent* e)
{
    if (e->type() == QEvent::UpdateRequest) {
        //是否在所有屏幕上显示任务栏，变为true时
        //创建副屏的任务栏
        if(m_showPanelOnAllScreens) {
            for(Screen * screen : m_screenManager->screens()) {
                if(!screen->isValid()) {
                    continue;
                }
                if(m_currentVersion == PanelType::Version4) {
                    if(!m_panelsVersion4.contains(screen)) {
                        auto panel = createPanel(screen, QString("panel%1").arg(m_panelsVersion4.size()));
                        connect(m_screenManager, &ScreensManager::primaryScreenChanged, panel, &Panel::primaryScreenChanged);
                        m_panelsVersion4.insert(screen, panel);
                    }
                } else {
                    if(!m_panelsVersion5.contains(screen)) {
                        m_panelsVersion5.insert(screen, createPanelNext(screen, QString("panel%1").arg(m_panelsVersion5.size())));
                        m_panelTopBars.insert(screen, createPanelTopBar(screen, QString("panelTopBar%1").arg(m_panelTopBars.size())));
                    }
                }
            }
        } else {
            //是否在所有屏幕上显示任务栏，变为false时
            //删除不是主屏任务栏的所有任务栏
            for(Screen * screen : m_screenManager->screens()) {
                if(m_currentVersion == PanelType::Version4) {
                    if(m_panelsVersion4.contains(screen) && screen != m_screenManager->primaryScreen()) {
                        delete m_panelsVersion4.take(screen);
                    }
                } else {
                    if(m_panelsVersion5.contains(screen) && screen != m_screenManager->primaryScreen()) {
                        delete m_panelsVersion5.take(screen);
                    }
                    if(m_panelTopBars.contains(screen) && screen != m_screenManager->primaryScreen()) {
                        delete m_panelTopBars.take(screen);
                    }
                }
            }
        }
        m_configDataChange = false;
        e->accept();
    }
    return QObject::event(e);
}
