// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include <xcb/shape.h>
#include <xcb/xfixes.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/xwayland/shell.h>

#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "scene/surface.h"
#include "security.h"
#include "server.h"
#include "util/macros.h"
#include "util/string.h"
#include "view/view.h"
#include "view/workspace.h"
#include "xwayland_p.h"

static const char *const atom_map[ATOM_LAST] = {
    [NET_WM_WINDOW_TYPE_DESKTOP] = "_NET_WM_WINDOW_TYPE_DESKTOP",
    [NET_WM_WINDOW_TYPE_DOCK] = "_NET_WM_WINDOW_TYPE_DOCK",
    [NET_WM_WINDOW_TYPE_TOOLBAR] = "_NET_WM_WINDOW_TYPE_TOOLBAR",
    [NET_WM_WINDOW_TYPE_MENU] = "_NET_WM_WINDOW_TYPE_MENU",
    [NET_WM_WINDOW_TYPE_UTILITY] = "_NET_WM_WINDOW_TYPE_UTILITY",
    [NET_WM_WINDOW_TYPE_SPLASH] = "_NET_WM_WINDOW_TYPE_SPLASH",
    [NET_WM_WINDOW_TYPE_DIALOG] = "_NET_WM_WINDOW_TYPE_DIALOG",
    [NET_WM_WINDOW_TYPE_DROPDOWN_MENU] = "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
    [NET_WM_WINDOW_TYPE_POPUP_MENU] = "_NET_WM_WINDOW_TYPE_POPUP_MENU",
    [NET_WM_WINDOW_TYPE_COMBO] = "_NET_WM_WINDOW_TYPE_COMBO",
    [NET_WM_WINDOW_TYPE_TOOLTIP] = "_NET_WM_WINDOW_TYPE_TOOLTIP",
    [NET_WM_WINDOW_TYPE_DND] = "_NET_WM_WINDOW_TYPE_DND",
    [NET_WM_WINDOW_TYPE_NOTIFICATION] = "_NET_WM_WINDOW_TYPE_NOTIFICATION",
    [NET_WM_WINDOW_TYPE_NORMAL] = "_NET_WM_WINDOW_TYPE_NORMAL",

    [NET_MOVERESIZE_WINDOW] = "_NET_MOVERESIZE_WINDOW",
    [NET_SHOWING_DESKTOP] = "_NET_SHOWING_DESKTOP",

    [KDE_NET_WM_WINDOW_TYPE_OVERRIDE] = "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",
    [KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY] = "_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY",

    [UKUI_NET_WM_WINDOW_TYPE_SYSTEMWINDOW] = "_UKUI_NET_WM_WINDOW_TYPE_SYSTEMWINDOW",
    [UKUI_NET_WM_WINDOW_TYPE_INPUTPANEL] = "_UKUI_NET_WM_WINDOW_TYPE_INPUTPANEL",
    [UKUI_NET_WM_WINDOW_TYPE_LOGOUT] = "_UKUI_NET_WM_WINDOW_TYPE_LOGOUT",
    [UKUI_NET_WM_WINDOW_TYPE_SCREENLOCK] = "_UKUI_NET_WM_WINDOW_TYPE_SCREENLOCK",
    [UKUI_NET_WM_WINDOW_TYPE_SCREENLOCKNOTIFICATION] =
        "_UKUI_NET_WM_WINDOW_TYPE_SCREENLOCKNOTIFICATION",
    [UKUI_NET_WM_WINDOW_TYPE_WATERMARK] = "_UKUI_NET_WM_WINDOW_TYPE_WATERMARK",
    [KWIN_UKUI_DECORATION] = "_KWIN_UKUI_DECORAION", // it has been misspelled since V10

    [NET_WM_STATE] = "_NET_WM_STATE",
    [KDE_NET_WM_STATE_SKIP_SWITCHER] = "_KDE_NET_WM_STATE_SKIP_SWITCHER",
    [KDE_NET_WM_BLUR_BEHIND_REGION] = "_KDE_NET_WM_BLUR_BEHIND_REGION",

    [NET_WM_ICON] = "_NET_WM_ICON",
    [NET_WM_WINDOW_OPACITY] = "_NET_WM_WINDOW_OPACITY",
    [NET_WM_OPAQUE_REGION] = "_NET_WM_OPAQUE_REGION",

    [UTF8_STRING] = "UTF8_STRING",
    [NET_WM_NAME] = "_NET_WM_NAME",
    [NET_SUPPORTING_WM_CHECK] = "_NET_SUPPORTING_WM_CHECK",

    [INCR] = "INCR",
    [TEXT] = "TEXT",
    [WL_SELECTION] = "_WL_SELECTION",

    [DND_SELECTION] = "XdndSelection",
    [DND_AWARE] = "XdndAware",
    [DND_STATUS] = "XdndStatus",
    [DND_POSITION] = "XdndPosition",
    [DND_ENTER] = "XdndEnter",
    [DND_LEAVE] = "XdndLeave",
    [DND_DROP] = "XdndDrop",
    [DND_FINISHED] = "XdndFinished",
    [DND_PROXY] = "XdndProxy",
    [DND_TYPE_LIST] = "XdndTypeList",
    [DND_ACTION_MOVE] = "XdndActionMove",
    [DND_ACTION_COPY] = "XdndActionCopy",
    [DND_ACTION_ASK] = "XdndActionAsk",
    [DND_ACTION_PRIVATE] = "XdndActionPrivate",

    [WM_PROTOCOLS] = "WM_PROTOCOLS",
    [NET_WM_SYNC_REQUEST] = "_NET_WM_SYNC_REQUEST",
    [NET_WM_SYNC_REQUEST_COUNTER] = "_NET_WM_SYNC_REQUEST_COUNTER",
    [XWAYLAND_ALLOW_COMMITS] = "_XWAYLAND_ALLOW_COMMITS",
};

static struct xwayland_server *xwayland = NULL;

