// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <unistd.h>

#include "buffer.h"
#include "portal.h"
#include "util/dialog.h"
#include "util/event.h"
#include "util/nls.h"
#include "util/spawn.h"
#include "util/string.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.Screenshot";

enum CAPTURE_TYPE {
    CAPTURE_TYPE_NONE,
    CAPTURE_TYPE_OUTPUT,
    CAPTURE_TYPE_TOPLEVEL,
};

struct permission_dialog_data {
    struct portal_request *request;
    sd_bus_message *msg;
    int socket_fd;
    struct event_context *event_loop;

    int result_value;
    int capture_type;
    char *uuid;
    bool without_decoration;
};

static bool thumbnail_handle_buffer(kywc_thumbnail *thumbnail,
                                    const struct kywc_thumbnail_buffer *buffer, void *data)
{
    struct portal_request *request = data;
    struct portal_context *ctx = request->ctx;
    struct permission_dialog_data *dialog_data = request->data;

    if (!ctx->wayland.helper || !dialog_data) {
        portal_request_destroy(request);
        return false;
    }

    sd_bus_message *msg = dialog_data->msg;

    char path[] = "/tmp/screenshot-XXXXXX.bmp";
    char uri[sizeof(path) + 7];
    bool ok = false;

    int fd = mkstemps(path, 4);
    if (fd < 0) {
        kywc_log(KYWC_ERROR, "mkstemps failed");
        goto reply;
    }
    close(fd);
    snprintf(uri, sizeof(uri), "file://%s", path);

    struct kywc_buffer *kywc_buffer =
        kywc_buffer_helper_import_thumbnail(ctx->wayland.helper, thumbnail, buffer);
    if (!kywc_buffer) {
        goto reply;
    }

    kywc_log(KYWC_DEBUG, "Write thumbnail buffer to %s", path);
    kywc_thumbnail_set_user_data(thumbnail, kywc_buffer);
    ok = kywc_buffer_write_to_file(kywc_buffer, 0, 0, buffer->width, buffer->height, path);

reply:
    kywc_log(KYWC_DEBUG, "Reply screenshot request %d", ok);
    if (ok) {
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 1, "uri", "s", uri);
    } else {
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
    }
    sd_bus_message_unref(msg);
    dialog_data->msg = NULL;
    portal_request_destroy(request);

    return false;
}

static void thumbnail_handle_destroy(kywc_thumbnail *thumbnail, void *data)
{
    struct kywc_buffer *kywc_buffer = kywc_thumbnail_get_user_data(thumbnail);
    kywc_buffer_destroy(kywc_buffer);
}

static struct kywc_thumbnail_interface thumbnail_impl = {
    .buffer = thumbnail_handle_buffer,
    .destroy = thumbnail_handle_destroy,
};

static bool add_output_obj(kywc_output *output, void *data)
{
    if (!output || !output->enabled) {
        return false;
    }

    json_object *output_arr = data;

    json_object *obj = json_object_new_object();
    json_object_object_add(obj, "key", json_object_new_string(output->uuid));
    json_object_object_add(obj, "title", json_object_new_string(output->name));

    char *physical_size = string_create("%s : %d mm x %d mm", tr("physical size"),
                                        output->physical_width, output->physical_height);
    json_object_object_add(obj, "sub_title", json_object_new_string(physical_size));
    free(physical_size);

    char *mode = string_create("%s: %d, %d", tr("position"), output->x, output->y);
    json_object_object_add(obj, "text", json_object_new_string(mode));
    free(mode);

    if (output->primary) {
        json_object_object_add(obj, "selected", json_object_new_boolean(true));
    }
    json_object_array_add(output_arr, obj);
    return false;
}

static json_object *get_output_list(struct portal_context *ctx)
{
    json_object *output_list = json_object_new_object();
    json_object_object_add(output_list, "name", json_object_new_string("output"));
    json_object_object_add(output_list, "mode", json_object_new_string("icon"));
    json_object *output_arry = json_object_new_array();
    kywc_context_for_each_output(ctx->wayland.kywc, add_output_obj, output_arry);
    json_object_object_add(output_list, "data", output_arry);

    return output_list;
}

