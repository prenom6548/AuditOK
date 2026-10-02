#include "waylanddisplay.h"
#include "loghelper.h"
#include "plasma-window-management.h"
#include "ukui-startup-v1.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <poll.h>
#include <thread>

#include <QDebug>

namespace fs = std::filesystem;

static WaylandDisplay *s_waylandDisplay = nullptr;
WaylandDisplay *waylandDisplay()
{
    if (!s_waylandDisplay) {
        s_waylandDisplay = new WaylandDisplay();
        s_waylandDisplay->start();
    }

    return s_waylandDisplay;
}

WaylandDisplay::WaylandDisplay()
{
    m_stopEventQueueThread = false;
    m_screenMode = 0;
}

WaylandDisplay::~WaylandDisplay()
{
    m_stopEventQueueThread = true;
    wait();

    if (m_plasmaWindowManagement) {
        org_kde_plasma_window_management_destroy(m_plasmaWindowManagement);
        m_plasmaWindowManagement = nullptr;
    }

    // 清理窗口 - 使用智能指针，自动释放
    {
        std::unique_lock<std::shared_mutex> lock(m_windowsMutex);
        for (auto &pair : m_windows) {
            if (pair.second && pair.second->window) {
                org_kde_plasma_window_destroy(pair.second->window);
            }
        }
        m_windows.clear(); // 自动释放 shared_ptr
    }

    // 清理启动信息
    for (auto &pair : m_startupInfoMap) {
        if (pair.second) {
            ukui_startup_info_v2_destroy(pair.second);
        }
    }
    m_startupInfoMap.clear();

    m_outputs.clear();

    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }

    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
}

std::shared_ptr<plasma_window> WaylandDisplay::getWindowByUuid(const std::string &uuid)
{
    std::shared_lock<std::shared_mutex> lock(m_windowsMutex);
    auto it = m_windows.find(uuid);
    if (it != m_windows.end()) {
        return it->second; // 返回 shared_ptr，线程安全
    }
    return nullptr;
}

std::unordered_map<std::string, std::shared_ptr<plasma_window>> WaylandDisplay::getWindows()
{
    std::shared_lock<std::shared_mutex> lock(m_windowsMutex);
    return m_windows; // 返回拷贝，但值是 shared_ptr
}

void WaylandDisplay::avtivateWindow(const std::string &uuid)
{
    std::shared_ptr<plasma_window> window = getWindowByUuid(uuid);
    if (window && window->window) {
        org_kde_plasma_window_set_state(window->window,
                                        ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE,
                                        ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE);
    } else {
        PRINT_DBG("Window with UUID: %s\n", uuid.c_str());
    }
}

void WaylandDisplay::setStartupGeometryForApp(int pid, int x, int y, int width, int height)
{
    if (!m_startupManagement)
        return;

    ukui_startup_management_v1_set_startup_geometry(m_startupManagement, pid, x, y, width, height);
}

void WaylandDisplay::createStartupInfoForAppId(const std::string &app_id, int x, int y, int width, int height)
{
    if (!m_startupManagementV2)
        return;

    auto *startupInfoV2 = ukui_startup_management_v2_create_startup_info(m_startupManagementV2);
    if (!startupInfoV2)
        return;

    m_startupInfoMap[app_id] = startupInfoV2;
    ukui_startup_info_v2_set_appid(startupInfoV2, app_id.c_str());
    ukui_startup_info_v2_set_startup_geometry(startupInfoV2, x, y, width, height);
    wl_display_flush(m_display);
}

void WaylandDisplay::setPidForStartupInfoV2(const std::string &app_id, int pid)
{
    if (!m_startupManagementV2)
        return;

    auto it = m_startupInfoMap.find(app_id);
    if (it == m_startupInfoMap.end())
        return;

    ukui_startup_info_v2_set_pid(it->second, pid);
    wl_display_roundtrip(m_display);
}

