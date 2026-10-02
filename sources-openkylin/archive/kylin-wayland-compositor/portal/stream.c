// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "portal.h"
#include "stream.h"
#include "util/dialog.h"
#include "util/event.h"
#include "util/nls.h"
#include "util/spawn.h"
#include "util/string.h"

enum stream_persist_mode {
    STREAM_PERSIST_MODE_NONE = 0,
    STREAM_PERSIST_MODE_TEMPORARY = 1,
    STREAM_PERSIST_MODE_PERSISTENT = 2,
};

struct dialog_data {
    int socket_fd;
    struct sd_bus_message *msg;
    struct event_context *event_loop;
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

static json_object *get_output_list(struct portal_context *ctx, bool multiple)
{
    json_object *output_list = json_object_new_object();
    json_object_object_add(output_list, "name", json_object_new_string("output"));
    json_object_object_add(output_list, "mode", json_object_new_string("icon"));
    json_object *output_arry = json_object_new_array();
    kywc_context_for_each_output(ctx->wayland.kywc, add_output_obj, output_arry);
    json_object_object_add(output_list, "data", output_arry);
    json_object_object_add(output_list, "multiple", json_object_new_boolean(multiple));

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

    json_object_array_add(toplevel_arr, obj);
    return false;
}

static json_object *get_toplevel_list(struct portal_context *ctx, bool multiple)
{
    json_object *toplevel_list = json_object_new_object();
    json_object_object_add(toplevel_list, "name", json_object_new_string("toplevel"));
    json_object *toplevel_arry = json_object_new_array();
    kywc_context_for_each_toplevel(ctx->wayland.kywc, add_toplevel_obj, toplevel_arry);
    json_object_object_add(toplevel_list, "data", toplevel_arry);
    json_object_object_add(toplevel_list, "multiple", json_object_new_boolean(multiple));

    return toplevel_list;
}

static void add_output_custom(json_object *arr, bool multiple, struct portal_context *ctx)
{
    json_object *custom_output = json_object_new_object();
    json_object_object_add(custom_output, "title", json_object_new_string(tr("Choose screen:")));
    json_object *output_list = get_output_list(ctx, multiple);
    json_object_object_add(custom_output, "list", output_list);
    json_object_array_add(arr, custom_output);
}

static void add_toplevel_custom(json_object *arr, bool multiple, struct portal_context *ctx)
{
    json_object *custom_toplevel = json_object_new_object();
    json_object_object_add(custom_toplevel, "title", json_object_new_string(tr("Choose window:")));
    json_object *toplevel_list = get_toplevel_list(ctx, multiple);
    json_object_object_add(custom_toplevel, "list", toplevel_list);
    json_object_array_add(arr, custom_toplevel);
}

static void add_options_json(json_object *out_obj, uint32_t source_type, bool multiple,
                             struct portal_context *ctx)
{
    if (source_type == SOURCE_TYPE_MONITOR) {
        json_object *custom_arr = json_object_new_array();
        add_output_custom(custom_arr, multiple, ctx);
        json_object_object_add(out_obj, "customs", custom_arr);
    } else if (source_type == SOURCE_TYPE_WINDOW) {
        json_object *custom_arr = json_object_new_array();
        add_toplevel_custom(custom_arr, multiple, ctx);
        json_object_object_add(out_obj, "customs", custom_arr);
    } else if (source_type == SOURCE_TYPE_VIRTUAL) {
        /* Virtual screen option is not supported for now. */
        multiple = false;
    } else {
        if (multiple) {
            json_object *custom_arr = json_object_new_array();
            if (source_type & SOURCE_TYPE_MONITOR) {
                add_output_custom(custom_arr, multiple, ctx);
            }

            if (source_type & SOURCE_TYPE_WINDOW) {
                add_toplevel_custom(custom_arr, multiple, ctx);
            }

            // TODO: add virtual screen

            json_object_object_add(out_obj, "customs", custom_arr);
        } else {
            json_object *page_date = json_object_new_array();

            if (source_type & SOURCE_TYPE_MONITOR) {
                json_object *output_obj = json_object_new_object();
                json_object_object_add(output_obj, "title", json_object_new_string(tr("Screen")));
                json_object_object_add(output_obj, "name", json_object_new_string("output"));
                json_object *output_list = get_output_list(ctx, false);
                json_object_object_add(output_obj, "list", output_list);

                json_object_array_add(page_date, output_obj);
            }

            if (source_type & SOURCE_TYPE_WINDOW) {
                json_object *toplevel_page = json_object_new_object();
                json_object_object_add(toplevel_page, "title",
                                       json_object_new_string(tr("Window")));
                json_object_object_add(toplevel_page, "name", json_object_new_string("toplevel"));
                json_object *toplevel_list = get_toplevel_list(ctx, false);
                json_object_object_add(toplevel_page, "list", toplevel_list);

                json_object_array_add(page_date, toplevel_page);
            }
            // TODO: add virtual screen

            json_object *page_obj = json_object_new_object();
            json_object_object_add(page_obj, "data", page_date);
            json_object_object_add(page_obj, "name", json_object_new_string("screencast"));
            json_object_object_add(out_obj, "tab_page", page_obj);
        }
    }
}

static void handle_stream_ready(kywc_stream *stream, uint32_t node, void *data)
{
    kywc_log(KYWC_DEBUG, "handle_stream_ready");

    struct portal_session *session = data;
    struct dialog_data *dialog_data = session->data;
    if (!dialog_data) {
        return;
    }

    struct portal_stream *portal_stream = calloc(1, sizeof(*portal_stream));
    if (!portal_stream) {
        return;
    }

    if (stream->type == KYWC_STREAM_TYPE_OUTPUT) {
        portal_stream->source_type = SOURCE_TYPE_MONITOR;
        kywc_output *output =
            kywc_context_find_output(session->ctx->wayland.kywc, stream->source_uuid);
        if (output) {
            portal_stream->geometry.x = output->x, portal_stream->geometry.y = output->y;
            portal_stream->geometry.width = output->width,
            portal_stream->geometry.height = output->height;
        }
    } else if (stream->type == KYWC_STREAM_TYPE_TOPLEVEL) {
        portal_stream->source_type = SOURCE_TYPE_WINDOW;
        kywc_toplevel *toplevel =
            kywc_context_find_toplevel(session->ctx->wayland.kywc, stream->source_uuid);
        if (toplevel) {
            portal_stream->geometry.x = toplevel->x, portal_stream->geometry.y = toplevel->y;
            portal_stream->geometry.width = toplevel->width,
            portal_stream->geometry.height = toplevel->height;
        }
    }
    portal_stream->node = node;
    portal_stream->stream = stream;

    int list_streams = -1;
    int stream_counts = 0;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        wl_list_insert(&session->remote_desktop.streams, &portal_stream->link);
        list_streams = wl_list_length(&session->remote_desktop.streams);
        stream_counts = session->remote_desktop.stream_counts;
    } else {
        wl_list_insert(&session->screencast.streams, &portal_stream->link);
        list_streams = wl_list_length(&session->screencast.streams);
        stream_counts = session->screencast.stream_counts;
    }

