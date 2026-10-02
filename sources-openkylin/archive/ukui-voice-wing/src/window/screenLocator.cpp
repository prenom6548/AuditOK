/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * 通过 ukui_window_management Wayland 协议，获取当前活动窗口的几何信息，
 * 并据此判断该窗口落在哪块屏幕上，供语音输入法界面定位使用。
 *
 * 整体流程：
 *   1. 连接 Wayland，绑定 ukui_window_management 全局对象
 *   2. 监听 window_created，对每个窗口调用 create_window 建立跟踪
 *   3. 通过 state_changed / geometry / pid_changed 维护窗口状态
 *   4. screenForActiveWindow() 查询活动窗口坐标，再映射到 QScreen
 */

#include "screenLocator.h"
#include "logger.h"

#include <ukui-window-management-client-protocol.h>

#include <algorithm>
#include <unistd.h>

#include <QGuiApplication>
#include <QHash>
#include <QScreen>
#include <QSocketNotifier>

namespace {

constexpr uint32_t kStateActive = UKUI_WINDOW_STATE_ACTIVE;
constexpr uint32_t kStateMinimized = UKUI_WINDOW_STATE_MINIMIZED;

// 单个被跟踪窗口的协议状态（仅保留定位所需字段）
struct WindowState {
    uint32_t pid = 0;     // 所属进程，用于过滤语音输入法自身窗口
    uint32_t state = 0;   // 窗口状态位，含 active / minimized 等
    QRect geometry;       // 合成器坐标系下的绝对几何（多屏时 x/y 可跨屏）
};

// Wayland 连接及窗口缓存，生命周期与 ScreenLocator 单例一致
struct WaylandState {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    ukui_window_management *windowManagement = nullptr;
    QSocketNotifier *notifier = nullptr; // 将 Wayland fd 接入 Qt 事件循环

