// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pango/pangocairo.h>

#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/util/log.h>

#include <kywc/log.h>

#include "backend/backend.h"
#include "config.h"
#include "effect/action.h"
#include "effect/effect.h"
#include "input/function.h"
#include "input/input.h"
#include "output.h"
#include "plugin.h"
#include "render/allocator.h"
#include "render/pixman.h"
#include "render/renderer.h"
#include "security.h"
#include "server.h"
#include "theme.h"
#include "util/dbus.h"
#include "util/sysfs.h"
#include "view/view.h"
#include "xwayland.h"

static const char *service = "org.freedesktop.login1";
static const char *path = "/org/freedesktop/login1";
static const char *interface = "org.freedesktop.login1.Manager";

static inline void server_drop_inhibitor(struct server *server)
{
    if (server->inhibitor >= 0) {
        close(server->inhibitor);
        server->inhibitor = -1;
    }
}

static int server_set_inhibitor(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct server *server = userdata;
    server_drop_inhibitor(server);

    int ret = sd_bus_message_read(msg, "h", &server->inhibitor);
    if (ret < 0) {
        kywc_log(KYWC_WARN, "Failed to get inhibitor: %s", strerror(-ret));
    }
    server->inhibitor = fcntl(server->inhibitor, F_DUPFD_CLOEXEC, 3);
    return 0;
}

static void server_create_inhibitor(struct server *server)
{
    if (!server->need_inhibit || server->inhibitor >= 0) {
        return;
    }

    dbus_call_system_methodv(service, path, interface, "Inhibit", server_set_inhibitor, server,
                             "ssss", "sleep", "kylin-wlcom", "Power off outputs when hibernate",
                             "delay");
}

static int handle_prepare_for_sleep(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct server *server = userdata;
    /* "b" apparently reads into an int, not a bool */
    int going_down = 1;
    int ret = sd_bus_message_read(msg, "b", &going_down);
    if (ret < 0) {
        kywc_log(KYWC_WARN, "Failed to parse D-Bus response for Sleep: %s", strerror(-ret));
        return 0;
    }
    if (!going_down) {
        server_create_inhibitor(server);
        wl_signal_emit_mutable(&server->events.resume, NULL);
    } else if (!server->need_inhibit) {
        wl_signal_emit_mutable(&server->events.suspend, NULL);
    }
    return 0;
}

static int handle_prepare_for_suspend(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct server *server = userdata;
    /* "b" apparently reads into an int, not a bool */
    int going_down = 1;
    int ret = sd_bus_message_read(msg, "b", &going_down);
    if (ret < 0) {
        kywc_log(KYWC_WARN, "Failed to parse D-Bus response for Suspend: %s", strerror(-ret));
        return 0;
    }
    if (going_down) {
        wl_signal_emit_mutable(&server->events.suspend, NULL);
        /* close the fd to drop inhibit lock */
        server_drop_inhibitor(server);
    }
    return 0;
}

static int handle_prepare_for_hibernate(sd_bus_message *msg, void *userdata,
                                        sd_bus_error *ret_error)
{
    struct server *server = userdata;
    /* "b" apparently reads into an int, not a bool */
    int going_down = 1;
    int ret = sd_bus_message_read(msg, "b", &going_down);
    if (ret < 0) {
        kywc_log(KYWC_WARN, "Failed to parse D-Bus response for Hibernate: %s", strerror(-ret));
        return 0;
    }
    if (going_down) {
        wl_signal_emit_mutable(&server->events.hibernate, NULL);
        /* close the fd to drop inhibit lock */
        server_drop_inhibitor(server);
    }
    return 0;
}

static void server_init_power(struct server *server)
{
    server->inhibitor = -1;

    sd_bus *bus = dbus_context_get_bus(true);
    if (!bus) {
        return;
    }

    sd_bus_message *reply = NULL;
    int r = sd_bus_call_method(bus, service, path, "org.freedesktop.DBus.Introspectable",
                               "Introspect", NULL, &reply, NULL, NULL);
    if (r < 0) {
        kywc_log(KYWC_ERROR, "Failed to Introspect %s", service);
    } else {
        const char *xml = NULL;
        r = sd_bus_message_read(reply, "s", &xml);
        server->need_inhibit =
            r >= 0 && xml &&
            (strstr(xml, "PrepareForSuspend") || strstr(xml, "PrepareForHibernate"));
        sd_bus_message_unref(reply);
    }

    dbus_match_system_signal(service, path, interface, "PrepareForSleep", handle_prepare_for_sleep,
                             server);

    if (server->need_inhibit) {
        kywc_log(KYWC_INFO, "Create inhibitor for hibernate");
        server_create_inhibitor(server);
        dbus_match_system_signal(service, path, interface, "PrepareForSuspend",
                                 handle_prepare_for_suspend, server);
        dbus_match_system_signal(service, path, interface, "PrepareForHibernate",
                                 handle_prepare_for_hibernate, server);
    }
}

