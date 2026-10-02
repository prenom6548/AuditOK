// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <getopt.h>
#include <libintl.h>
#include <locale.h>
#include <stdlib.h>

#include "locales.h"
#include "portal.h"
#include "util/logger.h"
#include "util/macros.h"

static const char service_name[] = "org.freedesktop.impl.portal.desktop.wlcom";

static const struct option long_options[] = {
    { "help", no_argument, NULL, 'h' },    { "debug", no_argument, NULL, 'd' },
    { "version", no_argument, NULL, 'v' }, { "replace", no_argument, NULL, 'r' },
    { "verbose", no_argument, NULL, 'V' }, { 0, 0, 0, 0 },
};

static const char usage[] =
    "Usage: xdg-desktop-portal-wlcom [options]\n"
    "\n"
    "  -h, --help               Show help message and quit.\n"
    "  -d, --debug              Enables full logging, including debug information.\n"
    "  -v, --version            Show the version number and quit.\n"
    "  -r, --replace            Replace a running instance.\n"
    "  -V, --verbose            Enables more verbose logging.\n"
    "\n";

static int handle_name_lost(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    kywc_log(KYWC_INFO, "DBus: lost name, closing connection");
    sd_bus_close(sd_bus_message_get_bus(m));
    return 1;
}

static int handle_dbus_event(uint32_t mask, void *data)
{
    if (mask & (EVENT_HANGUP | EVENT_ERROR)) {
        kywc_log(KYWC_FATAL, "xdg-desktop-portal dbus eventfd failed");
        return -1;
    }

    struct sd_bus *bus = data;
    int ret = 0;

    do {
        ret = sd_bus_process(bus, NULL);
    } while (ret > 0);

    if (ret < 0) {
        kywc_log(KYWC_ERROR, "sd_bus_process failed: %s", strerror(-ret));
        return -1;
    }

    return 0;
}

static bool portal_init_dbus(struct portal_context *ctx, bool enable_replace)
{
    int ret = sd_bus_open_user(&ctx->bus);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "DBus: failed to connect to user bus: %s", strerror(-ret));
        return false;
    }
    kywc_log(KYWC_DEBUG, "DBus: connected to session bus");

    uint64_t flags = SD_BUS_NAME_ALLOW_REPLACEMENT;
    if (enable_replace) {
        flags |= SD_BUS_NAME_REPLACE_EXISTING;
    }
    ret = sd_bus_request_name(ctx->bus, service_name, flags);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "DBus: failed to acquire service name: %s", strerror(-ret));
        return false;
    }

    const char *unique_name;
    ret = sd_bus_get_unique_name(ctx->bus, &unique_name);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "DBus: failed to get unique bus name: %s", strerror(-ret));
        return false;
    }

    static char match[1024];
    snprintf(match, sizeof(match),
             "sender='org.freedesktop.DBus',"
             "type='signal',"
             "interface='org.freedesktop.DBus',"
             "member='NameOwnerChanged',"
             "path='/org/freedesktop/DBus',"
             "arg0='%s',"
             "arg1='%s'",
             service_name, unique_name);

    ret = sd_bus_add_match(ctx->bus, NULL, match, handle_name_lost, NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "DBus: failed to add NameOwnerChanged signal match: %s",
                 strerror(-ret));
        return false;
    }

    int bus_fd = sd_bus_get_fd(ctx->bus);
    if (event_loop_add_source(ctx->event_loop, bus_fd, handle_dbus_event, ctx->bus) < 0) {
        kywc_log(KYWC_ERROR, "DBus: add to event loop failed");
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

#if HAVE_NLS
    nls_setup_message("xdg-desktop-portal-wlcom", locales, ARRAY_SIZE(locales));
#endif

    bool enable_debug = false;
    bool enable_verbose = false;
    bool enable_replace = false;

    while (1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "hdvrV", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'h':
            printf("%s", usage);
            exit(EXIT_SUCCESS);
            break;
        case 'd':
            enable_debug = true;
            break;
        case 'r':
            enable_replace = true;
            break;
        case 'v': // version
            printf("xdg-desktop-portal-wlcom version " KYWC_VERSION "\n");
            exit(EXIT_SUCCESS);
            break;
        case 'V': // verbose
            enable_verbose = true;
            break;
        default:
            fprintf(stderr, "%s", usage);
            exit(EXIT_FAILURE);
        }
    }

    enum kywc_log_level level = KYWC_WARN;
    if (enable_debug) {
        level = KYWC_DEBUG;
    } else if (enable_verbose) {
        level = KYWC_INFO;
    }
    // log to stdout and in realtime
    logger_init(level, NULL, true);
    kywc_log(KYWC_SILENT, "xdg-desktop-portal-wlcom %s starting...", KYWC_VERSION);

    struct portal_context ctx = { 0 };
    int exit_value = EXIT_SUCCESS;

    ctx.event_loop = event_loop_create();
    if (!ctx.event_loop) {
        kywc_log(KYWC_FATAL, "Event loop: failed to create event loop");
        goto shutdown;
    }

    if (!portal_init_dbus(&ctx, enable_replace)) {
        kywc_log(KYWC_FATAL, "DBus: failed to init it");
        goto shutdown;
    }

    if (!portal_wayland_init(&ctx)) {
        kywc_log(KYWC_FATAL, "Wayland: failed to init it");
        goto shutdown;
    }

    if (!portal_remote_desktop_init(&ctx)) {
        kywc_log(KYWC_FATAL, "RemoteDesktop: failed to init it");
        goto shutdown;
    }

    if (!portal_input_capture_init(&ctx)) {
        kywc_log(KYWC_FATAL, "InputCapture: failed to init it");
        goto shutdown;
    }

    if (!portal_access_init(&ctx)) {
        kywc_log(KYWC_FATAL, "Access: failed to init it");
        goto shutdown;
    }

    if (!portal_screenshot_init(&ctx)) {
        kywc_log(KYWC_FATAL, "Screenshot: failed to init it");
        goto shutdown;
    }

    if (!portal_screencast_init(&ctx)) {
        kywc_log(KYWC_FATAL, "ScreenCast: failed to init it");
        goto shutdown;
    }

    kywc_log(KYWC_INFO, "start main event loop...");
    handle_dbus_event(0, ctx.bus);

    while (1) {
        if (event_loop_dispatch(ctx.event_loop, 1000) != 0) {
            exit_value = EXIT_FAILURE;
            break;
        }
    }

shutdown:
    portal_screencast_fini(&ctx);
    portal_screenshot_fini(&ctx);
    portal_access_fini(&ctx);
    portal_input_capture_fini(&ctx);
    portal_remote_desktop_fini(&ctx);
    portal_wayland_fini(&ctx);
    sd_bus_unref(ctx.bus);
    event_loop_destroy(ctx.event_loop);

    kywc_log(KYWC_SILENT, "xdg-desktop-portal-wlcom finished...\n");
    logger_finish();

    return exit_value;
}
