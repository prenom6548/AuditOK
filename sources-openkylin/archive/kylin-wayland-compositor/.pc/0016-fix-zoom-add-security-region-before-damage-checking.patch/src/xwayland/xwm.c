// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <float.h>

#include <xcb/composite.h>
#include <xcb/res.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xfixes.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/region.h>
#include <wlr/xcursor.h>

#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "painter.h"
#include "server.h"
#include "util/string.h"
#include "view/view.h"
#include "view/workspace.h"
#include "xwayland_p.h"

#define STARTUP_INFO_REMOVE_PREFIX "remove: ID="
struct pending_startup_id {
    char *msg;
    size_t len;
    xcb_window_t window;
    struct wl_list link;
};

static const struct wlr_addon_interface surface_addon_impl;

struct xwayland_surface *xwayland_surface_try_from_wlr_surface(struct wlr_surface *surface)
{
    struct wlr_addon *addon = wlr_addon_find(&surface->addons, NULL, &surface_addon_impl);
    if (addon == NULL) {
        return NULL;
    }
    struct xwayland_surface *xsurface = wl_container_of(addon, xsurface, surface_addon);
    return xsurface;
}

// TODO: replace this with hash table?
struct xwayland_surface *xwm_lookup_surface(struct xwm *xwm, xcb_window_t window_id)
{
    struct xwayland_surface *surface;
    wl_list_for_each(surface, &xwm->surfaces, link) {
        if (surface->window_id == window_id) {
            return surface;
        }
    }
    return NULL;
}

static int xwayland_surface_handle_ping_timeout(void *data)
{
    struct xwayland_surface *xsurface = data;

    if (xsurface->impl && xsurface->impl->ping_timeout) {
        xsurface->impl->ping_timeout(xsurface);
    }
    xsurface->pinging = false;
    return 1;
}

static void read_surface_client_id(struct xwm *xwm, struct xwayland_surface *xsurface,
                                   xcb_res_query_client_ids_cookie_t cookie)
{
    xcb_res_query_client_ids_reply_t *reply =
        xcb_res_query_client_ids_reply(xwm->xcb_conn, cookie, NULL);
    if (reply == NULL) {
        return;
    }

    uint32_t *pid = NULL;
    xcb_res_client_id_value_iterator_t iter = xcb_res_query_client_ids_ids_iterator(reply);
    while (iter.rem > 0) {
        if (iter.data->spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID &&
            xcb_res_client_id_value_value_length(iter.data) > 0) {
            pid = xcb_res_client_id_value_value(iter.data);
            break;
        }
        xcb_res_client_id_value_next(&iter);
    }
    if (pid == NULL) {
        free(reply);
        return;
    }
    xsurface->pid = *pid;
    free(reply);
}

static void xwm_set_net_active_window(struct xwm *xwm, xcb_window_t window)
{
    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_ACTIVE_WINDOW].atom, xwm->atoms[WINDOW].atom, 32, 1,
                        &window);
}

/*
 * Wrapper for xcb_send_event, which ensures that the event data is 32 byte big.
 */
void xwm_send_event_with_size(xcb_connection_t *c, uint8_t propagate, xcb_window_t destination,
                              uint32_t event_mask, const void *event, uint32_t length)
{
    if (length == 32) {
        xcb_send_event(c, propagate, destination, event_mask, event);
    } else if (length < 32) {
        char buf[32];
        memcpy(buf, event, length);
        memset(buf + length, 0, 32 - length);
        xcb_send_event(c, propagate, destination, event_mask, buf);
    } else {
        assert(false && "Event too long");
    }
}

static void xwm_send_wm_message(struct xwayland_surface *surface, xcb_client_message_data_t *data,
                                uint32_t event_mask)
{
    struct xwm *xwm = surface->xwm;

    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .sequence = 0,
        .window = surface->window_id,
        .type = xwm->atoms[WM_PROTOCOLS].atom,
        .data = *data,
    };

    xwm_send_event_with_size(xwm->xcb_conn, 0, surface->window_id, event_mask, &event,
                             sizeof(event));
    xcb_flush(xwm->xcb_conn);
}

static void xwm_set_net_client_list(struct xwm *xwm)
{
    // TODO: _NET_CLIENT_LIST is expected to be ordered by map time, but the
    // order of surfaces in `xwm->surfaces` is by creation time. The order of
    // windows _NET_CLIENT_LIST is wrong.

    size_t mapped_surfaces = 0;
    struct xwayland_surface *surface;
    wl_list_for_each(surface, &xwm->surfaces, link) {
        if (surface->surface != NULL && surface->surface->mapped) {
            mapped_surfaces++;
        }
    }

    xcb_window_t *windows = malloc(sizeof(xcb_window_t) * mapped_surfaces);
    if (!windows) {
        return;
    }

    size_t index = 0;
    wl_list_for_each(surface, &xwm->surfaces, link) {
        if (surface->surface != NULL && surface->surface->mapped) {
            windows[index++] = surface->window_id;
        }
    }

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_CLIENT_LIST].atom, XCB_ATOM_WINDOW, 32, mapped_surfaces,
                        windows);
    free(windows);
}

static void xwm_idle_set_net_client_list_stacking(void *data)
{
    struct xwm *xwm = data;
    xwm->idle_source = NULL;

    size_t num_surfaces = wl_list_length(&xwm->surfaces_in_stack_order);
    xcb_window_t *windows = malloc(sizeof(xcb_window_t) * num_surfaces);
    if (!windows) {
        return;
    }

    size_t i = 0;
    struct xwayland_surface *xsurface;
    wl_list_for_each(xsurface, &xwm->surfaces_in_stack_order, stack_link) {
        windows[i++] = xsurface->window_id;
    }

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_CLIENT_LIST_STACKING].atom, XCB_ATOM_WINDOW, 32,
                        num_surfaces, windows);
    free(windows);
}

static void xwm_schedule_net_client_list_stacking(struct xwm *xwm)
{
    if (xwm->idle_source) {
        return;
    }

    if (xwm->destroying) {
        xwm_idle_set_net_client_list_stacking(xwm);
        return;
    }

    struct wl_event_loop *loop = wl_display_get_event_loop(xwm->xwayland->server->display);
    xwm->idle_source = wl_event_loop_add_idle(loop, xwm_idle_set_net_client_list_stacking, xwm);
}

static void xsurface_set_net_wm_state(struct xwayland_surface *xsurface)
{
    struct xwm *xwm = xsurface->xwm;
    uint32_t property[13];
    size_t i = 0;

    if (xsurface->modal) {
        property[i++] = xwm->atoms[NET_WM_STATE_MODAL].atom;
    }
    if (xsurface->fullscreen) {
        property[i++] = xwm->atoms[NET_WM_STATE_FULLSCREEN].atom;
    }
    if (xsurface->maximized_vert) {
        property[i++] = xwm->atoms[NET_WM_STATE_MAXIMIZED_VERT].atom;
    }
    if (xsurface->maximized_horz) {
        property[i++] = xwm->atoms[NET_WM_STATE_MAXIMIZED_HORZ].atom;
    }
    if (xsurface->minimized) {
        property[i++] = xwm->atoms[NET_WM_STATE_HIDDEN].atom;
    }
    if (xsurface->sticky) {
        property[i++] = xwm->atoms[NET_WM_STATE_STICKY].atom;
    }
    if (xsurface->skip_taskbar) {
        property[i++] = xwm->atoms[NET_WM_STATE_SKIP_TASKBAR].atom;
    }
    if (xsurface->above) {
        property[i++] = xwm->atoms[NET_WM_STATE_ABOVE].atom;
    }
    if (xsurface->below) {
        property[i++] = xwm->atoms[NET_WM_STATE_BELOW].atom;
    }
    if (xsurface->demands_attention) {
        property[i++] = xwm->atoms[NET_WM_STATE_DEMANDS_ATTENTION].atom;
    }
    if (xsurface == xwm->focus_surface) {
        property[i++] = xwm->atoms[NET_WM_STATE_FOCUSED].atom;
    }
    if (xsurface->skip_switcher) {
        property[i++] = xwm->atoms[KDE_NET_WM_STATE_SKIP_SWITCHER].atom;
    }
    assert(i <= sizeof(property) / sizeof(property[0]));

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xsurface->window_id,
                        xwm->atoms[NET_WM_STATE].atom, XCB_ATOM_ATOM, 32, i, property);
}

// Gives input (keyboard) focus to a window.
// Normally followed by xwm_set_focused_window().
static void xwm_focus_window(struct xwm *xwm, struct xwayland_surface *xsurface)
{
    // We handle cases where focus_surface == xsurface because we
    // want to be able to deny FocusIn events.
    if (!xsurface) {
        xcb_set_input_focus_checked(xwm->xcb_conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                                    xwm->no_focus_window, XCB_CURRENT_TIME);
        return;
    }

    if (xsurface->override_redirect) {
        return;
    }

    xcb_client_message_data_t message_data = { 0 };
    message_data.data32[0] = xwm->atoms[WM_TAKE_FOCUS].atom;
    message_data.data32[1] = XCB_TIME_CURRENT_TIME;

    if (xsurface->hints && !xsurface->hints->input) {
        // if the surface doesn't allow the focus request, we will send him
        // only the take focus event. It will get the focus by itself.
        xwm_send_wm_message(xsurface, &message_data, XCB_EVENT_MASK_NO_EVENT);
    } else {
        xwm_send_wm_message(xsurface, &message_data, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);

        xcb_void_cookie_t cookie = xcb_set_input_focus(xwm->xcb_conn, XCB_INPUT_FOCUS_POINTER_ROOT,
                                                       xsurface->window_id, XCB_CURRENT_TIME);
        xwm->last_focus_seq = cookie.sequence;
    }
}

// Updates _NET_ACTIVE_WINDOW and _NET_WM_STATE when focus changes.
static void xwm_set_focused_window(struct xwm *xwm, struct xwayland_surface *xsurface)
{
    struct xwayland_surface *unfocus_surface = xwm->focus_surface;

    if (xwm->focus_surface == xsurface) {
        return;
    }
    xwm->focus_surface = xsurface;
    // cancel any pending focus offer
    xwm->offered_focus = xsurface;

    if ((xsurface && xsurface->override_redirect)) {
        return;
    }

    if (unfocus_surface) {
        xsurface_set_net_wm_state(unfocus_surface);
    }

    if (xsurface) {
        xsurface_set_net_wm_state(xsurface);
        xwm_set_net_active_window(xwm, xsurface->window_id);
    } else {
        xwm_set_net_active_window(xwm, xwm->no_focus_window);
    }
}

static bool xwm_atoms_contains(struct xwm *xwm, xcb_atom_t *atoms, size_t num_atoms,
                               enum atom_name needle);

static void xwayland_surface_offer_focus(struct xwayland_surface *xsurface)
{
    if (!xsurface || xsurface->override_redirect) {
        return;
    }

    struct xwm *xwm = xsurface->xwm;
    if (!xwm_atoms_contains(xwm, xsurface->protocols, xsurface->protocols_len, WM_TAKE_FOCUS)) {
        return;
    }

    xwm->offered_focus = xsurface;

    xcb_client_message_data_t message_data = { 0 };
    message_data.data32[0] = xwm->atoms[WM_TAKE_FOCUS].atom;
    message_data.data32[1] = XCB_TIME_CURRENT_TIME;
    xwm_send_wm_message(xsurface, &message_data, XCB_EVENT_MASK_NO_EVENT);

    xcb_flush(xwm->xcb_conn);
}

static void xwm_surface_activate(struct xwm *xwm, struct xwayland_surface *xsurface)
{
    if (xsurface && xsurface->override_redirect) {
        return;
    }

    if (xsurface != xwm->focus_surface && xsurface != xwm->offered_focus) {
        if (xsurface && xwayland_surface_icccm_input_model(xsurface) == ICCCM_INPUT_MODEL_GLOBAL) {
            xwayland_surface_offer_focus(xsurface);
            return;
        }
        xwm_focus_window(xwm, xsurface);
    }

    xwm_set_focused_window(xwm, xsurface);

    xcb_flush(xwm->xcb_conn);
}

static void xwayland_surface_dissociate(struct xwayland_surface *xsurface)
{
    if (xsurface->surface != NULL) {
        wlr_surface_unmap(xsurface->surface);
        if (xsurface->impl && xsurface->impl->dissociate) {
            xsurface->impl->dissociate(xsurface);
        }

        if (xsurface == xsurface->xwm->focus_surface) {
            xwm_surface_activate(xsurface->xwm, NULL);
        }

        if (xsurface->sync.alarm) {
            xcb_sync_destroy_alarm(xsurface->xwm->xcb_conn, xsurface->sync.alarm);
            xwayland_surface_set_allow_commits(xsurface, true);
        }

        wl_list_remove(&xsurface->surface_commit.link);
        wl_list_remove(&xsurface->client_commit.link);
        wl_list_remove(&xsurface->surface_map.link);
        wl_list_remove(&xsurface->surface_unmap.link);
        wlr_addon_finish(&xsurface->surface_addon);
        xsurface->surface = NULL;
    }

    // Make sure we're not on the unpaired surface list or we
    // could be assigned a surface during surface creation that
    // was mapped before this unmap request.
    wl_list_remove(&xsurface->unpaired_link);
    wl_list_init(&xsurface->unpaired_link);
    xsurface->surface_id = 0;
    xsurface->serial = 0;

    // reset minimized flag
    xwayland_surface_set_minimized(xsurface, false);

    wl_list_remove(&xsurface->stack_link);
    wl_list_init(&xsurface->stack_link);
    xwm_schedule_net_client_list_stacking(xsurface->xwm);
}

static void xwayland_surface_icon_destroy(struct xwayland_surface_icon *icon)
{
    wl_list_remove(&icon->link);
    wlr_buffer_drop(icon->buffer);
    free(icon->data);
    free(icon);
}

static void xwayland_surface_clear_icon(struct xwayland_surface *xsurface)
{
    struct xwayland_surface_icon *icon, *tmp;
    wl_list_for_each_safe(icon, tmp, &xsurface->icons, link) {
        xwayland_surface_icon_destroy(icon);
    }
}

static void xwayland_surface_destroy(struct xwayland_surface *xsurface)
{
    xwayland_surface_dissociate(xsurface);

    if (xsurface->impl && xsurface->impl->destroy) {
        xsurface->impl->destroy(xsurface);
    }

    if (xsurface == xsurface->xwm->focus_surface) {
        xwm_surface_activate(xsurface->xwm, NULL);
    }
    if (xsurface == xsurface->xwm->offered_focus) {
        xsurface->xwm->offered_focus = NULL;
    }

    wl_list_remove(&xsurface->link);
    wl_list_remove(&xsurface->stack_link);
    wl_list_remove(&xsurface->parent_link);
    wl_list_remove(&xsurface->unpaired_link);
    wl_event_source_remove(xsurface->ping_timer);

    struct xwayland_surface *child, *next;
    wl_list_for_each_safe(child, next, &xsurface->children, parent_link) {
        wl_list_remove(&child->parent_link);
        wl_list_init(&child->parent_link);
        child->parent = NULL;
    }

    xwayland_surface_clear_icon(xsurface);

    pixman_region32_fini(&xsurface->blur_region);
    pixman_region32_fini(&xsurface->opaque_region);
    free(xsurface->title);
    free(xsurface->class);
    free(xsurface->instance);
    free(xsurface->role);
    free(xsurface->window_type);
    free(xsurface->protocols);
    free(xsurface->startup_id);
    free(xsurface->hints);
    free(xsurface->size_hints);
    free(xsurface->strut_partial);
    free(xsurface);
}

static void xwm_handle_surface_class(struct xwm *xwm, struct xwayland_surface *xsurface,
                                     xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_STRING && reply->type != xwm->atoms[UTF8_STRING].atom) {
        return;
    }

    size_t len = xcb_get_property_value_length(reply);
    char *class = xcb_get_property_value(reply);

    // Unpack two sequentially stored strings: instance, class
    size_t instance_len = strnlen(class, len);
    free(xsurface->instance);
    if (len > 0 && instance_len < len) {
        xsurface->instance = strndup(class, instance_len);
        class += instance_len + 1;
    } else {
        xsurface->instance = NULL;
    }
    free(xsurface->class);
    if (len > 0) {
        xsurface->class = strndup(class, len);
    } else {
        xsurface->class = NULL;
    }

    if (xsurface->impl && xsurface->impl->set_class) {
        xsurface->impl->set_class(xsurface);
    }
}