static bool add_toplevel_obj(kywc_toplevel *toplevel, void *data)
{
    if (!toplevel || toplevel->capabilities) {
        return false;
    }

    json_object *toplevel_arr = data;

    json_object *obj = json_object_new_object();
    json_object_object_add(obj, "key", json_object_new_string(toplevel->uuid));
    json_object_object_add(obj, "title", json_object_new_string(toplevel->app_id));
    json_object_object_add(obj, "sub_title", json_object_new_string(toplevel->title));
    json_object_object_add(obj, "icon", json_object_new_string(toplevel->icon));
    json_object_object_add(obj, "pid", json_object_new_int(toplevel->pid));
    if (toplevel->activated) {
        json_object_object_add(obj, "selected", json_object_new_boolean(true));
    }

    json_object_array_add(toplevel_arr, obj);
    return false;
}

static json_object *get_toplevel_list(struct portal_context *ctx)
{
    json_object *toplevel_list = json_object_new_object();
    json_object_object_add(toplevel_list, "name", json_object_new_string("toplevel"));
    json_object *toplevel_arry = json_object_new_array();
    kywc_context_for_each_toplevel(ctx->wayland.kywc, add_toplevel_obj, toplevel_arry);
    json_object_object_add(toplevel_list, "data", toplevel_arry);

    return toplevel_list;
}

static void build_request_json(json_object **out_obj, const char *app_id, int modal,
                               const char *parent_window, struct portal_context *ctx)
{
    json_object *obj = json_object_new_object();
    if (parent_window) {
        json_object_object_add(obj, "parent_window", json_object_new_string(parent_window));
    }
    json_object_object_add(obj, "modal", json_object_new_boolean(modal > 0 ? true : false));
    json_object_object_add(obj, "title_bar", json_object_new_string("no_maximizable"));
    json_object_object_add(obj, "window_title", json_object_new_string(tr("Request Screenshot")));
    json_object_object_add(obj, "window_icon",
                           json_object_new_string(app_id ? app_id : "dialog-warning"));
    json_object_object_add(obj, "title_icon",
                           json_object_new_string(app_id ? app_id : "dialog-warning"));

    json_object *title_obj = json_object_new_object();
    char title_str[256];
    snprintf(title_str, sizeof(title_str), tr("%s Request Screenshot"), app_id);
    json_object_object_add(title_obj, "text", json_object_new_string(title_str));
    json_object_object_add(title_obj, "font_bold", json_object_new_boolean(1));
    json_object_object_add(obj, "title", title_obj);

    json_object *page_date = json_object_new_array();
    json_object *output_obj = json_object_new_object();
    json_object_object_add(output_obj, "title", json_object_new_string(tr("Screen")));
    json_object_object_add(output_obj, "name", json_object_new_string("output"));
    json_object *output_list = get_output_list(ctx);
    json_object_object_add(output_obj, "list", output_list);

    json_object *toplevel_page = json_object_new_object();
    json_object_object_add(toplevel_page, "title", json_object_new_string(tr("Window")));
    json_object_object_add(toplevel_page, "name", json_object_new_string("toplevel"));
    json_object *toplevel_list = get_toplevel_list(ctx);
    json_object_object_add(toplevel_page, "list", toplevel_list);

    json_object_array_add(page_date, output_obj);
    json_object_array_add(page_date, toplevel_page);
    json_object *page_obj = json_object_new_object();
    json_object_object_add(page_obj, "data", page_date);
    json_object_object_add(page_obj, "name", json_object_new_string("screenshot"));
    json_object_object_add(obj, "tab_page", page_obj);

    json_object *check_boxes = json_object_new_array();
    json_object *box = json_object_new_object();
    json_object_object_add(box, "name", json_object_new_string("without_decoration"));
    json_object_object_add(box, "text", json_object_new_string(tr("Without window borders")));
    json_object_object_add(box, "default", json_object_new_boolean(false));
    json_object_array_add(check_boxes, box);
    json_object_object_add(obj, "check_boxes", check_boxes);

    json_object_object_add(obj, "button_mode", json_object_new_int64(0));
    json_object_object_add(obj, "allow_text", json_object_new_string(tr("save")));
    json_object_object_add(obj, "deny_text", json_object_new_string(tr("cancel")));

    json_object *dialog = json_object_new_object();
    json_object_object_add(dialog, "dialog", obj);
    *out_obj = dialog;
}