static void handle_new_xwayland_surface(struct wl_listener *listener, void *data)
{
    struct wlr_xwayland_surface *wlr_xwayland_surface = data;
    /* dnd window catcher no need map */
    if (wlr_xwayland_surface->window_id == xwayland->window_catcher) {
        return;
    }

    if (wlr_xwayland_surface->override_redirect) {
        xwayland_unmanaged_create(xwayland, wlr_xwayland_surface);
        return;
    }

    xwayland_view_create(xwayland, wlr_xwayland_surface);
}

static void xwayland_update_xresources(xcb_connection_t *xcb_conn)
{
    struct seat *seat = seat_from_wlr_seat(xwayland->wlr_xwayland->seat);

    const char *prop_str =
        string_create("Xft.dpi:\t%d\nXcursor.size:\t%d\nXcursor.theme:\t%s\n",
                      (int)(xwayland->scale * 96), (int)(seat->state.cursor_size * xwayland->scale),
                      seat->state.cursor_theme ? seat->state.cursor_theme : "default");
    if (!prop_str) {
        return;
    }

    xcb_change_property(xcb_conn, XCB_PROP_MODE_REPLACE, xwayland->screen->root,
                        XCB_ATOM_RESOURCE_MANAGER, XCB_ATOM_STRING, 8, strlen(prop_str), prop_str);
    xcb_flush(xcb_conn);

    free((void *)prop_str);
}

static void handle_max_scale(struct wl_listener *listener, void *data)
{
    float scale = output_manager_get_max_scale();
    if (xwayland->scale == scale) {
        return;
    }

    xwayland->scale = scale;
    kywc_log(KYWC_INFO, "Xwayland set scale to %f", xwayland->scale);

    /* xwayland server is destroyed or not ready */
    if (!xwayland->wlr_xwayland || !xwayland->wlr_xwayland->xwm) {
        return;
    }

    /* update default cursor with current scale */
    xwayland_set_cursor(seat_from_wlr_seat(xwayland->wlr_xwayland->seat));
    /* configure xwayland view geometry by the new scale */
    struct kywc_output *output = data;
    xwayland_view_configure_all(xwayland, output);
}

static void xwayland_get_atoms(xcb_connection_t *xcb_conn)
{
    xcb_intern_atom_cookie_t cookies[ATOM_LAST];
    for (size_t i = 0; i < ATOM_LAST; i++) {
        cookies[i] = xcb_intern_atom(xcb_conn, 0, strlen(atom_map[i]), atom_map[i]);
    }
    for (size_t i = 0; i < ATOM_LAST; i++) {
        xcb_generic_error_t *error = NULL;
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xcb_conn, cookies[i], &error);
        if (reply != NULL && error == NULL) {
            xwayland->atoms[i] = reply->atom;
        }
        free(reply);

        if (error != NULL) {
            kywc_log(KYWC_ERROR, "Could not resolve atom %s, X11 error code %d", atom_map[i],
                     error->error_code);
            free(error);
            break;
        }
    }
}

static void xwayland_get_shape_extension(xcb_connection_t *xcb_conn)
{
    xwayland->shape = xcb_get_extension_data(xcb_conn, &xcb_shape_id);
    if (!xwayland->shape || !xwayland->shape->present) {
        kywc_log(KYWC_WARN, "Shape not available");
        xwayland->shape = NULL;
        return;
    }

    xcb_shape_query_version_cookie_t shape_cookie;
    xcb_shape_query_version_reply_t *shape_reply;
    shape_cookie = xcb_shape_query_version(xcb_conn);
    shape_reply = xcb_shape_query_version_reply(xcb_conn, shape_cookie, NULL);

    kywc_log(KYWC_INFO, "Shape version: %" PRIu32 ".%" PRIu32, shape_reply->major_version,
             shape_reply->minor_version);
    free(shape_reply);
}

static void xwayland_get_xfixes_extension(xcb_connection_t *xcb_conn)
{
    xwayland->xfixes = xcb_get_extension_data(xcb_conn, &xcb_xfixes_id);
    if (!xwayland->xfixes || !xwayland->xfixes->present) {
        kywc_log(KYWC_WARN, "Xfixes not available");
        xwayland->xfixes = NULL;
        return;
    }

    xcb_xfixes_query_version_cookie_t xfixes_cookie;
    xcb_xfixes_query_version_reply_t *xfixes_reply;
    xfixes_cookie =
        xcb_xfixes_query_version(xcb_conn, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);
    xfixes_reply = xcb_xfixes_query_version_reply(xcb_conn, xfixes_cookie, NULL);

    kywc_log(KYWC_INFO, "Xfixes version: %" PRIu32 ".%" PRIu32, xfixes_reply->major_version,
             xfixes_reply->minor_version);
    free(xfixes_reply);
}

static void xwayland_get_sync_extension(xcb_connection_t *xcb_conn)
{
    xwayland->sync = xcb_get_extension_data(xcb_conn, &xcb_sync_id);
    if (!xwayland->sync || !xwayland->sync->present) {
        kywc_log(KYWC_WARN, "Sync not available");
        xwayland->sync = NULL;
        return;
    }

    xcb_sync_initialize_cookie_t sync_cookie =
        xcb_sync_initialize(xcb_conn, XCB_SYNC_MAJOR_VERSION, XCB_SYNC_MINOR_VERSION);
    xcb_sync_initialize_reply_t *sync_reply =
        xcb_sync_initialize_reply(xcb_conn, sync_cookie, NULL);

    kywc_log(KYWC_INFO, "Sync version: %" PRIu32 ".%" PRIu32, sync_reply->major_version,
             sync_reply->minor_version);
    free(sync_reply);
}

