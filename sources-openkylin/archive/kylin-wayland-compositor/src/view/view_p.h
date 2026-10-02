// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _VIEW_P_H_
#define _VIEW_P_H_

#include <kywc/log.h>

#include "server.h"
#include "view/view.h"
#include "view/workspace.h"

#define VIEW_DEFAULT_GAP 100

struct view_mode_interface {
    const char *name;

    void (*view_map)(struct view *view);
    void (*view_unmap)(struct view *view);

    void (*view_request_move)(struct view *view, int x, int y);
    void (*view_request_resize)(struct view *view, struct kywc_box *geometry);
    void (*view_request_minimized)(struct view *view, bool minimized);
    void (*view_request_maximized)(struct view *view, bool maximized,
                                   struct kywc_output *kywc_output);
    void (*view_request_fullscreen)(struct view *view, bool fullscreen,
                                    struct kywc_output *kywc_output);
    void (*view_request_tiled)(struct view *view, enum kywc_tile tile,
                               struct kywc_output *kywc_output);
    void (*view_request_activate)(struct view *view);

    void (*view_request_show_menu)(struct view *view, struct seat *seat, int x, int y);

    void (*view_request_show_tile_flyout)(struct view *view, struct seat *seat,
                                          struct kywc_box *box);

    void (*view_request_show_tile_linkage_bar)(struct view *view, uint32_t edges);

    void (*view_click)(struct seat *seat, struct view *view, uint32_t button, bool pressed,
                       enum click_state state);
    void (*view_hover)(struct seat *seat, struct view *view);

    void (*view_mode_enter)(void);
    void (*view_mode_leave)(void);

    void (*mode_destroy)(void);
};

enum minimize_effect_type {
    MINIMIZE_EFFECT_TYPE_SCALE,
    MINIMIZE_EFFECT_TYPE_MAGIC_LAMP,
    MINIMIZE_EFFECT_TYPE_NUM,
};

struct view_mode {
    struct wl_list link;
    const struct view_mode_interface *impl;
};

struct view_manager_impl {
    void (*get_startup_geometry)(struct view *view, void *data);
    void (*set_tiled)(struct view *view, enum kywc_tile tile, struct kywc_output *kywc_output);
    void (*get_tiled_geometry)(struct view *view, struct kywc_box *geometry,
                               struct kywc_output *kywc_output, enum kywc_tile tile);
    void (*fix_geometry)(struct view *view, struct kywc_box *geo, struct kywc_output *kywc_output);
    void (*show_tile_assist)(struct view *view, struct seat *seat, struct kywc_output *kywc_output);
    void (*show_hang_window)(struct view *view);
    action_effect_options_adjust_func_t action_effect_options_adjust;
};

struct view_manager {
    struct server *server;
    struct wl_list views;

    struct wlr_xdg_foreign_registry *foreign_registry;

    struct view *global_authentication_view;
    struct view *desktop;
    struct view *highlight;

    struct {
        struct wl_signal new_view;
        struct wl_signal new_mapped_view;
        struct wl_signal show_desktop;
        struct wl_signal activate_view;
        struct wl_signal highlight;
    } events;

    struct view_layer layers[LAYER_NUMBER];

    // TODO: views keyboard focused when multi-seat
    struct {
        /* only one activated view in all workspaces at once */
        struct view *view;
        struct wl_listener minimize;
        struct wl_listener unmap;
    } activated;

    struct config *config;
    struct view_manager_impl impl;

    struct {
        uint32_t num_workspaces;
        const char *workspace_names[MAX_WORKSPACES];
        uint32_t view_adsorption;
        bool csd_round_corner;
        enum minimize_effect_type minimize_effect_type;
        /* 0 is bottom_gap. 1 is left_gap. 2 is right_gap */
        int32_t view_gap[3];
        /* 0 is wayland resize_filter. 1 is xwayland resize_filter */
        uint32_t resize_filter[2];
        uint32_t configure_timeout[2];
    } state;

    struct wl_listener theme_update;
    struct wl_listener theme_icon_update;
    struct wl_listener new_xdg_toplevel;
    struct wl_listener display_destroy;
    struct wl_listener server_terminate;
    struct wl_listener server_ready;
    struct wl_listener server_destroy;

    bool show_desktop_enabled;
    bool show_activated_only_enabled;
    bool switcher_shown;

    struct wl_list view_modes; // struct view_mode.link
    struct view_mode *mode;
};

bool view_manager_config_init(struct view_manager *view_manager);

void view_manager_set_switcher_shown(bool shown);

bool view_read_config(struct view_manager *view_manager);

void view_write_config(struct view_manager *view_manager);

void view_close_popups(struct view *view);

void view_update_round_corner(struct view *view);

bool xdg_shell_init(struct view_manager *view_manager);

struct wlr_xdg_surface;

void xdg_view_add_icon_buffer(struct wlr_xdg_surface *wlr_xdg_surface, int scale,
                              struct wlr_buffer *wlr_buffer);

void xdg_view_clear_icon_buffer(struct wlr_xdg_surface *wlr_xdg_surface);

bool decoration_manager_create(struct view_manager *view_manager);

