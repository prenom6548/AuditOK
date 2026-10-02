// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _XWM_H_
#define _XWM_H_

#include <pixman.h>

#include <wlr/types/wlr_data_device.h>
#include <wlr/util/addon.h>

#include <xcb/sync.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xfixes.h>

#include "kywc_config.h"

struct kywc_box;
struct kywc_output;

/* selection */
#define XDND_VERSION 5

#define INCR_CHUNK_SIZE (64 * 1024)

struct xwm_selection {
    struct xwm *xwm;

    bool window_mapped;
    xcb_window_t window;

    xcb_atom_t atom;
    xcb_window_t owner;
    xcb_timestamp_t timestamp;

    struct wl_list incoming;
    struct wl_list outgoing;
};

struct xwm_selection_transfer {
    struct xwm_selection *selection;

    bool incr;
    bool flush_property_on_delete;
    bool property_set;
    struct wl_array source_data;
    int wl_client_fd;
    struct wl_event_source *event_source;
    struct wl_list link;

    // when sending to x11
    xcb_selection_request_event_t request;

    // when receiving from x11
    int property_start;
    xcb_get_property_reply_t *property_reply;
    xcb_window_t incoming_window;
};

struct drag_x11 {
    struct xwm *xwm;
    struct x11_data_source *source;

    struct wlr_surface *hovered_surface;
    struct wl_listener surface_destroy;
    struct wlr_seat_client *hovered_client;
    struct wl_listener seat_client_destroy;

    // TODO: listen touch, tablet
    struct wl_listener cursor_motion;
    struct wl_listener cursor_button;
    struct wl_listener seat_destroy;
};

struct x11_data_source {
    struct wlr_data_source base;
    struct xwm_selection *selection;
    struct wl_array mime_types_atoms;
};

/* drag x11 */
bool xwm_start_drag_x11(struct xwm *xwm, struct x11_data_source *source);

void xwm_end_drag_x11(struct xwm *xwm);

/* dnd */
enum wl_data_device_manager_dnd_action data_device_manager_dnd_action_from_atom(struct xwm *xwm,
                                                                                xcb_atom_t atom);

xcb_atom_t data_device_manager_dnd_action_to_atom(struct xwm *xwm,
                                                  enum wl_data_device_manager_dnd_action action);

void xwm_send_dnd_status(struct xwm *xwm, xcb_window_t requestor, xcb_window_t window,
                         uint32_t action);

void xwm_send_dnd_finish(struct xwm *xwm, xcb_window_t requestor, xcb_window_t window, bool accept,
                         uint32_t action);

void xwm_seat_handle_start_drag(struct xwm *xwm, struct wlr_drag *drag);

void xwm_map_dnd_selection_window(struct xwm *xwm, bool map);

int xwm_handle_selection_client_message(struct xwm *xwm, xcb_client_message_event_t *ev);

/* incoming */
struct xwm_selection_transfer *
xwm_selection_find_incoming_transfer_by_window(struct xwm_selection *selection,
                                               xcb_window_t window);
void xwm_get_incr_chunk(struct xwm_selection_transfer *transfer);

void xwm_handle_selection_notify(struct xwm *xwm, xcb_selection_notify_event_t *event);

int xwm_handle_xfixes_selection_notify(struct xwm *xwm, xcb_xfixes_selection_notify_event_t *event);

bool data_source_is_xwayland(struct wlr_data_source *wlr_source);

bool primary_selection_source_is_xwayland(struct wlr_primary_selection_source *source);

/* selection */
void xwm_selection_transfer_remove_event_source(struct xwm_selection_transfer *transfer);

void xwm_selection_transfer_close_wl_client_fd(struct xwm_selection_transfer *transfer);

void xwm_selection_transfer_destroy_property_reply(struct xwm_selection_transfer *transfer);

void xwm_selection_transfer_init(struct xwm_selection_transfer *transfer,
                                 struct xwm_selection *selection);

xcb_atom_t xwm_mime_type_to_atom(struct xwm *xwm, char *mime_type);

char *xwm_mime_type_from_atom(struct xwm *xwm, xcb_atom_t atom);

struct xwm_selection *xwm_get_selection(struct xwm *xwm, xcb_atom_t selection_atom);

void xwm_selection_transfer_destroy(struct xwm_selection_transfer *transfer);

void xwm_selection_init(struct xwm_selection *selection, struct xwm *xwm, xcb_atom_t atom);

void xwm_selection_finish(struct xwm_selection *selection);

int xwm_handle_selection_event(struct xwm *xwm, xcb_generic_event_t *event);