static void xwayland_get_resources(xcb_connection_t *xcb_conn)
{
    xcb_prefetch_extension_data(xcb_conn, &xcb_shape_id);
    xcb_prefetch_extension_data(xcb_conn, &xcb_sync_id);

    xwayland_get_atoms(xcb_conn);

    /**
     * workaround to fix java apps show blank window
     * wlcom is not a reparenting window manager
     * or set _JAVA_AWT_WM_NONREPARENTING=1
     */
    const char name[] = "LG3D";
    xcb_change_property(xcb_conn, XCB_PROP_MODE_REPLACE, xwayland->screen->root,
                        xwayland->atoms[NET_SUPPORTING_WM_CHECK], XCB_ATOM_WINDOW, 32, 1,
                        &xwayland->screen->root);
    xcb_change_property(xcb_conn, XCB_PROP_MODE_REPLACE, xwayland->screen->root,
                        xwayland->atoms[NET_WM_NAME], xwayland->atoms[UTF8_STRING], 8, strlen(name),
                        name);

    xwayland_get_shape_extension(xcb_conn);
    xwayland_get_xfixes_extension(xcb_conn);
    xwayland_get_sync_extension(xcb_conn);
}

void xwayland_surface_shape_select_input(struct wlr_xwayland_surface *surface, bool enabled)
{
    if (xwayland->shape) {
        xcb_shape_select_input(xwayland->xcb_conn, surface->window_id, enabled);
        xcb_flush(xwayland->xcb_conn);
    }
}

static bool xwayland_get_shape_region(xcb_window_t window, xcb_shape_kind_t kind,
                                      pixman_region32_t *region, int *count)
{
    xcb_shape_get_rectangles_reply_t *reply = xcb_shape_get_rectangles_reply(
        xwayland->xcb_conn, xcb_shape_get_rectangles_unchecked(xwayland->xcb_conn, window, kind),
        NULL);
    if (!reply) {
        return false;
    }

    xcb_rectangle_t *rects = xcb_shape_get_rectangles_rectangles(reply);
    int len = xcb_shape_get_rectangles_rectangles_length(reply);
    if (region) {
        for (int i = 0; i < len; i++) {
            pixman_region32_union_rect(
                region, region, xwayland_unscale(rects[i].x), xwayland_unscale(rects[i].y),
                xwayland_unscale(rects[i].width), xwayland_unscale(rects[i].height));
        }
    }
    if (count) {
        *count = len;
    }

    free(reply);
    return true;
}

void xwayland_surface_apply_shape_region(struct wlr_xwayland_surface *surface)
{
    if (!xwayland->shape || !surface->surface) {
        return;
    }

    struct scene_buffer *buffer = scene_buffer_try_from_surface(surface->surface);
    if (!buffer) {
        return;
    }

    xcb_shape_query_extents_reply_t *reply = xcb_shape_query_extents_reply(
        xwayland->xcb_conn,
        xcb_shape_query_extents_unchecked(xwayland->xcb_conn, surface->window_id), NULL);
    if (!reply) {
        return;
    }

    pixman_region32_t region;
    pixman_region32_init(&region);

    if (reply->clip_shaped) {
        if (xwayland_get_shape_region(surface->window_id, XCB_SHAPE_SK_CLIP, &region, NULL)) {
            scene_node_set_clip_region(&buffer->node, &region);
        }
    } else if (reply->bounding_shaped) {
        if (xwayland_get_shape_region(surface->window_id, XCB_SHAPE_SK_BOUNDING, &region, NULL)) {
            scene_node_set_clip_region(&buffer->node, &region);
            scene_node_set_input_region(&buffer->node, &region);
        }
    }

    int count = 0;
    xwayland_get_shape_region(surface->window_id, XCB_SHAPE_SK_INPUT, NULL, &count);
    scene_node_set_input_bypassed(&buffer->node, count == 0);

    pixman_region32_fini(&region);
    free(reply);
}

static int xwayland_handle_shape_notify(xcb_shape_notify_event_t *notify)
{
    pixman_region32_t region;
    pixman_region32_init(&region);

    if (!xwayland_get_shape_region(notify->affected_window, notify->shape_kind, &region, NULL)) {
        pixman_region32_fini(&region);
        return 1;
    }

    if (!xwayland_unmanaged_set_shape_region(xwayland, notify->affected_window, notify->shape_kind,
                                             &region)) {
        xwayland_view_set_shape_region(xwayland, notify->affected_window, notify->shape_kind,
                                       &region);
    }

    pixman_region32_fini(&region);
    return 1;
}

void xwayland_set_cursor(struct seat *seat)
{
    if (!xwayland || !xwayland->wlr_xwayland || xwayland->wlr_xwayland->seat != seat->wlr_seat) {
        return;
    }

    wlr_xcursor_manager_load(seat->cursor->xcursor_manager, xwayland->scale);
    struct wlr_xcursor *xcursor =
        wlr_xcursor_manager_get_xcursor(seat->cursor->xcursor_manager, "left_ptr", xwayland->scale);
    if (xcursor) {
        struct wlr_xcursor_image *image = xcursor->images[0];
        wlr_xwayland_set_cursor(xwayland->wlr_xwayland, image->buffer, image->width * 4,
                                image->width, image->height, image->hotspot_x, image->hotspot_y);
    }

    xwayland_update_xresources(xwayland->xcb_conn);
}

static void xwayland_set_seat(struct seat *seat)
{
    if (xwayland->wlr_xwayland->seat == seat->wlr_seat) {
        return;
    }

    wlr_xwayland_set_seat(xwayland->wlr_xwayland, seat->wlr_seat);
    wl_list_remove(&xwayland->seat_destroy.link);
    wl_signal_add(&seat->events.destroy, &xwayland->seat_destroy);

    /* update xwayland cursor */
    xwayland_set_cursor(seat);
}

void xwayland_update_seat(struct seat *seat)
{
    // hover by input_rebase will run here when xwm_destroy
    if (xwayland->server->terminate || !xwayland->wlr_xwayland || !xwayland->wlr_xwayland->seat) {
        return;
    }

    xwayland_set_seat(seat);
}