    if (list_streams == stream_counts) {
        kywc_log(KYWC_DEBUG, "list_streams is ready");

        sd_bus_message *msg = dialog_data->msg;
        sd_bus_message *reply = NULL;
        sd_bus_message_new_method_return(msg, &reply);

        sd_bus_message_append(reply, "u", PORTAL_RESPONSE_SUCCESS);

        sd_bus_message_open_container(reply, 'a', "{sv}");
        sd_bus_message_open_container(reply, 'e', "sv");

        if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
            append_streams_entry(&session->remote_desktop.streams, &reply);
        } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
            append_streams_entry(&session->screencast.streams, &reply);
        }

        sd_bus_message_close_container(reply);

        if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
            remote_desktop_restore_send(&reply, session);
        } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
            screencast_restore_send(&reply, session);
        }

        sd_bus_message_close_container(reply);
        sd_bus_send(NULL, reply, NULL);
        sd_bus_message_unref(reply);
        sd_bus_message_unref(msg);
        free(dialog_data);
        session->data = NULL;
    }
}

static void handle_stream_destroy(kywc_stream *stream, void *data)
{
    struct portal_session *session = data;
    if (!session) {
        return;
    }

    struct portal_stream *portal_stream, *tmp;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        wl_list_for_each_safe(portal_stream, tmp, &session->remote_desktop.streams, link) {
            if (stream == portal_stream->stream) {
                wl_list_remove(&portal_stream->link);
                free(portal_stream);
            }
        }
    } else {
        wl_list_for_each_safe(portal_stream, tmp, &session->screencast.streams, link) {
            if (stream == portal_stream->stream) {
                wl_list_remove(&portal_stream->link);
                free(portal_stream);
            }
        }
    }

    struct dialog_data *dialog_data = session->data;
    if (!dialog_data) {
        return;
    }

    sd_bus_message *msg = dialog_data->msg;
    if (msg) {
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
        sd_bus_message_unref(msg);
        free(dialog_data);
        session->data = NULL;
    }
}

static const struct kywc_stream_interface stream_impl = {
    .ready = handle_stream_ready,
    .destroy = handle_stream_destroy,
};