void WaylandDisplay::setStartupGeometryForStartupInfoV2(const std::string &app_id, int x, int y, int width, int height)
{
    if (!m_startupManagementV2)
        return;

    auto it = m_startupInfoMap.find(app_id);
    if (it == m_startupInfoMap.end())
        return;

    ukui_startup_info_v2_set_startup_geometry(it->second, x, y, width, height);
    wl_display_roundtrip(m_display);
}

void WaylandDisplay::finishSetupInfoV2(const std::string &app_id)
{
    if (!m_startupManagementV2)
        return;

    auto it = m_startupInfoMap.find(app_id);
    if (it == m_startupInfoMap.end())
        return;

    ukui_startup_info_v2_destroy(it->second);
    m_startupInfoMap.erase(it);
}

void WaylandDisplay::handle_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
    // 保持不变
    WaylandDisplay *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    if (strcmp(interface, zwlr_output_manager_v1_interface.name) == 0) {
        self->m_zwlrOutputManager = static_cast<zwlr_output_manager_v1*>(wl_registry_bind(wl_registry, name, &zwlr_output_manager_v1_interface, 1));
        zwlr_output_manager_v1_add_listener(self->m_zwlrOutputManager, &WaylandDisplay::m_zwlrOutputManagerListener, self);
    }

    if (strcmp(interface, org_kde_plasma_window_management_interface.name) == 0) {
        self->m_plasmaWindowManagement = static_cast<org_kde_plasma_window_management *>(wl_registry_bind(self->m_registry, name, &org_kde_plasma_window_management_interface,
                                                                                                          std::min<uint32_t>(version, org_kde_plasma_window_management_interface.version)));
        org_kde_plasma_window_management_add_listener(self->m_plasmaWindowManagement, &WaylandDisplay::m_plasmaWindowManagementListener, self);
    } else if (strcmp(interface, ukui_startup_management_v1_interface.name) == 0) {
        self->m_startupManagement = static_cast<ukui_startup_management_v1 *>(wl_registry_bind(self->m_registry, name, &ukui_startup_management_v1_interface,
                                                                                               std::min<uint32_t>(version, ukui_startup_management_v1_interface.version)));
    } else if (strcmp(interface, ukui_startup_management_v2_interface.name) == 0) {
        self->m_startupManagementV2 = static_cast<ukui_startup_management_v2 *>(wl_registry_bind(self->m_registry, name, &ukui_startup_management_v2_interface,
                                                                                                 std::min<uint32_t>(version, ukui_startup_management_v2_interface.version)));
    }
}

void WaylandDisplay::handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
    // This function can be left empty if you don't need to handle global removal
    // or you can implement logic to handle the removal of globals if necessary.
}

wl_registry_listener WaylandDisplay::m_registryListener = {
    .global = WaylandDisplay::handle_global,
    .global_remove = WaylandDisplay::handle_global_remove,
};

void WaylandDisplay::handle_show_desktop_changed(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state)
{
}

void WaylandDisplay::handle_window(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id)
{
}

void WaylandDisplay::handle_stacking_order_changed(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, struct wl_array *ids)
{
}

void WaylandDisplay::handle_stacking_order_uuid_changed(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, const char *uuids)
{
}

void WaylandDisplay::handle_window_with_uuid(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id, const char *uuid)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self || !self->m_plasmaWindowManagement) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    if (!uuid || !*uuid) {
        PRINT_NOTICE("Invalid UUID provided.\n");
        return;
    }

    std::string uuid_str(uuid);
    org_kde_plasma_window *window = org_kde_plasma_window_management_get_window_by_uuid(self->m_plasmaWindowManagement, uuid);
    if (!window) {
        PRINT_NOTICE("Window with UUID %s not found.\n", uuid);
        return;
    }

    // 使用 make_shared 创建智能指针
    auto plasmaWindow = std::make_shared<struct plasma_window>();
    plasmaWindow->uuid = uuid_str; // 复制字符串
    plasmaWindow->pid = -1;
    plasmaWindow->state = -1;
    plasmaWindow->window = window;
    plasmaWindow->init = false;

    {
        std::unique_lock<std::shared_mutex> lock(self->m_windowsMutex);
        self->m_windows[uuid_str] = plasmaWindow;
    }

    org_kde_plasma_window_add_listener(plasmaWindow->window, &WaylandDisplay::m_plasmaWindowListener, data);
    wl_display_roundtrip(self->m_display);
}