void xwayland_update_hovered_surface(struct wlr_surface *surface)
{
    if (!xwayland || xwayland->hoverd_surface == surface) {
        return;
    }

    if (xwayland->hoverd_surface) {
        wl_list_remove(&xwayland->surface_unmap.link);
        wl_list_remove(&xwayland->view_minimized.link);
        wl_list_remove(&xwayland->view_unmap.link);
    }

    xwayland->hoverd_surface = surface;
    wl_signal_add(&surface->events.unmap, &xwayland->surface_unmap);
    struct view *view = view_try_from_wlr_surface(surface);
    if (view) {
        wl_signal_add(&view->base.events.minimize, &xwayland->view_minimized);
        wl_signal_add(&view->base.events.unmap, &xwayland->view_unmap);
    } else {
        wl_list_init(&xwayland->view_minimized.link);
        wl_list_init(&xwayland->view_unmap.link);
    }
}

static void handle_xwayland_ready(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_INFO, "Xwayland is ready");

    xwayland->xcb_conn = wlr_xwayland_get_xwm_connection(xwayland->wlr_xwayland);
    xcb_screen_iterator_t screen_iterator =
        xcb_setup_roots_iterator(xcb_get_setup(xwayland->xcb_conn));
    xwayland->screen = screen_iterator.data;

    /* use the default seat0 */
    xwayland_set_seat(input_manager_get_default_seat());

    xwayland_get_resources(xwayland->xcb_conn);

    // for selection
    int width = 0, height = 0;
    output_layout_get_size(&width, &height);
    xwayland_create_seletion_window(xwayland, &xwayland->window_catcher, 0, 0, width, height);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&xwayland->server_destroy.link);
    wl_list_remove(&xwayland->max_scale.link);
    wl_list_remove(&xwayland->surface_unmap.link);
    wl_list_remove(&xwayland->view_minimized.link);
    wl_list_remove(&xwayland->view_unmap.link);
    wl_list_remove(&xwayland->active_workspace.link);
    free(xwayland);
    xwayland = NULL;
}

static bool xwayland_filter_global(const struct wl_client *client, void *data)
{
    /* only expose this global to Xwayland clients */
    return xwayland_check_client(client);
}

xcb_sync_counter_t xwayland_get_sync_counter(xcb_window_t window_id)
{
    if (!xwayland->sync) {
        return XCB_NONE;
    }

    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwayland->xcb_conn, 0, window_id,
                         xwayland->atoms[NET_WM_SYNC_REQUEST_COUNTER], XCB_ATOM_CARDINAL, 0, 1);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (reply == NULL || reply->value_len == 0) {
        kywc_log(KYWC_INFO, "Failed to get window 0x%x sync request counter", window_id);
        free(reply);
        return XCB_NONE;
    }

    uint32_t *data = (uint32_t *)xcb_get_property_value(reply);
    xcb_sync_counter_t counter = data[0];
    free(reply);

    if (counter == XCB_NONE) {
        return XCB_NONE;
    }

    xcb_sync_int64_t value = { .hi = 0, .lo = 0 };
    xcb_sync_set_counter(xwayland->xcb_conn, counter, value);
    xcb_flush(xwayland->xcb_conn);

    xwayland_view_reset_sync_counter(xwayland, window_id);

    return counter;
}

xcb_sync_alarm_t xwayland_create_sync_alarm(xcb_window_t window_id, xcb_sync_counter_t counter)
{
    if (!xwayland->sync || counter == XCB_NONE) {
        return XCB_NONE;
    }

    const uint32_t mask =
        XCB_SYNC_CA_COUNTER | XCB_SYNC_CA_VALUE_TYPE | XCB_SYNC_CA_TEST_TYPE | XCB_SYNC_CA_EVENTS;
    const uint32_t values[] = { counter, XCB_SYNC_VALUETYPE_RELATIVE,
                                XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON, 1 };
    xcb_sync_alarm_t alarm = xcb_generate_id(xwayland->xcb_conn);
    xcb_void_cookie_t cookie =
        xcb_sync_create_alarm_checked(xwayland->xcb_conn, alarm, mask, values);
    xcb_generic_error_t *error = xcb_request_check(xwayland->xcb_conn, cookie);
    if (error) {
        alarm = XCB_NONE;
    } else {
        xcb_sync_change_alarm_value_list_t value = { .value.lo = 1, .delta.lo = 1 };
        xcb_sync_change_alarm_aux(xwayland->xcb_conn, alarm, XCB_SYNC_CA_VALUE | XCB_SYNC_CA_DELTA,
                                  &value);
        xcb_flush(xwayland->xcb_conn);
    }

    return alarm;
}

void xwayland_send_sync_request(xcb_window_t window_id, xcb_sync_int64_t *value)
{
    const uint32_t lo = value->lo++;
    if (lo > value->lo) {
        value->hi++;
    }

    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .window = window_id,
        .type = xwayland->atoms[WM_PROTOCOLS],
        .data.data32[0] = xwayland->atoms[NET_WM_SYNC_REQUEST],
        .data.data32[1] = XCB_TIME_CURRENT_TIME,
        .data.data32[2] = value->lo,
        .data.data32[3] = value->hi,
    };

    xcb_send_event(xwayland->xcb_conn, false, window_id, XCB_EVENT_MASK_NO_EVENT,
                   (const char *)&event);
    xcb_flush(xwayland->xcb_conn);
}

void xwayland_set_allow_commits(xcb_window_t window_id, bool allow)
{
    uint32_t value = allow;
    xcb_change_property(xwayland->xcb_conn, XCB_PROP_MODE_REPLACE, window_id,
                        xwayland->atoms[XWAYLAND_ALLOW_COMMITS], XCB_ATOM_CARDINAL, 32, 1, &value);
    xcb_flush(xwayland->xcb_conn);
}

