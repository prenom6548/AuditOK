/*
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

#include "wayland-window-manager.h"
#include <QDebug>
#include <KWindowSystem>
#include <QQuickWindow>
#include <QTimer>

namespace UkuiQuick {
WaylandWindowManager::WaylandWindowManager(QObject *parent) : AbstractWindowManager(parent)
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);

    if (!m_connection) {
        return;
    }
    m_registry = new UkuiQuick::WaylandClient::Registry(this);
    m_registry->create(m_connection);


    QObject::connect(m_registry, &UkuiQuick::WaylandClient::Registry::ukuiWindowManagementAnnounced, [&](quint32 name, quint32 version){
        m_windowManagement = m_registry->createUkuiWindowManagement(name, version, this);
        connect(m_windowManagement, &UkuiWindowManagement::windowCreated, this, &WaylandWindowManager::addWindow);
        connect(m_windowManagement, &UkuiWindowManagement::activeWindowChanged, [&](){
            auto w = m_windowManagement->activeWindow();
            if(w) {
                if(m_windows.contains(w)) {
                    Q_EMIT activeWindowChanged(w->uuid());
                }
            } else {
                Q_EMIT activeWindowChanged("");
            }
        });
    });
    connect(m_registry, &UkuiQuick::WaylandClient::Registry::ukuiWorkspaceManagerAnnounced, [&](quint32 name, quint32 version){
        m_workspaceManager = m_registry->createUkuiWorkspaceManager(name, version, this);
        if(m_workspaceManager) {
            connect(m_workspaceManager, &UkuiWorkspaceManager::workspaceCreated, this, &WaylandWindowManager::desktopCreated);
        }
    });
    m_registry->setup();
    QTimer::singleShot(0, this, [&]() {
        m_connection->roundtrip();
    });

}

WaylandWindowManager::~WaylandWindowManager()
= default;

void WaylandWindowManager::addWindow(UkuiWindow *window)
{
    if (m_windows.indexOf(window) != -1) {
        return;
    }
    const QString uuid = window->uuid();
    m_windows.append(window);
    m_uuidToWindow.insert(uuid, window);

    auto removeWindow = [this, window, uuid] {
        if (m_windows.removeAll(window)) {
            m_uuidToWindow.remove(uuid);
            Q_EMIT windowRemoved(uuid);
        }
    };

    connect(window, &UkuiWindow::unmapped, this, removeWindow);
    connect(window, &UkuiWindow::destroyed, this, removeWindow);
    connect(window, &UkuiWindow::titleChanged, this, [&, window](){Q_EMIT titleChanged(window->uuid());});
    connect(window, &UkuiWindow::iconChanged, this, [&, window](){Q_EMIT iconChanged(window->uuid());});
    connect(window, &UkuiWindow::skipTaskbarChanged, this, [&, window](){Q_EMIT skipTaskbarChanged(window->uuid());});
    connect(window, &UkuiWindow::onAllDesktopsChanged, this, [&, window](){Q_EMIT onAllDesktopsChanged(window->uuid());});
    connect(window, &UkuiWindow::ukuiVirtualDesktopEntered, this, [&, window](){Q_EMIT windowDesktopChanged(window->uuid());});
    connect(window, &UkuiWindow::ukuiVirtualDesktopLeft, this, [&, window](){Q_EMIT windowDesktopChanged(window->uuid());});
    connect(window, &UkuiWindow::demandsAttentionChanged, this, [&, window](){Q_EMIT demandsAttentionChanged(window->uuid());});
    connect(window, &UkuiWindow::geometryChanged, this, [&, window](){Q_EMIT geometryChanged(window->uuid());});
    connect(window, &UkuiWindow::maximizedChanged, this, [&, window](){Q_EMIT maximizedChanged(window->uuid());});
    connect(window, &UkuiWindow::fullscreenChanged, this, [&, window](){Q_EMIT fullscreenChanged(window->uuid());});
    connect(window, &UkuiWindow::skipSwitcherChanged, this, [&, window](){Q_EMIT skipSwitcherChanged(window->uuid());});

    Q_EMIT windowAdded(window->uuid());
    if(m_windowManagement->activeWindow() == window) {
        Q_EMIT activeWindowChanged(window->uuid());
    }
}

QStringList WaylandWindowManager::windows()
{
    return m_uuidToWindow.keys();
}

QIcon WaylandWindowManager::windowIcon(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->icon();
    }
    return {};
}

QString WaylandWindowManager::windowTitle(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->title();
    }
    return {};
}

bool WaylandWindowManager::skipTaskBar(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->skipTaskbar();
    }
    return false;
}

QString WaylandWindowManager::windowGroup(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->appId();
    }
    return {};
}

bool WaylandWindowManager::isMaximized(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isMaximized();
    }
    return false;
}

void WaylandWindowManager::maximizeWindow(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        m_uuidToWindow.value(wid.toUtf8())->requestToggleMaximized();
    }
}

bool WaylandWindowManager::isMinimized(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isMinimized();
    }
    return false;
}

void WaylandWindowManager::minimizeWindow(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        m_uuidToWindow.value(wid.toUtf8())->requestToggleMinimized();
    }
}

bool WaylandWindowManager::isKeepAbove(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isKeepAbove();
    }
    return false;
}

void WaylandWindowManager::keepAboveWindow(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        m_uuidToWindow.value(wid.toUtf8())->requestToggleKeepAbove();
    }
}

bool WaylandWindowManager::isOnAllDesktops(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isOnAllDesktops();
    }
    return false;
}

bool WaylandWindowManager::isOnCurrentDesktop(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->ukuiVirtualDesktops().contains(m_currentDesktop);
    }
    return false;
}

void WaylandWindowManager::activateWindow(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        m_uuidToWindow.value(wid.toUtf8())->requestActivate();
    }
}

QString WaylandWindowManager::currentActiveWindow()
{
    if(!m_windowManagement) {
        return {};
    }

    m_connection->roundtrip();
    auto window =  m_windowManagement->activeWindow();
    if(window) {
        return window->uuid();
    }
    return {};
}

void WaylandWindowManager::closeWindow(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        m_uuidToWindow.value(wid.toUtf8())->requestClose();
    }
}

void WaylandWindowManager::restoreWindow(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        auto window = m_uuidToWindow.value(wid.toUtf8());
        window->requestActivate();
        if (window->isMaximized()) {
            window->requestToggleMaximized();
        }
    }
}

bool WaylandWindowManager::isDemandsAttention(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isDemandingAttention();
    }
    return false;
}

void WaylandWindowManager::desktopCreated(UkuiQuick::WaylandClient::UkuiWorkspace *workspace, const QString &uuid)
{
    if(workspace->isActivated()) {
        setCurrentDesktop(workspace->uuid());
    }
    connect(workspace, &UkuiWorkspace::activated, this, [&, workspace](){
        setCurrentDesktop(workspace->uuid());
    });
}

void WaylandWindowManager::setCurrentDesktop(const QString &id)
{
    if(id != m_currentDesktop) {
        m_currentDesktop = id;
        Q_EMIT currentDesktopChanged();
    }
}

quint32 WaylandWindowManager::pid(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->pid();
    }
    return 0;
}

QString WaylandWindowManager::appId(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->appId();
    }
    return {};
}

QRect WaylandWindowManager::geometry(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->geometry();
    }
    return {};
}

void WaylandWindowManager::setStartupGeometry(const QString &wid, QQuickItem *item)
{
    if(!item || !item->window() || !m_uuidToWindow.contains(wid.toUtf8())) {
        return;
    }
    KWayland::Client::Surface *s = KWayland::Client::Surface::fromWindow(item->window());
    m_uuidToWindow.value(wid.toUtf8())->setStartupGeometry(s, {item->mapToScene({0, 0}).toPoint(), item->size().toSize()});
}

void WaylandWindowManager::setMinimizedGeometry(const QString &wid, QQuickItem *item)
{
    if(!item || !item->window() || !m_uuidToWindow.contains(wid.toUtf8())) {
        return;
    }
    KWayland::Client::Surface *s = KWayland::Client::Surface::fromWindow(item->window());
    m_uuidToWindow.value(wid.toUtf8())->setMinimizedGeometry(s, {item->mapToScene({0, 0}).toPoint(), item->size().toSize()});
}

void WaylandWindowManager::unsetMinimizedGeometry(const QString &wid, QQuickItem *item)
{
    if(!item || !item->window() || !m_uuidToWindow.contains(wid.toUtf8())) {
        return;
    }
    KWayland::Client::Surface *s = KWayland::Client::Surface::fromWindow(item->window());
    m_uuidToWindow.value(wid.toUtf8())->unsetMinimizedGeometry(s);
}

void WaylandWindowManager::activateWindowView(const QStringList &wids)
{
    if(qgetenv("XDG_SESSION_DESKTOP") == "kylin-wlcom" || qgetenv("DESKTOP_SESSION") == "kylin-wlcom") {
        if(wids.isEmpty()) {
            for(auto window : m_windows) {
                if(window->isHighlight()) {
                    window->unsetHightlight();
                }
            }
        } else {
            //wlcom只能高亮1个窗口
            if(m_uuidToWindow.contains(wids.first().toUtf8())) {
                return m_uuidToWindow.value(wids.first().toUtf8())->setHighlight();
            }
        }
    }
    AbstractWindowManager::activateWindowView(wids);
}

bool WaylandWindowManager::showingDesktop()
{
    if(!m_windowManagement) {
        return false;
    }

    m_connection->roundtrip();
    return  m_windowManagement->isShowingDesktop();
}

void WaylandWindowManager::setShowingDesktop(bool showing)
{
    if(m_windowManagement) {
        m_windowManagement->setShowingDesktop(showing);
    }
}

bool WaylandWindowManager::isFullscreen(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isFullscreen();
    }
    return false;
}

void WaylandWindowManager::moveWindowToDesktop(const QString& winId, const QString& currentDesktop, const QString& targetDesktop)
{
    if (currentDesktop == targetDesktop) {
        qWarning() << "moveWindowToDesktop failed, Window already on target desktop" << winId;
        return;
    }
    if (auto* window = m_uuidToWindow.value(winId.toUtf8())) {
        if (!targetDesktop.isEmpty()) {
            window->requestEnterVirtualDesktop(targetDesktop);
        }
        if (!currentDesktop.isEmpty()) {
            window->requestLeaveVirtualDesktop(currentDesktop);
        }
        return;
    }
    qWarning() << "moveWindowToDesktop failed, Window not found" << winId;
}

QStringList WaylandWindowManager::windowDesktops(const QString& winId)
{
    if (const auto* window = m_uuidToWindow.value(winId.toUtf8())) {
        return window->ukuiVirtualDesktops();
    }
    qWarning() << "get windowDesktops failed, Window not found" << winId;
    return {};
}

bool WaylandWindowManager::skipSwitcher(const QString& wid)
{
    if (const auto* window = m_uuidToWindow.value(wid.toUtf8())) {
        return window->skipSwitcher();
    }
    qWarning() << "skipSwitcher failed, Window not found" << wid;
    return false;
}

bool WaylandWindowManager::isMaximizable(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isMaximizeable();
    }
    return false;
}

bool WaylandWindowManager::isMinimizable(const QString &wid)
{
    if(m_uuidToWindow.contains(wid.toUtf8())) {
        return m_uuidToWindow.value(wid.toUtf8())->isMinimizeable();
    }
    return false;
}
}