void WaylandDisplay::handle_stacking_order_changed_2(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management)
{
}

org_kde_plasma_window_management_listener WaylandDisplay::m_plasmaWindowManagementListener = {
    .show_desktop_changed = WaylandDisplay::handle_show_desktop_changed,
    .window = WaylandDisplay::handle_window,
    .stacking_order_changed = WaylandDisplay::handle_stacking_order_changed,
    .stacking_order_uuid_changed = WaylandDisplay::handle_stacking_order_uuid_changed,
    .window_with_uuid = WaylandDisplay::handle_window_with_uuid,
    .stacking_order_changed_2 = WaylandDisplay::handle_stacking_order_changed_2};

void WaylandDisplay::handle_title_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *title)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    std::shared_lock<std::shared_mutex> lock(self->m_windowsMutex);

    auto it = std::find_if(self->m_windows.begin(), self->m_windows.end(),
                           [org_kde_plasma_window](const auto &pair) {
                               return pair.second && pair.second->window == org_kde_plasma_window;
                           });

    if (it != self->m_windows.end()) {
        std::string title_str = title ? title : "";
        it->second->title = title_str;
        QString qstr_uuid = QString::fromStdString(it->second->uuid);
        QString qstr_title = QString::fromStdString(title_str);
        QMetaObject::invokeMethod(
            self,
            [self, qstr_uuid, qstr_title]() {
                Q_EMIT self->windowTitleChanged(qstr_uuid.toStdString(), qstr_title.toStdString());
            },
            Qt::QueuedConnection
        );
    } else {
        PRINT_DBG("Window not found for title change.\n");
    }
}

void WaylandDisplay::handle_app_id_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *app_id)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    std::shared_lock<std::shared_mutex> lock(self->m_windowsMutex);

    auto it = std::find_if(self->m_windows.begin(), self->m_windows.end(),
                           [org_kde_plasma_window](const auto &pair) {
                               return pair.second && pair.second->window == org_kde_plasma_window;
                           });

    if (it != self->m_windows.end()) {
        std::string app_id_str = app_id ? app_id : "";
        it->second->appId = app_id_str;
        QString qstr_uuid = QString::fromStdString(it->second->uuid);
        QString qstr_appid = QString::fromStdString(app_id_str);
        QMetaObject::invokeMethod(
            self,
            [self, qstr_uuid, qstr_appid]() {
                Q_EMIT self->windowAppIdChanged(qstr_uuid.toStdString(), qstr_appid.toStdString());
            },
            Qt::QueuedConnection
        );
    } else {
        PRINT_DBG("Window not found for app ID change.\n");
    }
}

void WaylandDisplay::handle_state_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, uint32_t flags)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    std::shared_lock<std::shared_mutex> lock(self->m_windowsMutex);

    auto it = std::find_if(self->m_windows.begin(), self->m_windows.end(),
                           [org_kde_plasma_window](const auto &pair) {
                               return pair.second && pair.second->window == org_kde_plasma_window;
                           });

    if (it != self->m_windows.end()) {
        it->second->state = flags;
        if (it->second->init) {
            QString qstr_uuid = QString::fromStdString(it->second->uuid);
            uint32_t state = it->second->state;
            QMetaObject::invokeMethod(
                self,
                [self, qstr_uuid, state]() {
                    Q_EMIT self->windowStateChanged(qstr_uuid.toStdString(), state);
                },
                Qt::QueuedConnection
            );
        }
    } else {
        PRINT_DBG("Window not found for state change.\n");
    }
}

