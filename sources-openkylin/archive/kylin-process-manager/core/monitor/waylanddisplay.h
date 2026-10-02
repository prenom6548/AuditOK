#pragma once

#include <mutex>
#include <shared_mutex>
#include <memory>  // 用于智能指针

// #include "ukui-window-management.h"
#include "plasma-window-management.h"
#include "wlr-output-management-unstable-v1_client.h"
#include "ukui-startup-v1.h"
#include "ukui-startup-v2.h"

#include <atomic>

#include <QThread>

class WaylandDisplay;

WaylandDisplay *waylandDisplay();

// 结构体定义保持不变
struct plasma_window
{
    org_kde_plasma_window *window;
    std::string uuid;  // 确保这是 std::string
    std::string title;
    std::string appId;
    uint64_t pid;
    uint32_t state;
    bool init;
};

struct out_put_state {
    bool enabled;
    int32_t x;
    int32_t y;
};

class WaylandDisplay : public QThread
{
    Q_OBJECT
public:
    WaylandDisplay();
    ~WaylandDisplay();

    // 线程安全的外部接口
    std::shared_ptr<plasma_window> getWindowByUuid(const std::string &uuid);
    std::unordered_map<std::string, std::shared_ptr<plasma_window>> getWindows();
    void avtivateWindow(const std::string &uuid);

    // 启动管理接口
    void setStartupGeometryForApp(int pid, int x, int y, int width, int height);
    void createStartupInfoForAppId(const std::string &app_id, int x, int y, int width, int height);
    void setPidForStartupInfoV2(const std::string &app_id, int pid);
    void setStartupGeometryForStartupInfoV2(const std::string &app_id, int x, int y, int width, int height);
    void finishSetupInfoV2(const std::string &app_id);

Q_SIGNALS:
    void windowAdded(const std::string &uuid);
    void windowRemoved(const std::string &uuid);
    void windowTitleChanged(const std::string &uuid, const std::string &title);
    void windowAppIdChanged(const std::string &uuid, const std::string &appId);
    void windowPidChanged(const std::string &uuid, uint64_t pid);
    void windowStateChanged(const std::string &uuid, uint32_t state);

protected:
    void run() override;

private:
    // 读写锁保护 m_windows
    mutable std::shared_mutex m_windowsMutex;
    
    // 修改存储类型为 shared_ptr
    std::unordered_map<std::string, std::shared_ptr<plasma_window>> m_windows;
    
    static wl_registry_listener m_registryListener;
    static void handle_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version);
    static void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);

    static org_kde_plasma_window_management_listener m_plasmaWindowManagementListener;
    static void handle_show_desktop_changed(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state);
    static void handle_window(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id);
    static void handle_stacking_order_changed(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, struct wl_array *ids);
    static void handle_stacking_order_uuid_changed(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, const char *uuids);
    static void handle_window_with_uuid(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id, const char *uuid);
    static void handle_stacking_order_changed_2(void *data, struct org_kde_plasma_window_management *org_kde_plasma_window_management);

    static org_kde_plasma_window_listener m_plasmaWindowListener;
    static void handle_title_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *title);
    static void handle_app_id_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *app_id);
    static void handle_state_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, uint32_t flags);
    static void handle_virtual_desktop_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, int32_t number);
    static void handle_themed_icon_name_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *name);
    static void handle_unmapped(void *data, struct org_kde_plasma_window *org_kde_plasma_window);
    static void handle_initial_state(void *data, struct org_kde_plasma_window *org_kde_plasma_window);
    static void handle_parent_window(void *data, struct org_kde_plasma_window *org_kde_plasma_window, struct org_kde_plasma_window *parent);
    static void handle_geometry(void *data, struct org_kde_plasma_window *org_kde_plasma_window, int32_t x, int32_t y, uint32_t width, uint32_t height);
    static void handle_icon_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window);
    static void handle_pid_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, uint32_t pid);
    static void handle_virtual_desktop_entered(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void handle_virtual_desktop_left(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *is);
    static void handle_application_menu(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *service_name, const char *object_path);
    static void handle_activity_entered(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void handle_activity_left(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void handle_resource_name_changed(void *data, struct org_kde_plasma_window *org_kde_plasma_window, const char *resource_name);
    static void handle_client_geometry(void *data, struct org_kde_plasma_window *org_kde_plasma_window, int32_t x, int32_t y, uint32_t width, uint32_t height);

    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;

    std::atomic<bool> m_stopEventQueueThread = false;

    // Wayland interfaces
    org_kde_plasma_window_management *m_plasmaWindowManagement = nullptr;
    // wayland::ukui_window_management_t m_ukuiWindowManagement;

    ukui_startup_management_v1 *m_startupManagement = nullptr;
    ukui_startup_management_v2 *m_startupManagementV2 = nullptr;
    std::unordered_map<std::string, ukui_startup_info_v2 *> m_startupInfoMap;

    bool connectToWayland();
    void disconnectWaylandDisplay();
    void initWaylandConnections();
    void readEventQueue();

    friend WaylandDisplay *waylandDisplay();

public:
    int getScreenMode();

Q_SIGNALS:
    void screenModeChanged(int screenMode);

private:
    static zwlr_output_head_v1_listener m_zwlrOutputHeadListener;

    static void output_handle_name(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *name);
    static void output_handle_description(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *description);
    static void output_handle_physical_size(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t width, int32_t height);
    static void output_handle_mode(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, struct zwlr_output_mode_v1 *mode);
    static void output_handle_enabled(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t enabled);
    static void output_handle_current_mode(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, struct zwlr_output_mode_v1 *mode);
    static void output_handle_position(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t x, int32_t y);
    static void output_handle_transform(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t transform);
    static void output_handle_scale(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, wl_fixed_t scale);
    static void output_handle_finished(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1);
    static void output_handle_make(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *make);
    static void output_handle_model(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *model);
    static void output_handle_serial_number(void *data, struct zwlr_output_head_v1 *zwlr_output_head_v1, const char *serial_number);

    zwlr_output_manager_v1* m_zwlrOutputManager = nullptr;

    static zwlr_output_manager_v1_listener m_zwlrOutputManagerListener;
    static void output_manager_handle_head(void* data, struct zwlr_output_manager_v1* zwlr_output_manager_v1, struct zwlr_output_head_v1* head);
    static void output_manager_handle_done(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1, uint32_t serial);
    static void output_manager_handle_finished(void *data, struct zwlr_output_manager_v1 *zwlr_output_manager_v1);

    std::vector<out_put_state> m_outputs;
    std::atomic<int> m_screenMode;
};