void server_add_destroy_listener(struct server *server, struct wl_listener *listener)
{
    struct wl_signal *signal = &server->events.destroy;
    wl_list_insert(&signal->listener_list, &listener->link);
}

struct wlr_renderer *server_get_pixman_renderer(struct server *server)
{
    if (wlr_renderer_is_pixman_renderer(server->renderer)) {
        return server->renderer;
    }

    if (!server->pixman_renderer) {
        server->pixman_renderer = pixman_renderer_create();
    }
    return server->pixman_renderer;
}

static void kywc_log_callback(enum wlr_log_importance verbosity, const char *fmt, va_list args)
{
    /* switch wlr_log_importance to kywc_log_level */
    enum kywc_log_level level = KYWC_WARN;
    switch (verbosity) {
    case WLR_SILENT:
        level = KYWC_SILENT;
        break;
    case WLR_ERROR:
        level = KYWC_ERROR;
        break;
    case WLR_INFO:
        level = KYWC_INFO;
        break;
    case WLR_DEBUG:
        level = KYWC_DEBUG;
        break;
    default:
        break;
    }
    kywc_vlog(level, fmt, args);
}

static void kywc_log_callback_dumb(enum wlr_log_importance verbosity, const char *fmt,
                                   va_list args) {};

static void server_get_active_vt(struct server *server)
{
    char buf[64] = { 0 };
    size_t len = sysfs_read_data("/sys/class/tty/tty0/active", buf, 63);
    if (len > 0) {
        sscanf(buf, "tty%u", &server->vtnr);
        kywc_log(KYWC_INFO, "Current VT is %u", server->vtnr);
    } else {
        server->vtnr = 0;
    }
}

static void handle_session_active(struct wl_listener *listener, void *data)
{
    struct server *server = wl_container_of(listener, server, session_active);
    struct wlr_session *session = server->session;
    kywc_log(KYWC_INFO, "Session becomes %sactive", session->active ? "" : "in");
    server->active = session->active;
    wl_signal_emit_mutable(&server->events.active, NULL);
}

static bool wlroots_server_init(struct server *server)
{
    /* verbosity is not used when we replaced log_callback */
    wlr_log_init(WLR_DEBUG, kywc_log_callback);

    server->backend = backend_autocreate(server->display, &server->session);
    if (!server->backend) {
        kywc_log(KYWC_FATAL, "Unable to create backend");
        return false;
    }

    if (server->session) {
        server->active = server->session->active;
        server->session_active.notify = handle_session_active;
        wl_signal_add(&server->session->events.active, &server->session_active);
        server_get_active_vt(server);
    } else {
        // mark active if in nested backend
        server->active = true;
    }

    server->renderer = renderer_autocreate(server->backend);
    if (!server->renderer) {
        kywc_log(KYWC_FATAL, "Unable to create renderer");
        return false;
    }

    server->allocator = allocator_create(server->backend, server->renderer);
    if (!server->allocator) {
        kywc_log(KYWC_FATAL, "Unable to create allocator");
        return false;
    }

    // TODO: set renderer to NULL, drop wlr_client_buffer
    server->compositor = wlr_compositor_create(server->display, 6, server->renderer);
    wlr_subcompositor_create(server->display);

    renderer_init_wl_display(server->renderer, server->backend, server->display,
                             &server->linux_dmabuf_v1);

    server->layout = wlr_output_layout_create();
    server->scene = scene_create();
    if (server->linux_dmabuf_v1) {
        scene_set_linux_dmabuf_v1(server->scene, server->linux_dmabuf_v1);
    }

    struct wlr_tearing_control_manager_v1 *tearing_control_v1 =
        wlr_tearing_control_manager_v1_create(server->display, 1);
    if (tearing_control_v1) {
        scene_set_tearing_control_v1(server->scene, tearing_control_v1);
    }

    struct wlr_presentation *presentation =
        wlr_presentation_create(server->display, server->backend);
    if (presentation) {
        scene_set_presentation(server->scene, presentation);
    }

    wlr_export_dmabuf_manager_v1_create(server->display);
    wlr_viewporter_create(server->display);
    wlr_fractional_scale_manager_v1_create(server->display, 1);
    alpha_modifier_v1_create(server->display);

    return true;
}