void WaylandDisplay::handle_virtual_desktop_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, int32_t number)
{
}

void WaylandDisplay::handle_themed_icon_name_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *name)
{
}

void WaylandDisplay::handle_unmapped(void *data, struct org_kde_plasma_window *org_kde_plasma_window)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    std::string uuid_to_remove;
    std::shared_ptr<plasma_window> window_to_delete;

    {
        std::unique_lock<std::shared_mutex> lock(self->m_windowsMutex);

        auto it = std::find_if(self->m_windows.begin(), self->m_windows.end(),
                               [org_kde_plasma_window](const auto &pair) {
                                   return pair.second && pair.second->window == org_kde_plasma_window;
                               });

        if (it != self->m_windows.end()) {
            uuid_to_remove = it->second->uuid;
            window_to_delete = it->second; // 保存 shared_ptr
            self->m_windows.erase(it);
        }
    }

    // 在锁外执行清理
    if (window_to_delete && window_to_delete->window) {
        org_kde_plasma_window_destroy(window_to_delete->window);
        QString qstr_uuid = QString::fromStdString(uuid_to_remove);
        QMetaObject::invokeMethod(
            self,
            [self, qstr_uuid]() {
                Q_EMIT self->windowRemoved(qstr_uuid.toStdString());
            },
            Qt::QueuedConnection
        );

        // window_to_delete 自动释放
    } else {
        PRINT_DBG("Window not found for unmapped event.\n");
    }
}

void WaylandDisplay::handle_initial_state(void *data, struct org_kde_plasma_window *org_kde_plasma_window)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    std::shared_lock<std::shared_mutex> lock(self->m_windowsMutex);

    auto it = std::find_if(self->m_windows.begin(), self->m_windows.end(),
                           [org_kde_plasma_window](const auto &pair) {
                               return pair.second && pair.second->window == org_kde_plasma_window;
                           });

    if (it != self->m_windows.end()) {
        it->second->init = true;
        QString qstr_uuid = QString::fromStdString(it->second->uuid);
        uint32_t state = it->second->state;
        QMetaObject::invokeMethod(
            self,
            [self, qstr_uuid, state]() {
                Q_EMIT self->windowAdded(qstr_uuid.toStdString());
                Q_EMIT self->windowStateChanged(qstr_uuid.toStdString(), state);
            },
            Qt::QueuedConnection
        );
    } else {
        PRINT_DBG("Window not found for title change.\n");
    }
}

void WaylandDisplay::handle_parent_window(void *data, struct org_kde_plasma_window *org_kde_plasma_window, struct org_kde_plasma_window *parent)
{
}

void WaylandDisplay::handle_geometry(void *data, struct org_kde_plasma_window *org_kde_plasma_window, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
}

void WaylandDisplay::handle_icon_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window)
{
}

void WaylandDisplay::handle_pid_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, uint32_t pid)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    std::shared_lock<std::shared_mutex> lock(self->m_windowsMutex);

    auto it = std::find_if(self->m_windows.begin(), self->m_windows.end(),
                           [org_kde_plasma_window](const auto &pair) {
                               return pair.second && pair.second->window == org_kde_plasma_window;
                           });

    if (it != self->m_windows.end()) {
        it->second->pid = pid;
        QString qstr_uuid = QString::fromStdString(it->second->uuid);
        QMetaObject::invokeMethod(
            self,
            [self, qstr_uuid, pid]() {
                Q_EMIT self->windowPidChanged(qstr_uuid.toStdString(), pid);
            },
            Qt::QueuedConnection
        );
    } else {
        PRINT_DBG("Window not found for PID change.\n");
    }
}

void WaylandDisplay::handle_virtual_desktop_entered(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
}