/* outgoing */
void xwm_selection_transfer_destroy_outgoing(struct xwm_selection_transfer *transfer);

void xwm_send_incr_chunk(struct xwm_selection_transfer *transfer);

void xwm_handle_selection_request(struct xwm *xwm, xcb_selection_request_event_t *req);

void xwm_handle_selection_destroy_notify(struct xwm *xwm, xcb_destroy_notify_event_t *event);

/* eavesdrops */
bool eavesdrops_manager_create(struct xwm *xwm);

void eavesdrops_manager_destroy(void);

void eavesdrops_manager_set_seat(struct seat *seat);

/* xwayland surface */
enum xwayland_surface_functions {
    XWAYLAND_SURFACE_FUNCTIONS_ALL = 0,
    XWAYLAND_SURFACE_FUNCTIONS_NO_MINIMIZE = 1,
    XWAYLAND_SURFACE_FUNCTIONS_NO_MAXIMIZE = 2,
};

/**
 * This represents the input focus described as follows:
 *
 * https://www.x.org/releases/X11R7.6/doc/xorg-docs/specs/ICCCM/icccm.html#input_focus
 */
enum xwayland_icccm_input_model {
    ICCCM_INPUT_MODEL_NONE = 0,
    ICCCM_INPUT_MODEL_PASSIVE = 1,
    ICCCM_INPUT_MODEL_LOCAL = 2,
    ICCCM_INPUT_MODEL_GLOBAL = 3,
};

struct xwayland_surface_icon {
    struct wl_list link;
    uint32_t width, height;
    uint32_t size;
    unsigned char *data;
    struct wlr_buffer *buffer;
};

struct xwayland_surface_request_configure {
    int16_t x, y;
    uint16_t width, height;
    uint16_t mask; // xcb_config_window_t
};

struct xwayland_surface {
    struct xwm *xwm;

    struct wl_list link;
    struct wl_list stack_link;
    struct wl_list unpaired_link;

    uint64_t serial;
    uint32_t surface_id;
    xcb_window_t window_id;

    struct wlr_surface *surface;
    struct wlr_addon surface_addon;
    struct wl_listener surface_commit;
    struct wl_listener client_commit;
    struct wl_listener surface_map;
    struct wl_listener surface_unmap;

    int16_t x, y;
    uint16_t width, height;

    bool override_redirect;
    struct scene_node *surface_node;
    const struct xwayland_surface_impl *impl;
    union {
        struct xwayland_view *xwayland_view;
        struct xwayland_unmanaged *unmanaged;
    };

    char *class;
    char *instance;
    char *role;
    char *startup_id;
    char *title;
    bool has_utf8_title;
    pid_t pid;

    struct wl_list children; // xwayland_surface.parent_link
    struct xwayland_surface *parent;
    struct wl_list parent_link; // xwayland_surface.children

    xcb_atom_t *window_type;
    size_t window_type_len;

    xcb_atom_t *protocols;
    size_t protocols_len;

    uint32_t functions;
    uint32_t decorations;
    xcb_icccm_wm_hints_t *hints;
    xcb_size_hints_t *size_hints;
    /*
     * _NET_WM_STRUT_PARTIAL (used by e.g. XWayland panels).
     * Note that right/bottom values are offsets from the lower
     * right corner of the X11 screen, and the exact relation
     * between X11 screen coordinates and the wlr_output_layout
     * depends on the XWayland implementation.
     */
    xcb_ewmh_wm_strut_partial_t *strut_partial;

    /* XdndProxy window */
    xcb_window_t proxy_window;

    bool pinging;
    struct wl_event_source *ping_timer;

    bool modal;
    bool fullscreen;
    bool maximized_vert, maximized_horz;
    bool minimized;
    bool withdrawn;
    bool remap;
    bool sticky;
    bool skip_taskbar;
    bool skip_switcher;
    bool above;
    bool below;
    bool demands_attention;
    bool has_alpha;
    float opacity;
    pixman_region32_t blur_region;
    pixman_region32_t opaque_region;

    struct wl_list icons; // struct xwayland_surface_icon.link
    struct {
        xcb_sync_counter_t counter;
        xcb_sync_int64_t value;
        xcb_sync_alarm_t alarm;
        bool blocked;
    } sync;
};

struct xwayland_surface_impl {
    struct scene_node *(*associate)(struct xwayland_surface *xsurface);
    void (*dissociate)(struct xwayland_surface *xsurface);
    void (*map)(struct xwayland_surface *xsurface);
    void (*unmap)(struct xwayland_surface *xsurface);
    void (*commit)(struct xwayland_surface *xsurface);
    void (*destroy)(struct xwayland_surface *xsurface);