    uint32_t selfPid = 0;
    ukui_window *activeWindow = nullptr;           // 最近一次变为 active 的窗口
    QHash<ukui_window *, WindowState> windows;     // 已 create_window 的窗口表
};

WaylandState g_wayland;

// 判断窗口是否可作为“当前输入目标”：有坐标、非最小化、非本进程、且处于 active
bool isActiveWindow(const WindowState &state)
{
    if (!state.geometry.isValid()) {
        return false;
    }
    if (state.pid != 0 && state.pid == g_wayland.selfPid) {
        return false;
    }
    if ((state.state & kStateMinimized) != 0) {
        return false;
    }
    return (state.state & kStateActive) != 0;
}

// 阻塞式同步：确保在查询前收到合成器最新事件
void syncWayland()
{
    if (g_wayland.display == nullptr) {
        return;
    }
    // roundtrip 1：等待 registry 返回全局对象并完成 bind
    wl_display_roundtrip(g_wayland.display);
    // roundtrip 2：接收已存在窗口的 window_created 及初始状态
    wl_display_roundtrip(g_wayland.display);
}

// registry 回调：发现 ukui_window_management 全局对象并完成 bind
void registryGlobal(void * /*data*/, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    if (interface == nullptr || strcmp(interface, ukui_window_management_interface.name) != 0) {
        return;
    }

    const uint32_t bindVersion = std::min<uint32_t>(version, 1);
    g_wayland.windowManagement = static_cast<ukui_window_management *>(
        wl_registry_bind(registry, name, &ukui_window_management_interface, bindVersion));
}

void registryGlobalRemove(void * /*data*/, wl_registry * /*registry*/, uint32_t /*name*/) {}

const wl_registry_listener kRegistryListener = {
    registryGlobal,
    registryGlobalRemove,
};

// libwayland 要求每个可能被合成器发送的事件都有非空回调，否则直接 abort
void showDesktopChanged(void * /*data*/, ukui_window_management * /*management*/, uint32_t /*state*/) {}

void stackingOrderChanged(void * /*data*/, ukui_window_management * /*management*/, const char * /*uuids*/) {}

void windowTitleChanged(void * /*data*/, ukui_window * /*window*/, const char * /*title*/) {}

void windowAppIdChanged(void * /*data*/, ukui_window * /*window*/, const char * /*appId*/) {}

void windowThemedIconNameChanged(void * /*data*/, ukui_window * /*window*/, const char * /*name*/) {}

void windowInitialState(void * /*data*/, ukui_window * /*window*/) {}

void windowParentChanged(void * /*data*/, ukui_window * /*window*/, ukui_window * /*parent*/) {}

void windowIconChanged(void * /*data*/, ukui_window * /*window*/) {}

void windowVirtualDesktopEntered(void * /*data*/, ukui_window * /*window*/, const char * /*id*/) {}

void windowVirtualDesktopLeft(void * /*data*/, ukui_window * /*window*/, const char * /*id*/) {}

void windowApplicationMenu(void * /*data*/, ukui_window * /*window*/, const char * /*serviceName*/,
                          const char * /*objectPath*/) {}

void windowActivityEntered(void * /*data*/, ukui_window * /*window*/, const char * /*id*/) {}

void windowActivityLeft(void * /*data*/, ukui_window * /*window*/, const char * /*id*/) {}

void windowResourceNameChanged(void * /*data*/, ukui_window * /*window*/, const char * /*resourceName*/) {}

// ukui_window 事件：活动状态变化时更新 activeWindow 指针
void windowStateChanged(void * /*data*/, ukui_window *window, uint32_t state)
{
    if (!g_wayland.windows.contains(window)) {
        return;
    }

    WindowState &info = g_wayland.windows[window];
    info.state = state;
    if (isActiveWindow(info)) {
        g_wayland.activeWindow = window;
    } else if (g_wayland.activeWindow == window) {
        g_wayland.activeWindow = nullptr;
    }
}

// 窗口关闭/卸载：销毁协议对象并移出跟踪表
void windowUnmapped(void * /*data*/, ukui_window *window)
{
    if (g_wayland.activeWindow == window) {
        g_wayland.activeWindow = nullptr;
    }
    if (g_wayland.windows.contains(window)) {
        ukui_window_destroy(window);
        g_wayland.windows.remove(window);
    }
}

// 窗口几何变化：坐标为全局桌面坐标（非相对父窗口）
void windowGeometry(void * /*data*/, ukui_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    if (!g_wayland.windows.contains(window)) {
        return;
    }
    g_wayland.windows[window].geometry = QRect(x, y, static_cast<int>(width), static_cast<int>(height));
}

void windowPidChanged(void * /*data*/, ukui_window *window, uint32_t pid)
{
    if (!g_wayland.windows.contains(window)) {
        return;
    }
    g_wayland.windows[window].pid = pid;
}

const ukui_window_listener kWindowListener = {
    windowTitleChanged,
    windowAppIdChanged,
    windowStateChanged,
    windowThemedIconNameChanged,
    windowUnmapped,
    windowInitialState,
    windowParentChanged,
    windowGeometry,
    windowIconChanged,
    windowPidChanged,
    windowVirtualDesktopEntered,
    windowVirtualDesktopLeft,
    windowApplicationMenu,
    windowActivityEntered,
    windowActivityLeft,
    windowResourceNameChanged,
};

// 新窗口映射时：create_window 绑定 uuid，开始接收该窗口事件
void windowCreated(void * /*data*/, ukui_window_management * /*management*/, const char *uuid)
{
    if (g_wayland.windowManagement == nullptr || uuid == nullptr) {
        return;
    }

    ukui_window *window = ukui_window_management_create_window(g_wayland.windowManagement, uuid);
    if (window == nullptr) {
        return;
    }

    g_wayland.windows.insert(window, WindowState{});
    ukui_window_add_listener(window, &kWindowListener, nullptr);
}

const ukui_window_management_listener kWindowManagementListener = {
    showDesktopChanged,
    stackingOrderChanged,
    windowCreated,
};

void destroyWayland()
{
    if (g_wayland.notifier != nullptr) {
        g_wayland.notifier->setEnabled(false);
        delete g_wayland.notifier;
        g_wayland.notifier = nullptr;
    }

    for (auto it = g_wayland.windows.constBegin(); it != g_wayland.windows.constEnd(); ++it) {
        ukui_window_destroy(it.key());
    }
    g_wayland.windows.clear();
    g_wayland.activeWindow = nullptr;

    if (g_wayland.windowManagement != nullptr) {
        ukui_window_management_destroy(g_wayland.windowManagement);
        g_wayland.windowManagement = nullptr;
    }
    if (g_wayland.registry != nullptr) {
        wl_registry_destroy(g_wayland.registry);
        g_wayland.registry = nullptr;
    }
    if (g_wayland.display != nullptr) {
        wl_display_disconnect(g_wayland.display);
        g_wayland.display = nullptr;
    }
}

// 首次调用时建立独立 Wayland 连接（与 Qt 主连接并行）
bool initWayland()
{
    if (g_wayland.display != nullptr) {
        return g_wayland.windowManagement != nullptr;
    }

    g_wayland.selfPid = static_cast<uint32_t>(getpid());
    g_wayland.display = wl_display_connect(nullptr);
    if (g_wayland.display == nullptr) {
        VW_LOG(Logger::Level::Warning) << "ScreenLocator: failed to connect wayland display";
        return false;
    }

    g_wayland.registry = wl_display_get_registry(g_wayland.display);
    if (g_wayland.registry == nullptr) {
        VW_LOG(Logger::Level::Warning) << "ScreenLocator: failed to get registry";
        return false;
    }

    wl_registry_add_listener(g_wayland.registry, &kRegistryListener, nullptr);
    // roundtrip 1：等待 registry 返回全局对象并完成 bind
    if (wl_display_roundtrip(g_wayland.display) < 0) {
        VW_LOG(Logger::Level::Warning) << "ScreenLocator: registry roundtrip failed";
        return false;
    }

    if (g_wayland.windowManagement == nullptr) {
        VW_LOG(Logger::Level::Warning) << "ScreenLocator: ukui_window_management global not found";
        return false;
    }

    ukui_window_management_add_listener(g_wayland.windowManagement, &kWindowManagementListener, nullptr);
    // roundtrip 2：接收已存在窗口的 window_created 及初始状态
    if (wl_display_roundtrip(g_wayland.display) < 0) {
        VW_LOG(Logger::Level::Warning) << "ScreenLocator: window state roundtrip failed";
        return false;
    }

    // 后续窗口变化通过 Qt 事件循环异步 dispatch
    g_wayland.notifier = new QSocketNotifier(wl_display_get_fd(g_wayland.display), QSocketNotifier::Read);
    QObject::connect(g_wayland.notifier, &QSocketNotifier::activated, [](int) {
        if (g_wayland.display != nullptr) {
            wl_display_dispatch_pending(g_wayland.display);
        }
    });

    return true;
}

// 返回当前活动窗口的全局几何；优先用缓存的 activeWindow，否则遍历窗口表
QRect queryActiveWindowGeometry()
{
    syncWayland();

    if (g_wayland.activeWindow != nullptr) {
        const WindowState state = g_wayland.windows.value(g_wayland.activeWindow);
        if (isActiveWindow(state)) {
            return state.geometry;
        }
    }

    for (auto it = g_wayland.windows.constBegin(); it != g_wayland.windows.constEnd(); ++it) {
        if (isActiveWindow(it.value())) {
            g_wayland.activeWindow = it.key();
            return it.value().geometry;
        }
    }

    return QRect();
}

} // namespace