void WaylandDisplay::handle_virtual_desktop_left(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *is)
{
}

void WaylandDisplay::handle_application_menu(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *service_name, const char *object_path)
{
}

void WaylandDisplay::handle_activity_entered(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
}

void WaylandDisplay::handle_activity_left(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
}

void WaylandDisplay::handle_resource_name_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *resource_name)
{
}

void WaylandDisplay::handle_client_geometry(void *data, struct org_kde_plasma_window *org_kde_plasma_window, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
}

org_kde_plasma_window_listener WaylandDisplay::m_plasmaWindowListener = {
    .title_changed = handle_title_changed,
    .app_id_changed = handle_app_id_changed,
    .state_changed = handle_state_changed,
    .virtual_desktop_changed = handle_virtual_desktop_changed,
    .themed_icon_name_changed = handle_themed_icon_name_changed,
    .unmapped = handle_unmapped,
    .initial_state = handle_initial_state,
    .parent_window = handle_parent_window,
    .geometry = handle_geometry,
    .icon_changed = handle_icon_changed,
    .pid_changed = handle_pid_changed,
    .virtual_desktop_entered = handle_virtual_desktop_entered,
    .virtual_desktop_left = handle_virtual_desktop_left,
    .application_menu = handle_application_menu,
    .activity_entered = handle_activity_entered,
    .activity_left = handle_activity_left,
    .resource_name_changed = handle_resource_name_changed,
    .client_geometry = handle_client_geometry};