static void xwm_handle_surface_startup_id(struct xwm *xwm, struct xwayland_surface *xsurface,
                                          xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_STRING && reply->type != xwm->atoms[UTF8_STRING].atom) {
        return;
    }

    size_t len = xcb_get_property_value_length(reply);
    char *startup_id = xcb_get_property_value(reply);

    free(xsurface->startup_id);
    if (len > 0) {
        xsurface->startup_id = strndup(startup_id, len);
    } else {
        xsurface->startup_id = NULL;
    }

    kywc_log(KYWC_DEBUG, "XCB_ATOM_NET_STARTUP_ID: %s",
             xsurface->startup_id ? xsurface->startup_id : "(null)");

    if (xsurface->impl && xsurface->impl->set_startup_id) {
        xsurface->impl->set_startup_id(xsurface);
    }
}

static void xwm_handle_surface_role(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_STRING && reply->type != xwm->atoms[UTF8_STRING].atom) {
        return;
    }

    size_t len = xcb_get_property_value_length(reply);
    char *role = xcb_get_property_value(reply);

    free(xsurface->role);
    if (len > 0) {
        xsurface->role = strndup(role, len);
    } else {
        xsurface->role = NULL;
    }

    if (xsurface->impl && xsurface->impl->set_role) {
        xsurface->impl->set_role(xsurface);
    }
}

static void xwm_handle_surface_title(struct xwm *xwm, struct xwayland_surface *xsurface,
                                     xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_STRING && reply->type != xwm->atoms[UTF8_STRING].atom) {
        return;
    }

    bool is_utf8 = reply->type == xwm->atoms[UTF8_STRING].atom;
    if (!is_utf8 && xsurface->has_utf8_title) {
        return;
    }

    size_t len = xcb_get_property_value_length(reply);
    char *title = xcb_get_property_value(reply);

    free(xsurface->title);
    if (len > 0) {
        xsurface->title = strndup(title, len);
    } else {
        xsurface->title = NULL;
    }
    xsurface->has_utf8_title = is_utf8;

    if (xsurface->impl && xsurface->impl->set_title) {
        xsurface->impl->set_title(xsurface);
    }
}

static bool has_parent(struct xwayland_surface *parent, struct xwayland_surface *child)
{
    while (parent) {
        if (child == parent) {
            return true;
        }
        parent = parent->parent;
    }
    return false;
}

static void xwm_handle_surface_parent(struct xwm *xwm, struct xwayland_surface *xsurface,
                                      xcb_get_property_reply_t *reply, bool delete)
{
    struct xwayland_surface *found_parent = NULL;
    if (reply->type != XCB_ATOM_WINDOW) {
        return;
    }

    xcb_window_t *xid = xcb_get_property_value(reply);
    if (xid != NULL) {
        found_parent = xwm_lookup_surface(xwm, *xid);
        if (!has_parent(found_parent, xsurface)) {
            xsurface->parent = found_parent;
        } else {
            kywc_log(KYWC_INFO, "%p with %p would create a loop", xsurface, found_parent);
        }
    } else {
        xsurface->parent = NULL;
    }

    wl_list_remove(&xsurface->parent_link);
    if (xsurface->parent != NULL) {
        wl_list_insert(&xsurface->parent->children, &xsurface->parent_link);
    } else {
        wl_list_init(&xsurface->parent_link);
    }

    if (xsurface->impl && xsurface->impl->set_parent) {
        xsurface->impl->set_parent(xsurface);
    }
}

static void xwm_handle_surface_window_type(struct xwm *xwm, struct xwayland_surface *xsurface,
                                           xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_ATOM) {
        return;
    }

    xcb_atom_t *atoms = xcb_get_property_value(reply);
    size_t atoms_len = reply->value_len;
    size_t atoms_size = sizeof(xcb_atom_t) * atoms_len;

    free(xsurface->window_type);
    xsurface->window_type = malloc(atoms_size);
    if (xsurface->window_type == NULL) {
        return;
    }
    memcpy(xsurface->window_type, atoms, atoms_size);
    xsurface->window_type_len = atoms_len;

    if (xsurface->impl && xsurface->impl->set_window_type) {
        xsurface->impl->set_window_type(xsurface);
    }
}

static void xwm_handle_surface_protocols(struct xwm *xwm, struct xwayland_surface *xsurface,
                                         xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_ATOM) {
        return;
    }

    xcb_atom_t *atoms = xcb_get_property_value(reply);
    size_t atoms_len = reply->value_len;
    size_t atoms_size = sizeof(xcb_atom_t) * atoms_len;

    free(xsurface->protocols);
    xsurface->protocols = malloc(atoms_size);
    if (xsurface->protocols == NULL) {
        return;
    }
    memcpy(xsurface->protocols, atoms, atoms_size);
    xsurface->protocols_len = atoms_len;
}

static void xwm_handle_surface_hints(struct xwm *xwm, struct xwayland_surface *xsurface,
                                     xcb_get_property_reply_t *reply, bool delete)
{
    // According to the docs, reply->type == xwm->atoms[WM_HINTS]
    // In practice, reply->type == XCB_ATOM_ATOM
    if (reply->value_len == 0) {
        return;
    }

    free(xsurface->hints);
    xsurface->hints = calloc(1, sizeof(*xsurface->hints));
    if (xsurface->hints == NULL) {
        return;
    }
    xcb_icccm_get_wm_hints_from_reply(xsurface->hints, reply);

    if (!(xsurface->hints->flags & XCB_ICCCM_WM_HINT_INPUT)) {
        // The client didn't specify whether it wants input.
        // Assume it does.
        xsurface->hints->input = true;
    }

    if (xsurface->impl && xsurface->impl->set_hints) {
        xsurface->impl->set_hints(xsurface);
    }
}

static void xwm_handle_surface_normal_hints(struct xwm *xwm, struct xwayland_surface *xsurface,
                                            xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != xwm->atoms[WM_SIZE_HINTS].atom || reply->value_len == 0) {
        return;
    }

    free(xsurface->size_hints);
    xsurface->size_hints = calloc(1, sizeof(*xsurface->size_hints));
    if (!xsurface->size_hints) {
        return;
    }
    xcb_icccm_get_wm_size_hints_from_reply(xsurface->size_hints, reply);

    int32_t flags = xsurface->size_hints->flags;
    bool has_min_size_hints = (flags & XCB_ICCCM_SIZE_HINT_P_MIN_SIZE) != 0;
    bool has_base_size_hints = (flags & XCB_ICCCM_SIZE_HINT_BASE_SIZE) != 0;
    /* ICCCM says that if absent, min size is equal to base size and vice versa */
    if (!has_min_size_hints && !has_base_size_hints) {
        xsurface->size_hints->min_width = -1;
        xsurface->size_hints->min_height = -1;
        xsurface->size_hints->base_width = -1;
        xsurface->size_hints->base_height = -1;
    } else if (!has_base_size_hints) {
        xsurface->size_hints->base_width = xsurface->size_hints->min_width;
        xsurface->size_hints->base_height = xsurface->size_hints->min_height;
    } else if (!has_min_size_hints) {
        xsurface->size_hints->min_width = xsurface->size_hints->base_width;
        xsurface->size_hints->min_height = xsurface->size_hints->base_height;
    }

    if ((flags & XCB_ICCCM_SIZE_HINT_P_MAX_SIZE) == 0) {
        xsurface->size_hints->max_width = -1;
        xsurface->size_hints->max_height = -1;
    }

    if (xsurface->impl && xsurface->impl->set_size_hints) {
        xsurface->impl->set_size_hints(xsurface);
    }
}

#define MWM_HINTS_FLAGS_FIELD 0
#define MWM_HINTS_FUNCTIONS_FIELD 1
#define MWM_HINTS_DECORATIONS_FIELD 2

#define MWM_HINTS_FUNCTIONS (1 << 0)
#define MWM_HINTS_DECORATIONS (1 << 1)

#define MWM_FUNC_ALL (1 << 0)
#define MWM_FUNC_MINIMIZE (1 << 3)
#define MWM_FUNC_MAXIMIZE (1 << 4)

#define MWM_DECOR_ALL (1 << 0)
#define MWM_DECOR_BORDER (1 << 1)
#define MWM_DECOR_TITLE (1 << 3)

static void xwm_handle_surface_motif_hints(struct xwm *xwm, struct xwayland_surface *xsurface,
                                           xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->value_len < 5) {
        return;
    }

    uint32_t *motif_hints = xcb_get_property_value(reply);
    if (motif_hints[MWM_HINTS_FLAGS_FIELD] & MWM_HINTS_FUNCTIONS) {
        xsurface->functions = XWAYLAND_SURFACE_FUNCTIONS_ALL;
        uint32_t functions = motif_hints[MWM_HINTS_FUNCTIONS_FIELD];
        bool toggle_value = functions & MWM_FUNC_ALL;
        if ((functions & MWM_FUNC_MINIMIZE) == toggle_value) {
            xsurface->functions |= XWAYLAND_SURFACE_FUNCTIONS_NO_MINIMIZE;
        }
        if ((functions & MWM_FUNC_MAXIMIZE) == toggle_value) {
            xsurface->functions |= XWAYLAND_SURFACE_FUNCTIONS_NO_MAXIMIZE;
        }
        if (xsurface->impl && xsurface->impl->set_functions) {
            xsurface->impl->set_functions(xsurface);
        }
    }

    if (motif_hints[MWM_HINTS_FLAGS_FIELD] & MWM_HINTS_DECORATIONS) {
        uint32_t decorations = motif_hints[MWM_HINTS_DECORATIONS_FIELD];
        if ((decorations & MWM_DECOR_ALL) == 0) {
            if ((decorations & MWM_DECOR_BORDER) == 0) {
                xsurface->decorations &= ~KYWC_SSD_BORDER;
            }
            if ((decorations & MWM_DECOR_TITLE) == 0) {
                xsurface->decorations &= ~KYWC_SSD_TITLE;
            }
        }
        if (xsurface->impl && xsurface->impl->set_decorations) {
            xsurface->impl->set_decorations(xsurface);
        }
    }
}

static void xwm_handle_surface_strut_partial(struct xwm *xwm, struct xwayland_surface *xsurface,
                                             xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->type != XCB_ATOM_CARDINAL || reply->format != 32 ||
        xcb_get_property_value_length(reply) != sizeof(xcb_ewmh_wm_strut_partial_t)) {
        return;
    }

    free(xsurface->strut_partial);
    xsurface->strut_partial = calloc(1, sizeof(*xsurface->strut_partial));
    if (xsurface->strut_partial == NULL) {
        return;
    }
    xcb_ewmh_get_wm_strut_partial_from_reply(xsurface->strut_partial, reply);

    if (xsurface->impl && xsurface->impl->set_strut_partial) {
        xsurface->impl->set_strut_partial(xsurface);
    }
}

static void xwm_handle_surface_net_wm_state(struct xwm *xwm, struct xwayland_surface *xsurface,
                                            xcb_get_property_reply_t *reply, bool delete)
{
    xcb_atom_t *atom = xcb_get_property_value(reply);
    for (uint32_t i = 0; i < reply->value_len; i++) {
        for (uint32_t j = 0; j < ATOM_LAST; j++) {
            if (atom[i] == xwm->atoms[j].atom) {
                if (xwm->atoms[j].handle_net_wm_state) {
                    xwm->atoms[j].handle_net_wm_state(xwm, xsurface, 1);
                } else {
                    char *prop_name = xwm_get_atom_name(xwm, atom[i]);
                    kywc_log(KYWC_DEBUG,
                             "Unhandled net_wm_state property %" PRIu32 " (%s) for window %" PRIu32,
                             atom[i], prop_name ? prop_name : "(null)", xsurface->window_id);
                    free(prop_name);
                }
                break;
            }
        }
    }
}

static void xwm_handle_surface_icon(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    xcb_get_property_reply_t *reply, bool delete)
{
    if (reply->value_len == 0) {
        return;
    }
    if (reply->type != XCB_ATOM_CARDINAL || reply->value_len < 3 || reply->format != 32) {
        kywc_log(KYWC_INFO, "Failed to get window 0x%x net icon", xsurface->window_id);
        return;
    }

    /* clear icon first */
    xwayland_surface_clear_icon(xsurface);

    uint32_t *data = (uint32_t *)xcb_get_property_value(reply);
    for (unsigned int i = 0; i < reply->value_len - 2;) {
        uint32_t width = data[i++], height = data[i++];
        uint32_t size = width * height * sizeof(uint32_t);
        if (i + width * height > reply->value_len) {
            kywc_log(KYWC_WARN, "Proposed size leads to out of bounds (%d x %d) ", width, height);
            break;
        }
        if (width > 1024 || height > 1024) {
            kywc_log(KYWC_WARN, "Found huge icon may be ill-encoded (%d x %d)", width, height);
        }

        /* remove old icon */
        struct xwayland_surface_icon *old_icon, *tmp;
        wl_list_for_each_safe(old_icon, tmp, &xsurface->icons, link) {
            if (old_icon->width == width && old_icon->height == height) {
                xwayland_surface_icon_destroy(old_icon);
            }
        }

        struct xwayland_surface_icon *icon = calloc(1, sizeof(*icon));
        if (!icon) {
            return;
        }
        wl_list_insert(&xsurface->icons, &icon->link);
        icon->width = width;
        icon->height = height;
        icon->size = size;
        icon->data = (unsigned char *)malloc(size);
        if (!icon->data) {
            wl_list_remove(&icon->link);
            free(icon);
            return;
        }
        memcpy(icon->data, (unsigned char *)&data[i], size);
        i += width * height;
    }

    if (xsurface->impl && xsurface->impl->set_icon) {
        xsurface->impl->set_icon(xsurface);
    }
}

static void xwm_handle_surface_opacity(struct xwm *xwm, struct xwayland_surface *xsurface,
                                       xcb_get_property_reply_t *reply, bool delete)
{
    if (!xsurface->surface_node) {
        return;
    }

    if (delete) {
        xsurface->opacity = 1.0;
        scene_buffer_set_opacity(scene_buffer_from_node(xsurface->surface_node), xsurface->opacity);
    }

    if (reply->type != XCB_ATOM_CARDINAL || reply->value_len != 1 || reply->format != 32) {
        return;
    }

    uint32_t *value = (uint32_t *)xcb_get_property_value(reply);
    xsurface->opacity = value[0] == 0xffffffff ? 1.0 : value[0] * 1.0 / 0xffffffff;
    scene_buffer_set_opacity(scene_buffer_from_node(xsurface->surface_node), xsurface->opacity);
}

static void xwm_handle_surface_sync_counter(struct xwm *xwm, struct xwayland_surface *xsurface,
                                            xcb_get_property_reply_t *reply, bool delete)
{
    if (!xwm->sync) {
        return;
    }

    if (reply->value_len == 0) {
        return;
    }
    if (reply->type != XCB_ATOM_CARDINAL) {
        kywc_log(KYWC_INFO, "Failed to get window 0x%x sync request counter", xsurface->window_id);
        return;
    }

    uint32_t *data = (uint32_t *)xcb_get_property_value(reply);
    xcb_sync_counter_t counter = data[0];

    if (counter == XCB_NONE) {
        return;
    }

    xcb_sync_int64_t value = { .hi = 0, .lo = 0 };
    xcb_sync_set_counter(xwm->xcb_conn, counter, value);
    xcb_flush(xwm->xcb_conn);

    xsurface->sync.counter = counter;
    if (xsurface->surface) {
        xsurface->sync.value = (xcb_sync_int64_t){ 0, 0 };
    }
}

static xcb_sync_alarm_t xwayland_surface_create_sync_alarm(struct xwayland_surface *xsurface,
                                                           xcb_sync_counter_t counter)
{
    if (!xsurface->xwm->sync || counter == XCB_NONE) {
        return XCB_NONE;
    }

    const uint32_t mask =
        XCB_SYNC_CA_COUNTER | XCB_SYNC_CA_VALUE_TYPE | XCB_SYNC_CA_TEST_TYPE | XCB_SYNC_CA_EVENTS;
    const uint32_t values[] = { counter, XCB_SYNC_VALUETYPE_RELATIVE,
                                XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON, 1 };
    xcb_sync_alarm_t alarm = xcb_generate_id(xsurface->xwm->xcb_conn);
    xcb_void_cookie_t cookie =
        xcb_sync_create_alarm_checked(xsurface->xwm->xcb_conn, alarm, mask, values);
    xcb_generic_error_t *error = xcb_request_check(xsurface->xwm->xcb_conn, cookie);
    if (error) {
        alarm = XCB_NONE;
    } else {
        xcb_sync_change_alarm_value_list_t value = { .value.lo = 1, .delta.lo = 1 };
        xcb_sync_change_alarm_aux(xsurface->xwm->xcb_conn, alarm,
                                  XCB_SYNC_CA_VALUE | XCB_SYNC_CA_DELTA, &value);
        xcb_flush(xsurface->xwm->xcb_conn);
    }

    return alarm;
}