int xwayland_read_wm_state(xcb_window_t window_id)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(
        xwayland->xcb_conn, 0, window_id, xwayland->atoms[NET_WM_STATE], XCB_ATOM_ANY, 0, 2048);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (reply == NULL) {
        kywc_log(KYWC_ERROR, "Failed to get window state property");
        return 0;
    }

    /* nothing to be handled */
    if (reply->value_len == 0) {
        free(reply);
        return 1;
    }

    struct wlr_xwayland_surface *surface = xwayland_view_look_surface(xwayland, window_id);
    if (!surface) {
        free(reply);
        return 0;
    }

    xcb_atom_t *atoms = xcb_get_property_value(reply);
    int ret = 0;
    for (uint32_t i = 0; i < reply->value_len; i++) {
        if (atoms[i] == xwayland->atoms[KDE_NET_WM_STATE_SKIP_SWITCHER]) {
            xwayland_view_set_skip_switcher(surface, true);
            ret = reply->value_len == 1;
            break;
        }
    }

    free(reply);
    return ret;
}

int xwayland_read_wm_icon(xcb_window_t window_id)
{
    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwayland->xcb_conn, 0, window_id, xwayland->atoms[NET_WM_ICON],
                         XCB_ATOM_CARDINAL, 0, 0xffffffff);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (!reply || reply->value_len < 3 || reply->format != 32) {
        free(reply);
        return 0;
    }

    struct wlr_xwayland_surface *surface = xwayland_view_look_surface(xwayland, window_id);
    if (!surface || !surface->surface) {
        free(reply);
        return 0;
    }

    /* clear icon first */
    xwayland_view_clear_wm_icon(surface);

    uint32_t *data = (uint32_t *)xcb_get_property_value(reply);
    for (unsigned int j = 0; j < reply->value_len - 2;) {
        uint32_t width = data[j++], height = data[j++];
        uint32_t size = width * height * sizeof(uint32_t);
        if (j + width * height > reply->value_len) {
            kywc_log(KYWC_WARN, "Proposed size leads to out of bounds (%d x %d) ", width, height);
            break;
        }
        if (width > 1024 || height > 1024) {
            kywc_log(KYWC_WARN, "Found huge icon may be ill-encoded (%d x %d)", width, height);
        }

        xwayland_view_add_new_wm_icon(surface, width, height, size, &data[j]);
        j += width * height;
    }
    xwayland_view_update_icon(surface);

    free(reply);
    return 1;
}

int xwayland_apply_wm_window_opacity(xcb_window_t window_id, bool delete)
{
    if (delete) {
        if (!xwayland_unmanaged_set_opacity(xwayland, window_id, 1)) {
            return xwayland_view_set_opacity(xwayland, window_id, 1);
        }
        return 1;
    }

    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwayland->xcb_conn, 0, window_id, xwayland->atoms[NET_WM_WINDOW_OPACITY],
                         XCB_ATOM_CARDINAL, 0, 1);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (!reply || reply->value_len != 1 || reply->format != 32) {
        free(reply);
        return 0;
    }

    uint32_t *value = (uint32_t *)xcb_get_property_value(reply);
    float opacity = value[0] == 0xffffffff ? 1.0 : value[0] * 1.0 / 0xffffffff;
    free(reply);

    if (!xwayland_unmanaged_set_opacity(xwayland, window_id, opacity)) {
        return xwayland_view_set_opacity(xwayland, window_id, opacity);
    }

    return 1;
}

int xwayland_apply_blur_region(xcb_window_t window_id, bool delete)
{
    if (delete) {
        if (!xwayland_unmanaged_set_blur_region(xwayland, window_id, NULL)) {
            xwayland_view_set_blur_region(xwayland, window_id, NULL);
        }
        return 1;
    }

    xcb_get_property_cookie_t cookie = xcb_get_property(
        xwayland->xcb_conn, 0, window_id, xwayland->atoms[KDE_NET_WM_BLUR_BEHIND_REGION],
        XCB_ATOM_CARDINAL, 0, 0xffffffff);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (!reply || reply->format != 32) {
        free(reply);
        return 0;
    }

    /* from V10sp1, the last value is the blur strength.we ignore it. */
    uint32_t len;
    if (reply->value_len % 4 == 0) {
        len = reply->value_len;
    } else if (reply->value_len % 4 == 1) {
        len = reply->value_len - 1;
    } else {
        return 0;
    }

    pixman_region32_t region;
    pixman_region32_init(&region);
    uint32_t *value = (uint32_t *)xcb_get_property_value(reply);
    for (uint32_t i = 0; i < len; i += 4) {
        pixman_region32_union_rect(&region, &region, xwayland_unscale(value[i]),
                                   xwayland_unscale(value[i + 1]), xwayland_unscale(value[i + 2]),
                                   xwayland_unscale(value[i + 3]));
    }

    if (!xwayland_unmanaged_set_blur_region(xwayland, window_id, &region)) {
        xwayland_view_set_blur_region(xwayland, window_id, &region);
    }

    free(reply);
    pixman_region32_fini(&region);
    return 1;
}

int xwayland_apply_opaque_region(xcb_window_t window_id, bool delete)
{
    pixman_region32_t region;
    pixman_region32_init(&region);
    if (delete) {
        if (!xwayland_unmanaged_set_opaque_region(xwayland, window_id, &region)) {
            xwayland_view_set_opaque_region(xwayland, window_id, &region);
        }
        pixman_region32_fini(&region);
        return 1;
    }

    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwayland->xcb_conn, 0, window_id, xwayland->atoms[NET_WM_OPAQUE_REGION],
                         XCB_ATOM_CARDINAL, 0, 0xffffffff);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (!reply || reply->format != 32 || reply->value_len % 4) {
        free(reply);
        return 0;
    }

    uint32_t *data = (uint32_t *)xcb_get_property_value(reply);
    for (unsigned int i = 0; i < reply->value_len; i += 4) {
        pixman_region32_union_rect(&region, &region, xwayland_unscale(data[i]),
                                   xwayland_unscale(data[i + 1]), xwayland_unscale(data[i + 2]),
                                   xwayland_unscale(data[i + 3]));
    }

    if (!xwayland_unmanaged_set_opaque_region(xwayland, window_id, &region)) {
        xwayland_view_set_opaque_region(xwayland, window_id, &region);
    }

    free(reply);
    pixman_region32_fini(&region);
    return 1;
}