static void parse_dialog_reply(json_object *obj, struct permission_dialog_data *dialog_data)
{
    if (!obj || !dialog_data) {
        return;
    }

    dialog_data->result_value = PORTAL_RESPONSE_ENDED;
    dialog_data->capture_type = CAPTURE_TYPE_NONE;
    dialog_data->without_decoration = false;
    dialog_data->uuid = NULL;

    json_object *result_obj, *tab_pages, *checkboxes_obj;

    if (json_object_object_get_ex(obj, "user_action", &result_obj)) {
        dialog_data->result_value = json_object_get_int(result_obj);
    }

    if (json_object_object_get_ex(obj, "tab_pages", &tab_pages) &&
        json_object_is_type(tab_pages, json_type_array)) {
        int array_len = json_object_array_length(tab_pages);
        for (int i = 0; i < array_len; i++) {
            json_object *item = json_object_array_get_idx(tab_pages, i);
            if (!item || !json_object_is_type(item, json_type_object))
                continue;

            json_object *page_obj;
            if (json_object_object_get_ex(item, "cur_page", &page_obj)) {
                if (strcmp("toplevel", json_object_get_string(page_obj)) == 0) {
                    dialog_data->capture_type = CAPTURE_TYPE_TOPLEVEL;
                } else if (strcmp("output", json_object_get_string(page_obj)) == 0) {
                    dialog_data->capture_type = CAPTURE_TYPE_OUTPUT;
                }
            }

            if (dialog_data->capture_type == CAPTURE_TYPE_NONE) {
                kywc_log(KYWC_WARN, "invalid data in dialog_data");
                return;
            }

            json_object *data_obj, *lists_obj;
            if (json_object_object_get_ex(item, "data", &data_obj) &&
                json_object_object_get_ex(data_obj, "lists", &lists_obj) &&
                json_object_is_type(lists_obj, json_type_array)) {

                int list_len = json_object_array_length(lists_obj);
                for (int j = 0; j < list_len; j++) {
                    json_object *list_item = json_object_array_get_idx(lists_obj, j);
                    json_object *uuid_obj;
                    if (json_object_object_get_ex(list_item, "items", &uuid_obj) &&
                        json_object_is_type(uuid_obj, json_type_array)) {
                        json_object *uuid_item = json_object_array_get_idx(uuid_obj, 0);
                        const char *uuid = json_object_get_string(uuid_item);
                        dialog_data->uuid = strdup(uuid);
                        break;
                    }
                }
            }
        }
    }

    if (json_object_object_get_ex(obj, "check_boxes", &checkboxes_obj) &&
        json_object_is_type(checkboxes_obj, json_type_array)) {
        int array_len = json_object_array_length(checkboxes_obj);
        for (int i = 0; i < array_len; i++) {
            json_object *item = json_object_array_get_idx(checkboxes_obj, i);
            json_object *name_obj, *checked_obj;
            if (json_object_object_get_ex(item, "name", &name_obj) &&
                json_object_object_get_ex(item, "checked", &checked_obj)) {
                const char *name = json_object_get_string(name_obj);
                if (name && strcmp(name, "without_decoration") == 0) {
                    dialog_data->without_decoration = json_object_get_boolean(checked_obj);
                }
            }
        }
    }
}

static int handle_permission_dialog(uint32_t mask, void *data)
{
    if (!data) {
        kywc_log(KYWC_WARN, "handle_permission_dialog: data is NULL");
        return 0;
    }

    struct portal_request *request = data;
    struct permission_dialog_data *dialog_data = request->data;
    struct portal_context *ctx = request->ctx;
    if (!dialog_data || !ctx || !dialog_data->msg) {
        kywc_log(KYWC_WARN, "handle_permission_dialog: invalid fields in dialog_data");
        portal_request_destroy(request);
        return 0;
    }

    sd_bus_message *msg = dialog_data->msg;
    if (mask & (EVENT_HANGUP | EVENT_ERROR)) {
        kywc_log(KYWC_WARN, "Dialog socket hungup or error");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        portal_request_destroy(request);
        return 0;
    }

    kywc_thumbnail *thumbnail = NULL;
    int fd = dialog_data->socket_fd;
    dialog_data->result_value = PORTAL_RESPONSE_ENDED;
    json_object *obj = dialog_get_reply(fd);
    if (obj) {
        parse_dialog_reply(obj, dialog_data);
        json_object_put(obj);
    }

    if (!dialog_data->uuid) {
        kywc_log(KYWC_WARN, "Screenshot failed: uuid is empty or null");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
        goto cleanup;
    }

    if (dialog_data->result_value != PORTAL_RESPONSE_SUCCESS) {
        sd_bus_reply_method_return(msg, "ua{sv}", dialog_data->result_value, 0);
        goto cleanup;
    }
    dialog_data->socket_fd = -1;

    if (dialog_data->capture_type == CAPTURE_TYPE_TOPLEVEL) {
        thumbnail = kywc_thumbnail_create_from_toplevel(ctx->wayland.kywc, dialog_data->uuid,
                                                        dialog_data->without_decoration,
                                                        &thumbnail_impl, request);
    } else if (dialog_data->capture_type == CAPTURE_TYPE_OUTPUT) {
        thumbnail = kywc_thumbnail_create_from_output(ctx->wayland.kywc, dialog_data->uuid,
                                                      &thumbnail_impl, request);
    }
    if (!thumbnail) {
        kywc_log(KYWC_ERROR, "Create thumbnail failed");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
        goto cleanup;
    }

cleanup:
    if (fd >= 0) {
        event_loop_remove_source(ctx->event_loop, fd);
        close(fd);
    }

    if (!thumbnail) {
        portal_request_destroy(request);
    }

    return 0;
}