    void (*request_configure)(struct xwayland_surface *xsurface,
                              struct xwayland_surface_request_configure *configure);
    void (*request_move)(struct xwayland_surface *xsurface);
    void (*request_resize)(struct xwayland_surface *xsurface, uint32_t edges);
    void (*request_minimize)(struct xwayland_surface *xsurface);
    void (*request_maximize)(struct xwayland_surface *xsurface);
    void (*request_fullscreen)(struct xwayland_surface *xsurface);
    void (*request_activate)(struct xwayland_surface *xsurface);
    void (*request_sticky)(struct xwayland_surface *xsurface);
    void (*request_shaded)(struct xwayland_surface *xsurface);
    void (*request_skip_taskbar)(struct xwayland_surface *xsurface);
    void (*request_skip_switcher)(struct xwayland_surface *xsurface);
    void (*request_skip_pager)(struct xwayland_surface *xsurface);
    void (*request_above)(struct xwayland_surface *xsurface);
    void (*request_below)(struct xwayland_surface *xsurface);
    void (*request_demands_attention)(struct xwayland_surface *xsurface);
    void (*request_modal)(struct xwayland_surface *xsurface);
    void (*request_move_resize)(struct xwayland_surface *xsurface, struct kywc_box *box);

    void (*set_title)(struct xwayland_surface *xsurface);
    void (*set_class)(struct xwayland_surface *xsurface);
    void (*set_role)(struct xwayland_surface *xsurface);
    void (*set_parent)(struct xwayland_surface *xsurface);
    void (*set_startup_id)(struct xwayland_surface *xsurface);
    void (*set_window_type)(struct xwayland_surface *xsurface);
    void (*set_hints)(struct xwayland_surface *xsurface);
    void (*set_size_hints)(struct xwayland_surface *xsurface);
    void (*set_functions)(struct xwayland_surface *xsurface);
    void (*set_decorations)(struct xwayland_surface *xsurface);
    void (*set_strut_partial)(struct xwayland_surface *xsurface);
    void (*set_geometry)(struct xwayland_surface *xsurface);
    void (*set_icon)(struct xwayland_surface *xsurface);

    void (*ping_timeout)(struct xwayland_surface *xsurface);

    /* reconfigure xwayland surface geometry by the new scale */
    void (*try_reconfigure)(struct xwayland_surface *xsurface, struct kywc_output *changed_output);
};

enum {
    INPUT_MASK_POINTER = 1 << 0,
    INPUT_MASK_KEYBOARD = 1 << 1,
};

void xwayland_surface_restack(struct xwayland_surface *surface, struct xwayland_surface *sibling,
                              enum xcb_stack_mode_t mode);

void xwayland_surface_activate(struct xwayland_surface *xsurface, bool activated);

void xwayland_surface_configure(struct xwayland_surface *surface, int16_t x, int16_t y,
                                uint16_t width, uint16_t height);

void xwayland_surface_close(struct xwayland_surface *surface);

void xwayland_surface_set_minimized(struct xwayland_surface *surface, bool minimized);

void xwayland_surface_set_maximized(struct xwayland_surface *surface, bool maximized);

void xwayland_surface_set_fullscreen(struct xwayland_surface *surface, bool fullscreen);

void xwayland_surface_set_sticky(struct xwayland_surface *surface, bool sticky);

void xwayland_surface_set_skip_taskbar(struct xwayland_surface *surface, bool skip_taskbar);

void xwayland_surface_set_skip_switcher(struct xwayland_surface *surface, bool skip_switcher);

void xwayland_surface_set_above(struct xwayland_surface *surface, bool above);

void xwayland_surface_set_below(struct xwayland_surface *surface, bool below);

void xwayland_surface_set_demands_attention(struct xwayland_surface *surface,
                                            bool demands_attention);

struct xwayland_surface *xwayland_surface_try_from_wlr_surface(struct wlr_surface *surface);

void xwayland_surface_ping(struct xwayland_surface *surface);

bool xwayland_surface_wants_focus(const struct xwayland_surface *xsurface);

enum xwayland_icccm_input_model
xwayland_surface_icccm_input_model(const struct xwayland_surface *xsurface);

struct wlr_buffer *xwayland_surface_get_icon_buffer(struct xwayland_surface *xsurface, int size,
                                                    float scale);

bool xwayland_surface_has_input(struct xwayland_surface *xwayland_surface, uint32_t input);