void xwayland_surface_send_sync_request(struct xwayland_surface *xsurface, xcb_sync_int64_t *value)
{
    const uint32_t lo = value->lo++;
    if (lo > value->lo) {
        value->hi++;
    }

    xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .window = xsurface->window_id,
        .type = xsurface->xwm->atoms[WM_PROTOCOLS].atom,
        .data.data32[0] = xsurface->xwm->atoms[NET_WM_SYNC_REQUEST].atom,
        .data.data32[1] = XCB_TIME_CURRENT_TIME,
        .data.data32[2] = value->lo,
        .data.data32[3] = value->hi,
    };

    xwm_send_event_with_size(xsurface->xwm->xcb_conn, 0, xsurface->window_id,
                             XCB_EVENT_MASK_NO_EVENT, (const char *)&event, sizeof(event));
    xcb_flush(xsurface->xwm->xcb_conn);
}

void xwayland_surface_set_allow_commits(struct xwayland_surface *xsurface, bool allow)
{
    xsurface->sync.blocked = !allow;

    uint32_t value = allow;
    xcb_change_property(xsurface->xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xsurface->window_id,
                        xsurface->xwm->atoms[XWAYLAND_ALLOW_COMMITS].atom, XCB_ATOM_CARDINAL, 32, 1,
                        &value);
    xcb_flush(xsurface->xwm->xcb_conn);
}

static void xwm_handle_surface_alarm_notify(struct xwm *xwm, void *data)
{
    xcb_sync_alarm_notify_event_t *ev = data;
    xcb_sync_alarm_t alarm = ev->alarm;
    xcb_sync_int64_t value = ev->counter_value;

    struct xwayland_surface *xsurface;
    wl_list_for_each(xsurface, &xwm->surfaces, link) {
        if (xsurface->sync.alarm != alarm) {
            continue;
        }
        if (xsurface->sync.value.lo == value.lo && xsurface->sync.value.hi == value.hi &&
            xsurface->sync.blocked) {
            xwayland_surface_set_allow_commits(xsurface, true);
        }
    }
}

static void xwm_handle_surface_blur_region(struct xwm *xwm, struct xwayland_surface *xsurface,
                                           xcb_get_property_reply_t *reply, bool delete)
{
    if (!xsurface->surface_node) {
        return;
    }

    if (delete) {
        pixman_region32_clear(&xsurface->blur_region);
        scene_node_set_blur_region(xsurface->surface_node, NULL);
        return;
    }

    if (reply->type != XCB_ATOM_CARDINAL || reply->format != 32) {
        return;
    }

    /* from V10sp1, the last value is the blur strength.we ignore it. */
    uint32_t len;
    if (reply->value_len % 4 == 0) {
        len = reply->value_len;
    } else if (reply->value_len % 4 == 1) {
        len = reply->value_len - 1;
    } else {
        return;
    }

    pixman_region32_clear(&xsurface->blur_region);
    uint32_t *value = (uint32_t *)xcb_get_property_value(reply);
    for (uint32_t i = 0; i < len; i += 4) {
        pixman_region32_union_rect(&xsurface->blur_region, &xsurface->blur_region, value[i],
                                   value[i + 1], value[i + 2], value[i + 3]);
    }
    wlr_region_scale(&xsurface->blur_region, &xsurface->blur_region, 1.0 / xwm->scale);
    scene_node_set_blur_region(xsurface->surface_node, &xsurface->blur_region);
}

static void xwm_handle_surface_opaque_region(struct xwm *xwm, struct xwayland_surface *xsurface,
                                             xcb_get_property_reply_t *reply, bool delete)
{
    if (!xsurface->surface_node) {
        return;
    }

    if (delete) {
        pixman_region32_clear(&xsurface->opaque_region);
        scene_buffer_set_opaque_region(scene_buffer_from_node(xsurface->surface_node),
                                       &xsurface->opaque_region);
        return;
    }

    if (reply->type != XCB_ATOM_CARDINAL || reply->format != 32 || reply->value_len % 4) {
        return;
    }

    pixman_region32_clear(&xsurface->opaque_region);
    uint32_t *data = (uint32_t *)xcb_get_property_value(reply);
    for (unsigned int i = 0; i < reply->value_len; i += 4) {
        pixman_region32_union_rect(&xsurface->opaque_region, &xsurface->opaque_region, data[i],
                                   data[i + 1], data[i + 2], data[i + 3]);
    }
    wlr_region_scale(&xsurface->opaque_region, &xsurface->opaque_region, 1.0 / xwm->scale);
    scene_buffer_set_opaque_region(scene_buffer_from_node(xsurface->surface_node),
                                   &xsurface->opaque_region);
}

static void xwm_handle_surface_ukui_decoration(struct xwm *xwm, struct xwayland_surface *xsurface,
                                               xcb_get_property_reply_t *reply, bool delete)
{
    if (delete) {
        xsurface->decorations |= KYWC_SSD_TITLE;
        if (xsurface->impl && xsurface->impl->set_decorations) {
            xsurface->impl->set_decorations(xsurface);
        }
    }

    if (reply->value_len == 0) {
        return;
    }

    uint8_t *data = (uint8_t *)xcb_get_property_value(reply);
    if (data && data[0]) {
        xsurface->decorations &= ~KYWC_SSD_TITLE;
    }

    if (xsurface->impl && xsurface->impl->set_decorations) {
        xsurface->impl->set_decorations(xsurface);
    }
}

static bool xwayland_surface_get_shape_region(struct xwayland_surface *xsurface,
                                              xcb_shape_kind_t kind, pixman_region32_t *region,
                                              int *count)
{
    xcb_shape_get_rectangles_reply_t *reply = xcb_shape_get_rectangles_reply(
        xsurface->xwm->xcb_conn,
        xcb_shape_get_rectangles_unchecked(xsurface->xwm->xcb_conn, xsurface->window_id, kind),
        NULL);
    if (!reply) {
        return false;
    }

    xcb_rectangle_t *rects = xcb_shape_get_rectangles_rectangles(reply);
    int len = xcb_shape_get_rectangles_rectangles_length(reply);
    if (region) {
        for (int i = 0; i < len; i++) {
            pixman_region32_union_rect(region, region, rects[i].x, rects[i].y, rects[i].width,
                                       rects[i].height);
        }
        wlr_region_scale(region, region, 1.0 / xsurface->xwm->scale);
    }
    if (count) {
        *count = len;
    }

    free(reply);
    return true;
}

static void xwayland_surface_apply_shape_region(struct xwayland_surface *xsurface)
{
    if (!xsurface->xwm->shape || !xsurface->surface || !xsurface->surface_node) {
        return;
    }

    xcb_shape_query_extents_reply_t *reply = xcb_shape_query_extents_reply(
        xsurface->xwm->xcb_conn,
        xcb_shape_query_extents_unchecked(xsurface->xwm->xcb_conn, xsurface->window_id), NULL);
    if (!reply) {
        return;
    }

    pixman_region32_t region;
    pixman_region32_init(&region);

    if (reply->clip_shaped) {
        if (xwayland_surface_get_shape_region(xsurface, XCB_SHAPE_SK_CLIP, &region, NULL)) {
            scene_node_set_clip_region(xsurface->surface_node, &region);
        }
    } else if (reply->bounding_shaped) {
        if (xwayland_surface_get_shape_region(xsurface, XCB_SHAPE_SK_BOUNDING, &region, NULL)) {
            scene_node_set_clip_region(xsurface->surface_node, &region);
            scene_node_set_input_region(xsurface->surface_node, &region);
        }
    }

    int count = 0;
    xwayland_surface_get_shape_region(xsurface, XCB_SHAPE_SK_INPUT, NULL, &count);
    scene_node_set_input_bypassed(xsurface->surface_node, count == 0);

    pixman_region32_fini(&region);
    free(reply);
}

static void xwayland_surface_shape_select_input(struct xwayland_surface *surface, bool enabled)
{
    if (surface->xwm->shape) {
        xcb_shape_select_input(surface->xwm->xcb_conn, surface->window_id, enabled);
        xcb_flush(surface->xwm->xcb_conn);
    }
}

static void xwm_handle_surface_shape_notify(struct xwm *xwm, void *data)
{

    xcb_shape_notify_event_t *ev = data;
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->affected_window);
    if (!xsurface || !xsurface->surface_node) {
        return;
    }

    pixman_region32_t region;
    pixman_region32_init(&region);
    if (!xwayland_surface_get_shape_region(xsurface, ev->shape_kind, &region, NULL)) {
        pixman_region32_fini(&region);
        return;
    }

    if (ev->shape_kind == XCB_SHAPE_SK_BOUNDING || ev->shape_kind == XCB_SHAPE_SK_CLIP) {
        scene_node_set_clip_region(xsurface->surface_node, &region);
    }
    if (ev->shape_kind == XCB_SHAPE_SK_BOUNDING || ev->shape_kind == XCB_SHAPE_SK_INPUT) {
        scene_node_set_input_region(xsurface->surface_node, &region);
        /* empty input region means no input support */
        bool need_bypassed =
            ev->shape_kind == XCB_SHAPE_SK_INPUT && !pixman_region32_not_empty(&region);
        scene_node_set_input_bypassed(xsurface->surface_node, need_bypassed);
    }

    /* disable ssd if the window has clip region */
    if (pixman_region32_not_empty(&region)) {
        xsurface->decorations = KYWC_SSD_NONE;
        if (xsurface->impl && xsurface->impl->set_decorations) {
            xsurface->impl->set_decorations(xsurface);
        }
    }

    pixman_region32_fini(&region);
}

static void xwm_handle_surface_property(struct xwm *xwm, struct xwayland_surface *xsurface,
                                        xcb_atom_t property, xcb_get_property_reply_t *reply,
                                        bool delete)
{
    struct xwm_atom *xwm_atom = NULL;
    for (size_t i = 0; i < ATOM_LAST; i++) {
        if (property == xwm->atoms[i].atom) {
            xwm_atom = &xwm->atoms[i];
            break;
        }
    }

    if (xwm_atom && xwm_atom->handle_surface_property) {
        xwm_atom->handle_surface_property(xwm, xsurface, reply, delete);
    } /* Traditional ICCCM attributes */
    else if (property == XCB_ATOM_WM_CLASS) {
        xwm_handle_surface_class(xwm, xsurface, reply, delete);
    } else if (property == XCB_ATOM_WM_NAME) {
        xwm_handle_surface_title(xwm, xsurface, reply, delete);
    } else if (property == XCB_ATOM_WM_TRANSIENT_FOR) {
        xwm_handle_surface_parent(xwm, xsurface, reply, delete);
    } else {
        char *prop_name = xwm_get_atom_name(xwm, property);
        kywc_log(KYWC_DEBUG, "Unhandled X11 property %" PRIu32 " (%s) for window %" PRIu32,
                 property, prop_name ? prop_name : "(null)", xsurface->window_id);
        free(prop_name);
    }
}

static void xwayland_surface_handle_commit(struct wl_listener *listener, void *data)
{
    struct xwayland_surface *xsurface = wl_container_of(listener, xsurface, surface_commit);
    if (xsurface->surface->mapped && xsurface->impl && xsurface->impl->commit) {
        xsurface->impl->commit(xsurface);
    }

    if (wlr_surface_has_buffer(xsurface->surface)) {
        wlr_surface_map(xsurface->surface);
    }
}

static void xwayland_surface_handle_client_commit(struct wl_listener *listener, void *data)
{
    struct xwayland_surface *xsurface = wl_container_of(listener, xsurface, client_commit);
    if (xsurface->xwm->scale == 1.0) {
        return;
    }

    struct wlr_surface_state *pending = &xsurface->surface->pending;
    pending->width = xwayland_unscale(pending->width);
    pending->height = xwayland_unscale(pending->height);

    float scale = 1.0 / xsurface->xwm->scale;
    if (pending->committed & WLR_SURFACE_STATE_SURFACE_DAMAGE) {
        wlr_region_scale(&pending->surface_damage, &pending->surface_damage, scale);
    }
    if (pending->committed & WLR_SURFACE_STATE_OPAQUE_REGION) {
        wlr_region_scale(&pending->opaque, &pending->opaque, scale);
    }
    if (pending->committed & WLR_SURFACE_STATE_INPUT_REGION) {
        wlr_region_scale(&pending->input, &pending->input, scale);
    }
    if (pending->committed & WLR_SURFACE_STATE_OFFSET) {
        pending->dx = xwayland_unscale(pending->dx);
        pending->dy = xwayland_unscale(pending->dy);
    }
}

static void xwayland_surface_handle_map(struct wl_listener *listener, void *data)
{
    struct xwayland_surface *xsurface = wl_container_of(listener, xsurface, surface_map);
    xwm_set_net_client_list(xsurface->xwm);

    if (xsurface->impl && xsurface->impl->map) {
        xsurface->impl->map(xsurface);
    }
}

static void xwayland_surface_handle_unmap(struct wl_listener *listener, void *data)
{
    struct xwayland_surface *xsurface = wl_container_of(listener, xsurface, surface_unmap);
    xwm_set_net_client_list(xsurface->xwm);
    if (xsurface->impl && xsurface->impl->unmap) {
        xsurface->impl->unmap(xsurface);
    }
}

static void xwayland_surface_handle_addon_destroy(struct wlr_addon *addon)
{
    struct xwayland_surface *xsurface = wl_container_of(addon, xsurface, surface_addon);
    xwayland_surface_dissociate(xsurface);
}

static const struct wlr_addon_interface surface_addon_impl = {
    .name = "xwayland_surface",
    .destroy = xwayland_surface_handle_addon_destroy,
};

static xcb_get_property_cookie_t get_property(struct xwm *xwm, xcb_window_t window_id,
                                              xcb_atom_t atom)
{
    uint32_t len = 2048;
    if (atom == xwm->atoms[NET_WM_ICON].atom ||
        atom == xwm->atoms[KDE_NET_WM_BLUR_BEHIND_REGION].atom ||
        atom == xwm->atoms[NET_WM_OPAQUE_REGION].atom) {
        /* icon/region need big length  */
        len = UINT32_MAX;
    }
    return xcb_get_property(xwm->xcb_conn, 0, window_id, atom, XCB_ATOM_ANY, 0, len);
}

static void read_all_surface_properties(struct xwayland_surface *xsurface)
{
    struct xwm *xwm = xsurface->xwm;
    const xcb_atom_t props[] = {
        XCB_ATOM_WM_CLASS,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_WM_TRANSIENT_FOR,
        xwm->atoms[WM_PROTOCOLS].atom,
        xwm->atoms[WM_HINTS].atom,
        xwm->atoms[WM_NORMAL_HINTS].atom,
        xwm->atoms[MOTIF_WM_HINTS].atom,
        xwm->atoms[NET_STARTUP_ID].atom,
        xwm->atoms[NET_WM_STATE].atom,
        xwm->atoms[NET_WM_STRUT_PARTIAL].atom,
        xwm->atoms[NET_WM_WINDOW_TYPE].atom,
        xwm->atoms[NET_WM_NAME].atom,
        xwm->atoms[NET_WM_ICON].atom,
        xwm->atoms[NET_WM_WINDOW_OPACITY].atom,
        xwm->atoms[KDE_NET_WM_BLUR_BEHIND_REGION].atom,
        xwm->atoms[NET_WM_OPAQUE_REGION].atom,
        xwm->atoms[NET_WM_SYNC_REQUEST_COUNTER].atom,
    };

    xcb_get_property_cookie_t cookies[sizeof(props) / sizeof(props[0])] = { 0 };
    for (size_t i = 0; i < sizeof(props) / sizeof(props[0]); i++) {
        cookies[i] = get_property(xwm, xsurface->window_id, props[i]);
    }

    for (size_t i = 0; i < sizeof(props) / sizeof(props[0]); i++) {
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xwm->xcb_conn, cookies[i], NULL);
        if (reply == NULL) {
            kywc_log(KYWC_DEBUG, "Failed to get window property");
            continue;
        }
        xwm_handle_surface_property(xwm, xsurface, props[i], reply, false);
        free(reply);
    }
}

static void xwayland_surface_associate(struct xwm *xwm, struct xwayland_surface *xsurface,
                                       struct wlr_surface *surface)
{
    assert(xsurface->surface == NULL);

    wl_list_remove(&xsurface->unpaired_link);
    wl_list_init(&xsurface->unpaired_link);
    xsurface->surface_id = 0;

    xsurface->surface = surface;
    wlr_addon_init(&xsurface->surface_addon, &surface->addons, NULL, &surface_addon_impl);

    /**
     * create surface node before apply property
     * apply property need surface node, associate will create surface node.
     */
    if (xsurface->impl && xsurface->impl->associate) {
        xsurface->surface_node = xsurface->impl->associate(xsurface);
    }

    xsurface->surface_commit.notify = xwayland_surface_handle_commit;
    wl_signal_add(&surface->events.commit, &xsurface->surface_commit);
    xsurface->client_commit.notify = xwayland_surface_handle_client_commit;
    wl_signal_add(&surface->events.client_commit, &xsurface->client_commit);
    xsurface->surface_map.notify = xwayland_surface_handle_map;
    wl_signal_add(&surface->events.map, &xsurface->surface_map);
    xsurface->surface_unmap.notify = xwayland_surface_handle_unmap;
    wl_signal_add(&surface->events.unmap, &xsurface->surface_unmap);