ScreenLocator &ScreenLocator::instance()
{
    static ScreenLocator locator;
    return locator;
}

ScreenLocator::~ScreenLocator()
{
    if (m_ready) {
        destroyWayland();
        m_ready = false;
    }
}

bool ScreenLocator::ensureInitialized()
{
    if (m_initAttempted) {
        return m_ready;
    }

    m_initAttempted = true;
    m_ready = initWayland();
    if (!m_ready) {
        VW_LOG(Logger::Level::Warning) << "ScreenLocator: wayland init failed";
    }
    return m_ready;
}

// 将活动窗口全局坐标映射到 QScreen：以窗口左上角判定所在屏，失败则按交集面积最大选取
QScreen *ScreenLocator::screenForGeometry(const QRect &geometry) const
{
    if (!geometry.isValid()) {
        return nullptr;
    }

    // 以左上角坐标判定窗口所在屏（跨屏窗口时比中心点更准确）
    if (QScreen *screen = QGuiApplication::screenAt(geometry.topLeft())) {
        return screen;
    }

    QScreen *bestScreen = nullptr;
    int bestArea = 0;
    for (QScreen *screen : QGuiApplication::screens()) {
        const QRect intersection = screen->geometry().intersected(geometry);
        const int area = intersection.width() * intersection.height();
        if (area > bestArea) {
            bestArea = area;
            bestScreen = screen;
        }
    }
    return bestScreen;
}

// 对外入口：返回活动窗口所在屏幕，获取失败时回退到主屏
QScreen *ScreenLocator::screenForActiveWindow()
{
    if (!ensureInitialized()) {
        VW_LOG(Logger::Level::Warning)
            << "ScreenLocator: wayland unavailable, cannot locate input window";
        return QGuiApplication::primaryScreen();
    }

    const QRect windowGeometry = queryActiveWindowGeometry();
    if (!windowGeometry.isValid()) {
        VW_LOG(Logger::Level::Info) << "ScreenLocator: no active window from protocol";
        return QGuiApplication::primaryScreen();
    }

    if (QScreen *screen = screenForGeometry(windowGeometry)) {
        return screen;
    }

    VW_LOG(Logger::Level::Warning)
        << "ScreenLocator: cannot map geometry to screen, geometry =" << windowGeometry;
    return QGuiApplication::primaryScreen();
}