int xwayland_apply_ukui_decoration(xcb_window_t window_id, bool delete)
{
    if (delete) {
        xwayland_view_set_no_title(xwayland, window_id, false);
        return 1;
    }

    xcb_get_property_cookie_t cookie =
        xcb_get_property(xwayland->xcb_conn, 0, window_id, xwayland->atoms[KWIN_UKUI_DECORATION],
                         XCB_ATOM_ANY, 0, 1);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwayland->xcb_conn, cookie, NULL);
    if (!reply || reply->value_len == 0) {
        free(reply);
        return 0;
    }

    uint8_t *data = (uint8_t *)xcb_get_property_value(reply);
    if (data && data[0]) {
        xwayland_view_set_no_title(xwayland, window_id, true);
    }

    free(reply);
    return 1;
}

static void xwayland_handle_moveresize_message(struct xwayland_server *xwayland,
                                               xcb_client_message_event_t *client_message)
{
    struct wlr_xwayland_surface *surface =
        xwayland_view_look_surface(xwayland, client_message->window);
    if (!surface || !surface->surface) {
        return;
    }

    // gravity = client_message->data.data32[0] & 0xff);
    int value_mask = (client_message->data.data32[0] & 0xf00) >> 8;
    // source = (client_message->data.data32[0] & 0xf000) >> 12;
    struct kywc_box box = {
        .x = value_mask & XCB_CONFIG_WINDOW_X ? (int)client_message->data.data32[1] : surface->x,
        .y = value_mask & XCB_CONFIG_WINDOW_Y ? (int)client_message->data.data32[2] : surface->y,
        .width = value_mask & XCB_CONFIG_WINDOW_WIDTH ? (int)client_message->data.data32[3]
                                                      : surface->width,
        .height = value_mask & XCB_CONFIG_WINDOW_HEIGHT ? (int)client_message->data.data32[4]
                                                        : surface->height
    };
    xwayland_view_move_resize(surface, &box);
}

static void xwayland_handle_showing_desktop_message(xcb_client_message_event_t *client_message)
{
    bool state = client_message->data.data32[0];
    view_manager_show_desktop(state, true);
}

/* return 0 as we only handle few things */
static int xwayland_handle_event(struct wlr_xwm *xwm, xcb_generic_event_t *event)
{
    const uint8_t response_type = event->response_type & 0x7f;

    if (response_type == XCB_PROPERTY_NOTIFY) {
        xcb_property_notify_event_t *ev = (xcb_property_notify_event_t *)event;
        bool delete = ev->state == XCB_PROPERTY_DELETE;
        if (ev->atom == xwayland->atoms[NET_WM_STATE]) {
            return xwayland_read_wm_state(ev->window);
        } else if (ev->atom == xwayland->atoms[NET_WM_ICON]) {
            return xwayland_read_wm_icon(ev->window);
        } else if (ev->atom == xwayland->atoms[NET_WM_WINDOW_OPACITY]) {
            return xwayland_apply_wm_window_opacity(ev->window, delete);
        } else if (ev->atom == xwayland->atoms[NET_WM_SYNC_REQUEST_COUNTER]) {
            xwayland_get_sync_counter(ev->window);
            return 1;
        } else if (ev->atom == xwayland->atoms[KDE_NET_WM_BLUR_BEHIND_REGION]) {
            return xwayland_apply_blur_region(ev->window, delete);
        } else if (ev->atom == xwayland->atoms[NET_WM_OPAQUE_REGION]) {
            return xwayland_apply_opaque_region(ev->window, delete);
        } else if (ev->atom == xwayland->atoms[KWIN_UKUI_DECORATION]) {
            return xwayland_apply_ukui_decoration(ev->window, delete);
        }
    } else if (response_type == XCB_CLIENT_MESSAGE) {
        xcb_client_message_event_t *ev = (xcb_client_message_event_t *)event;
        if (ev->type == xwayland->atoms[NET_MOVERESIZE_WINDOW]) {
            xwayland_handle_moveresize_message(xwayland, ev);
            return 1;
        } else if (ev->type == xwayland->atoms[NET_SHOWING_DESKTOP]) {
            xwayland_handle_showing_desktop_message(ev);
            return 1;
        } else if (xwayland_handle_dnd_message(xwayland, ev)) {
            return 1;
        }
    } else if (xwayland->shape &&
               response_type == xwayland->shape->first_event + XCB_SHAPE_NOTIFY) {
        return xwayland_handle_shape_notify((xcb_shape_notify_event_t *)event);
    } else if (xwayland->sync &&
               response_type == xwayland->sync->first_event + XCB_SYNC_ALARM_NOTIFY) {
        xcb_sync_alarm_notify_event_t *ev = (xcb_sync_alarm_notify_event_t *)event;
        xwayland_view_ack_sync(xwayland, ev->alarm, ev->counter_value);
        return 1;
    } else if (xwayland_handle_selection_event(xwayland, event)) {
        return 1;
    }

    return 0;
}

static void handle_surface_unmap(struct wl_listener *listener, void *data)
{
    wl_list_remove(&xwayland->surface_unmap.link);
    wl_list_init(&xwayland->surface_unmap.link);
    wl_list_remove(&xwayland->view_minimized.link);
    wl_list_init(&xwayland->view_minimized.link);
    wl_list_remove(&xwayland->view_unmap.link);
    wl_list_init(&xwayland->view_unmap.link);

    xwayland->hoverd_surface = NULL;
}

static void handle_view_minimized(struct wl_listener *listener, void *data)
{
    struct view *view = view_try_from_wlr_surface(xwayland->hoverd_surface);
    if (!view->base.minimized) {
        return;
    }

    wl_list_remove(&xwayland->surface_unmap.link);
    wl_list_init(&xwayland->surface_unmap.link);
    wl_list_remove(&xwayland->view_minimized.link);
    wl_list_init(&xwayland->view_minimized.link);
    wl_list_remove(&xwayland->view_unmap.link);
    wl_list_init(&xwayland->view_unmap.link);

    xwayland->hoverd_surface = NULL;
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&xwayland->seat_destroy.link);
    wl_list_init(&xwayland->seat_destroy.link);

    xwayland_update_seat(input_manager_get_default_seat());
}