bool WaylandDisplay::connectToWayland()
{
    auto *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (!xdg_runtime_dir) {
        // qDebug() << "XDG_RUNTIME_DIR environment variable is not set.";
        PRINT_NOTICE("XDG_RUNTIME_DIR environment variable is not set.\n");
        return false;
    }

    std::string runtime_dir = xdg_runtime_dir;
    std::vector<std::string> socket_names;
    while (true) {
        for (const auto &entry : fs::directory_iterator(runtime_dir)) {
            if (entry.is_socket()) {
                std::string name = entry.path().filename();
                if (name.find("wayland-") == 0) {
                    socket_names.push_back(name);
                    break;
                }
            }
        }
        if (!socket_names.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    m_display = wl_display_connect(socket_names[0].c_str());

    if (!m_display)
        return false;

    m_registry = wl_display_get_registry(m_display);

    wl_registry_add_listener(m_registry, &m_registryListener, this);

    wl_display_roundtrip(m_display);
    return true;
}

void WaylandDisplay::disconnectWaylandDisplay()
{
    if (m_startupManagement) {
        ukui_startup_management_v1_destroy(m_startupManagement);
        m_startupManagement = nullptr;
    }

    if (m_startupManagementV2) {
        ukui_startup_management_v2_destroy(m_startupManagementV2);
        m_startupManagementV2 = nullptr;
    }

    if (m_plasmaWindowManagement) {
        org_kde_plasma_window_management_destroy(m_plasmaWindowManagement);
        m_plasmaWindowManagement = nullptr;
    }

    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }

    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
}

void WaylandDisplay::run()
{
    while (!m_stopEventQueueThread) {
        if (!m_display) {
            if (connectToWayland()) {
                PRINT_NOTICE("Connected to Wayland display.\n");
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }

        int fd = wl_display_get_fd(m_display);
        pollfd pfd = {fd, POLLIN, 0};

        if (wl_display_prepare_read(m_display) != 0) {
            wl_display_dispatch_pending(m_display);
            continue;
        }

        wl_display_flush(m_display);

        int ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            wl_display_cancel_read(m_display);
            continue;
        }

        if (pfd.revents & POLLIN) {
            if (wl_display_read_events(m_display) == -1) {
                // qDebug() << "Wayland connection lost. Reconnecting...\n";
                PRINT_NOTICE("Wayland connection lost. Reconnecting...\n");
                disconnectWaylandDisplay();
                continue;
            }
            wl_display_dispatch_pending(m_display);
        } else {
            wl_display_cancel_read(m_display);
        }
    }
}

int WaylandDisplay::getScreenMode()
{
    return m_screenMode;
}

void WaylandDisplay::output_handle_name(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *name)
{
}

void WaylandDisplay::output_handle_description(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *description)
{
}

void WaylandDisplay::output_handle_physical_size(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t width, int32_t height)
{
}

void WaylandDisplay::output_handle_mode(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, struct zwlr_output_mode_v1 *mode)
{
}

void WaylandDisplay::output_handle_enabled(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t enabled)
{
    static_cast<out_put_state *>(data)->enabled = enabled;
}

void WaylandDisplay::output_handle_current_mode(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, struct zwlr_output_mode_v1 *mode)
{
}

void WaylandDisplay::output_handle_position(void* data, struct zwlr_output_head_v1* zwlr_output_head_v1, int32_t x, int32_t y)
{
    static_cast<out_put_state*>(data)->x = x;
    static_cast<out_put_state*>(data)->y = y;
}

void WaylandDisplay::output_handle_transform(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t transform)
{
}

void WaylandDisplay::output_handle_scale(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, wl_fixed_t scale)
{
}

void WaylandDisplay::output_handle_finished(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1)
{
}

void WaylandDisplay::output_handle_make(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *make)
{
}

void WaylandDisplay::output_handle_model(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *model)
{
}

void WaylandDisplay::output_handle_serial_number(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *serial_number)
{
}

zwlr_output_head_v1_listener WaylandDisplay::m_zwlrOutputHeadListener = {
    .name = output_handle_name,
    .description = output_handle_description,
    .physical_size = output_handle_physical_size,
    .mode = output_handle_mode,
    .enabled = output_handle_enabled,
    .current_mode = output_handle_current_mode,
    .position = output_handle_position,
    .transform = output_handle_transform,
    .scale = output_handle_scale,
    .finished = output_handle_finished,
    .make = output_handle_make,
    .model = output_handle_model,
    .serial_number = output_handle_serial_number};

void WaylandDisplay::output_manager_handle_head(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1, struct zwlr_output_head_v1 *zwlr_output_head_v1)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        PRINT_NOTICE("WaylandDisplay instance is null.\n");
        return;
    }

    self->m_outputs.emplace_back();
    zwlr_output_head_v1_add_listener(zwlr_output_head_v1, &m_zwlrOutputHeadListener, &self->m_outputs.back());
}

void WaylandDisplay::output_manager_handle_done(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1, uint32_t serial)
{
    auto *self = static_cast<WaylandDisplay *>(data);
    if (!self) {
        return;
    }

    std::vector<out_put_state> enabled;
    enabled.reserve(self->m_outputs.size());
    for (const auto& out : self->m_outputs) {
        if (out.enabled) enabled.push_back(out);
    }

    if (enabled.empty()) {
        self->m_screenMode = 3;
    } else if (enabled.size() == 1) {
        self->m_screenMode = 0;
    } else {
        bool all_same = std::all_of(enabled.begin(), enabled.end(),
            [&](const out_put_state& o) {
                return o.x == enabled[0].x && o.y == enabled[0].y;
            });
        if (all_same){
            self->m_screenMode = 2;
        } else {
            self->m_screenMode = 1;
        }
    }
    int screenMode = self->m_screenMode;
    QMetaObject::invokeMethod(
        self,
        [self, screenMode]() {
            Q_EMIT self->screenModeChanged(screenMode);
        },
        Qt::QueuedConnection
    );
}

void WaylandDisplay::output_manager_handle_finished(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1)
{
}

zwlr_output_manager_v1_listener WaylandDisplay::m_zwlrOutputManagerListener = {
    .head = output_manager_handle_head,
    .done = output_manager_handle_done,
    .finished = output_manager_handle_finished,
};
