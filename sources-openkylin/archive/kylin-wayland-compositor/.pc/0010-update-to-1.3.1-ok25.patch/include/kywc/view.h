// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _KYWC_VIEW_H_
#define _KYWC_VIEW_H_

#include <stdbool.h>

#include <wayland-server-core.h>

#include "boxes.h"

struct kywc_output;

enum kywc_edges {
    KYWC_EDGE_NONE = 0,
    KYWC_EDGE_TOP = 1 << 0,
    KYWC_EDGE_BOTTOM = 1 << 1,
    KYWC_EDGE_LEFT = 1 << 2,
    KYWC_EDGE_RIGHT = 1 << 3,
};

enum kywc_tile {
    KYWC_TILE_NONE = 0,
    KYWC_TILE_TOP,
    KYWC_TILE_BOTTOM,
    KYWC_TILE_LEFT,
    KYWC_TILE_RIGHT,
    KYWC_TILE_TOP_LEFT,
    KYWC_TILE_BOTTOM_LEFT,
    KYWC_TILE_TOP_RIGHT,
    KYWC_TILE_BOTTOM_RIGHT,
    KYWC_TILE_CENTER,
    KYWC_TILE_ALL,
};

enum kywc_ssd {
    KYWC_SSD_NONE = 0,
    KYWC_SSD_TITLE = 1 << 0,
    KYWC_SSD_BORDER = 1 << 1,
    KYWC_SSD_RESIZE = 1 << 2,
    KYWC_SSD_ALL = (1 << 3) - 1,
};

enum kywc_view_role {
    KYWC_VIEW_ROLE_UNDEF = -1,
    KYWC_VIEW_ROLE_NORMAL = 0,
    KYWC_VIEW_ROLE_DESKTOP,
    KYWC_VIEW_ROLE_PANEL,
    KYWC_VIEW_ROLE_ONSCREENDISPLAY,
    KYWC_VIEW_ROLE_NOTIFICATION,
    KYWC_VIEW_ROLE_TOOLTIP,
    KYWC_VIEW_ROLE_CRITICALNOTIFICATION,
    KYWC_VIEW_ROLE_SYSTEMWINDOW,
    KYWC_VIEW_ROLE_INPUTPANEL,
    KYWC_VIEW_ROLE_LOGOUT,
    KYWC_VIEW_ROLE_SCREENLOCK,
    KYWC_VIEW_ROLE_SCREENLOCKNOTIFICATION,
    KYWC_VIEW_ROLE_WATERMARK,
    KYWC_VIEW_ROLE_APPLETPOPUP,
    KYWC_VIEW_ROLE_SWITCHER,
    KYWC_VIEW_ROLE_AUTHENTICATION,
};

#define KYWC_VIEW_HAS_FIXED_SIZE(kywc_view)                                                        \
    ((kywc_view)->max_width != 0 && (kywc_view)->max_height != 0 &&                                \
     (kywc_view)->min_width == (kywc_view)->max_width &&                                           \
     (kywc_view)->min_height == (kywc_view)->max_height)

#define KYWC_VIEW_IS_MINIMIZABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_MINIMIZABLE)
#define KYWC_VIEW_IS_MAXIMIZABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_MAXIMIZABLE)
#define KYWC_VIEW_IS_CLOSEABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_CLOSEABLE)
#define KYWC_VIEW_IS_FULLSCREENABLE(kywc_view)                                                     \
    ((kywc_view)->capabilities & KYWC_VIEW_FULLSCREENABLE)
#define KYWC_VIEW_IS_MOVABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_MOVABLE)
#define KYWC_VIEW_IS_RESIZABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_RESIZABLE)
#define KYWC_VIEW_IS_FOCUSABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_FOCUSABLE)
#define KYWC_VIEW_IS_ACTIVATABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_ACTIVATABLE)
#define KYWC_VIEW_IS_ABOVEABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_ABOVEABLE)
#define KYWC_VIEW_IS_BELOWABLE(kywc_view) ((kywc_view)->capabilities & KYWC_VIEW_BELOWABLE)
#define KYWC_VIEW_NEED_MINIMIZE_BUTTON(kywc_view)                                                  \
    ((kywc_view)->capabilities & KYWC_VIEW_MINIMIZE_BUTTON)
#define KYWC_VIEW_NEED_MAXIMIZE_BUTTON(kywc_view)                                                  \
    ((kywc_view)->capabilities & KYWC_VIEW_MAXIMIZE_BUTTON)

enum kywc_view_capabilities {
    KYWC_VIEW_MINIMIZABLE = 1 << 0,
    KYWC_VIEW_MAXIMIZABLE = 1 << 1,
    KYWC_VIEW_CLOSEABLE = 1 << 2,
    KYWC_VIEW_FULLSCREENABLE = 1 << 3,
    KYWC_VIEW_MOVABLE = 1 << 4,
    KYWC_VIEW_RESIZABLE = 1 << 5,
    KYWC_VIEW_FOCUSABLE = 1 << 6,
    KYWC_VIEW_ACTIVATABLE = 1 << 7,
    KYWC_VIEW_ABOVEABLE = 1 << 8,
    KYWC_VIEW_BELOWABLE = 1 << 9,
    KYWC_VIEW_MINIMIZE_BUTTON = 1 << 10,
    KYWC_VIEW_MAXIMIZE_BUTTON = 1 << 11,
    KYWC_VIEW_CAPABILITIES_ALL = (1 << 12) - 1,
};