static bool xwayland_view_iterator(struct kywc_view *kywc_view, void *data)
{
    struct view *view = view_from_kywc_view(kywc_view);
    if (xwayland_check_view(view)) {
        xwayland_view_restack(view, NULL, true);
    }
    return false;
}

static void handle_active_workspace(struct wl_listener *listener, void *data)
{
    /* xwayland server is destroyed or not ready */
    if (!xwayland->wlr_xwayland || !xwayland->wlr_xwayland->xwm) {
        return;
    }

    view_manager_for_each_stack_view(xwayland_view_iterator, NULL, true, NULL);
    xwayland_restack_unmanaged(xwayland);
}

static void handle_shell_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&xwayland->shell_destroy.link);
    security_remove_global_filter(xwayland->shell->global);
    xwayland->shell = NULL;
}

bool xwayland_server_create(struct server *server)
{
    if (!server->options.enable_xwayland) {
        kywc_log(KYWC_INFO, "Xwayland is disabled by cmdline");
        return false;
    }

    xwayland = calloc(1, sizeof(*xwayland));
    if (!xwayland) {
        kywc_log(KYWC_ERROR, "Cannot create xwayland server");
        return false;
    }

    xwayland->wlr_xwayland =
        wlr_xwayland_create(server->display, server->compositor, server->options.lazy_xwayland);
    if (!xwayland->wlr_xwayland) {
        kywc_log(KYWC_ERROR, "Cannot create wlroots xwayland server");
        free(xwayland);
        xwayland = NULL;
        return false;
    }

    if (server->session) {
        generate_xauthority_file(xwayland->wlr_xwayland->server->display);
    } else {
        unsetenv("XAUTHORITY");
    }

    xwayland->scale = 1.0;
    xwayland->server = server;
    wl_list_init(&xwayland->surfaces);
    wl_list_init(&xwayland->unmanaged_surfaces);
    xwayland->wlr_xwayland->user_event_handler = xwayland_handle_event;

    xwayland->new_xwayland_surface.notify = handle_new_xwayland_surface;
    wl_signal_add(&xwayland->wlr_xwayland->events.new_surface, &xwayland->new_xwayland_surface);
    xwayland->xwayland_ready.notify = handle_xwayland_ready;
    wl_signal_add(&xwayland->wlr_xwayland->events.ready, &xwayland->xwayland_ready);
    xwayland->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &xwayland->server_destroy);
    xwayland->max_scale.notify = handle_max_scale;
    output_manager_add_max_scale_listener(&xwayland->max_scale);
    xwayland->seat_destroy.notify = handle_seat_destroy;
    wl_list_init(&xwayland->seat_destroy.link);
    xwayland->active_workspace.notify = handle_active_workspace;
    workspace_manager_add_active_listener(&xwayland->active_workspace);

    xwayland->surface_unmap.notify = handle_surface_unmap;
    wl_list_init(&xwayland->surface_unmap.link);
    xwayland->view_minimized.notify = handle_view_minimized;
    wl_list_init(&xwayland->view_minimized.link);
    xwayland->view_unmap.notify = handle_surface_unmap;
    wl_list_init(&xwayland->view_unmap.link);

    xwayland->shell = xwayland->wlr_xwayland->shell_v1;
    xwayland->shell_destroy.notify = handle_shell_destroy;
    wl_signal_add(&xwayland->shell->events.destroy, &xwayland->shell_destroy);
    security_add_global_filter(xwayland->shell->global, xwayland_filter_global, NULL);

    setenv("DISPLAY", xwayland->wlr_xwayland->display_name, true);
    kywc_log(KYWC_INFO, "Xwayland is running on display %s", xwayland->wlr_xwayland->display_name);

    return true;
}

void xwayland_server_destroy(void)
{
    if (!xwayland) {
        return;
    }

    wl_list_remove(&xwayland->xwayland_ready.link);
    wl_list_remove(&xwayland->new_xwayland_surface.link);
    wl_list_remove(&xwayland->seat_destroy.link);

    xwayland_end_drag_x11(xwayland);

    struct wlr_xwayland *wlr_xwayland = xwayland->wlr_xwayland;
    /* prevent xwayland_update_seat in hover */
    xwayland->wlr_xwayland = NULL;
    wlr_xwayland_destroy(wlr_xwayland);
}

bool xwayland_check_client(const struct wl_client *client)
{
    return xwayland && xwayland->wlr_xwayland && xwayland->wlr_xwayland->server &&
           xwayland->wlr_xwayland->server->client == client;
}

int xwayland_unscale(int value)
{
    if (!xwayland) {
        return value;
    }

    int32_t K = (int32_t)(xwayland->scale * 256.0f);
    int64_t n = (int64_t)value * 256;
    int64_t b = n >= 0 ? (n + K / 2) / K : (n - K / 2) / K;
    return CLAMP(b, INT32_MIN, INT32_MAX);
}

int xwayland_scale(int value)
{
    if (!xwayland) {
        return value;
    }

    int32_t K = (int32_t)(xwayland->scale * 256.0f);
    int64_t p = (int64_t)value * K;
    int64_t a = (p >= 0) ? (p + 128) / 256 : (p - 128) / 256;
    return CLAMP(a, INT32_MIN, INT32_MAX);
}

float xwayland_get_scale(void)
{
    return xwayland ? xwayland->scale : 1.0;
}

bool xwayland_surface_has_type(struct wlr_xwayland_surface *wlr_xwayland_surface, int type)
{
    for (size_t i = 0; i < wlr_xwayland_surface->window_type_len; ++i) {
        xcb_atom_t atom = wlr_xwayland_surface->window_type[i];
        if (atom == xwayland->atoms[type]) {
            return true;
        }
    }
    return false;
}