static int handle_exit(int signal, void *data)
{
    kywc_log(KYWC_SILENT, "Received signal %s(%u), exit...", strsignal(signal), signal);
    struct server *server = data;
    if (!server->ready) {
        exit(EXIT_SUCCESS);
    } else if (!server->terminate) {
        wl_display_terminate(server->display);
    }
    return 0;
}

bool server_init(struct server *server)
{
    server->display = wl_display_create();
    server->event_loop = wl_display_get_event_loop(server->display);
    wl_display_set_default_max_buffer_size(server->display, 1024 * 1024);

    wl_signal_init(&server->events.ready);
    wl_signal_init(&server->events.start);
    wl_signal_init(&server->events.terminate);
    wl_signal_init(&server->events.destroy);
    wl_signal_init(&server->events.suspend);
    wl_signal_init(&server->events.hibernate);
    wl_signal_init(&server->events.resume);
    wl_signal_init(&server->events.active);

    /* use wl event source to sync these signals in multi-thread */
    server->sources.sighup =
        wl_event_loop_add_signal(server->event_loop, SIGHUP, handle_exit, server);
    server->sources.sigterm =
        wl_event_loop_add_signal(server->event_loop, SIGTERM, handle_exit, server);

    dbus_context_create(server);
    server_init_power(server);

    server->queue = queue_create(64, 4, server);

    config_manager_create(server);
    security_manager_create(server);
    theme_manager_create(server);

    if (!wlroots_server_init(server)) {
        return false;
    }

    function_manager_init();
    output_manager_create(server);
    input_manager_create(server);
    view_manager_create(server);
    xwayland_server_create(server);

    action_effect_manager_create(server);
    effect_manager_create(server);
    plugin_manager_create(server);

    server->ready = true;
    wl_signal_emit_mutable(&server->events.ready, NULL);

    return true;
}

bool server_start(struct server *server)
{
    /* Add a Unix socket to the Wayland display. */
    const char *socket = wl_display_add_socket_auto(server->display);
    if (!socket) {
        kywc_log_errno(KYWC_FATAL, "unable to open wayland socket");
        return false;
    }

    if (setenv("WAYLAND_DISPLAY", socket, true) < 0) {
        kywc_log_errno(KYWC_ERROR, "unable to set WAYLAND_DISPLAY");
    } else {
        kywc_log(KYWC_DEBUG, "WAYLAND_DISPLAY = %s", socket);
    }

    if (!wlr_backend_start(server->backend)) {
        kywc_log(KYWC_FATAL, "Unable to start the wlroots backend");
        return false;
    }

    server->start = true;
    wl_signal_emit_mutable(&server->events.start, NULL);

    return true;
}

void server_run(struct server *server)
{
    kywc_log(KYWC_INFO, "Running wayland compositor on wayland display '%s'",
             getenv("WAYLAND_DISPLAY"));
    wl_display_run(server->display);
}

void server_finish(struct server *server)
{
    server->terminate = true;
    /* suppress all messages from wlroots */
    wlr_log_init(WLR_DEBUG, kywc_log_callback_dumb);

    wl_event_source_remove(server->sources.sighup);
    wl_event_source_remove(server->sources.sigterm);

    wl_signal_emit_mutable(&server->events.terminate, NULL);

    queue_destroy(server->queue);

    wl_display_destroy_clients(server->display);
    /* make sure all xwayland-shells are destroyed */
    xwayland_server_destroy();

    wl_display_destroy(server->display);

    /* call all server_destroy listeners */
    wl_signal_emit_mutable(&server->events.destroy, NULL);

    assert(wl_list_empty(&server->events.ready.listener_list));
    assert(wl_list_empty(&server->events.start.listener_list));
    assert(wl_list_empty(&server->events.terminate.listener_list));
    assert(wl_list_empty(&server->events.destroy.listener_list));
    assert(wl_list_empty(&server->events.suspend.listener_list));
    assert(wl_list_empty(&server->events.hibernate.listener_list));
    assert(wl_list_empty(&server->events.resume.listener_list));
    assert(wl_list_empty(&server->events.active.listener_list));

    /* scene may be NULL when server_init failed */
    if (server->scene) {
        scene_node_destroy(&server->scene->tree.node);
    }
    wlr_output_layout_destroy(server->layout);
    wlr_allocator_destroy(server->allocator);
    wlr_renderer_destroy(server->pixman_renderer);
    wlr_renderer_destroy(server->renderer);
    /* free memory in fontconfig */
    pango_cairo_font_map_set_default(NULL);

    kywc_log(KYWC_SILENT, "Kylin-wlcom finished...\n");
}