static inline enum kywc_stream_cursor_mode get_cursor_mode(uint32_t mode)
{
    return mode == CURSOR_MODE_EMBEDDED ? KYWC_STREAM_CURSOR_MODE_EMBEDDED
                                        : KYWC_STREAM_CURSOR_MODE_HIDDEN;
}

static bool portal_stream_create_from_output(struct portal_session *session, char *uuid)
{
    if (!session || !uuid) {
        return false;
    }

    struct portal_context *ctx = session->ctx;
    kywc_stream *stream = NULL;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        stream = kywc_stream_create_from_output(
            ctx->wayland.kywc, uuid, get_cursor_mode(session->remote_desktop.select.cursor_mode),
            &stream_impl, session);
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        stream = kywc_stream_create_from_output(
            ctx->wayland.kywc, uuid, get_cursor_mode(session->screencast.select.cursor_mode),
            &stream_impl, session);
    }
    if (!stream) {
        kywc_log(KYWC_ERROR, "Create stream failed");
        return false;
    }

    return true;
}

static bool portal_stream_create_from_toplevel(struct portal_session *session, char *uuid)
{
    if (!session || !uuid) {
        return false;
    }

    struct portal_context *ctx = session->ctx;
    kywc_stream *stream = NULL;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        stream = kywc_stream_create_from_toplevel(
            ctx->wayland.kywc, uuid, get_cursor_mode(session->remote_desktop.select.cursor_mode),
            &stream_impl, session);
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        stream = kywc_stream_create_from_toplevel(
            ctx->wayland.kywc, uuid, get_cursor_mode(session->screencast.select.cursor_mode),
            &stream_impl, session);
    }
    if (!stream) {
        kywc_log(KYWC_ERROR, "Create stream failed");
        return false;
    }
    return true;
}

static bool array_add_uuid(struct wl_array *array, const char *uuid)
{
    if (!uuid) {
        return false;
    }
    const char **dst = wl_array_add(array, sizeof(char *));
    if (!dst) {
        return false;
    }
    *dst = strdup(uuid);
    return *dst != NULL;
}

