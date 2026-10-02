/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <xf86drm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <xcb/xcb.h>
#include <xcb/dri2.h>


#include <wayland-client.h>

#include "drm-client-protocol.h"

static struct wl_display *awl_display = NULL;
static struct wl_registry *awl_registry = NULL;
static struct wl_drm *awl_drm = NULL;
static struct wl_event_queue *awl_queue = NULL;

static void registry_add_object (void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void) data;
    (void) version;
    if (!strcmp(interface, "wl_drm")) {
        awl_drm = wl_registry_bind(registry, name, &wl_drm_interface, 1);
    }
}

static void registry_remove_object (void *data, struct wl_registry *registry, uint32_t name)
{
    (void) data;
    (void) registry;
    (void) name;
}

static struct wl_registry_listener registry_listener = {
    &registry_add_object,
    &registry_remove_object,
};

static int authenticate_magic_wl(drm_magic_t magic)
{
    int ret = 0;
    int count = 0;
    int try_times = 5;

    awl_display = wl_display_connect(NULL);
    if (!awl_display) {
        return -1;
    }

    awl_queue = wl_display_create_queue(awl_display);
    if (!awl_queue) {
        ret = -1;
        goto out;
    }

    awl_registry = wl_display_get_registry(awl_display);
    if (!awl_registry) {
        ret = -1;
        goto out;
    }

    wl_proxy_set_queue((struct wl_proxy*)awl_registry, awl_queue);

    wl_registry_add_listener(awl_registry, &registry_listener, NULL);

    wl_display_roundtrip_queue(awl_display, awl_queue);

    while (count++ < try_times) {
        if (awl_drm) {
            break;
        }
        wl_display_roundtrip_queue(awl_display, awl_queue);
    }

    if (!awl_drm) {
        ret = -1;
        goto out;
    }

    wl_proxy_set_queue((struct wl_proxy*)awl_drm, awl_queue);

    wl_drm_authenticate(awl_drm, magic);

    wl_display_roundtrip_queue(awl_display, awl_queue);
    wl_display_flush(awl_display);
    wl_display_roundtrip_queue(awl_display, awl_queue);

out:
    if (awl_drm) {
        wl_drm_destroy(awl_drm);
        awl_drm = NULL;
    }

    if (awl_registry) {
        wl_registry_destroy(awl_registry);
        awl_registry = NULL;
    }

    if (awl_queue) {
        wl_event_queue_destroy(awl_queue);
        awl_queue = NULL;
    }

    if (awl_display) {
        wl_display_disconnect(awl_display);
        awl_display = NULL;
    }

    return ret;
}


static int authenticate_magic_xcb(drm_magic_t magic)
{
    xcb_connection_t *conn = xcb_connect(NULL, NULL);
    unsigned int authenticated = 0;

    if (!conn) {
        return -1;
    }

    if (xcb_connection_has_error(conn)) {
        xcb_disconnect(conn);
        return -1;
    }

    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    xcb_window_t window = screen->root;

    /* Authenticate our client via the X server using the magic. */
    xcb_dri2_authenticate_cookie_t auth_cookie = xcb_dri2_authenticate(conn, window, magic);
    xcb_flush(conn);

    xcb_dri2_authenticate_reply_t *auth_reply = xcb_dri2_authenticate_reply(conn, auth_cookie, NULL);
    if (auth_reply) {
        authenticated = auth_reply->authenticated;
        free(auth_reply);
    }

    xcb_flush(conn);
    xcb_disconnect(conn);

    if (!authenticated) {
        return -1;
    }

    return 0;
}

int authenticate_magic(drm_magic_t magic)
{
    if (magic == 0) {
        return -1;
    }
    if (authenticate_magic_wl(magic) == 0) {
        return 0;
    }
    return authenticate_magic_xcb(magic);
}