    read_all_surface_properties(xsurface);
    xsurface->sync.alarm = xwayland_surface_create_sync_alarm(xsurface, xsurface->sync.counter);
    xwayland_surface_shape_select_input(xsurface, true);
    xwayland_surface_apply_shape_region(xsurface);
}

static void xwayland_surface_create(struct xwm *xwm, xcb_window_t window_id, int16_t x, int16_t y,
                                    uint16_t width, uint16_t height, bool override_redirect)
{
    struct xwayland_surface *xsurface = calloc(1, sizeof(*xsurface));
    if (!xsurface) {
        kywc_log(KYWC_ERROR, "Could not allocate xwayland surface");
        return;
    }

    xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry(xwm->xcb_conn, window_id);
    xcb_res_query_client_ids_cookie_t client_id_cookie;
    if (xwm->xres) {
        xcb_res_client_id_spec_t spec = { .client = window_id,
                                          .mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID };
        client_id_cookie = xcb_res_query_client_ids(xwm->xcb_conn, 1, &spec);
    }

    uint32_t values[1];
    values[0] = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(xwm->xcb_conn, window_id, XCB_CW_EVENT_MASK, values);

    xsurface->xwm = xwm;
    xsurface->window_id = window_id;
    xsurface->x = x;
    xsurface->y = y;
    xsurface->width = width;
    xsurface->height = height;
    xsurface->override_redirect = override_redirect;
    xsurface->opacity = 1.0;
    xsurface->decorations = KYWC_SSD_ALL;
    pixman_region32_init(&xsurface->blur_region);
    pixman_region32_init(&xsurface->opaque_region);

    wl_list_init(&xsurface->stack_link);
    wl_list_init(&xsurface->unpaired_link);
    wl_list_init(&xsurface->children);
    wl_list_init(&xsurface->parent_link);
    wl_list_init(&xsurface->icons);
    wl_list_insert(&xwm->surfaces, &xsurface->link);

    xcb_get_geometry_reply_t *geometry_reply =
        xcb_get_geometry_reply(xwm->xcb_conn, geometry_cookie, NULL);
    if (geometry_reply != NULL) {
        xsurface->has_alpha = geometry_reply->depth == 32;
    }
    free(geometry_reply);

    struct wl_display *display = xwm->xwayland->server->display;
    struct wl_event_loop *loop = wl_display_get_event_loop(display);
    xsurface->ping_timer =
        wl_event_loop_add_timer(loop, xwayland_surface_handle_ping_timeout, xsurface);
    if (xsurface->ping_timer == NULL) {
        free(xsurface);
        kywc_log(KYWC_ERROR, "Could not add timer to event loop");
        return;
    }

    if (xwm->xres) {
        read_surface_client_id(xwm, xsurface, client_id_cookie);
    }

    if (xsurface->override_redirect) {
        xwayland_unmanaged_create(xsurface);
    } else {
        xwayland_view_create(xsurface);
    }
}

static void xwm_handle_create_notify(struct xwm *xwm, xcb_create_notify_event_t *ev)
{
    if (ev->window == xwm->window || ev->window == xwm->primary_selection.window ||
        ev->window == xwm->clipboard_selection.window || ev->window == xwm->dnd_selection.window) {
        return;
    }

    xwayland_surface_create(xwm, ev->window, ev->x, ev->y, ev->width, ev->height,
                            ev->override_redirect);
}

static void xwm_handle_destroy_notify(struct xwm *xwm, xcb_destroy_notify_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        return;
    }
    xwayland_surface_destroy(xsurface);
    xwm_handle_selection_destroy_notify(xwm, ev);
}

static void xwm_handle_configure_request(struct xwm *xwm, xcb_configure_request_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        return;
    }

    // TODO: handle ev->{parent,sibling}?

    uint16_t mask = ev->value_mask;
    uint16_t geo_mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                        XCB_CONFIG_WINDOW_HEIGHT;
    if ((mask & geo_mask) == 0) {
        return;
    }

    struct xwayland_surface_request_configure configure = {
        .x = mask & XCB_CONFIG_WINDOW_X ? ev->x : xsurface->x,
        .y = mask & XCB_CONFIG_WINDOW_Y ? ev->y : xsurface->y,
        .width = mask & XCB_CONFIG_WINDOW_WIDTH ? ev->width : xsurface->width,
        .height = mask & XCB_CONFIG_WINDOW_HEIGHT ? ev->height : xsurface->height,
        .mask = mask,
    };

    if (xsurface->impl && xsurface->impl->request_configure) {
        xsurface->impl->request_configure(xsurface, &configure);
    }
}

static void xwayland_surface_set_override_redirect(struct xwayland_surface *xsurface,
                                                   bool override_redirect)
{
    if (xsurface->override_redirect == override_redirect) {
        return;
    }

    if (xsurface->surface) {
        if (xsurface->surface->mapped && xsurface->impl && xsurface->impl->unmap) {
            xsurface->impl->unmap(xsurface);
        }
        if (xsurface->impl && xsurface->impl->dissociate) {
            xsurface->impl->dissociate(xsurface);
        }
    }
    xsurface->impl->destroy(xsurface);

    xsurface->override_redirect = override_redirect;
    if (xsurface->override_redirect) {
        wl_list_remove(&xsurface->stack_link);
        wl_list_init(&xsurface->stack_link);
        xwm_schedule_net_client_list_stacking(xsurface->xwm);
        xwayland_unmanaged_create(xsurface);
    } else {
        xwayland_view_create(xsurface);
    }
}

static void xwm_handle_configure_notify(struct xwm *xwm, xcb_configure_notify_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (!xsurface) {
        return;
    }

    bool geometry_changed = (xsurface->x != ev->x || xsurface->y != ev->y ||
                             xsurface->width != ev->width || xsurface->height != ev->height);

    if (geometry_changed) {
        xsurface->x = ev->x;
        xsurface->y = ev->y;
        xsurface->width = ev->width;
        xsurface->height = ev->height;
    }

    xwayland_surface_set_override_redirect(xsurface, ev->override_redirect);

    if (geometry_changed) {
        if (xsurface->impl && xsurface->impl->set_geometry) {
            xsurface->impl->set_geometry(xsurface);
        }
    }
}

static void xsurface_set_wm_state(struct xwayland_surface *xsurface)
{
    struct xwm *xwm = xsurface->xwm;
    uint32_t property[] = { XCB_ICCCM_WM_STATE_NORMAL, XCB_WINDOW_NONE };

    if (xsurface->withdrawn) {
        property[0] = XCB_ICCCM_WM_STATE_WITHDRAWN;
    } else if (xsurface->minimized) {
        property[0] = XCB_ICCCM_WM_STATE_ICONIC;
    } else {
        property[0] = XCB_ICCCM_WM_STATE_NORMAL;
    }

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xsurface->window_id,
                        xwm->atoms[WM_STATE].atom, xwm->atoms[WM_STATE].atom,
                        32, // format
                        sizeof(property) / sizeof(property[0]), property);
}

static void xwayland_surface_set_withdrawn(struct xwayland_surface *xsurface, bool withdrawn)
{
    struct xwm *xwm = xsurface->xwm;
    xsurface->withdrawn = withdrawn;
    xsurface_set_wm_state(xsurface);
    // EWMH says _NET_WM_STATE should be unset if the window is withdrawn
    if (xsurface->withdrawn) {
        xsurface->remap = true;
        xcb_delete_property(xwm->xcb_conn, xsurface->window_id, xwm->atoms[NET_WM_STATE].atom);
        return;
    }
    if (xsurface->remap) {
        xsurface_set_net_wm_state(xsurface);
    }

    xcb_flush(xwm->xcb_conn);
}

static void xwm_handle_map_request(struct xwm *xwm, xcb_map_request_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (!xsurface) {
        return;
    }

    read_all_surface_properties(xsurface);
    xcb_map_window(xwm->xcb_conn, ev->window);
}

static void xwm_handle_map_notify(struct xwm *xwm, xcb_map_notify_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (!xsurface) {
        return;
    }

    xwayland_surface_set_override_redirect(xsurface, ev->override_redirect);
    if (!xsurface->override_redirect) {
        xwayland_surface_set_withdrawn(xsurface, false);
        uint32_t values = XCB_STACK_MODE_BELOW;
        xcb_configure_window(xwm->xcb_conn, xsurface->window_id, XCB_CONFIG_WINDOW_STACK_MODE,
                             &values);
        xcb_flush(xwm->xcb_conn);
    }
}

static void xwm_handle_unmap_notify(struct xwm *xwm, xcb_unmap_notify_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        return;
    }

    xwayland_surface_dissociate(xsurface);

    if (!xsurface->override_redirect) {
        xwayland_surface_set_withdrawn(xsurface, true);
    }
}

static void xwm_handle_property_notify(struct xwm *xwm, xcb_property_notify_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        return;
    }

    xcb_get_property_cookie_t cookie = get_property(xwm, xsurface->window_id, ev->atom);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(xwm->xcb_conn, cookie, NULL);
    if (reply == NULL) {
        kywc_log(KYWC_DEBUG, "Failed to get window property notify");
        return;
    }

    bool delete = ev->state == XCB_PROPERTY_DELETE;
    xwm_handle_surface_property(xwm, xsurface, ev->atom, reply, delete);
    free(reply);
}

static void xwm_handle_surface_id_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        kywc_log(KYWC_DEBUG, "client message WL_SURFACE_ID but no new window %u ?", ev->window);
        return;
    }
    /* Check if we got notified after wayland surface create event */
    uint32_t id = ev->data.data32[0];
    struct wl_resource *resource = wl_client_get_object(xwm->xwayland->xwayland_server->client, id);
    if (resource) {
        struct wlr_surface *surface = wlr_surface_from_resource(resource);
        xwayland_surface_associate(xwm, xsurface, surface);
    } else {
        xsurface->surface_id = id;
        wl_list_remove(&xsurface->unpaired_link);
        wl_list_insert(&xwm->unpaired_surfaces, &xsurface->unpaired_link);
    }
}

static void xwm_handle_surface_serial_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        kywc_log(KYWC_DEBUG, "Received client message WL_SURFACE_SERIAL but no X11 window %u",
                 ev->window);
        return;
    }
    if (xsurface->serial != 0) {
        kywc_log(KYWC_DEBUG,
                 "Received multiple client messages WL_SURFACE_SERIAL "
                 "for the same X11 window %u",
                 ev->window);
        return;
    }

    uint32_t serial_lo = ev->data.data32[0];
    uint32_t serial_hi = ev->data.data32[1];
    xsurface->serial = ((uint64_t)serial_hi << 32) | serial_lo;

    struct wlr_surface *surface =
        xwayland_shell_v1_surface_from_serial(xwm->xwayland->shell, xsurface->serial);
    if (surface != NULL) {
        xwayland_surface_associate(xwm, xsurface, surface);
    } else {
        wl_list_remove(&xsurface->unpaired_link);
        wl_list_insert(&xwm->unpaired_surfaces, &xsurface->unpaired_link);
    }
}

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT 0
#define _NET_WM_MOVERESIZE_SIZE_TOP 1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT 2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT 3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT 4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM 5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT 6
#define _NET_WM_MOVERESIZE_SIZE_LEFT 7
#define _NET_WM_MOVERESIZE_MOVE 8 // movement only
// #define _NET_WM_MOVERESIZE_SIZE_KEYBOARD 9  // size via keyboard
// #define _NET_WM_MOVERESIZE_MOVE_KEYBOARD 10 // move via keyboard
#define _NET_WM_MOVERESIZE_CANCEL 11 // cancel operation

static enum kywc_edges net_wm_edges_to_kywc(uint32_t net_wm_edges)
{
    switch (net_wm_edges) {
    case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
        return KYWC_EDGE_TOP | KYWC_EDGE_LEFT;
    case _NET_WM_MOVERESIZE_SIZE_TOP:
        return KYWC_EDGE_TOP;
    case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
        return KYWC_EDGE_TOP | KYWC_EDGE_RIGHT;
    case _NET_WM_MOVERESIZE_SIZE_RIGHT:
        return KYWC_EDGE_RIGHT;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
        return KYWC_EDGE_BOTTOM | KYWC_EDGE_RIGHT;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
        return KYWC_EDGE_BOTTOM;
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
        return KYWC_EDGE_BOTTOM | KYWC_EDGE_LEFT;
    case _NET_WM_MOVERESIZE_SIZE_LEFT:
        return KYWC_EDGE_LEFT;
    default:
        return KYWC_EDGE_NONE;
    }
}

static void xwm_handle_net_wm_moveresize_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (!xsurface) {
        return;
    }

    int detail = ev->data.data32[2];
    switch (detail) {
    case _NET_WM_MOVERESIZE_MOVE:
        if (xsurface->impl && xsurface->impl->request_move) {
            xsurface->impl->request_move(xsurface);
        }
        break;
    case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
    case _NET_WM_MOVERESIZE_SIZE_TOP:
    case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
    case _NET_WM_MOVERESIZE_SIZE_RIGHT:
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
    case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
    case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
    case _NET_WM_MOVERESIZE_SIZE_LEFT:;
        if (xsurface->impl && xsurface->impl->request_resize) {
            xsurface->impl->request_resize(xsurface, net_wm_edges_to_kywc(detail));
        }
        break;
    case _NET_WM_MOVERESIZE_CANCEL:
        break;
    }
}

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1
#define _NET_WM_STATE_TOGGLE 2

static bool update_state(int action, bool *state)
{
    int new_state, changed;

    switch (action) {
    case _NET_WM_STATE_REMOVE:
        new_state = false;
        break;
    case _NET_WM_STATE_ADD:
        new_state = true;
        break;
    case _NET_WM_STATE_TOGGLE:
        new_state = !*state;
        break;
    default:
        return false;
    }

    changed = (*state != new_state);
    *state = new_state;

    return changed;
}