static bool parse_dialog_reply(json_object *obj, struct portal_session *session)
{
    if (!obj || !session) {
        return false;
    }

    bool ret = false;
    int result_value = PORTAL_RESPONSE_ENDED;
    json_object *result_obj;
    if (json_object_object_get_ex(obj, "user_action", &result_obj)) {
        result_value = json_object_get_int(result_obj);
    }

    if (result_value != PORTAL_RESPONSE_SUCCESS) {
        return ret;
    }

    bool multiple = false;
    uint32_t persist_mode = STREAM_PERSIST_MODE_NONE;
    uint32_t source_type = 0;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        multiple = session->remote_desktop.select.multiple;
        persist_mode = session->remote_desktop.persist_mode;
        source_type = session->remote_desktop.select.source_types;
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        multiple = session->screencast.select.multiple;
        persist_mode = session->screencast.persist_mode;
        source_type = session->screencast.select.source_types;
    }

    struct wl_array output_array, toplevel_array;
    wl_array_init(&output_array);
    wl_array_init(&toplevel_array);

    /* TODO: add virtual support*/
    int uuid_count = 0;
    if (multiple) {
        json_object *lists_obj;
        if (json_object_object_get_ex(obj, "lists", &lists_obj) &&
            json_object_is_type(lists_obj, json_type_array)) {
            int list_len = json_object_array_length(lists_obj);

            for (int j = 0; j < list_len; j++) {
                json_object *list_item = json_object_array_get_idx(lists_obj, j);
                json_object *uuid_obj;
                if (json_object_object_get_ex(list_item, "items", &uuid_obj) &&
                    json_object_is_type(uuid_obj, json_type_array)) {

                    json_object *list_name;
                    const char *name = NULL;
                    if (json_object_object_get_ex(list_item, "name", &list_name)) {
                        name = json_object_get_string(list_name);
                    }

                    bool is_toplevel = name && strlen(name) >= 8 &&
                                       strcmp(name + strlen(name) - 8, "toplevel") == 0;
                    struct wl_array *target_array = is_toplevel ? &toplevel_array : &output_array;

                    int nums = json_object_array_length(uuid_obj);
                    for (int n = 0; n < nums; n++) {
                        json_object *uuid_item = json_object_array_get_idx(uuid_obj, n);
                        const char *uuid = json_object_get_string(uuid_item);

                        if (!array_add_uuid(target_array, uuid)) {
                            goto cleanup;
                        }
                        uuid_count++;
                    }
                }
            }
        }
    } else {
        if (source_type == SOURCE_TYPE_MONITOR) {
            json_object *lists_obj;
            if (json_object_object_get_ex(obj, "lists", &lists_obj) &&
                json_object_is_type(lists_obj, json_type_array)) {
                int list_len = json_object_array_length(lists_obj);

                for (int j = 0; j < list_len; j++) {
                    json_object *list_item = json_object_array_get_idx(lists_obj, j);
                    json_object *uuid_obj;
                    if (json_object_object_get_ex(list_item, "items", &uuid_obj) &&
                        json_object_is_type(uuid_obj, json_type_array)) {

                        json_object *uuid_item = json_object_array_get_idx(uuid_obj, 0);
                        const char *uuid = json_object_get_string(uuid_item);

                        if (!array_add_uuid(&output_array, uuid)) {
                            goto cleanup;
                        }
                        uuid_count++;
                        break;
                    }
                }
            }
        } else if (source_type == SOURCE_TYPE_WINDOW) {
            json_object *lists_obj;
            if (json_object_object_get_ex(obj, "lists", &lists_obj) &&
                json_object_is_type(lists_obj, json_type_array)) {
                int list_len = json_object_array_length(lists_obj);

                for (int j = 0; j < list_len; j++) {
                    json_object *list_item = json_object_array_get_idx(lists_obj, j);
                    json_object *uuid_obj;
                    if (json_object_object_get_ex(list_item, "items", &uuid_obj) &&
                        json_object_is_type(uuid_obj, json_type_array)) {

                        json_object *uuid_item = json_object_array_get_idx(uuid_obj, 0);
                        const char *uuid = json_object_get_string(uuid_item);

                        if (!array_add_uuid(&toplevel_array, uuid)) {
                            goto cleanup;
                        }
                        uuid_count++;
                        break;
                    }
                }
            }
        } else if (source_type == SOURCE_TYPE_VIRTUAL) {
            // TODO: get virtual screen
        } else {
            bool is_toplevel = false;
            json_object *tab_pages;
            if (json_object_object_get_ex(obj, "tab_pages", &tab_pages) &&
                json_object_is_type(tab_pages, json_type_array)) {
                int array_len = json_object_array_length(tab_pages);
                for (int i = 0; i < array_len; i++) {
                    json_object *item = json_object_array_get_idx(tab_pages, i);
                    if (!item || !json_object_is_type(item, json_type_object)) {
                        continue;
                    }

                    json_object *page_obj;
                    if (json_object_object_get_ex(item, "cur_page", &page_obj) &&
                        strcmp("toplevel", json_object_get_string(page_obj)) == 0) {
                        is_toplevel = true;
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

                                if (is_toplevel) {
                                    if (!array_add_uuid(&toplevel_array,
                                                        json_object_get_string(uuid_item))) {
                                        kywc_log(KYWC_ERROR, "Failed to add toplevel uuid");
                                        goto cleanup;
                                    }
                                } else {
                                    if (!array_add_uuid(&output_array,
                                                        json_object_get_string(uuid_item))) {
                                        kywc_log(KYWC_ERROR, "Failed to add output uuid");
                                        goto cleanup;
                                    }
                                }
                                uuid_count++;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    json_object *checkboxes_obj;
    if (json_object_object_get_ex(obj, "check_boxes", &checkboxes_obj)) {
        if (json_object_is_type(checkboxes_obj, json_type_array)) {
            int array_len = json_object_array_length(checkboxes_obj);
            for (int i = 0; i < array_len; i++) {
                json_object *item = json_object_array_get_idx(checkboxes_obj, i);
                if (!item || !json_object_is_type(item, json_type_object)) {
                    continue;
                }

                json_object *name_obj;
                const char *name = NULL;
                if (json_object_object_get_ex(item, "name", &name_obj)) {
                    name = json_object_get_string(name_obj);
                }

                int checked = 0;
                json_object *checked_obj;
                if (json_object_object_get_ex(item, "checked", &checked_obj)) {
                    checked = json_object_get_boolean(checked_obj);
                }

                kywc_log(KYWC_DEBUG, "Checkbox[%d]: name='%s', checked=%s", i, name ? name : "NULL",
                         checked ? "true" : "false");

                if (name && strcmp(name, "persist_mode") == 0) {
                    if (checked) {
                        persist_mode = STREAM_PERSIST_MODE_PERSISTENT;
                    } else {
                        persist_mode = STREAM_PERSIST_MODE_TEMPORARY;
                    }
                }
            }
        } else {
            kywc_log(KYWC_ERROR, "check_boxes is not an array");
        }
    }

    if (uuid_count == 0) {
        wl_array_release(&output_array);
        wl_array_release(&toplevel_array);
        return ret;
    }

    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        session->remote_desktop.persist_mode = persist_mode;
        session->remote_desktop.stream_counts = uuid_count;
        session->remote_desktop.select.multiple = multiple;
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        session->screencast.persist_mode = persist_mode;
        session->screencast.stream_counts = uuid_count;
        session->screencast.select.multiple = multiple;
    }

    char **uuid;
    wl_array_for_each(uuid, &output_array) {
        if (!(ret = portal_stream_create_from_output(session, *uuid))) {
            goto cleanup;
        }
    }
    wl_array_for_each(uuid, &toplevel_array) {
        if (!(ret = portal_stream_create_from_toplevel(session, *uuid))) {
            goto cleanup;
        }
    }

cleanup:
    wl_array_for_each(uuid, &output_array) {
        free(*uuid);
    }
    wl_array_for_each(uuid, &toplevel_array) {
        free(*uuid);
    }
    wl_array_release(&output_array);
    wl_array_release(&toplevel_array);

    return ret;
}

static void handle_request_destroy(void *data)
{
    struct portal_session *session = data;
    if (!session) {
        return;
    }

    struct dialog_data *dialog_data = session->data;
    if (!dialog_data) {
        return;
    }

    if (dialog_data->event_loop) {
        event_loop_remove_source(dialog_data->event_loop, dialog_data->socket_fd);
    }
    if (dialog_data->socket_fd >= 0) {
        dialog_close(dialog_data->socket_fd);
        close(dialog_data->socket_fd);
        dialog_data->socket_fd = -1;
    }
}

static int handle_screencast_dialog(uint32_t mask, void *data)
{
    if (!data) {
        kywc_log(KYWC_WARN, "handle_screencast_dialog: data is NULL");
        return 0;
    }

    struct portal_request *request = data;
    struct portal_session *session = request->data;
    if (!session) {
        kywc_log(KYWC_WARN, "handle_screencast_dialog: invalid fields in dialog_data");
        portal_request_destroy(request);
        return 0;
    }

    struct dialog_data *dialog_data = session->data;
    if (!dialog_data) {
        kywc_log(KYWC_WARN, "handle_screencast_dialog: invalid fields in dialog_data");
        portal_request_destroy(request);
        return 0;
    }

    sd_bus_message *msg = dialog_data->msg;
    int fd = dialog_data->socket_fd;

    if (mask & (EVENT_HANGUP | EVENT_ERROR)) {
        kywc_log(KYWC_WARN, "Dialog socket hungup or error");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        sd_bus_message_unref(msg);
        free(dialog_data);
        session->data = NULL;
        goto cleanup;
    }

    json_object *obj = dialog_get_reply(fd);
    if (obj) {
        bool ret = parse_dialog_reply(obj, session);
        if (!ret) {
            sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
            sd_bus_message_unref(msg);
            free(dialog_data);
            session->data = NULL;
        } else {
            dialog_data->socket_fd = -1;
        }
        json_object_put(obj);
    }

cleanup:
    if (fd >= 0) {
        event_loop_remove_source(session->ctx->event_loop, fd);
        close(fd);
    }
    portal_request_destroy(request);

    return 0;
}

static void build_request_json(json_object **out_obj, const char *app_id, int modal,
                               const char *parent_window, struct portal_session *session)
{
    json_object *obj = json_object_new_object();
    if (parent_window) {
        json_object_object_add(obj, "parent_window", json_object_new_string(parent_window));
    }

    json_object_object_add(obj, "modal", json_object_new_boolean(modal > 0 ? true : false));
    json_object_object_add(obj, "title_bar", json_object_new_string("no_maximizable"));
    json_object_object_add(obj, "window_title", json_object_new_string(tr("Request Screencast")));

    const char *icon = app_id ? app_id : "dialog-warning";
    json_object_object_add(obj, "window_icon", json_object_new_string(icon));
    json_object_object_add(obj, "title_icon", json_object_new_string(icon));

    uint32_t source_type = 0;
    bool multiple = false;
    uint32_t persist_mode = 0;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        source_type = session->remote_desktop.select.source_types;
        multiple = session->remote_desktop.select.multiple;
        persist_mode = session->remote_desktop.persist_mode;
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        source_type = session->screencast.select.source_types;
        multiple = session->screencast.select.multiple;
        persist_mode = session->screencast.persist_mode;
    }

    struct portal_context *ctx = session->ctx;
    json_object *title_obj = json_object_new_object();
    if (source_type == SOURCE_TYPE_MONITOR) {
        if (wl_list_length(&ctx->wayland.outputs) == 1) {
            if (app_id) {
                char title_str[256];
                snprintf(title_str, sizeof(title_str), tr("Share current screen with %s"), app_id);
                json_object_object_add(title_obj, "text", json_object_new_string(title_str));
            } else {
                json_object_object_add(title_obj, "text",
                                       json_object_new_string(tr("Share current screen")));
            }
        } else {
            if (multiple) {
                if (app_id) {
                    char title_str[256];
                    snprintf(title_str, sizeof(title_str), tr("Choose screens to share with %s"),
                             app_id);
                    json_object_object_add(title_obj, "text", json_object_new_string(title_str));
                } else {
                    json_object_object_add(title_obj, "text",
                                           json_object_new_string(tr("Choose screens to share")));
                }
            } else {
                if (app_id) {
                    char title_str[256];
                    snprintf(title_str, sizeof(title_str), tr("Choose a screen to share with %s"),
                             app_id);
                    json_object_object_add(title_obj, "text", json_object_new_string(title_str));
                } else {
                    json_object_object_add(title_obj, "text",
                                           json_object_new_string(tr("Choose a screen to share")));
                }
            }
        }
    } else if (source_type == SOURCE_TYPE_WINDOW) {
        if (multiple) {
            if (app_id) {
                char title_str[256];
                snprintf(title_str, sizeof(title_str), tr("Choose windows to share with %s"),
                         app_id);
                json_object_object_add(title_obj, "text", json_object_new_string(title_str));
            } else {
                json_object_object_add(title_obj, "text",
                                       json_object_new_string(tr("Choose windows to share")));
            }
        } else {
            if (app_id) {
                char title_str[256];
                snprintf(title_str, sizeof(title_str), tr("Choose a window to share with %s"),
                         app_id);
                json_object_object_add(title_obj, "text", json_object_new_string(title_str));
            } else {
                json_object_object_add(title_obj, "text",
                                       json_object_new_string(tr("Choose a window to share")));
            }
        }
    } else if (source_type == SOURCE_TYPE_VIRTUAL) {
        if (app_id) {
            char title_str[256];
            snprintf(title_str, sizeof(title_str),
                     tr("Create a new virtual display to share with %s"), app_id);
            json_object_object_add(title_obj, "text", json_object_new_string(title_str));
        } else {
            json_object_object_add(
                title_obj, "text",
                json_object_new_string(tr("Create a new virtual display to share")));
        }
        multiple = false;
    } else {
        if (multiple) {
            if (app_id) {
                char title_str[256];
                snprintf(title_str, sizeof(title_str), tr("Choose streams to share with %s"),
                         app_id);
                json_object_object_add(title_obj, "text", json_object_new_string(title_str));
            } else {
                json_object_object_add(title_obj, "text",
                                       json_object_new_string(tr("Choose streams to share")));
            }
        } else {
            if (app_id) {
                char title_str[256];
                snprintf(title_str, sizeof(title_str), tr("Choose what to share with %s"), app_id);
                json_object_object_add(title_obj, "text", json_object_new_string(title_str));
            } else {
                json_object_object_add(title_obj, "text",
                                       json_object_new_string(tr("Choose what to share")));
            }
        }
    }
    json_object_object_add(title_obj, "font_bold", json_object_new_boolean(1));
    json_object_object_add(obj, "title", title_obj);

    add_options_json(obj, source_type, multiple, ctx);

    if (persist_mode != STREAM_PERSIST_MODE_NONE) {
        json_object *check_boxes = json_object_new_array();
        json_object *box = json_object_new_object();
        json_object_object_add(box, "name", json_object_new_string("persist_mode"));
        json_object_object_add(
            box, "text", json_object_new_string(tr("Remember this choice for future sessions")));
        if (persist_mode == STREAM_PERSIST_MODE_TEMPORARY) {
            json_object_object_add(box, "default", json_object_new_boolean(false));
        } else {
            json_object_object_add(box, "default", json_object_new_boolean(true));
        }
        json_object_array_add(check_boxes, box);
        json_object_object_add(obj, "check_boxes", check_boxes);
    }

    json_object_object_add(obj, "button_mode", json_object_new_int64(0));
    json_object_object_add(obj, "allow_text", json_object_new_string(tr("save")));
    json_object_object_add(obj, "deny_text", json_object_new_string(tr("cancel")));

    json_object *dialog = json_object_new_object();
    json_object_object_add(dialog, "dialog", obj);
    *out_obj = dialog;
}

static void handle_session_destroy(void *data)
{
    struct dialog_data *dialog_data = data;
    if (!dialog_data) {
        return;
    }

    if (dialog_data->msg) {
        sd_bus_message_unref(dialog_data->msg);
        dialog_data->msg = NULL;
    }

    if (dialog_data->event_loop) {
        event_loop_remove_source(dialog_data->event_loop, dialog_data->socket_fd);
    }
    if (dialog_data->socket_fd >= 0) {
        dialog_close(dialog_data->socket_fd);
        close(dialog_data->socket_fd);
        dialog_data->socket_fd = -1;
    }

    free(dialog_data);
}

void portal_stream_select(struct portal_session *session, char *app_id,
                          struct portal_request *request, sd_bus_message *msg)
{
    if (!session) {
        return;
    }

    struct dialog_data *dialog_data = calloc(1, sizeof(*dialog_data));
    if (!dialog_data) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog data");
        return;
    }

    if (request) {
        request->data = session;
    } else {
        request = portal_request_create(session->ctx, session->session_handle, session);
        if (!request) {
            free(dialog_data);
            return;
        }
    }

    dialog_data->socket_fd = -1;
    dialog_data->msg = msg;
    session->data = dialog_data;
    request->destroy = handle_request_destroy;
    session->destroy = handle_session_destroy;

    json_object *json_req = NULL;
    build_request_json(&json_req, app_id, 0, NULL, session);
    if (!json_req) {
        kywc_log(KYWC_ERROR, "Failed to build request JSON object");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        sd_bus_message_unref(msg);
        free(dialog_data);
        session->data = NULL;
        return;
    }

    int dialog_socket = spawn_dialog("wldialog");
    if (!dialog_send_request(dialog_socket, json_req)) {
        close(dialog_socket);
        json_object_put(json_req);
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        sd_bus_message_unref(msg);
        portal_request_destroy(request);
        free(dialog_data);
        session->data = NULL;
        return;
    }
    json_object_put(json_req);

    dialog_data->socket_fd = dialog_socket;
    dialog_data->event_loop = session->ctx->event_loop;
    if (event_loop_add_source(session->ctx->event_loop, dialog_data->socket_fd,
                              handle_screencast_dialog, request) < 0) {
        kywc_log(KYWC_ERROR, "Failed to add dialog socket to event loop");
        portal_request_destroy(request);
        return;
    }
}

bool screencast_restore_streams_from_data(struct portal_session *session, sd_bus_message *msg)
{
    struct portal_context *ctx = session->ctx;
    struct screencast_restore_data *restore_data = session->screencast.restore_data;

    struct dialog_data *dialog_data = calloc(1, sizeof(*dialog_data));
    if (!dialog_data) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog data");
        return false;
    }
    dialog_data->msg = msg;
    dialog_data->socket_fd = -1;
    session->data = dialog_data;
    session->destroy = handle_session_destroy;

    int stream_count = 0;
    char **uuid;
    wl_array_for_each(uuid, &restore_data->outputs) {
        stream_count++;
    }
    wl_array_for_each(uuid, &restore_data->toplevels) {
        stream_count++;
    }
    session->screencast.stream_counts = stream_count;

    wl_array_for_each(uuid, &restore_data->outputs) {
        kywc_output *output = kywc_context_find_output(ctx->wayland.kywc, *uuid);
        if (output) {
            if (!(portal_stream_create_from_output(session, *uuid))) {
                session->destroy(session->data);
                session->data = NULL;
                return false;
            }
        }
    }

    wl_array_for_each(uuid, &restore_data->toplevels) {
        kywc_toplevel *toplevel = kywc_context_find_toplevel(ctx->wayland.kywc, *uuid);
        if (toplevel) {
            if (!(portal_stream_create_from_toplevel(session, *uuid))) {
                session->destroy(session->data);
                session->data = NULL;
                return false;
            }
        }
    }

    return true;
}