static void handle_request_destroy(void *data)
{
    struct permission_dialog_data *dialog_data = data;
    if (!dialog_data) {
        return;
    }

    if (dialog_data->msg) {
        sd_bus_message_unref(dialog_data->msg);
    }

    if (dialog_data->event_loop) {
        event_loop_remove_source(dialog_data->event_loop, dialog_data->socket_fd);
    }
    if (dialog_data->socket_fd >= 0) {
        dialog_close(dialog_data->socket_fd);
        close(dialog_data->socket_fd);
        dialog_data->socket_fd = -1;
    }

    if (dialog_data->uuid) {
        free(dialog_data->uuid);
    }

    free(dialog_data);
    dialog_data = NULL;
}

static int handle_screenshot(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "Screenshot: Screenshot called");

    char *handle, *app_id, *parent_window;
    int ret = sd_bus_message_read(msg, "oss", &handle, &app_id, &parent_window);
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    /* options */
    int interactive = 0, modal = 1, permission_store_checked = 0;

    int inner_ret = 0;
    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "interactive") == 0) {
            sd_bus_message_read(msg, "v", "b", &interactive);
            kywc_log(KYWC_DEBUG, "Screenshot: option interactive: %d", interactive);
        } else if (strcmp(key, "modal") == 0) {
            sd_bus_message_read(msg, "v", "b", &modal);
            kywc_log(KYWC_DEBUG, "Screenshot: option modal: %d", modal);
        } else if (strcmp(key, "permission_store_checked") == 0) {
            sd_bus_message_read(msg, "v", "b", &permission_store_checked);
            kywc_log(KYWC_DEBUG, "Screenshot: option permission_store_checked: %d",
                     permission_store_checked);
        } else {
            kywc_log(KYWC_WARN, "Screenshot: unknown option %s", key);
            sd_bus_message_skip(msg, "v");
        }

        inner_ret = sd_bus_message_exit_container(msg);
        if (inner_ret < 0) {
            return inner_ret;
        }
    }
    if (ret < 0) {
        return ret;
    }
    ret = sd_bus_message_exit_container(msg);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG,
             "Screenshot by handle %s app_id %s parent_window %s, interactive %d modal %d "
             "permission_store_checked %d",
             handle, app_id, parent_window, interactive, modal, permission_store_checked);

    struct portal_context *ctx = data;

    struct permission_dialog_data *dialog_data = calloc(1, sizeof(*dialog_data));
    if (!dialog_data) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog data");
        return -ENOMEM;
    }

    struct portal_request *request = portal_request_create(ctx, handle, NULL);
    if (!request) {
        free(dialog_data);
        return -ENOMEM;
    }

    dialog_data->msg = sd_bus_message_ref(msg);
    dialog_data->socket_fd = -1;
    dialog_data->uuid = NULL;
    request->data = dialog_data;
    request->destroy = handle_request_destroy;
    wl_list_insert(&ctx->screenshot.requests, &request->link);

    if (interactive == 1) {
        json_object *json_req = NULL;
        build_request_json(&json_req, app_id, 0, NULL, ctx);
        if (!json_req) {
            kywc_log(KYWC_ERROR, "Failed to build request JSON object");
            return -1;
        }
        int dialog_socket = spawn_dialog("wldialog");
        if (!dialog_send_request(dialog_socket, json_req)) {
            close(dialog_socket);
            json_object_put(json_req);
            portal_request_destroy(request);
            return -1;
        }
        json_object_put(json_req);

        dialog_data->socket_fd = dialog_socket;
        dialog_data->event_loop = ctx->event_loop;
        if (event_loop_add_source(ctx->event_loop, dialog_data->socket_fd, handle_permission_dialog,
                                  request) < 0) {
            kywc_log(KYWC_ERROR, "Failed to add dialog socket to event loop");
            portal_request_destroy(request);
            return -1;
        }
    } else {
        struct portal_output *output = wl_container_of(ctx->wayland.outputs.next, output, link);
        kywc_log(KYWC_DEBUG, "Start capture output %s", output->output->name);

        kywc_thumbnail *thumbnail = kywc_thumbnail_create_from_output(
            ctx->wayland.kywc, output->output->uuid, &thumbnail_impl, request);
        if (!thumbnail) {
            kywc_log(KYWC_ERROR, "Create thumbnail for output failed");
            portal_request_destroy(request);
            return -1;
        }
    }
    return 1;
}