void xwayland_surface_set_allow_commits(struct xwayland_surface *xsurface, bool allow);

void xwayland_surface_send_sync_request(struct xwayland_surface *xsurface, xcb_sync_int64_t *value);

bool xwayland_surface_has_type(struct xwayland_surface *xsurface, int type);

void xwayland_surface_debug_type(struct xwayland_surface *xwayland_surface);

/* xwm */
/* This is in xcb/xcb_event.h, but pulling xcb-util just for a constant
 * others redefine anyway is meh
 */
#define XCB_EVENT_RESPONSE_TYPE_MASK (0x7f)

enum atom_name {
    WL_SURFACE_ID,
    WL_SURFACE_SERIAL,
    WM_DELETE_WINDOW,
    WM_HINTS,
    WM_NORMAL_HINTS,
    WM_SIZE_HINTS,
    WM_WINDOW_ROLE,
    MOTIF_WM_HINTS,
    WM_S0,
    NET_SUPPORTED,
    NET_WM_CM_S0,
    NET_WM_PID,
    NET_WM_STRUT_PARTIAL,
    WM_TAKE_FOCUS,
    WINDOW,
    NET_ACTIVE_WINDOW,
    NET_WM_MOVERESIZE,
    NET_SUPPORTING_WM_CHECK,
    NET_WM_STATE_FOCUSED,
    NET_WM_STATE_MODAL,
    NET_WM_STATE_FULLSCREEN,
    NET_WM_STATE_MAXIMIZED_VERT,
    NET_WM_STATE_MAXIMIZED_HORZ,
    NET_WM_STATE_HIDDEN,
    NET_WM_STATE_STICKY,
    NET_WM_STATE_SKIP_TASKBAR,
    NET_WM_STATE_ABOVE,
    NET_WM_STATE_BELOW,
    NET_WM_STATE_DEMANDS_ATTENTION,
    NET_WM_PING,
    WM_CHANGE_STATE,
    WM_STATE,
    CLIPBOARD,
    PRIMARY,
    TIMESTAMP,
    DELETE,
    NET_STARTUP_ID,
    NET_STARTUP_INFO,
    NET_STARTUP_INFO_BEGIN,
    NET_CLIENT_LIST,
    NET_CLIENT_LIST_STACKING,
    NET_WORKAREA,

    NET_WM_WINDOW_TYPE,
    NET_WM_WINDOW_TYPE_DESKTOP,
    NET_WM_WINDOW_TYPE_DOCK,
    NET_WM_WINDOW_TYPE_TOOLBAR,
    NET_WM_WINDOW_TYPE_MENU,
    NET_WM_WINDOW_TYPE_UTILITY,
    NET_WM_WINDOW_TYPE_SPLASH,
    NET_WM_WINDOW_TYPE_DIALOG,
    NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
    NET_WM_WINDOW_TYPE_POPUP_MENU,
    NET_WM_WINDOW_TYPE_COMBO,
    NET_WM_WINDOW_TYPE_TOOLTIP,
    NET_WM_WINDOW_TYPE_DND,
    NET_WM_WINDOW_TYPE_NOTIFICATION,
    NET_WM_WINDOW_TYPE_NORMAL,

    NET_MOVERESIZE_WINDOW,
    NET_SHOWING_DESKTOP,

    /* kde extensions */
    KDE_NET_WM_WINDOW_TYPE_OVERRIDE,
    KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY,

    /* ukui atoms */
    UKUI_NET_WM_WINDOW_TYPE_SYSTEMWINDOW,
    UKUI_NET_WM_WINDOW_TYPE_INPUTPANEL,
    UKUI_NET_WM_WINDOW_TYPE_LOGOUT,
    UKUI_NET_WM_WINDOW_TYPE_SCREENLOCK,
    UKUI_NET_WM_WINDOW_TYPE_SCREENLOCKNOTIFICATION,
    UKUI_NET_WM_WINDOW_TYPE_WATERMARK,
    KWIN_UKUI_DECORATION,

    NET_WM_STATE,
    // KDE-specific atom
    KDE_NET_WM_STATE_SKIP_SWITCHER,
    KDE_NET_WM_BLUR_BEHIND_REGION,

    NET_WM_ICON,
    NET_WM_WINDOW_OPACITY,
    NET_WM_OPAQUE_REGION,

    UTF8_STRING,
    NET_WM_NAME,

    INCR,
    TEXT,
    TARGETS,
    WL_SELECTION,
    CLIPBOARD_MANAGER,