void append_streams_entry(struct wl_list *streams, sd_bus_message **reply)
{
    sd_bus_message_append(*reply, "s", "streams");

    sd_bus_message_open_container(*reply, 'v', "a(ua{sv})");
    sd_bus_message_open_container(*reply, 'a', "(ua{sv})");

    struct portal_stream *stream, *tmp;
    wl_list_for_each_safe(stream, tmp, streams, link) {
        sd_bus_message_open_container(*reply, 'r', "ua{sv}");

        sd_bus_message_append(*reply, "u", stream->node);
        sd_bus_message_open_container(*reply, 'a', "{sv}");

        if (stream->source_type == SOURCE_TYPE_MONITOR) {
            sd_bus_message_append(*reply, "{sv}", "position", "(ii)", stream->geometry.x,
                                  stream->geometry.y);
        }
        sd_bus_message_append(*reply, "{sv}", "size", "(ii)", stream->geometry.width,
                              stream->geometry.height);
        sd_bus_message_append(*reply, "{sv}", "source_type", "u", stream->source_type);

        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);

        // mapping_id (s)
    }

    sd_bus_message_close_container(*reply);
    sd_bus_message_close_container(*reply);
}

static void portal_stream_toplevel_update(struct portal_context *ctx, bool multiple, int fd)
{
    json_object *toplevel_list = get_toplevel_list(ctx, multiple);
    json_object *toplevel = json_object_new_object();
    json_object_object_add(toplevel, "list", toplevel_list);
    json_object *update = json_object_new_object();
    json_object_object_add(update, "update", toplevel);

    if (!dialog_send_request(fd, update)) {
        kywc_log(KYWC_ERROR, "update toplevel list failed");
    }
    json_object_put(update);
}