static bool xwm_handle_net_wm_hidden(struct xwm *xwm, struct xwayland_surface *xsurface,
                                     uint32_t action)
{
    bool changed = update_state(action, &xsurface->minimized);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_minimize) {
            xsurface->impl->request_minimize(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_maximize_vert(struct xwm *xwm, struct xwayland_surface *xsurface,
                                            uint32_t action)
{
    bool changed = update_state(action, &xsurface->maximized_vert);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_maximize) {
            xsurface->impl->request_maximize(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_maximize_horz(struct xwm *xwm, struct xwayland_surface *xsurface,
                                            uint32_t action)
{
    bool changed = update_state(action, &xsurface->maximized_horz);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_maximize) {
            xsurface->impl->request_maximize(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_fullscreen(struct xwm *xwm, struct xwayland_surface *xsurface,
                                         uint32_t action)
{
    bool changed = update_state(action, &xsurface->fullscreen);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_fullscreen) {
            xsurface->impl->request_fullscreen(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_modal(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    uint32_t action)
{
    bool changed = update_state(action, &xsurface->modal);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_modal) {
            xsurface->impl->request_modal(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_sticky(struct xwm *xwm, struct xwayland_surface *xsurface,
                                     uint32_t action)
{
    bool changed = update_state(action, &xsurface->sticky);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_sticky) {
            xsurface->impl->request_sticky(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_skip_taskbar(struct xwm *xwm, struct xwayland_surface *xsurface,
                                           uint32_t action)
{
    bool changed = update_state(action, &xsurface->skip_taskbar);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_skip_taskbar) {
            xsurface->impl->request_skip_taskbar(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_skip_switcher(struct xwm *xwm, struct xwayland_surface *xsurface,
                                            uint32_t action)
{
    bool changed = update_state(action, &xsurface->skip_switcher);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_skip_switcher) {
            xsurface->impl->request_skip_switcher(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_above(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    uint32_t action)
{
    bool changed = update_state(action, &xsurface->above);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_above) {
            xsurface->impl->request_above(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_below(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    uint32_t action)
{
    bool changed = update_state(action, &xsurface->below);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_below) {
            xsurface->impl->request_below(xsurface);
        }
    }
    return changed;
}

static bool xwm_handle_net_wm_demands_attention(struct xwm *xwm, struct xwayland_surface *xsurface,
                                                uint32_t action)
{
    bool changed = update_state(action, &xsurface->demands_attention);
    if (changed) {
        if (xsurface->impl && xsurface->impl->request_demands_attention) {
            xsurface->impl->request_demands_attention(xsurface);
        }
    }
    return changed;
}

static void xwm_handle_net_wm_state_message(struct xwm *xwm,
                                            xcb_client_message_event_t *client_message)
{
    if (client_message->format != 32) {
        return;
    }
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, client_message->window);
    if (!xsurface) {
        return;
    }

    // client_message->data.data32[3] is the source indication
    uint32_t action = client_message->data.data32[0];
    xcb_atom_t first_property = client_message->data.data32[1];
    xcb_atom_t second_property = client_message->data.data32[2];
    struct xwm_atom *first_atom = NULL, *second_atom = NULL;
    for (size_t i = 0; i < ATOM_LAST; i++) {
        if (first_property == xwm->atoms[i].atom) {
            first_atom = &xwm->atoms[i];
            break;
        }
    }

    for (size_t i = 0; i < ATOM_LAST; i++) {
        if (second_property == xwm->atoms[i].atom) {
            second_atom = &xwm->atoms[i];
            break;
        }
    }

    bool changed = false;
    if (first_atom) {
        if (first_atom->handle_net_wm_state) {
            changed |= first_atom->handle_net_wm_state(xwm, xsurface, action);
        } else {
            char *prop_name = xwm_get_atom_name(xwm, first_atom->atom);
            kywc_log(KYWC_DEBUG,
                     "Unhandled net_wm_state message %" PRIu32 " (%s) for window %" PRIu32,
                     first_atom->atom, prop_name ? prop_name : "(null)", xsurface->window_id);
            free(prop_name);
        }
    }
    if (second_atom) {
        if (second_atom->handle_net_wm_state) {
            changed |= second_atom->handle_net_wm_state(xwm, xsurface, action);
        } else {
            char *prop_name = xwm_get_atom_name(xwm, second_atom->atom);
            kywc_log(KYWC_DEBUG,
                     "Unhandled net_wm_state message %" PRIu32 " (%s) for window %" PRIu32,
                     second_atom->atom, prop_name ? prop_name : "(null)", xsurface->window_id);
            free(prop_name);
        }
    }
    if (changed) {
        xsurface_set_net_wm_state(xsurface);
    }
}

static void xwm_handle_wm_protocols_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    xcb_atom_t type = ev->data.data32[0];

    if (type == xwm->atoms[NET_WM_PING].atom) {
        xcb_window_t window_id = ev->data.data32[2];

        struct xwayland_surface *surface = xwm_lookup_surface(xwm, window_id);
        if (surface == NULL) {
            return;
        }

        if (!surface->pinging) {
            return;
        }

        wl_event_source_timer_update(surface->ping_timer, 0);
        surface->pinging = false;
    } else {
        char *type_name = xwm_get_atom_name(xwm, type);
        kywc_log(KYWC_DEBUG, "unhandled WM_PROTOCOLS client message %" PRIu32 " (%s)", type,
                 type_name ? type_name : "(null)");
        free(type_name);
    }
}

static void xwm_handle_net_active_window_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (xsurface == NULL) {
        return;
    }

    if (xsurface->impl && xsurface->impl->request_activate) {
        xsurface->impl->request_activate(xsurface);
    }
}

static void pending_startup_id_destroy(struct pending_startup_id *pending)
{
    wl_list_remove(&pending->link);
    free(pending->msg);
    free(pending);
}

static void xwm_handle_net_startup_info_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct pending_startup_id *pending, *curr = NULL;
    wl_list_for_each(pending, &xwm->pending_startup_ids, link) {
        if (pending->window == ev->window) {
            curr = pending;
            break;
        }
    }

    char *start;
    size_t buf_len = sizeof(ev->data);
    if (curr) {
        curr->msg = realloc(curr->msg, curr->len + buf_len);
        if (!curr->msg) {
            pending_startup_id_destroy(curr);
            return;
        }
        start = curr->msg + curr->len;
        curr->len += buf_len;
    } else {
        curr = calloc(1, sizeof(*curr));
        if (!curr)
            return;
        curr->window = ev->window;
        curr->msg = malloc(buf_len);
        if (!curr->msg) {
            free(curr);
            return;
        }
        start = curr->msg;
        curr->len = buf_len;
        wl_list_insert(&xwm->pending_startup_ids, &curr->link);
    }

    char *id = NULL;
    const char *data = (const char *)ev->data.data8;
    for (size_t i = 0; i < buf_len; i++) {
        start[i] = data[i];
        if (start[i] == '\0') {
            if (strncmp(curr->msg, STARTUP_INFO_REMOVE_PREFIX,
                        strlen(STARTUP_INFO_REMOVE_PREFIX)) == 0 &&
                strlen(curr->msg) > strlen(STARTUP_INFO_REMOVE_PREFIX)) {
                id = curr->msg + strlen(STARTUP_INFO_REMOVE_PREFIX);
                break;
            } else {
                kywc_log(KYWC_ERROR, "Unhandled message '%s'\n", curr->msg);
                pending_startup_id_destroy(curr);
                return;
            }
        }
    }

    if (id) {
        // TODO: startup id?
        kywc_log(KYWC_DEBUG, "Got startup id: %s", id);
        pending_startup_id_destroy(curr);
    }
}

static void xwm_handle_wm_change_state_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (!xsurface) {
        return;
    }

    bool minimize;
    uint32_t detail = ev->data.data32[0];
    if (detail == XCB_ICCCM_WM_STATE_ICONIC) {
        minimize = true;
    } else if (detail == XCB_ICCCM_WM_STATE_NORMAL) {
        minimize = false;
    } else {
        kywc_log(KYWC_DEBUG, "unhandled wm_change_state event %u", detail);
        return;
    }

    xsurface->minimized = minimize;
    if (xsurface->impl && xsurface->impl->request_minimize) {
        xsurface->impl->request_minimize(xsurface);
    }
}

static void xwm_handle_moveresize_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwayland_surface *xsurface = xwm_lookup_surface(xwm, ev->window);
    if (!xsurface || !xsurface->surface) {
        return;
    }

    // gravity = ev->data.data32[0] & 0xff);
    int value_mask = (ev->data.data32[0] & 0xf00) >> 8;
    // source = (ev->data.data32[0] & 0xf000) >> 12;
    struct kywc_box box = {
        .x = value_mask & XCB_CONFIG_WINDOW_X ? (int)ev->data.data32[1] : xsurface->x,
        .y = value_mask & XCB_CONFIG_WINDOW_Y ? (int)ev->data.data32[2] : xsurface->y,
        .width = value_mask & XCB_CONFIG_WINDOW_WIDTH ? (int)ev->data.data32[3] : xsurface->width,
        .height = value_mask & XCB_CONFIG_WINDOW_HEIGHT ? (int)ev->data.data32[4] : xsurface->height
    };

    if (xsurface->impl && xsurface->impl->request_move_resize) {
        xsurface->impl->request_move_resize(xsurface, &box);
    }
}

static void xwm_handle_showing_desktop_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    bool state = ev->data.data32[0];
    view_manager_show_desktop(state, true);
}

static void xwm_handle_client_message(struct xwm *xwm, xcb_client_message_event_t *ev)
{
    struct xwm_atom *xwm_atom = NULL;
    for (size_t i = 0; i < ATOM_LAST; i++) {
        if (ev->type == xwm->atoms[i].atom) {
            xwm_atom = &xwm->atoms[i];
            break;
        }
    }

    if (xwm_atom && xwm_atom->handle_client_message) {
        xwm_atom->handle_client_message(xwm, ev);
    } else {
        if (!xwm_handle_selection_client_message(xwm, ev)) {
            char *type_name = xwm_get_atom_name(xwm, ev->type);
            kywc_log(KYWC_DEBUG, "unhandled x11 client message %" PRIu32 " (%s)", ev->type,
                     type_name ? type_name : "(null)");
            free(type_name);
        }
    }
}

static bool validate_focus_serial(uint16_t last_focus_seq, uint16_t event_seq)
{
    uint16_t rev_dist = event_seq - last_focus_seq;
    if (rev_dist >= UINT16_MAX / 2) {
        // Probably overflow or too old
        return false;
    }

    return true;
}

static void xwm_handle_focus_in(struct xwm *xwm, xcb_focus_in_event_t *ev)
{
    if (ev->event == xwm->guard_window || ev->event == xwm->no_focus_window) {
        return;
    }

    // Do not interfere with grabs
    if (ev->mode == XCB_NOTIFY_MODE_GRAB || ev->mode == XCB_NOTIFY_MODE_UNGRAB) {
        return;
    }
    // Ignore pointer focus change events
    if (ev->detail == XCB_NOTIFY_DETAIL_POINTER) {
        return;
    }

    // Ignore any out-of-date FocusIn event (older than the last
    // known WM-initiated focus change) to avoid race conditions.
    // https://github.com/swaywm/wlroots/issues/2324
    if (!validate_focus_serial(xwm->last_focus_seq, ev->sequence)) {
        return;
    }

    // Do not let X clients change the focus behind the compositor's
    // back. Reset the focus to the old one if it changed.
    //
    // Note: Some applications rely on being able to change focus, for ex. Steam:
    // https://github.com/swaywm/sway/issues/1865
    struct xwayland_surface *requested_focus = xwm_lookup_surface(xwm, ev->event);
    if (requested_focus &&
        ((xwm->focus_surface && requested_focus->pid == xwm->focus_surface->pid) ||
         requested_focus->override_redirect ||
         (xwm->offered_focus && requested_focus->pid == xwm->offered_focus->pid))) {
        if (requested_focus->surface && (requested_focus != xwm->focus_surface)) {
            xwm_set_focused_window(xwm, requested_focus);
            /* the offer focus surface is unfocusable, so we wait for reply, then focus surface */
            if (xwm->seat && xwm->offered_focus &&
                requested_focus->pid == xwm->offered_focus->pid) {
                seat_focus_surface(xwm->seat, requested_focus->surface);
            }
        }
    } else if (xwm->focus_surface) {
        xwm_focus_window(xwm, xwm->focus_surface);
    }
}

static void xwm_handle_reparent_notify(struct xwm *xwm, xcb_reparent_notify_event_t *ev)
{
    if (ev->parent == xwm->screen->root) {
        struct xwayland_surface *surface = xwm_lookup_surface(xwm, ev->window);
        if (!surface) {
            xwayland_surface_create(xwm, ev->window, ev->x, ev->y, 10, 10, ev->override_redirect);
        }
    }
}

static void xwm_handle_xcb_error(struct xwm *xwm, xcb_value_error_t *ev)
{
#if HAVE_XCB_ERRORS
    const char *major_name =
        xcb_errors_get_name_for_major_code(xwm->errors_context, ev->major_opcode);
    if (!major_name) {
        kywc_log(KYWC_DEBUG, "xcb error happened, but could not get major name");
        goto log_raw;
    }

    const char *minor_name =
        xcb_errors_get_name_for_minor_code(xwm->errors_context, ev->major_opcode, ev->minor_opcode);

    const char *extension;
    const char *error_name =
        xcb_errors_get_name_for_error(xwm->errors_context, ev->error_code, &extension);
    if (!error_name) {
        kywc_log(KYWC_DEBUG, "xcb error happened, but could not get error name");
        goto log_raw;
    }

    kywc_log(KYWC_DEBUG,
             "xcb error: op %s (%s), code %s (%s), sequence %" PRIu16 ", value %" PRIu32,
             major_name, minor_name ? minor_name : "no minor", error_name,
             extension ? extension : "no extension", ev->sequence, ev->bad_value);

    return;
log_raw:
#endif
    kywc_log(KYWC_DEBUG,
             "xcb error: op %" PRIu8 ":%" PRIu16 ", code %" PRIu8 ", sequence %" PRIu16
             ", value %" PRIu32,
             ev->major_opcode, ev->minor_opcode, ev->error_code, ev->sequence, ev->bad_value);
}

static void xwm_handle_unhandled_event(struct xwm *xwm, xcb_generic_event_t *ev)
{
#if HAVE_XCB_ERRORS
    const char *extension;
    const char *event_name = xcb_errors_get_name_for_xcb_event(xwm->errors_context, ev, &extension);
    if (!event_name) {
        kywc_log(KYWC_DEBUG, "no name for unhandled event: %u", ev->response_type);
        return;
    }

    kywc_log(KYWC_DEBUG, "unhandled X11 event: %s (%u)", event_name, ev->response_type);
#else
    kywc_log(KYWC_DEBUG, "unhandled X11 event: %u", ev->response_type);
#endif
}

static int x11_event_handler(int fd, uint32_t mask, void *data)
{
    int count = 0;
    xcb_generic_event_t *event;
    struct xwm *xwm = data;
    uint8_t response_type;

    if ((mask & WL_EVENT_HANGUP) || (mask & WL_EVENT_ERROR)) {
        xwm_destroy(xwm);
        return 0;
    }

    while ((event = xcb_poll_for_event(xwm->xcb_conn))) {
        count++;
        response_type = event->response_type & XCB_EVENT_RESPONSE_TYPE_MASK;
        if (xwm_handle_selection_event(xwm, event)) {
            free(event);
            continue;
        } else if (xwm->shape && response_type == xwm->shape->first_event + XCB_SHAPE_NOTIFY) {
            xwm_handle_surface_shape_notify(xwm, event);
            free(event);
            continue;
        } else if (xwm->sync && response_type == xwm->sync->first_event + XCB_SYNC_ALARM_NOTIFY) {
            xwm_handle_surface_alarm_notify(xwm, event);
            free(event);
            continue;
        }

        switch (response_type) {
        case XCB_CREATE_NOTIFY:
            xwm_handle_create_notify(xwm, (xcb_create_notify_event_t *)event);
            break;
        case XCB_DESTROY_NOTIFY:
            xwm_handle_destroy_notify(xwm, (xcb_destroy_notify_event_t *)event);
            break;
        case XCB_CONFIGURE_REQUEST:
            xwm_handle_configure_request(xwm, (xcb_configure_request_event_t *)event);
            break;
        case XCB_CONFIGURE_NOTIFY:
            xwm_handle_configure_notify(xwm, (xcb_configure_notify_event_t *)event);
            break;
        case XCB_MAP_REQUEST:
            xwm_handle_map_request(xwm, (xcb_map_request_event_t *)event);
            break;
        case XCB_MAP_NOTIFY:
            xwm_handle_map_notify(xwm, (xcb_map_notify_event_t *)event);
            break;
        case XCB_UNMAP_NOTIFY:
            xwm_handle_unmap_notify(xwm, (xcb_unmap_notify_event_t *)event);
            break;
        case XCB_PROPERTY_NOTIFY:
            xwm_handle_property_notify(xwm, (xcb_property_notify_event_t *)event);
            break;
        case XCB_CLIENT_MESSAGE:
            xwm_handle_client_message(xwm, (xcb_client_message_event_t *)event);
            break;
        case XCB_FOCUS_IN:
            xwm_handle_focus_in(xwm, (xcb_focus_in_event_t *)event);
            break;
        case XCB_REPARENT_NOTIFY:
            xwm_handle_reparent_notify(xwm, (xcb_reparent_notify_event_t *)event);
            break;
        case 0:
            xwm_handle_xcb_error(xwm, (xcb_value_error_t *)event);
            break;
        default:
            xwm_handle_unhandled_event(xwm, event);
            break;
        }
        free(event);
    }

    if (count) {
        xcb_flush(xwm->xcb_conn);
    }

    return count;
}

static void handle_compositor_new_surface(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, compositor_new_surface);
    struct wlr_surface *surface = data;

    struct wl_client *client = wl_resource_get_client(surface->resource);
    if (client != xwm->xwayland->xwayland_server->client) {
        return;
    }

    kywc_log(KYWC_DEBUG, "New xwayland surface: %p", surface);

    uint32_t surface_id = wl_resource_get_id(surface->resource);
    struct xwayland_surface *xsurface;
    wl_list_for_each(xsurface, &xwm->unpaired_surfaces, unpaired_link) {
        if (xsurface->surface_id == surface_id) {
            xwayland_surface_associate(xwm, xsurface, surface);
            xcb_flush(xwm->xcb_conn);
            return;
        }
    }
}

static void handle_compositor_destroy(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, compositor_destroy);
    wl_list_remove(&xwm->compositor_new_surface.link);
    wl_list_remove(&xwm->compositor_destroy.link);
    wl_list_init(&xwm->compositor_new_surface.link);
    wl_list_init(&xwm->compositor_destroy.link);
}

static void handle_shell_v1_new_surface(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, shell_v1_new_surface);
    struct xwayland_surface_v1 *shell_surface = data;

    struct xwayland_surface *xsurface;
    wl_list_for_each(xsurface, &xwm->unpaired_surfaces, unpaired_link) {
        if (xsurface->serial == shell_surface->serial) {
            xwayland_surface_associate(xwm, xsurface, shell_surface->wlr_surface);
            return;
        }
    }
}

static void handle_shell_v1_destroy(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, shell_v1_destroy);
    wl_list_remove(&xwm->shell_v1_new_surface.link);
    wl_list_remove(&xwm->shell_v1_destroy.link);
    wl_list_init(&xwm->shell_v1_new_surface.link);
    wl_list_init(&xwm->shell_v1_destroy.link);
}

char *xwm_get_atom_name(struct xwm *xwm, xcb_atom_t atom)
{
    xcb_get_atom_name_cookie_t name_cookie = xcb_get_atom_name(xwm->xcb_conn, atom);
    xcb_get_atom_name_reply_t *name_reply =
        xcb_get_atom_name_reply(xwm->xcb_conn, name_cookie, NULL);
    if (name_reply == NULL) {
        return strdup("UNKNOWN");
    }
    size_t len = xcb_get_atom_name_name_length(name_reply);
    char *buf = xcb_get_atom_name_name(name_reply);
    char *name = strndup(buf, len);
    free(name_reply);
    return name;
}

void xwayland_surface_activate(struct xwayland_surface *xsurface, bool activated)
{
    /* handle active NULL in xwm_handle_activate_view */
    if (activated) {
        xwm_surface_activate(xsurface->xwm, xsurface);
    }
}

void xwayland_surface_configure(struct xwayland_surface *xsurface, int16_t x, int16_t y,
                                uint16_t width, uint16_t height)
{
    int old_w = xsurface->width;
    int old_h = xsurface->height;

    xsurface->x = x;
    xsurface->y = y;
    xsurface->width = width;
    xsurface->height = height;

    struct xwm *xwm = xsurface->xwm;
    uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
                    XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH;
    uint32_t values[] = { x, y, width, height, 0 };
    xcb_configure_window(xwm->xcb_conn, xsurface->window_id, mask, values);

    // If the window size did not change, then we cannot rely on
    // the X server to generate a ConfigureNotify event. Instead,
    // we are supposed to send a synthetic event. See ICCCM part
    // 4.1.5. But we ignore override-redirect windows as ICCCM does
    // not apply to them.
    if (width == old_w && height == old_h && !xsurface->override_redirect) {
        xcb_configure_notify_event_t configure_notify = {
            .response_type = XCB_CONFIGURE_NOTIFY,
            .event = xsurface->window_id,
            .window = xsurface->window_id,
            .x = x,
            .y = y,
            .width = width,
            .height = height,
        };

        xwm_send_event_with_size(xwm->xcb_conn, 0, xsurface->window_id,
                                 XCB_EVENT_MASK_STRUCTURE_NOTIFY, &configure_notify,
                                 sizeof(configure_notify));
    }

    xcb_flush(xwm->xcb_conn);
}

void xwayland_surface_close(struct xwayland_surface *xsurface)
{
    struct xwm *xwm = xsurface->xwm;

    bool supports_delete = false;
    for (size_t i = 0; i < xsurface->protocols_len; i++) {
        if (xsurface->protocols[i] == xwm->atoms[WM_DELETE_WINDOW].atom) {
            supports_delete = true;
            break;
        }
    }

    if (supports_delete) {
        xcb_client_message_data_t message_data = { 0 };
        message_data.data32[0] = xwm->atoms[WM_DELETE_WINDOW].atom;
        message_data.data32[1] = XCB_CURRENT_TIME;
        xwm_send_wm_message(xsurface, &message_data, XCB_EVENT_MASK_NO_EVENT);
    } else {
        xcb_kill_client(xwm->xcb_conn, xsurface->window_id);
        xcb_flush(xwm->xcb_conn);
    }
}

void xwm_destroy(struct xwm *xwm)
{
    if (!xwm) {
        return;
    }
    xwm->destroying = true;

    eavesdrops_manager_destroy();

    xwm_end_drag_x11(xwm);
    xwm_selection_finish(&xwm->clipboard_selection);
    xwm_selection_finish(&xwm->primary_selection);
    xwm_selection_finish(&xwm->dnd_selection);

    xwm_seat_unlink_drag_handlers(xwm);

    if (xwm->seat) {
        if (xwm->seat->wlr_seat->selection_source &&
            data_source_is_xwayland(xwm->seat->wlr_seat->selection_source)) {
            wlr_seat_set_selection(xwm->seat->wlr_seat, NULL,
                                   wl_display_next_serial(xwm->xwayland->server->display));
        }

        if (xwm->seat->wlr_seat->primary_selection_source &&
            primary_selection_source_is_xwayland(xwm->seat->wlr_seat->primary_selection_source)) {
            wlr_seat_set_primary_selection(xwm->seat->wlr_seat, NULL,
                                           wl_display_next_serial(xwm->xwayland->server->display));
        }

        xwayland_update_seat(NULL);
    }

    if (xwm->cursor) {
        xcb_free_cursor(xwm->xcb_conn, xwm->cursor);
    }
    if (xwm->colormap) {
        xcb_free_colormap(xwm->xcb_conn, xwm->colormap);
    }
    if (xwm->window) {
        xcb_destroy_window(xwm->xcb_conn, xwm->window);
    }
    if (xwm->no_focus_window) {
        xcb_destroy_window(xwm->xcb_conn, xwm->no_focus_window);
    }
    if (xwm->guard_window) {
        xcb_destroy_window(xwm->xcb_conn, xwm->guard_window);
    }
    if (xwm->event_source) {
        wl_event_source_remove(xwm->event_source);
    }
    if (xwm->idle_source) {
        wl_event_source_remove(xwm->idle_source);
    }
#if HAVE_XCB_ERRORS
    if (xwm->errors_context) {
        xcb_errors_context_free(xwm->errors_context);
    }
#endif
    struct xwayland_surface *xsurface, *tmp;
    wl_list_for_each_safe(xsurface, tmp, &xwm->surfaces, link) {
        xwayland_surface_destroy(xsurface);
    }
    wl_list_for_each_safe(xsurface, tmp, &xwm->unpaired_surfaces, unpaired_link) {
        xwayland_surface_destroy(xsurface);
    }
    wl_list_remove(&xwm->compositor_new_surface.link);
    wl_list_remove(&xwm->compositor_destroy.link);
    wl_list_remove(&xwm->shell_v1_new_surface.link);
    wl_list_remove(&xwm->shell_v1_destroy.link);
    wl_list_remove(&xwm->seat_destroy.link);
    wl_list_remove(&xwm->seat_set_selection.link);
    wl_list_remove(&xwm->seat_set_primary_selection.link);
    wl_list_remove(&xwm->seat_start_drag.link);
    wl_list_remove(&xwm->max_scale.link);
    wl_list_remove(&xwm->active_workspace.link);
    wl_list_remove(&xwm->activate_view.link);
    xcb_disconnect(xwm->xcb_conn);

    struct pending_startup_id *pending, *next;
    wl_list_for_each_safe(pending, next, &xwm->pending_startup_ids, link) {
        pending_startup_id_destroy(pending);
    }

    xwm->xwayland->xwm = NULL;
    free(xwm);
}

static struct {
    const char *name;
    void (*handle_client_message)(struct xwm *xwm, xcb_client_message_event_t *ev);
    void (*handle_surface_property)(struct xwm *xwm, struct xwayland_surface *xsurface,
                                    xcb_get_property_reply_t *reply, bool delete);
    bool (*handle_net_wm_state)(struct xwm *xwm, struct xwayland_surface *xsurface,
                                uint32_t action);
} const atom_map[ATOM_LAST] = {
    [WM_S0] = {.name = "WM_S0",},
    [NET_SUPPORTED] = {.name = "_NET_SUPPORTED",},
    [NET_SUPPORTING_WM_CHECK] = {.name = "_NET_SUPPORTING_WM_CHECK",},
    [NET_WM_PING] = {.name = "_NET_WM_PING",},
    [NET_WORKAREA] = {.name = "_NET_WORKAREA",},
    
    // NET_WM
    [NET_WM_CM_S0] = {.name = "_NET_WM_CM_S0",},
    [NET_WM_PID] = {.name = "_NET_WM_PID",},
    [NET_WM_STRUT_PARTIAL] = {.name = "_NET_WM_STRUT_PARTIAL", 
                              .handle_surface_property = xwm_handle_surface_strut_partial},
    [NET_WM_ICON] = {.name = "_NET_WM_ICON",
                     .handle_surface_property = xwm_handle_surface_icon},
    [NET_WM_WINDOW_OPACITY] = {.name = "_NET_WM_WINDOW_OPACITY",
                               .handle_surface_property = xwm_handle_surface_opacity},
    [NET_WM_OPAQUE_REGION] = {.name = "_NET_WM_OPAQUE_REGION",
                              .handle_surface_property = xwm_handle_surface_opaque_region},
    [NET_WM_NAME] = {.name = "_NET_WM_NAME",
                    .handle_surface_property = xwm_handle_surface_title},

    // active focus
    [WM_TAKE_FOCUS] = {.name = "WM_TAKE_FOCUS",}, 
    [WINDOW] = {.name = "WINDOW",},
    [NET_ACTIVE_WINDOW] = {.name = "_NET_ACTIVE_WINDOW",
                           .handle_client_message = xwm_handle_net_active_window_message},

    // WM_STATE
    [WM_STATE] = {.name = "WM_STATE",},

    // NET_WM_STATE
    [NET_WM_STATE] = {.name = "_NET_WM_STATE",
                      .handle_client_message = xwm_handle_net_wm_state_message,
                      .handle_surface_property = xwm_handle_surface_net_wm_state},
    [NET_WM_STATE_FOCUSED] = {.name = "_NET_WM_STATE_FOCUSED",},
    [NET_WM_STATE_MODAL] = {.name = "_NET_WM_STATE_MODAL",
                            .handle_net_wm_state = xwm_handle_net_wm_modal},
    [NET_WM_STATE_FULLSCREEN] = {.name = "_NET_WM_STATE_FULLSCREEN",
                                 .handle_net_wm_state = xwm_handle_net_wm_fullscreen},
    [NET_WM_STATE_MAXIMIZED_VERT] = {.name = "_NET_WM_STATE_MAXIMIZED_VERT",
                                     .handle_net_wm_state = xwm_handle_net_wm_maximize_vert},
    [NET_WM_STATE_MAXIMIZED_HORZ] = {.name = "_NET_WM_STATE_MAXIMIZED_HORZ",
                                     .handle_net_wm_state = xwm_handle_net_wm_maximize_horz},
    [NET_WM_STATE_HIDDEN] = {.name = "_NET_WM_STATE_HIDDEN",
                             .handle_net_wm_state = xwm_handle_net_wm_hidden},
    [NET_WM_STATE_STICKY] = {.name = "_NET_WM_STATE_STICKY",
                             .handle_net_wm_state = xwm_handle_net_wm_sticky},
    [NET_WM_STATE_SKIP_TASKBAR] = {.name = "_NET_WM_STATE_SKIP_TASKBAR",
                                   .handle_net_wm_state = xwm_handle_net_wm_skip_taskbar},
    [NET_WM_STATE_ABOVE] = {.name = "_NET_WM_STATE_ABOVE",
                            .handle_net_wm_state = xwm_handle_net_wm_above},
    [NET_WM_STATE_BELOW] = {.name = "_NET_WM_STATE_BELOW",
                            .handle_net_wm_state = xwm_handle_net_wm_below},
    [NET_WM_STATE_DEMANDS_ATTENTION] = {.name = "_NET_WM_STATE_DEMANDS_ATTENTION",
                                        .handle_net_wm_state = xwm_handle_net_wm_demands_attention},
    [KDE_NET_WM_STATE_SKIP_SWITCHER] = {.name = "_KDE_NET_WM_STATE_SKIP_SWITCHER",
                                        .handle_net_wm_state = xwm_handle_net_wm_skip_switcher},

    [KWIN_UKUI_DECORATION] = {.name = "_KWIN_UKUI_DECORAION", // it has been misspelled since V10
                              .handle_surface_property = xwm_handle_surface_ukui_decoration},
    [KDE_NET_WM_BLUR_BEHIND_REGION] = {.name = "_KDE_NET_WM_BLUR_BEHIND_REGION",
                                       .handle_surface_property = xwm_handle_surface_blur_region},

    // client message
    [WL_SURFACE_ID] = {.name = "WL_SURFACE_ID",
                       .handle_client_message = xwm_handle_surface_id_message},
    [WL_SURFACE_SERIAL] = {.name = "WL_SURFACE_SERIAL",
                           .handle_client_message = xwm_handle_surface_serial_message},
    [WM_DELETE_WINDOW] = {.name = "WM_DELETE_WINDOW",},
    [WM_HINTS] = {.name = "WM_HINTS",
                  .handle_surface_property = xwm_handle_surface_hints},
    [WM_NORMAL_HINTS] = {.name = "WM_NORMAL_HINTS",
                         .handle_surface_property = xwm_handle_surface_normal_hints},
    [WM_SIZE_HINTS] = {.name = "WM_SIZE_HINTS",},
    [WM_WINDOW_ROLE] = {.name = "WM_WINDOW_ROLE",
                        .handle_surface_property = xwm_handle_surface_role},
    [MOTIF_WM_HINTS] = {.name = "_MOTIF_WM_HINTS",
                        .handle_surface_property = xwm_handle_surface_motif_hints},
    [NET_WM_MOVERESIZE] = {.name = "_NET_WM_MOVERESIZE",
                           .handle_client_message = xwm_handle_net_wm_moveresize_message},
    [WM_CHANGE_STATE] = {.name = "WM_CHANGE_STATE",
                         .handle_client_message = xwm_handle_wm_change_state_message},
    [NET_MOVERESIZE_WINDOW] = {.name = "_NET_MOVERESIZE_WINDOW", 
                               .handle_client_message = xwm_handle_moveresize_message},
    [NET_SHOWING_DESKTOP] = {.name = "_NET_SHOWING_DESKTOP", 
                               .handle_client_message = xwm_handle_showing_desktop_message},                           

    // WINDOW TYPE
    [NET_WM_WINDOW_TYPE] = {.name = "_NET_WM_WINDOW_TYPE",
                            .handle_surface_property = xwm_handle_surface_window_type},
    [NET_WM_WINDOW_TYPE_DESKTOP] = {.name = "_NET_WM_WINDOW_TYPE_DESKTOP",},
    [NET_WM_WINDOW_TYPE_DOCK] = {.name = "_NET_WM_WINDOW_TYPE_DOCK",},
    [NET_WM_WINDOW_TYPE_TOOLBAR] = {.name = "_NET_WM_WINDOW_TYPE_TOOLBAR",},
    [NET_WM_WINDOW_TYPE_MENU] = {.name = "_NET_WM_WINDOW_TYPE_MENU",},
    [NET_WM_WINDOW_TYPE_UTILITY] = {.name = "_NET_WM_WINDOW_TYPE_UTILITY",},
    [NET_WM_WINDOW_TYPE_SPLASH] = {.name = "_NET_WM_WINDOW_TYPE_SPLASH",},
    [NET_WM_WINDOW_TYPE_DIALOG] = {.name = "_NET_WM_WINDOW_TYPE_DIALOG",},
    [NET_WM_WINDOW_TYPE_DROPDOWN_MENU] = {.name = "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",},
    [NET_WM_WINDOW_TYPE_POPUP_MENU] = {.name = "_NET_WM_WINDOW_TYPE_POPUP_MENU",},
    [NET_WM_WINDOW_TYPE_COMBO] = {.name = "_NET_WM_WINDOW_TYPE_COMBO",},
    [NET_WM_WINDOW_TYPE_TOOLTIP] = {.name = "_NET_WM_WINDOW_TYPE_TOOLTIP",},
    [NET_WM_WINDOW_TYPE_DND] = {.name = "_NET_WM_WINDOW_TYPE_DND",},
    [NET_WM_WINDOW_TYPE_NOTIFICATION] = {.name = "_NET_WM_WINDOW_TYPE_NOTIFICATION",},
    [NET_WM_WINDOW_TYPE_NORMAL] = {.name = "_NET_WM_WINDOW_TYPE_NORMAL",},
    [UKUI_NET_WM_WINDOW_TYPE_SYSTEMWINDOW] = {.name = "_UKUI_NET_WM_WINDOW_TYPE_SYSTEMWINDOW",},
    [UKUI_NET_WM_WINDOW_TYPE_INPUTPANEL] = {.name = "_UKUI_NET_WM_WINDOW_TYPE_INPUTPANEL",},
    [UKUI_NET_WM_WINDOW_TYPE_LOGOUT] = {.name = "_UKUI_NET_WM_WINDOW_TYPE_LOGOUT",},
    [UKUI_NET_WM_WINDOW_TYPE_SCREENLOCK] = {.name = "_UKUI_NET_WM_WINDOW_TYPE_SCREENLOCK",},
    [UKUI_NET_WM_WINDOW_TYPE_SCREENLOCKNOTIFICATION] = {.name ="_UKUI_NET_WM_WINDOW_TYPE_SCREENLOCKNOTIFICATION",},
    [UKUI_NET_WM_WINDOW_TYPE_WATERMARK] = {.name = "_UKUI_NET_WM_WINDOW_TYPE_WATERMARK",},
    [KDE_NET_WM_WINDOW_TYPE_OVERRIDE] = {.name = "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",},
    [KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY] = {.name = "_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY",},

    // startup
    [NET_STARTUP_ID] = {.name = "_NET_STARTUP_ID",
                        .handle_surface_property = xwm_handle_surface_startup_id},
    [NET_STARTUP_INFO] = {.name = "_NET_STARTUP_INFO",
                          .handle_client_message = xwm_handle_net_startup_info_message},
    [NET_STARTUP_INFO_BEGIN] = {.name = "_NET_STARTUP_INFO_BEGIN",
                                .handle_client_message = xwm_handle_net_startup_info_message},

    // stacking
    [NET_CLIENT_LIST] = {.name = "_NET_CLIENT_LIST",},
    [NET_CLIENT_LIST_STACKING] = {.name = "_NET_CLIENT_LIST_STACKING",},
   
    // selection
    [CLIPBOARD] = {.name = "CLIPBOARD",},
    [PRIMARY] = {.name = "PRIMARY",},
    [TIMESTAMP] = {.name = "TIMESTAMP",},
    [DELETE] = {.name = "DELETE",},
    [UTF8_STRING] = {.name = "UTF8_STRING",},
    [INCR] = {.name = "INCR",},
    [TEXT] = {.name = "TEXT",},
    [TARGETS] = {.name = "TARGETS",},
    [WL_SELECTION] = {.name = "_WL_SELECTION",},
    [CLIPBOARD_MANAGER] = {.name = "CLIPBOARD_MANAGER",},
    // dnd
    [DND_SELECTION] = {.name = "XdndSelection",},
    [DND_AWARE] = {.name = "XdndAware",},
    [DND_STATUS] = {.name = "XdndStatus",},
    [DND_POSITION] = {.name = "XdndPosition",},
    [DND_ENTER] = {.name = "XdndEnter",},
    [DND_LEAVE] = {.name = "XdndLeave",},
    [DND_DROP] = {.name = "XdndDrop",},
    [DND_FINISHED] = {.name = "XdndFinished",},
    [DND_PROXY] = {.name = "XdndProxy",},
    [DND_TYPE_LIST] = {.name = "XdndTypeList",},
    [DND_ACTION_MOVE] = {.name = "XdndActionMove",},
    [DND_ACTION_COPY] = {.name = "XdndActionCopy",},
    [DND_ACTION_ASK] = {.name = "XdndActionAsk",},
    [DND_ACTION_PRIVATE] = {.name = "XdndActionPrivate",},

    [WM_PROTOCOLS] = {.name = "WM_PROTOCOLS",
                      .handle_client_message = xwm_handle_wm_protocols_message,
                      .handle_surface_property = xwm_handle_surface_protocols},

    // sync request
    [NET_WM_SYNC_REQUEST] = {.name = "_NET_WM_SYNC_REQUEST",},
    [NET_WM_SYNC_REQUEST_COUNTER] = {.name = "_NET_WM_SYNC_REQUEST_COUNTER", 
                                     .handle_surface_property = xwm_handle_surface_sync_counter},
    [XWAYLAND_ALLOW_COMMITS] = {.name = "_XWAYLAND_ALLOW_COMMITS",}
};

static void xwm_get_atoms(struct xwm *xwm)
{
    size_t i;
    xcb_intern_atom_cookie_t cookies[ATOM_LAST];

    for (i = 0; i < ATOM_LAST; i++) {
        cookies[i] = xcb_intern_atom(xwm->xcb_conn, 0, strlen(atom_map[i].name), atom_map[i].name);
    }
    for (i = 0; i < ATOM_LAST; i++) {
        xcb_generic_error_t *error;
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xwm->xcb_conn, cookies[i], &error);
        if (reply && !error) {
            xwm->atoms[i].atom = reply->atom;
            xwm->atoms[i].handle_surface_property = atom_map[i].handle_surface_property;
            xwm->atoms[i].handle_client_message = atom_map[i].handle_client_message;
            xwm->atoms[i].handle_net_wm_state = atom_map[i].handle_net_wm_state;
        }
        free(reply);

        if (error) {
            kywc_log(KYWC_ERROR, "Could not resolve atom %s, x11 error code %d", atom_map[i].name,
                     error->error_code);
            free(error);
            return;
        }
    }
}

static void xwm_get_shape_extension(struct xwm *xwm)
{
    xwm->shape = xcb_get_extension_data(xwm->xcb_conn, &xcb_shape_id);
    if (!xwm->shape || !xwm->shape->present) {
        kywc_log(KYWC_WARN, "Shape not available");
        xwm->shape = NULL;
        return;
    }

    xcb_shape_query_version_cookie_t shape_cookie;
    xcb_shape_query_version_reply_t *shape_reply;
    shape_cookie = xcb_shape_query_version(xwm->xcb_conn);
    shape_reply = xcb_shape_query_version_reply(xwm->xcb_conn, shape_cookie, NULL);
    if (!shape_reply) {
        return;
    }

    kywc_log(KYWC_INFO, "Shape version: %" PRIu32 ".%" PRIu32, shape_reply->major_version,
             shape_reply->minor_version);
    free(shape_reply);
}

static void xwm_get_xfixes_extension(struct xwm *xwm)
{
    xwm->xfixes = xcb_get_extension_data(xwm->xcb_conn, &xcb_xfixes_id);
    if (!xwm->xfixes || !xwm->xfixes->present) {
        kywc_log(KYWC_WARN, "Xfixes not available");
        xwm->xfixes = NULL;
        return;
    }

    xcb_xfixes_query_version_cookie_t xfixes_cookie;
    xcb_xfixes_query_version_reply_t *xfixes_reply;
    xfixes_cookie =
        xcb_xfixes_query_version(xwm->xcb_conn, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);
    xfixes_reply = xcb_xfixes_query_version_reply(xwm->xcb_conn, xfixes_cookie, NULL);
    if (!xfixes_reply) {
        return;
    }
    xwm->xfixes_major_version = xfixes_reply->major_version;

    kywc_log(KYWC_INFO, "Xfixes version: %" PRIu32 ".%" PRIu32, xfixes_reply->major_version,
             xfixes_reply->minor_version);
    free(xfixes_reply);
}

static void xwm_get_sync_extension(struct xwm *xwm)
{
    xwm->sync = xcb_get_extension_data(xwm->xcb_conn, &xcb_sync_id);
    if (!xwm->sync || !xwm->sync->present) {
        kywc_log(KYWC_WARN, "Sync not available");
        xwm->sync = NULL;
        return;
    }

    xcb_sync_initialize_cookie_t sync_cookie =
        xcb_sync_initialize(xwm->xcb_conn, XCB_SYNC_MAJOR_VERSION, XCB_SYNC_MINOR_VERSION);
    xcb_sync_initialize_reply_t *sync_reply =
        xcb_sync_initialize_reply(xwm->xcb_conn, sync_cookie, NULL);
    if (!sync_reply) {
        return;
    }

    kywc_log(KYWC_INFO, "Sync version: %" PRIu32 ".%" PRIu32, sync_reply->major_version,
             sync_reply->minor_version);
    free(sync_reply);
}

static void xwm_get_xres_extension(struct xwm *xwm)
{
    const xcb_query_extension_reply_t *xres = xcb_get_extension_data(xwm->xcb_conn, &xcb_res_id);
    if (!xres || !xres->present) {
        kywc_log(KYWC_WARN, "Xres not available");
        xwm->xres = NULL;
        return;
    }

    xcb_res_query_version_cookie_t xres_cookie =
        xcb_res_query_version(xwm->xcb_conn, XCB_RES_MAJOR_VERSION, XCB_RES_MINOR_VERSION);
    xcb_res_query_version_reply_t *xres_reply =
        xcb_res_query_version_reply(xwm->xcb_conn, xres_cookie, NULL);
    if (!xres_reply) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Xres version: %" PRIu32 ".%" PRIu32, xres_reply->server_major,
             xres_reply->server_minor);
    if (xres_reply->server_major > 1 ||
        (xres_reply->server_major == 1 && xres_reply->server_minor >= 2)) {
        xwm->xres = xres;
    }
    free(xres_reply);
}

static void xwm_get_resources(struct xwm *xwm)
{
    xcb_prefetch_extension_data(xwm->xcb_conn, &xcb_xfixes_id);
    xcb_prefetch_extension_data(xwm->xcb_conn, &xcb_composite_id);
    xcb_prefetch_extension_data(xwm->xcb_conn, &xcb_res_id);
    xcb_prefetch_extension_data(xwm->xcb_conn, &xcb_shape_id);
    xcb_prefetch_extension_data(xwm->xcb_conn, &xcb_sync_id);

    xwm_get_atoms(xwm);

    /**
     * workaround to fix java apps show blank window
     * wlcom is not a reparenting window manager
     * or set _JAVA_AWT_WM_NONREPARENTING=1
     */
    const char name[] = "LG3D";
    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_SUPPORTING_WM_CHECK].atom, XCB_ATOM_WINDOW, 32, 1,
                        &xwm->screen->root);
    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_WM_NAME].atom, xwm->atoms[UTF8_STRING].atom, 8, strlen(name),
                        name);

    xwm_get_shape_extension(xwm);
    xwm_get_xfixes_extension(xwm);
    xwm_get_sync_extension(xwm);
    xwm_get_xres_extension(xwm);
}

