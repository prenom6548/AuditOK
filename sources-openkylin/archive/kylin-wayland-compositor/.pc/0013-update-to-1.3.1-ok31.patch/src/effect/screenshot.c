// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "effect/capture.h"
#include "effect_p.h"
#include "util/dbus.h"

static const char *registry_bus = "org.ukui.KWin";
static const char *registry_path = "/Screenshot";
static const char *registry_interface = "org.ukui.kwin.Screenshot";

struct screenshot {
    struct capture *capture;
    sd_bus_message *msg;

    pid_t pid;
    uid_t uid;
    gid_t gid;
};

static void screnshot_get_credentials(struct screenshot *screenshot, sd_bus_message *call)
{
    sd_bus_creds *creds = NULL;
    uint64_t mask = SD_BUS_CREDS_AUGMENT | SD_BUS_CREDS_PID | SD_BUS_CREDS_UID | SD_BUS_CREDS_GID;
    int r = sd_bus_query_sender_creds(call, mask, &creds);
    if (r < 0) {
        return;
    }

    sd_bus_creds_get_pid(creds, &screenshot->pid);
    sd_bus_creds_get_uid(creds, &screenshot->uid);
    sd_bus_creds_get_gid(creds, &screenshot->gid);

    sd_bus_creds_unref(creds);
}

static void screenshot_finish(const char *path, void *data)
{
    sd_bus_message *msg = data;

    if (path) {
        sd_bus_reply_method_return(msg, "s", path);
        kywc_log(KYWC_DEBUG, "Screenshot done, send reply %s", path);
    } else {
        const sd_bus_error error = SD_BUS_ERROR_MAKE_CONST(
            "org.ukui.kwin.Screenshot.Error.Cancelled", "Screenshot got cancelled");
        sd_bus_reply_method_error(msg, &error);
    }

    sd_bus_message_unref(msg);
}

static void screenshot_handle_capture_update(struct capture *capture, struct wlr_buffer *buffer,
                                             bool buffer_changed, void *data)
{
    char path[32];
    snprintf(path, sizeof(path), "/tmp/%s", "kywc_screenshot_XXXXXX.bmp");
    kywc_identifier_rand_generate(path, 4);

    struct screenshot *screenshot = data;
    capture_write_file(buffer, path, screenshot_finish, screenshot->msg);

    capture_destroy(capture);
    free(screenshot);
}

static void screenshot_handle_capture_cancel(struct capture *capture, void *data)
{
    struct screenshot *screenshot = data;
    /* capture failed, send a error to client */
    screenshot_finish(NULL, screenshot->msg);
    free(screenshot);
}

static void sceenshot_handle_capture_client(struct capture *capture, struct client *client,
                                            void *data)
{
    struct screenshot *screenshot = data;
    *client = (struct client){
        .type = CLIENT_TYPE_DBUS,
        .pid = screenshot->pid,
        .uid = screenshot->uid,
        .gid = screenshot->gid,
    };
    client_set_comm(client);
}

static const struct capture_interface screenshot_impl = {
    .update = screenshot_handle_capture_update,
    .cancel = screenshot_handle_capture_cancel,
    .client = sceenshot_handle_capture_client,
};

static int screenshot_fullscreen(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct screenshot *screenshot = calloc(1, sizeof(*screenshot));
    if (!screenshot) {
        return 0;
    }

    screnshot_get_credentials(screenshot, msg);

    screenshot->capture = capture_create_from_fullscreen(
        CAPTURE_NEED_UNSCALED | CAPTURE_NEED_PRIVATE_BUFFER, &screenshot_impl, screenshot);
    if (!screenshot->capture) {
        free(screenshot);
        return 0;
    }

    screenshot->msg = sd_bus_message_ref(msg);

    return 1;
}

static int screenshot2_fullscreen(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    uint32_t cursor = 0;
    CK(sd_bus_message_read(msg, "b", &cursor));

    uint32_t options = CAPTURE_NEED_UNSCALED | CAPTURE_NEED_PRIVATE_BUFFER;
    options |= cursor ? CAPTURE_NEED_CURSOR : CAPTURE_NEED_NONE;

    struct screenshot *screenshot = calloc(1, sizeof(*screenshot));
    if (!screenshot) {
        return 0;
    }

    screnshot_get_credentials(screenshot, msg);

    screenshot->capture = capture_create_from_fullscreen(options, &screenshot_impl, screenshot);
    if (!screenshot->capture) {
        free(screenshot);
        return 0;
    }

    screenshot->msg = sd_bus_message_ref(msg);

    return 1;
}