void portal_stream_emit_toplevel_changed(struct portal_context *ctx)
{
    if (ctx->remote_desktop.version == 0 && ctx->screencast.version == 0) {
        return;
    }

    if (ctx->remote_desktop.version) {
        struct portal_session *session;

        wl_list_for_each(session, &ctx->remote_desktop.sessions, link) {
            if (!session->remote_desktop.screen_share_enabled) {
                continue;
            }

            if ((session->remote_desktop.select.source_types & SOURCE_TYPE_WINDOW) == 0) {
                continue;
            }

            struct dialog_data *dialog_data = session->data;
            if (!dialog_data || dialog_data->socket_fd < 0) {
                continue;
            }

            portal_stream_toplevel_update(ctx, session->remote_desktop.select.multiple,
                                          dialog_data->socket_fd);
        }
    }
    if (ctx->screencast.version) {
        struct portal_session *session;
        wl_list_for_each(session, &ctx->screencast.sessions, link) {
            if ((session->screencast.select.source_types & SOURCE_TYPE_WINDOW) == 0) {
                continue;
            }

            struct dialog_data *dialog_data = session->data;
            if (!dialog_data || dialog_data->socket_fd < 0) {
                continue;
            }

            portal_stream_toplevel_update(ctx, session->screencast.select.multiple,
                                          dialog_data->socket_fd);
        }
    }
}