static void xwm_create_wm_window(struct xwm *xwm)
{
    static const char name[] = "wlcom wm";

    xwm->window = xcb_generate_id(xwm->xcb_conn);

    xcb_create_window(xwm->xcb_conn, XCB_COPY_FROM_PARENT, xwm->window, xwm->screen->root, 0, 0, 10,
                      10, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, xwm->screen->root_visual, 0, NULL);

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->window,
                        xwm->atoms[NET_WM_NAME].atom, xwm->atoms[UTF8_STRING].atom, 8, strlen(name),
                        name);

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_SUPPORTING_WM_CHECK].atom, XCB_ATOM_WINDOW, 32, 1,
                        &xwm->window);

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->window,
                        xwm->atoms[NET_SUPPORTING_WM_CHECK].atom, XCB_ATOM_WINDOW, 32, 1,
                        &xwm->window);

    xcb_set_selection_owner(xwm->xcb_conn, xwm->window, xwm->atoms[WM_S0].atom, XCB_CURRENT_TIME);

    xcb_set_selection_owner(xwm->xcb_conn, xwm->window, xwm->atoms[NET_WM_CM_S0].atom,
                            XCB_CURRENT_TIME);
}

// TODO use me to support 32 bit color somehow
static void xwm_get_visual_and_colormap(struct xwm *xwm)
{
    xcb_depth_iterator_t d_iter;
    xcb_visualtype_iterator_t vt_iter;
    xcb_visualtype_t *visualtype;

    d_iter = xcb_screen_allowed_depths_iterator(xwm->screen);
    visualtype = NULL;
    while (d_iter.rem > 0) {
        if (d_iter.data->depth == 32) {
            vt_iter = xcb_depth_visuals_iterator(d_iter.data);
            visualtype = vt_iter.data;
            break;
        }

        xcb_depth_next(&d_iter);
    }

    if (visualtype == NULL) {
        kywc_log(KYWC_DEBUG, "No 32 bit visualtype\n");
        return;
    }

    xwm->visual_id = visualtype->visual_id;
    xwm->colormap = xcb_generate_id(xwm->xcb_conn);
    xcb_create_colormap(xwm->xcb_conn, XCB_COLORMAP_ALLOC_NONE, xwm->colormap, xwm->screen->root,
                        xwm->visual_id);
}