bool xwayland_surface_has_input(struct wlr_xwayland_surface *wlr_xwayland_surface, uint32_t input)
{
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(
        xwayland->xcb_conn,
        xcb_get_window_attributes_unchecked(xwayland->xcb_conn, wlr_xwayland_surface->window_id),
        NULL);
    if (!reply) {
        return true;
    }

    uint32_t input_mask = 0;
    if (input & INPUT_MASK_POINTER) {
        input_mask |= XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                      XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_ENTER_WINDOW |
                      XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_POINTER_MOTION;
    }
    if (input & INPUT_MASK_KEYBOARD) {
        input_mask |= XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;
    }

    bool has_input = reply->all_event_masks & input_mask;
    free(reply);

    return has_input;
}

static char *xwayland_get_atom_name(xcb_atom_t atom)
{
    xcb_get_atom_name_cookie_t name_cookie = xcb_get_atom_name(xwayland->xcb_conn, atom);
    xcb_get_atom_name_reply_t *name_reply =
        xcb_get_atom_name_reply(xwayland->xcb_conn, name_cookie, NULL);
    if (name_reply == NULL) {
        return strdup("UNKNOWN");
    }
    size_t len = xcb_get_atom_name_name_length(name_reply);
    char *buf = xcb_get_atom_name_name(name_reply); // not a C string
    char *name = strndup(buf, len);
    free(name_reply);
    return name;
}

char *xwayland_mime_type_from_atom(xcb_atom_t atom)
{
    if (atom == xwayland->atoms[UTF8_STRING]) {
        return strdup("text/plain;charset=utf-8");
    } else if (atom == xwayland->atoms[TEXT]) {
        return strdup("text/plain");
    } else {
        return xwayland_get_atom_name(atom);
    }
}

void xwayland_fixup_pointer_position(struct wlr_surface *surface)
{
    struct wlr_seat *seat = xwayland->wlr_xwayland->seat;
    if (xwayland->hoverd_surface || !xwayland->wlr_xwayland->seat) {
        return;
    }

    struct wl_client *wl_client = wl_resource_get_client(surface->resource);
    struct wlr_seat_client *client = wlr_seat_client_for_wl_client(seat, wl_client);
    if (!client) {
        return;
    }

    uint32_t serial = wlr_seat_client_next_serial(client);
    struct wl_resource *resource;
    wl_resource_for_each(resource, &client->pointers) {
        if (!wlr_seat_client_from_pointer_resource(resource)) {
            continue;
        }

        wl_pointer_send_enter(resource, serial, surface->resource, wl_fixed_from_double(FLT_MAX),
                              wl_fixed_from_double(FLT_MAX));
        wl_pointer_send_leave(resource, serial, surface->resource);
        if (wl_resource_get_version(resource) >= WL_POINTER_FRAME_SINCE_VERSION) {
            wl_pointer_send_frame(resource);
        }
    }
}

void xwayland_surface_debug_type(struct wlr_xwayland_surface *wlr_xwayland_surface)
{
    for (size_t i = 0; i < wlr_xwayland_surface->window_type_len; ++i) {
        xcb_atom_t atom = wlr_xwayland_surface->window_type[i];
        char *atom_name = xwayland_get_atom_name(atom);
        kywc_log(KYWC_INFO, "%s: type atom %s %ld(%ld)", wlr_xwayland_surface->class, atom_name, i,
                 wlr_xwayland_surface->window_type_len);
        free(atom_name);
    }
    kywc_log(KYWC_INFO, "%s: OR %d size %d x %d %d", wlr_xwayland_surface->class,
             wlr_xwayland_surface->override_redirect, wlr_xwayland_surface->width,
             wlr_xwayland_surface->height, wlr_xwayland_surface->fullscreen);
}

void xwayland_update_workarea(void)
{
    if (!xwayland || !xwayland->wlr_xwayland || !xwayland->wlr_xwayland->xwm) {
        return;
    }

    struct wlr_box box;
    output_layout_get_workarea(&box);
    box.x = xwayland_scale(box.x);
    box.y = xwayland_scale(box.y);
    box.width = xwayland_scale(box.width);
    box.height = xwayland_scale(box.height);
    wlr_xwayland_set_workareas(xwayland->wlr_xwayland, &box, 1);
}

enum wl_data_device_manager_dnd_action data_device_manager_dnd_action_from_atom(enum atom_name atom)
{
    if (atom == xwayland->atoms[DND_ACTION_COPY] || atom == xwayland->atoms[DND_ACTION_PRIVATE]) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    } else if (atom == xwayland->atoms[DND_ACTION_MOVE]) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    } else if (atom == xwayland->atoms[DND_ACTION_ASK]) {
        return WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
}

xcb_atom_t data_device_manager_dnd_action_to_atom(enum wl_data_device_manager_dnd_action action)
{
    if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
        return xwayland->atoms[DND_ACTION_COPY];
    } else if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
        return xwayland->atoms[DND_ACTION_MOVE];
    } else if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
        return xwayland->atoms[DND_ACTION_ASK];
    }
    return XCB_ATOM_NONE;
}

bool xwayland_is_dragging_x11(struct seat *seat)
{
    if (!xwayland || !xwayland->wlr_xwayland || !xwayland->wlr_xwayland->seat) {
        return false;
    }

    struct seat *xwayland_seat = seat_from_wlr_seat(xwayland->wlr_xwayland->seat);
    return (!seat || xwayland_seat == seat) && xwayland->drag_x11;
}

void xwayland_send_touch_size(struct input *input, int touch_id, double touch_major,
                              double touch_minor)
{
    if (xwayland->impl.send_touch_size) {
        xwayland->impl.send_touch_size(input, touch_id, touch_major, touch_minor);
    }
}

void xwayland_send_touch_info(struct input *input, int major_minimum, int major_maximum,
                              int major_resolution, int minor_minimum, int minor_maximum,
                              int minor_resolution)
{
    if (xwayland->impl.send_touch_info) {
        xwayland->impl.send_touch_info(input, major_minimum, major_maximum, major_resolution,
                                       minor_minimum, minor_maximum, minor_resolution);
    }
}