static void portal_stream_output_update(struct portal_context *ctx, bool multiple, int fd)
{
    json_object *output_list = get_output_list(ctx, multiple);
    json_object *output = json_object_new_object();
    json_object_object_add(output, "list", output_list);
    json_object *update = json_object_new_object();
    json_object_object_add(update, "update", output);

    if (!dialog_send_request(fd, update)) {
        kywc_log(KYWC_ERROR, "update output list failed");
    }
    json_object_put(update);
}

void portal_stream_emit_output_changed(struct portal_context *ctx)
{
    if (ctx->remote_desktop.version == 0 && ctx->screencast.version == 0) {
        return;
    }

    if (ctx->remote_desktop.version) {
        struct portal_session *session;
        wl_list_for_each(session, &ctx->remote_desktop.sessions, link) {
            if ((session->remote_desktop.select.source_types & SOURCE_TYPE_MONITOR) == 0) {
                continue;
            }
            struct dialog_data *dialog_data = session->data;
            if (!dialog_data || dialog_data->socket_fd < 0) {
                continue;
            }

            portal_stream_output_update(ctx, session->remote_desktop.select.multiple,
                                        dialog_data->socket_fd);
        }
    }
    if (ctx->screencast.version) {
        struct portal_session *session;
        wl_list_for_each(session, &ctx->screencast.sessions, link) {
            if ((session->screencast.select.source_types & SOURCE_TYPE_MONITOR) == 0) {
                continue;
            }
            struct dialog_data *dialog_data = session->data;
            if (!dialog_data || dialog_data->socket_fd < 0) {
                continue;
            }

            portal_stream_output_update(ctx, session->screencast.select.multiple,
                                        dialog_data->socket_fd);
        }
    }
}

void portal_stream_destroy(struct portal_session *session)
{
    if (!session) {
        return;
    }

    struct portal_stream *stream, *tmp;
    if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        wl_list_for_each_safe(stream, tmp, &session->remote_desktop.streams, link) {
            if (stream->stream) {
                kywc_stream_destroy(stream->stream);
            }
        }
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        wl_list_for_each_safe(stream, tmp, &session->screencast.streams, link) {
            if (stream->stream) {
                kywc_stream_destroy(stream->stream);
            }
        }
    }
}

struct portal_stream *portal_stream_get_from_remote_desktop(struct portal_session *session,
                                                            uint32_t node)
{
    struct portal_stream *stream;
    wl_list_for_each(stream, &session->remote_desktop.streams, link) {
        if (stream->node == node) {
            return stream;
        }
    }

    return NULL;
}