static void xwm_get_render_format(struct xwm *xwm)
{
    xcb_render_query_pict_formats_cookie_t cookie = xcb_render_query_pict_formats(xwm->xcb_conn);
    xcb_render_query_pict_formats_reply_t *reply =
        xcb_render_query_pict_formats_reply(xwm->xcb_conn, cookie, NULL);
    if (!reply) {
        kywc_log(KYWC_ERROR, "Did not get any reply from xcb_render_query_pict_formats");
        return;
    }
    xcb_render_pictforminfo_iterator_t iter = xcb_render_query_pict_formats_formats_iterator(reply);
    xcb_render_pictforminfo_t *format = NULL;
    while (iter.rem > 0) {
        if (iter.data->depth == 32) {
            format = iter.data;
            break;
        }

        xcb_render_pictforminfo_next(&iter);
    }

    if (format == NULL) {
        kywc_log(KYWC_DEBUG, "No 32 bit render format");
        free(reply);
        return;
    }

    xwm->render_format_id = format->id;
    free(reply);
}

static bool xwm_atoms_contains(struct xwm *xwm, xcb_atom_t *atoms, size_t num_atoms,
                               enum atom_name needle)
{
    xcb_atom_t atom = xwm->atoms[needle].atom;

    for (size_t i = 0; i < num_atoms; ++i) {
        if (atom == atoms[i]) {
            return true;
        }
    }

    return false;
}

static void xwm_update_xresources(struct xwm *xwm)
{
    struct seat *seat = xwm->seat;

    const char *prop_str =
        string_create("Xft.dpi:\t%d\nXcursor.size:\t%d\nXcursor.theme:\t%s\n",
                      (int)(xwm->scale * 96), (int)(seat->state.cursor_size * xwm->scale),
                      seat->state.cursor_theme ? seat->state.cursor_theme : "default");
    if (!prop_str) {
        return;
    }

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        XCB_ATOM_RESOURCE_MANAGER, XCB_ATOM_STRING, 8, strlen(prop_str), prop_str);
    xcb_flush(xwm->xcb_conn);

    free((void *)prop_str);
}

static void xwm_set_cursor(struct xwm *xwm, const uint8_t *pixels, uint32_t stride, uint32_t width,
                           uint32_t height, int32_t hotspot_x, int32_t hotspot_y)
{
    if (!xwm->render_format_id) {
        kywc_log(KYWC_ERROR, "Cannot set xwm cursor: no render format available");
        return;
    }
    if (xwm->cursor) {
        xcb_free_cursor(xwm->xcb_conn, xwm->cursor);
    }

    int depth = 32;

    xcb_pixmap_t pix = xcb_generate_id(xwm->xcb_conn);
    xcb_create_pixmap(xwm->xcb_conn, depth, pix, xwm->screen->root, width, height);

    xcb_render_picture_t pic = xcb_generate_id(xwm->xcb_conn);
    xcb_render_create_picture(xwm->xcb_conn, pic, pix, xwm->render_format_id, 0, 0);

    xcb_gcontext_t gc = xcb_generate_id(xwm->xcb_conn);
    xcb_create_gc(xwm->xcb_conn, gc, pix, 0, NULL);

    xcb_put_image(xwm->xcb_conn, XCB_IMAGE_FORMAT_Z_PIXMAP, pix, gc, width, height, 0, 0, 0, depth,
                  stride * height * sizeof(uint8_t), pixels);
    xcb_free_gc(xwm->xcb_conn, gc);

    xwm->cursor = xcb_generate_id(xwm->xcb_conn);
    xcb_render_create_cursor(xwm->xcb_conn, xwm->cursor, pic, hotspot_x, hotspot_y);
    xcb_free_pixmap(xwm->xcb_conn, pix);
    xcb_render_free_picture(xwm->xcb_conn, pic);

    uint32_t values[] = { xwm->cursor };
    xcb_change_window_attributes(xwm->xcb_conn, xwm->screen->root, XCB_CW_CURSOR, values);
    xcb_flush(xwm->xcb_conn);
}

void xwm_update_cursor(struct xwm *xwm)
{
    struct seat *seat = xwm->seat;
    if (!seat) {
        return;
    }

    wlr_xcursor_manager_load(seat->cursor->xcursor_manager, xwm->scale);
    struct wlr_xcursor *xcursor =
        wlr_xcursor_manager_get_xcursor(seat->cursor->xcursor_manager, "left_ptr", xwm->scale);
    if (xcursor) {
        struct wlr_xcursor_image *image = xcursor->images[0];
        xwm_set_cursor(xwm, image->buffer, image->width * 4, image->width, image->height,
                       image->hotspot_x, image->hotspot_y);
    }

    xwm_update_xresources(xwm);
}

static void xwm_handle_max_scale(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, max_scale);

    float scale = output_manager_get_max_scale();
    if (xwm->scale == scale) {
        return;
    }

    xwm->scale = scale;
    kywc_log(KYWC_INFO, "Xwayland set scale to %f", xwm->scale);

    /* update default cursor with current scale */
    xwayland_set_cursor(xwm->seat);
    /* configure xwayland surface geometry by the new scale */
    struct kywc_output *output = data;
    struct xwayland_surface *xsurface;
    wl_list_for_each(xsurface, &xwm->surfaces, link) {
        if (xsurface->impl && xsurface->impl->try_reconfigure) {
            xsurface->impl->try_reconfigure(xsurface, output);
        }
    }
}