    DND_SELECTION,
    DND_AWARE,
    DND_STATUS,
    DND_POSITION,
    DND_ENTER,
    DND_LEAVE,
    DND_DROP,
    DND_FINISHED,
    DND_PROXY,
    DND_TYPE_LIST,
    DND_ACTION_MOVE,
    DND_ACTION_COPY,
    DND_ACTION_ASK,
    DND_ACTION_PRIVATE,

    WM_PROTOCOLS,
    NET_WM_SYNC_REQUEST,
    NET_WM_SYNC_REQUEST_COUNTER,
    XWAYLAND_ALLOW_COMMITS,

    ATOM_LAST,
};

struct xwm_atom {
    xcb_atom_t atom;
    void (*handle_client_message)(struct xwm *xwm, xcb_client_message_event_t *ev);
    void (*handle_surface_property)(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    xcb_get_property_reply_t *reply, bool delete);
    bool (*handle_net_wm_state)(struct xwm *xwm, struct xwayland_surface *xsurface,
                                uint32_t action);
};

struct xwm {
    bool destroying;
    uint32_t ping_timeout;

    struct xwayland_manager *xwayland;
    struct wl_listener compositor_new_surface;
    struct wl_listener compositor_destroy;
    struct wl_listener shell_v1_new_surface;
    struct wl_listener shell_v1_destroy;
    struct wl_display *display;
    struct wl_event_source *event_source;

    struct xwm_atom atoms[ATOM_LAST];
    xcb_connection_t *xcb_conn;
    xcb_screen_t *screen;
    xcb_window_t window;
    xcb_visualid_t visual_id;
    xcb_colormap_t colormap;
    xcb_render_pictformat_t render_format_id;
    xcb_cursor_t cursor;

    struct wl_list surfaces;            // xwayland_surface.link surfaces in creation order
    struct wl_list unpaired_surfaces;   // xwayland_surface.unpaired_link
    struct wl_list pending_startup_ids; // pending_startup_id

    /* surfaces in bottom-to-top stacking order, for _NET_CLIENT_LIST_STACKING */
    struct wl_list surfaces_in_stack_order; // xwayland_surface.stack_link
    struct wl_event_source *idle_source;    // idle source for NET_CLIENT_LIST_STACKING update

    const xcb_query_extension_reply_t *shape;
    const xcb_query_extension_reply_t *xfixes;
    const xcb_query_extension_reply_t *sync;
    const xcb_query_extension_reply_t *xres;
    uint32_t xfixes_major_version;

    struct xwayland_surface *focus_surface;
    struct xwayland_surface *offered_focus;
    xcb_window_t no_focus_window;
    xcb_window_t guard_window;
    struct wl_listener activate_view;

#if HAVE_XCB_ERRORS
    xcb_errors_context_t *errors_context;
#endif
    unsigned int last_focus_seq;

    struct seat *seat;
    struct wl_listener seat_destroy;
    struct wl_listener seat_set_selection;
    struct wl_listener seat_set_primary_selection;
    struct wl_listener seat_start_drag;
    struct xwm_selection clipboard_selection;
    struct xwm_selection primary_selection;
    struct xwm_selection dnd_selection;

    struct wlr_drag *drag;
    struct wl_listener seat_drag_focus;
    struct wl_listener seat_drag_motion;
    struct wl_listener seat_drag_drop;
    struct wl_listener seat_drag_destroy;
    struct wl_listener seat_drag_source_destroy;
    struct xwayland_surface *drag_focus;
    struct wl_listener drag_focus_destroy;
    struct xwayland_surface *drop_focus;
    struct wl_listener drop_focus_destroy;

    struct drag_x11 *drag_x11;

    // output max scale changed
    float scale;
    struct wl_listener max_scale;

    // workspace changed
    struct wl_listener active_workspace;
};

struct xwm *xwm_create(struct xwayland_manager *xwayland, int wm_fd);

void xwm_destroy(struct xwm *xwm);

struct xwayland_surface *xwm_lookup_surface(struct xwm *xwm, xcb_window_t window_id);

void xwm_update_cursor(struct xwm *xwm);

void xwm_seat_unlink_drag_handlers(struct xwm *xwm);

void xwm_set_seat(struct xwm *xwm, struct seat *seat);

char *xwm_get_atom_name(struct xwm *xwm, xcb_atom_t atom);

void xwm_send_event_with_size(xcb_connection_t *c, uint8_t propagate, xcb_window_t destination,
                              uint32_t event_mask, const void *event, uint32_t length);

void xwm_set_workareas(struct xwm *xwm, const struct kywc_box *workareas, size_t num_workareas);

#endif /* _XWM_H_ */
