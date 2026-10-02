/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */

#include "windowsmanager.h"
#include <mutex>
#include <XdgDesktopFile>
#include <QDebug>

#ifdef KWINEFFECT
#include <kwineffects.h>
#endif
#include "screens-manager.h"

UkuiSearch::ApplicationInfo *WindowsManager::getApplicationInfo()
{
    static UkuiSearch::ApplicationInfo appInfo;
    return &appInfo;
}

QString WindowsManager::desktopFileFromPid(uint pid)
{
    QString desktopFile;
    getApplicationInfo()->tranPidToDesktopFp(pid, desktopFile);
    return desktopFile;
}

WindowsManager *WindowsManager::getInstance()
{
    static WindowsManager s_interface;
    return &s_interface;
}

WindowsManager::WindowsManager(QObject *parent) : QObject(parent)
{
    getApplicationInfo();
    m_windowManager = UkuiQuick::WindowManager::self();
    connect(m_windowManager, &UkuiQuick::WindowManager::windowAdded, this, [this](QString wid) {
        if (!m_windowManager->skipSwitcher(wid)) {
            Q_EMIT appWindowAdd(wid);
        }
    }, Qt::QueuedConnection);
    connect(m_windowManager, &UkuiQuick::WindowManager::windowRemoved, this, &WindowsManager::appWindowRemoved, Qt::QueuedConnection);
    connect(m_windowManager, &UkuiQuick::WindowManager::activeWindowChanged, this, &WindowsManager::appWindowActivated, Qt::QueuedConnection);
    connect(m_windowManager, &UkuiQuick::WindowManager::titleChanged, this, &WindowsManager::titleChanged, Qt::QueuedConnection);
    connect(m_windowManager, &UkuiQuick::WindowManager::iconChanged, this, &WindowsManager::iconChanged, Qt::QueuedConnection);
    connect(m_windowManager, &UkuiQuick::WindowManager::geometryChanged, this, &WindowsManager::geometryChanged, Qt::QueuedConnection);
#ifdef KWINEFFECT
    QObject::connect(KWin::effects, &KWin::EffectsHandler::desktopPresenceChanged, this, [this](KWin::EffectWindow *win, int oldD, int newD) {
        Q_UNUSED(oldD);
        Q_UNUSED(newD);
        if (!win || !win->parent()) {
            return;
        }
        emit appWindowDesktopChanged(win->parent()->property("windowId").toString());
    });
#else
    connect(m_windowManager, &UkuiQuick::WindowManager::windowDesktopChanged, this, &WindowsManager::appWindowDesktopChanged, Qt::QueuedConnection);
#endif
}

void WindowsManager::removeAppWindow(const QString &windowId)
{
    m_windowManager->closeWindow(windowId);
}

void WindowsManager::activateAppWindow(const QString &windowId)
{
    m_windowManager->activateWindow(windowId);
}

void WindowsManager::moveAppWindowToDesktop(const QString &windowId, const QString &srcDesktop, const QString &destDesktop)
{
    m_windowManager->moveWindowToDesktop(windowId, srcDesktop, destDesktop);
}

QStringList WindowsManager::windows() const
{
#ifdef KWINEFFECT
    // get windowlist with stackingOrder
    QStringList windows;
    const auto& effectWindowList = KWin::effects->stackingOrder();
    for (auto iter = effectWindowList.rbegin(); iter != effectWindowList.rend(); iter++) {
        windows.append((*iter)->parent()->property("windowId").toString());
    }
    return windows;
#else
    return m_windowManager->windows();
#endif
}

bool WindowsManager::skipSwitcher(const QString &windowId) const
{
    return m_windowManager->skipSwitcher(windowId);
}

/**
 * @brief WindowsManager::hasModalChildWindow 是否有模态子窗口，主要用于X环境工作区预览图绘制
 * @param windowId
 * @return bool wlcom环境返回false
 */
bool WindowsManager::hasModalChildWindow(const QString &windowId) const
{
#ifdef KWINEFFECT
    auto* window = KWin::effects->findWindow(windowId);
    if (window == nullptr) {
        return false;
    }
    return window->findModal() != nullptr;
#else
    return false;
#endif
}

QVariant WindowsManager::windowIcon(const QString &wid) const
{
    QIcon icon = m_windowManager->windowIcon(wid);
    if (!icon.isNull()) {
        return icon;
    }
    // UkuiQuick::WindowManager未找到应用图标，则通过ukui-search接口查找desktop文件，进而获取应用图标
    QString desktopFile = resolveDesktopFile(wid);
    XdgDesktopFile xdf;
    if (!xdf.load(desktopFile)) {
        qWarning() << "[WindowsManager] empty windowIcon, load desktop file failed:" << desktopFile;
        return icon;
    } else {
        return xdf.iconName();
    }
}

QString WindowsManager::windowTitle(const QString &wid) const
{
    QString title = m_windowManager->windowTitle(wid);
    if (!title.isEmpty()) {
        return title;
    }

    // 窗口标题为空时，显示应用名称
    QString desktopFile = resolveDesktopFile(wid);
    XdgDesktopFile xdf;
    if (!xdf.load(desktopFile)) {
        qWarning() << "[WindowsManager] empty windowTitle, load desktop file failed:" << desktopFile;
        return title;
    } else {
        return xdf.localizedValue("Name").toString();
    }
}

QRect WindowsManager::geometry(const QString &wid) const
{
    return m_windowManager->geometry(wid);
}

bool WindowsManager::isMinimized(const QString &wid) const
{
    return m_windowManager->isMinimized(wid);
}

QStringList WindowsManager::windowDesktops(const QString &wid) const
{
    return m_windowManager->windowDesktops(wid);
}

QString WindowsManager::windowScreen(const QString &wid) const
{
#ifdef KWINEFFECT
    auto* effectWindow = KWin::effects->findWindow(wid);
    if (effectWindow == nullptr) {
        return "";
    }
    if (effectWindow->screen()) {
        return effectWindow->screen()->name();
    } else {
        return "";
    }
#else
    // TODO
    return "";
#endif
}

int WindowsManager::windowScreenIndex(const QString &wid) const
{
#if 0
    // wait for kywc primaryOutput
    QString screenName = windowScreen(wid);
    QList<Screen *> screenList = ScreensManager::instance()->screens();
    for (int i = 0; i < screenList.size(); i++) {
        Screen *screen = screenList.at(i);
        if (screen == nullptr || screen->internal() == nullptr) {
            continue;
        }
        if (screenName == screen->internal()->name()) {
            return i;
        }
    }
    return 0;
#endif
    QRect windowRect = m_windowManager->geometry(wid);
    QList<Screen *> screenList = ScreensManager::instance()->screens();
    int maxArea = -1;
    int maxOverlapScreenInx = 0;
    for (int i = 0; i < screenList.size(); ++i) {
        Screen *screen = screenList.at(i);
        if (screen == nullptr) {
            continue;
        }

        QRect r = screen->geometry().intersected(windowRect);
        if (r.isEmpty()) {
            continue;
        }

        if (r == windowRect) {
            return i;
        }

        int area = r.width() * r.height();
        if (area > maxArea) {
            maxArea = area;
            maxOverlapScreenInx = i;
        }
    }
    return maxOverlapScreenInx;
}

QString WindowsManager::resolveDesktopFile(const QString &wid) const
{
    QString desktopFile;
    getApplicationInfo()->desktopFilePathFromName(m_windowManager->appId(wid), desktopFile);
    if (desktopFile.isEmpty()) {
        desktopFile = desktopFileFromPid(m_windowManager->pid(wid));
    }
    return desktopFile;
}