static bool view_iterator(struct kywc_view *kywc_view, void *data)
{
    struct view *view = view_from_kywc_view(kywc_view);
    struct xwayland_surface *xsurface = xwayland_surface_from_view(view);
    if (xsurface) {
        xwayland_surface_restack(xsurface, NULL, XCB_STACK_MODE_ABOVE);
    }
    return false;
}

static void xwm_handle_active_workspace(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, active_workspace);

    view_manager_for_each_stack_view(view_iterator, NULL, true, NULL);
}

static void xwm_handle_activate_view(struct wl_listener *listener, void *data)
{
    struct xwm *xwm = wl_container_of(listener, xwm, activate_view);
    if (!xwm->focus_surface) {
        return;
    }

    struct kywc_view *view = data;
    if (view && xwayland_check_view(view_from_kywc_view(view))) {
        return;
    }

    /* clear focus if focus to wayland */
    xwm_surface_activate(xwm, NULL);
}

static void xwm_create_no_focus_window(struct xwm *xwm)
{
    xwm->no_focus_window = xcb_generate_id(xwm->xcb_conn);
    kywc_log(KYWC_DEBUG, "create no focus window 0x%x", xwm->no_focus_window);

    uint32_t values[2] = { 1, XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                                  XCB_EVENT_MASK_FOCUS_CHANGE };
    xcb_create_window(xwm->xcb_conn, XCB_COPY_FROM_PARENT, xwm->no_focus_window, xwm->screen->root,
                      -100, -100, 1, 1, 0, XCB_WINDOW_CLASS_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT,
                      XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values);

    xcb_map_window(xwm->xcb_conn, xwm->no_focus_window);
}

static void xwm_create_guard_window(struct xwm *xwm)
{

    xwm->guard_window = xcb_generate_id(xwm->xcb_conn);
    uint32_t values[] = {
        XCB_EVENT_MASK_NO_EVENT,
        1,
    };
    xcb_create_window(xwm->xcb_conn, 0, xwm->guard_window, xwm->screen->root, 0, 0, 8192, 8192, 0,
                      XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT,
                      XCB_CW_EVENT_MASK | XCB_CW_OVERRIDE_REDIRECT, values);

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->guard_window, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, strlen("wlcom guard window"), "wlcom guard window");

    xcb_map_window(xwm->xcb_conn, xwm->guard_window);
    xcb_flush(xwm->xcb_conn);
}

struct xwm *xwm_create(struct xwayland_manager *xwayland, int wm_fd)
{
    struct xwm *xwm = calloc(1, sizeof(*xwm));
    if (!xwm) {
        return NULL;
    }

    xwm->xwayland = xwayland;
    xwm->display = xwayland->server->display;
    xwm->scale = output_manager_get_max_scale();
    wl_list_init(&xwm->surfaces);
    wl_list_init(&xwm->surfaces_in_stack_order);
    wl_list_init(&xwm->unpaired_surfaces);
    wl_list_init(&xwm->pending_startup_ids);
    wl_list_init(&xwm->seat_drag_source_destroy.link);
    wl_list_init(&xwm->drag_focus_destroy.link);
    wl_list_init(&xwm->drop_focus_destroy.link);

    xwm->ping_timeout = 10000;

    xwm->xcb_conn = xcb_connect_to_fd(wm_fd, NULL);

    int rc = xcb_connection_has_error(xwm->xcb_conn);
    if (rc) {
        kywc_log(KYWC_ERROR, "xcb connect failed: %d", rc);
        free(xwm);
        return NULL;
    }

#if HAVE_XCB_ERRORS
    if (xcb_errors_context_new(xwm->xcb_conn, &xwm->errors_context)) {
        kywc_log(KYWC_ERROR, "Could not allocate error context");
        xwm_destroy(xwm);
        return NULL;
    }
#endif

    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(xwm->xcb_conn));
    xwm->screen = screen_iterator.data;

    struct wl_event_loop *event_loop = wl_display_get_event_loop(xwayland->server->display);
    xwm->event_source =
        wl_event_loop_add_fd(event_loop, wm_fd, WL_EVENT_READABLE, x11_event_handler, xwm);
    wl_event_source_check(xwm->event_source);

    xwm_get_resources(xwm);
    xwm_get_visual_and_colormap(xwm);
    xwm_get_render_format(xwm);

    uint32_t values[] = {
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
            XCB_EVENT_MASK_PROPERTY_CHANGE,
    };
    xcb_change_window_attributes(xwm->xcb_conn, xwm->screen->root, XCB_CW_EVENT_MASK, values);

    xcb_composite_redirect_subwindows(xwm->xcb_conn, xwm->screen->root,
                                      XCB_COMPOSITE_REDIRECT_MANUAL);

    xcb_atom_t supported[] = {
        xwm->atoms[NET_WM_STATE].atom,
        xwm->atoms[NET_ACTIVE_WINDOW].atom,
        xwm->atoms[NET_WM_MOVERESIZE].atom,
        xwm->atoms[NET_WM_STATE_FOCUSED].atom,
        xwm->atoms[NET_WM_STATE_MODAL].atom,
        xwm->atoms[NET_WM_STATE_FULLSCREEN].atom,
        xwm->atoms[NET_WM_STATE_MAXIMIZED_VERT].atom,
        xwm->atoms[NET_WM_STATE_MAXIMIZED_HORZ].atom,
        xwm->atoms[NET_WM_STATE_HIDDEN].atom,
        xwm->atoms[NET_WM_STATE_STICKY].atom,
        xwm->atoms[NET_WM_STATE_SKIP_TASKBAR].atom,
        xwm->atoms[NET_WM_STATE_ABOVE].atom,
        xwm->atoms[NET_WM_STATE_BELOW].atom,
        xwm->atoms[NET_WM_STATE_DEMANDS_ATTENTION].atom,
        xwm->atoms[KDE_NET_WM_STATE_SKIP_SWITCHER].atom,
        xwm->atoms[NET_CLIENT_LIST].atom,
        xwm->atoms[NET_CLIENT_LIST_STACKING].atom,
    };
    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_SUPPORTED].atom, XCB_ATOM_ATOM, 32,
                        sizeof(supported) / sizeof(*supported), supported);

    if (xwm->xwayland->xwayland_server->options.terminate_delay > 0 &&
        xwm->xfixes_major_version >= 6) {
        xcb_xfixes_set_client_disconnect_mode(xwm->xcb_conn,
                                              XCB_XFIXES_CLIENT_DISCONNECT_FLAGS_TERMINATE);
    }

    xcb_flush(xwm->xcb_conn);

    xwm_set_net_active_window(xwm, XCB_WINDOW_NONE);

    xwm_selection_init(&xwm->clipboard_selection, xwm, xwm->atoms[CLIPBOARD].atom);
    xwm_selection_init(&xwm->primary_selection, xwm, xwm->atoms[PRIMARY].atom);
    xwm_selection_init(&xwm->dnd_selection, xwm, xwm->atoms[DND_SELECTION].atom);

    xwm->compositor_new_surface.notify = handle_compositor_new_surface;
    wl_signal_add(&xwayland->server->compositor->events.new_surface, &xwm->compositor_new_surface);
    xwm->compositor_destroy.notify = handle_compositor_destroy;
    wl_signal_add(&xwayland->server->compositor->events.destroy, &xwm->compositor_destroy);

    xwm->shell_v1_new_surface.notify = handle_shell_v1_new_surface;
    wl_signal_add(&xwayland->shell->events.new_surface, &xwm->shell_v1_new_surface);
    xwm->shell_v1_destroy.notify = handle_shell_v1_destroy;
    wl_signal_add(&xwayland->shell->events.destroy, &xwm->shell_v1_destroy);

    xwm->max_scale.notify = xwm_handle_max_scale;
    output_manager_add_max_scale_listener(&xwm->max_scale);
    xwm->active_workspace.notify = xwm_handle_active_workspace;
    workspace_manager_add_active_listener(&xwm->active_workspace);
    xwm->activate_view.notify = xwm_handle_activate_view;
    view_manager_add_activate_view_listener(&xwm->activate_view);

    xwm_create_wm_window(xwm);
    xwm_create_no_focus_window(xwm);
    xwm_create_guard_window(xwm);

    xcb_flush(xwm->xcb_conn);

    eavesdrops_manager_create(xwm);

    return xwm;
}

void xwayland_surface_restack(struct xwayland_surface *xsurface, struct xwayland_surface *sibling,
                              enum xcb_stack_mode_t mode)
{
    if (xsurface->override_redirect) {
        return;
    }

    // Not implementing XCB_STACK_MODE_TOP_IF | XCB_STACK_MODE_BOTTOM_IF | XCB_STACK_MODE_OPPOSITE.
    if (mode != XCB_STACK_MODE_ABOVE && mode != XCB_STACK_MODE_BELOW) {
        return;
    }

    struct xwm *xwm = xsurface->xwm;
    // X11 clients expect their override_redirect windows to stay on top.
    // Avoid interfering by restacking above the topmost managed surface.
    if (mode == XCB_STACK_MODE_ABOVE && !sibling && !wl_list_empty(&xwm->surfaces_in_stack_order)) {
        sibling = wl_container_of(xwm->surfaces_in_stack_order.prev, sibling, stack_link);
    }

    if (sibling == xsurface) {
        return;
    }

    uint32_t flags = XCB_CONFIG_WINDOW_STACK_MODE | XCB_CONFIG_WINDOW_SIBLING;
    uint32_t values[2];
    values[0] = sibling ? sibling->window_id : xwm->guard_window;
    values[1] = mode;
    xcb_configure_window(xwm->xcb_conn, xsurface->window_id, flags, values);
    xcb_flush(xwm->xcb_conn);

    wl_list_remove(&xsurface->stack_link);
    // remove from stack order
    if (!sibling && mode == XCB_STACK_MODE_BELOW) {
        wl_list_init(&xsurface->stack_link);
    } else {
        struct wl_list *node;
        if (mode == XCB_STACK_MODE_ABOVE) {
            node = sibling ? &sibling->stack_link : &xwm->surfaces_in_stack_order;
        } else {
            node = sibling->stack_link.prev;
        }
        wl_list_insert(node, &xsurface->stack_link);
    }
    xwm_schedule_net_client_list_stacking(xwm);
}

void xwayland_surface_set_minimized(struct xwayland_surface *surface, bool minimized)
{
    surface->minimized = minimized;
    xsurface_set_wm_state(surface);
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_maximized(struct xwayland_surface *surface, bool maximized)
{
    surface->maximized_horz = maximized;
    surface->maximized_vert = maximized;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_fullscreen(struct xwayland_surface *surface, bool fullscreen)
{
    surface->fullscreen = fullscreen;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_sticky(struct xwayland_surface *surface, bool sticky)
{
    surface->sticky = sticky;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_skip_taskbar(struct xwayland_surface *surface, bool skip_taskbar)
{
    surface->skip_taskbar = skip_taskbar;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_skip_switcher(struct xwayland_surface *surface, bool skip_switcher)
{
    surface->skip_switcher = skip_switcher;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_above(struct xwayland_surface *surface, bool above)
{
    surface->above = above;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_below(struct xwayland_surface *surface, bool below)
{
    surface->below = below;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_set_demands_attention(struct xwayland_surface *surface,
                                            bool demands_attention)
{
    surface->demands_attention = demands_attention;
    xsurface_set_net_wm_state(surface);
    xcb_flush(surface->xwm->xcb_conn);
}

void xwayland_surface_ping(struct xwayland_surface *surface)
{
    if (surface->pinging) {
        return;
    }

    // don't ping if client not supports
    bool supports_ping = false;
    for (size_t i = 0; i < surface->protocols_len; i++) {
        if (surface->protocols[i] == surface->xwm->atoms[NET_WM_PING].atom) {
            supports_ping = true;
            break;
        }
    }
    if (!supports_ping) {
        return;
    }

    xcb_client_message_data_t data = { 0 };
    data.data32[0] = surface->xwm->atoms[NET_WM_PING].atom;
    data.data32[1] = XCB_CURRENT_TIME;
    data.data32[2] = surface->window_id;

    xwm_send_wm_message(surface, &data, XCB_EVENT_MASK_NO_EVENT);

    wl_event_source_timer_update(surface->ping_timer, surface->xwm->ping_timeout);
    surface->pinging = true;
}

bool xwayland_surface_wants_focus(const struct xwayland_surface *xsurface)
{
    static const enum atom_name needles[] = {
        NET_WM_WINDOW_TYPE_COMBO,         NET_WM_WINDOW_TYPE_DND,
        NET_WM_WINDOW_TYPE_DROPDOWN_MENU, NET_WM_WINDOW_TYPE_MENU,
        NET_WM_WINDOW_TYPE_NOTIFICATION,  NET_WM_WINDOW_TYPE_POPUP_MENU,
        NET_WM_WINDOW_TYPE_SPLASH,        NET_WM_WINDOW_TYPE_TOOLTIP,
        NET_WM_WINDOW_TYPE_UTILITY,
    };

    for (size_t i = 0; i < sizeof(needles) / sizeof(needles[0]); ++i) {
        if (xwm_atoms_contains(xsurface->xwm, xsurface->window_type, xsurface->window_type_len,
                               needles[i])) {
            return false;
        }
    }

    return true;
}

enum xwayland_icccm_input_model
xwayland_surface_icccm_input_model(const struct xwayland_surface *xsurface)
{
    bool take_focus = xwm_atoms_contains(xsurface->xwm, xsurface->protocols,
                                         xsurface->protocols_len, WM_TAKE_FOCUS);

    if (!xsurface->hints || xsurface->hints->input) {
        if (take_focus) {
            return ICCCM_INPUT_MODEL_LOCAL;
        }
        return ICCCM_INPUT_MODEL_PASSIVE;
    } else {
        if (take_focus) {
            return ICCCM_INPUT_MODEL_GLOBAL;
        }
    }
    return ICCCM_INPUT_MODEL_NONE;
}

void xwm_set_workareas(struct xwm *xwm, const struct kywc_box *workareas, size_t num_workareas)
{
    uint32_t *data = malloc(4 * sizeof(uint32_t) * num_workareas);
    if (!data) {
        return;
    }

    for (size_t i = 0; i < num_workareas; i++) {
        data[4 * i] = workareas[i].x;
        data[4 * i + 1] = workareas[i].y;
        data[4 * i + 2] = workareas[i].width;
        data[4 * i + 3] = workareas[i].height;
    }

    xcb_change_property(xwm->xcb_conn, XCB_PROP_MODE_REPLACE, xwm->screen->root,
                        xwm->atoms[NET_WORKAREA].atom, XCB_ATOM_CARDINAL, 32, 4 * num_workareas,
                        data);
    free(data);
}

struct wlr_buffer *xwayland_surface_get_icon_buffer(struct xwayland_surface *xsurface, int size,
                                                    float scale)
{
    struct draw_info info = { .width = size, .height = size, .scale = scale };

    float scale_width = info.width * info.scale;
    float min_abs = FLT_MAX;
    float tmp_abs;
    struct xwayland_surface_icon *icon_similar = NULL;
    struct xwayland_surface_icon *icon;
    wl_list_for_each(icon, &xsurface->icons, link) {
        tmp_abs = fabs(icon->width - scale_width);
        if (tmp_abs < min_abs) {
            min_abs = tmp_abs;
            icon_similar = icon;
        }
    }
    if (!icon_similar) {
        return NULL;
    }

    if (icon_similar->buffer) {
        return icon_similar->buffer;
    }

    info.pixel.width = icon_similar->width;
    info.pixel.height = icon_similar->height;
    info.pixel.data = icon_similar->data;
    icon_similar->buffer = painter_draw_buffer(&info);

    return icon_similar->buffer;
}

bool xwayland_surface_has_type(struct xwayland_surface *xsurface, int type)
{
    struct xwm *xwm = xsurface->xwm;
    for (size_t i = 0; i < xsurface->window_type_len; ++i) {
        xcb_atom_t atom = xsurface->window_type[i];
        if (atom == xwm->atoms[type].atom) {
            return true;
        }
    }
    return false;
}

bool xwayland_surface_has_input(struct xwayland_surface *xsurface, uint32_t input)
{
    struct xwm *xwm = xsurface->xwm;
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(
        xwm->xcb_conn, xcb_get_window_attributes_unchecked(xwm->xcb_conn, xsurface->window_id),
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

void xwayland_surface_debug_type(struct xwayland_surface *xsurface)
{
    for (size_t i = 0; i < xsurface->window_type_len; ++i) {
        xcb_atom_t atom = xsurface->window_type[i];
        char *atom_name = xwm_get_atom_name(xsurface->xwm, atom);
        kywc_log(KYWC_INFO, "%s: type atom %s %ld(%ld)", xsurface->class, atom_name, i,
                 xsurface->window_type_len);
        free(atom_name);
    }
    kywc_log(KYWC_INFO, "%s: OR %d size %d x %d %d", xsurface->class, xsurface->override_redirect,
             xsurface->width, xsurface->height, xsurface->fullscreen);
}