/* margin if used ssd */
struct view_margin {
    int off_x, off_y, off_width, off_height;
};

/* padding for shadow */
struct view_padding {
    int top, bottom, left, right;
};

struct kywc_view_capabilities_event {
    uint32_t mask;
};

struct kywc_view {
    enum kywc_view_role role;

    /* current geometry in global layout */
    struct kywc_box geometry;

    struct view_margin margin;
    struct view_padding padding;

    /* minimize size client set or default */
    int min_width, min_height;
    int max_width, max_height;

    enum kywc_ssd ssd;
    bool has_initial_position;
    bool has_round_corner;
    /* no constraints by usable area if it`s true */
    bool unconstrained;

    /* have a buffer attached */
    bool mapped;

    /* view current states */
    bool minimized, maximized, fullscreen, activated;
    enum kywc_tile tiled;
    bool kept_above, kept_below, sticky;
    bool skip_taskbar, skip_switcher;
    bool demands_attention;
    bool modal;

    /* wm capabilities of the view */
    uint32_t capabilities;

    const char *uuid;
    /* app_id: class when xwayland shell */
    const char *title, *app_id, *exec;

    /* need focused by this seat when map */
    struct seat *focused_seat;

    /* window opacity, default to 1.0 */
    float opacity;

    struct {
        /* emit before map for position, ssd ... */
        struct wl_signal premap;
        /* emit when view is mapped */
        struct wl_signal map;
        /* emit when view is going to be unmapped */
        struct wl_signal unmap;
        /* emit when view is going to be destroyed */
        struct wl_signal destroy;

        /* emit when view's activated state has changed */
        struct wl_signal activate;
        /* emit when view's maximized state has changed */
        struct wl_signal maximize;
        /* emit when view's minimized state has changed */
        struct wl_signal minimize;
        /* emit when view's fullscreen state has changed */
        struct wl_signal fullscreen;
        /* emit when view's tiled state has changed */
        struct wl_signal tile;
        /* emit when view's above state has changed */
        struct wl_signal above;
        /* emit when view's below state has changed */
        struct wl_signal below;
        /* emit when view's sticky state has changed */
        struct wl_signal sticky;
        /* emit when view's skip_taskbar state has changed */
        struct wl_signal skip_taskbar;
        /* emit when view's skip_switcher state has changed */
        struct wl_signal skip_switcher;
        /* emit when view's demands_attention state has changed */
        struct wl_signal demands_attention;
        /* emit when view's modal state has changed */
        struct wl_signal modal;
        /* emit when view's capabilities state has changed */
        struct wl_signal capabilities;

        /* emit when view's title has changed */
        struct wl_signal title;
        /* emit when view's app_id(class) has changed */
        struct wl_signal app_id;

        /* emit when view's position has changed */
        struct wl_signal position;
        /* emit when view's size has changed */
        struct wl_signal size;
        /* emit when view's decoration mode has changed */
        struct wl_signal decoration;

        /* emit when view's opacity has changed */
        struct wl_signal opacity;
    } events;
};

/**
 * listen new_view signal for xdg-shell and xwayland-shell
 */
void kywc_view_add_new_listener(struct wl_listener *listener);

void kywc_view_add_new_mapped_listener(struct wl_listener *listener);

void kywc_view_close(struct kywc_view *kywc_view);

void kywc_view_move(struct kywc_view *kywc_view, int x, int y);

void kywc_view_resize(struct kywc_view *kywc_view, struct kywc_box *geometry);

void kywc_view_activate(struct kywc_view *kywc_view);

void kywc_view_set_tiled(struct kywc_view *kywc_view, enum kywc_tile tile,
                         struct kywc_output *kywc_output);

void kywc_view_set_minimized(struct kywc_view *kywc_view, bool minimized);

void kywc_view_toggle_minimized(struct kywc_view *kywc_view);

void kywc_view_set_maximized(struct kywc_view *kywc_view, bool maximized,
                             struct kywc_output *kywc_output);
void kywc_view_toggle_maximized(struct kywc_view *kywc_view);

void kywc_view_set_fullscreen(struct kywc_view *kywc_view, bool fullscreen,
                              struct kywc_output *kywc_output);
void kywc_view_toggle_fullscreen(struct kywc_view *kywc_view);

void kywc_view_set_kept_above(struct kywc_view *kywc_view, bool kept_above);

void kywc_view_toggle_kept_above(struct kywc_view *kywc_view);

void kywc_view_set_kept_below(struct kywc_view *kywc_view, bool kept_below);

void kywc_view_toggle_kept_below(struct kywc_view *kywc_view);

void kywc_view_set_skip_taskbar(struct kywc_view *kywc_view, bool skip_taskbar);

void kywc_view_set_skip_switcher(struct kywc_view *kywc_view, bool skip_switcher);

void kywc_view_set_demands_attention(struct kywc_view *kywc_view, bool demands_attention);

void kywc_view_set_modal(struct kywc_view *kywc_view, bool modal);

void kywc_view_set_sticky(struct kywc_view *kywc_view, bool sticky);

struct kywc_view *kywc_view_by_uuid(const char *uuid);

#endif /* _KYWC_VIEW_H_ */