void view_proxy_destroy(struct view_proxy *view_proxy);

void view_set_current_proxy(struct view *view, struct view_proxy *view_proxy);

struct view_proxy *view_proxy_by_workspace(struct view *view, struct workspace *workspace);

bool positioner_manager_create(struct view_manager *view_manager);

void positioner_add_new_view(struct view *view);

bool server_decoration_manager_create(struct view_manager *view_manager);

bool window_actions_create(struct view_manager *view_manager);

bool view_manager_actions_create(struct view_manager *view_manager);

bool window_menu_manager_create(struct view_manager *view_manager);

void window_menu_show(struct view *view, struct seat *seat, int x, int y);

bool maximize_switcher_create(struct view_manager *view_manager);

void maximize_switcher_show(void);

void global_authentication_create(struct view *view);

void view_manager_set_global_authentication(struct view *view);

struct view *view_manager_get_global_authentication(void);

bool tile_manager_create(struct view_manager *view_manager);

void view_manager_show_tile_assist(struct view *view, struct seat *seat,
                                   struct kywc_output *kywc_output);

bool tile_manager_check_resize_edges(struct view *view, uint32_t resize_edges);

void tile_linkage_bar_show(struct view *view, uint32_t edges);

bool tile_linkage_view_resize(struct view *view, uint32_t edges, double l_x, double l_y);

void tile_linkage_resize_done(struct view *view, bool cancel);

bool tile_flyout_manager_create(struct view_manager *view_manager);

void tile_flyout_show(struct view *view, struct seat *seat, struct kywc_box *box);

struct view *view_manager_get_highlight(void);

void view_manager_add_highlight_view_listener(struct wl_listener *listener);

void highlight_view(struct view *view, bool enable);

void view_fix_geometry(struct view *view, struct kywc_box *geo, struct kywc_box *src_box,
                       struct kywc_box *dst_box);

struct wlr_xdg_popup;
void xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup, struct scene_tree *shell,
                      struct view_layer *layer, bool use_usable_area);

bool ky_workspace_manager_create(struct server *server);

bool ky_toplevel_manager_create(struct server *server);

bool modal_manager_create(struct server *server);

bool xdg_dialog_create(struct server *server);

bool xdg_activation_create(struct server *server);

struct view_mode *view_manager_mode_from_name(const char *name);

struct view_mode *view_manager_mode_register(const struct view_mode_interface *impl);

void view_manager_mode_unregister(struct view_mode *mode);

void stack_mode_register(struct view_manager *view_manager);

void tablet_mode_handle_task_bar_gesture(enum gesture_stage stage, double dx, double dy);

void tablet_mode_register(struct view_manager *view_manager);

bool ext_foreign_toplevel_create(struct server *server);

void kill_dialog_manager_create(struct view_manager *view_manager);

#if HAVE_KDE_VIRTUAL_DESKTOP
bool kde_virtual_desktop_management_create(struct server *server);
#else
static
    __attribute__((unused)) inline bool kde_virtual_desktop_management_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_WLR_FOREIGN_TOPLEVEL
bool wlr_foreign_toplevel_manager_create(struct server *server);
#else
static
    __attribute__((unused)) inline bool wlr_foreign_toplevel_manager_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_WLR_LAYER_SHELL
bool wlr_layer_shell_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool wlr_layer_shell_manager_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_KDE_PLASMA_SHELL
bool kde_plasma_shell_create(struct server *server);
#else
static __attribute__((unused)) inline bool kde_plasma_shell_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_KDE_PLASMA_WINDOW_MANAGEMENT
bool kde_plasma_window_management_create(struct server *server);
#else
static
    __attribute__((unused)) inline bool kde_plasma_window_management_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_KDE_BLUR
bool kde_blur_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool kde_blur_manager_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_KDE_SLIDE
bool kde_slide_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool kde_slide_manager_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_UKUI_SHELL
bool ukui_shell_create(struct server *server);

uint32_t ukui_shell_get_surface_decoration_flags(struct wlr_surface *surface);
#else
static __attribute__((unused)) inline bool ukui_shell_create(struct server *server)
{
    return false;
}

static __attribute__((unused)) inline uint32_t
ukui_shell_get_surface_decoration_flags(struct wlr_surface *surface)
{
    return 0;
}
#endif

#if HAVE_UKUI_WINDOW_MANAGEMENT
bool ukui_window_management_create(struct server *server);
#else
static __attribute__((unused)) inline bool ukui_window_management_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_UKUI_BLUR
bool ukui_blur_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool ukui_blur_manager_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_UKUI_STARTUP
bool ukui_startup_management_create(struct view_manager *view_manager);
#else
static __attribute__((unused)) inline bool
ukui_startup_management_create(struct view_manager *view_manager)
{
    return false;
}
#endif

#if HAVE_XDG_TOPLEVEL_ICON
bool xdg_toplevel_icon_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool xdg_toplevel_icon_manager_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_XDG_TOPLEVEL_TAG
bool xdg_toplevel_tag_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool xdg_toplevel_tag_manager_create(struct server *server)
{
    return false;
}
#endif

#endif /* _VIEW_P_H_ */