static int handle_pick_color(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "Screenshot: PickColor called");

    char *handle, *app_id, *parent_window;
    int ret = sd_bus_message_read(msg, "oss", &handle, &app_id, &parent_window);
    if (ret < 0) {
        return ret;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "PickColor by handle %s app_id %s parent_window %s", handle, app_id,
             parent_window);

    struct portal_context *ctx = data;
    struct portal_request *request = portal_request_create(ctx, handle, NULL);
    if (!request) {
        return -ENOMEM;
    }

    double red = 0, green = 0, blue = 0;
    // TODO: call color picker dbus
    ret = sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 1, "color", "(ddd)",
                                     red, green, blue);
    portal_request_destroy(request);
    return ret;
}

static const sd_bus_vtable screenshot_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("version", "u", NULL, offsetof(struct portal_context, screenshot.version),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_METHOD("Screenshot", "ossa{sv}", "ua{sv}", handle_screenshot,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("PickColor", "ossa{sv}", "ua{sv}", handle_pick_color, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

bool portal_screenshot_init(struct portal_context *ctx)
{
    ctx->screenshot.version = 2;
    wl_list_init(&ctx->screenshot.requests);

    int ret = sd_bus_add_object_vtable(ctx->bus, NULL, object_path, interface_name,
                                       screenshot_vtable, ctx);
    if (ret < 0) {
        return false;
    }

    return true;
}

void portal_screenshot_emit_toplevel_changed(struct portal_context *ctx)
{
    if (ctx->screenshot.version == 0) {
        return;
    }

    struct portal_request *request;
    wl_list_for_each(request, &ctx->screenshot.requests, link) {
        struct permission_dialog_data *dialog_data = request->data;
        if (!dialog_data) {
            continue;
        }

        if (dialog_data->socket_fd >= 0) {
            json_object *toplevel_list = get_toplevel_list(ctx);
            json_object *toplevel = json_object_new_object();
            json_object_object_add(toplevel, "list", toplevel_list);
            json_object *update = json_object_new_object();
            json_object_object_add(update, "update", toplevel);

            if (!dialog_send_request(dialog_data->socket_fd, update)) {
                kywc_log(KYWC_ERROR, "Update screenshot toplevel failed");
            }
            json_object_put(update);
        }
    }
}

void portal_screenshot_emit_output_changed(struct portal_context *ctx)
{
    if (ctx->screenshot.version == 0) {
        return;
    }

    struct portal_request *request;
    wl_list_for_each(request, &ctx->screenshot.requests, link) {
        struct permission_dialog_data *dialog_data = request->data;
        if (!dialog_data) {
            continue;
        }

        if (dialog_data->socket_fd >= 0) {
            json_object *output_list = get_output_list(ctx);
            json_object *output = json_object_new_object();
            json_object_object_add(output, "list", output_list);
            json_object *update = json_object_new_object();
            json_object_object_add(update, "update", output);

            if (!dialog_send_request(dialog_data->socket_fd, update)) {
                kywc_log(KYWC_ERROR, "Update screenshot output failed");
            }
            json_object_put(update);
        }
    }
}

void portal_screenshot_fini(struct portal_context *ctx)
{
    if (ctx->screenshot.version == 0) {
        return;
    }

    struct portal_request *request, *tmp;
    wl_list_for_each_safe(request, tmp, &ctx->screenshot.requests, link) {
        portal_request_destroy(request);
    }
}