static int screenshot_full(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    uint32_t unscaled, cursor;
    CK(sd_bus_message_read(msg, "bb", &unscaled, &cursor));

    uint32_t options = unscaled ? CAPTURE_NEED_UNSCALED : CAPTURE_NEED_NONE;
    options |= cursor ? CAPTURE_NEED_CURSOR : CAPTURE_NEED_NONE;
    options |= CAPTURE_NEED_PRIVATE_BUFFER;

    struct screenshot *screenshot = calloc(1, sizeof(*screenshot));
    if (!screenshot) {
        return 0;
    }

    screnshot_get_credentials(screenshot, msg);

    screenshot->capture = capture_create_from_fullscreen(options, &screenshot_impl, screenshot);
    if (!screenshot->capture) {
        free(screenshot);
        return 0;
    }

    screenshot->msg = sd_bus_message_ref(msg);

    return 1;
}

static int screenshot_output(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    const char *name = NULL;
    uint32_t unscaled, cursor;
    CK(sd_bus_message_read(msg, "sbb", &name, &unscaled, &cursor));

    struct kywc_output *output = kywc_output_by_name(name);
    if (!output) {
        const sd_bus_error error = SD_BUS_ERROR_MAKE_CONST(
            "org.ukui.kwin.Screenshot.Error.InvalidOutput", "Invalid output requested");
        return sd_bus_reply_method_error(msg, &error);
    }

    uint32_t options = unscaled ? CAPTURE_NEED_UNSCALED : CAPTURE_NEED_NONE;
    options |= cursor ? CAPTURE_NEED_CURSOR : CAPTURE_NEED_NONE;
    options |= CAPTURE_NEED_PRIVATE_BUFFER;

    struct screenshot *screenshot = calloc(1, sizeof(*screenshot));
    if (!screenshot) {
        return 0;
    }

    screnshot_get_credentials(screenshot, msg);

    screenshot->capture = capture_create_from_output(output_from_kywc_output(output), options,
                                                     &screenshot_impl, screenshot);
    if (!screenshot->capture) {
        free(screenshot);
        return 0;
    }

    screenshot->msg = sd_bus_message_ref(msg);

    return 1;
}

static int screenshot_area(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    int x, y, width, height;
    uint32_t unscaled, cursor;
    CK(sd_bus_message_read(msg, "iiiibb", &x, &y, &width, &height, &unscaled, &cursor));

    uint32_t options = unscaled ? CAPTURE_NEED_UNSCALED : CAPTURE_NEED_NONE;
    options |= cursor ? CAPTURE_NEED_CURSOR : CAPTURE_NEED_NONE;
    options |= CAPTURE_NEED_PRIVATE_BUFFER;

    struct screenshot *screenshot = calloc(1, sizeof(*screenshot));
    if (!screenshot) {
        return 0;
    }

    screnshot_get_credentials(screenshot, msg);

    screenshot->capture = capture_create_from_area(&(struct kywc_box){ x, y, width, height },
                                                   options, &screenshot_impl, screenshot);
    if (!screenshot->capture) {
        free(screenshot);
        return 0;
    }

    screenshot->msg = sd_bus_message_ref(msg);

    return 1;
}

/**
 * sd-bus not support method overloaded, https://github.com/systemd/systemd/issues/578
 * Add org.kde.KWin screenshotFullscreen with a bool arg for linuxqq,
 * keep org.ukui.KWin screenshotFullscreen without args for wechat.
 * kylin-screenshot should use screenshotFull.
 */
static const sd_bus_vtable screenshot_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("screenshotFullscreen", "", "s", screenshot_fullscreen, 0),
    SD_BUS_METHOD("screenshotFull", "bb", "s", screenshot_full, 0),
    SD_BUS_METHOD("screenshotOutput", "sbb", "s", screenshot_output, 0),
    SD_BUS_METHOD("screenshotArea", "iiiibb", "s", screenshot_area, 0),
    SD_BUS_VTABLE_END,
};

static const sd_bus_vtable screenshot2_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("screenshotFullscreen", "b", "s", screenshot2_fullscreen, 0),
    SD_BUS_VTABLE_END,
};

bool screenshot_effect_create(struct effect_manager *effect_manager)
{
    if (!dbus_register_object(registry_bus, registry_path, registry_interface, screenshot_vtable,
                              NULL)) {
        return false;
    }

    dbus_register_object("org.kde.KWin", registry_path, "org.kde.kwin.Screenshot",
                         screenshot2_vtable, NULL);

    return true;
}
