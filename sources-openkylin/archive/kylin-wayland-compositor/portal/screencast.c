// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "portal.h"
#include "stream.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.ScreenCast";

enum screencast_persist_mode {
    SCREENCAST_PERSIST_MODE_NONE = 0,
    SCREENCAST_PERSIST_MODE_TEMPORARY = 1,
    SCREENCAST_PERSIST_MODE_PERSISTENT = 2,
};

static void screencast_restore_data_free(void *data)
{
    struct screencast_restore_data *restore = data;
    if (!restore) {
        return;
    }
    char **uuid;
    wl_array_for_each(uuid, &restore->outputs) {
        free(*uuid);
    }
    wl_array_for_each(uuid, &restore->toplevels) {
        free(*uuid);
    }
    wl_array_release(&restore->outputs);
    wl_array_release(&restore->toplevels);
    free(restore->vendor_name);
    free(restore);
}

static bool restore_array_add_uuid(struct wl_array *array, const char *uuid)
{
    if (!uuid) {
        return false;
    }
    char **dst = wl_array_add(array, sizeof(char *));
    if (!dst) {
        return false;
    }
    *dst = strdup(uuid);
    return *dst != NULL;
}

static int handle_create_session(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "ScreenCast: CreateSession called");

    char *handle, *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "oos", &handle, &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "CreateSession by handle %s session_handle %s app_id %s", handle,
             session_handle, app_id);

    struct portal_context *ctx = data;

    struct portal_session *session =
        portal_session_create(ctx, PORTAL_SESSION_TYPE_SCREENCAST, session_handle);
    if (!session) {
        return -ENOMEM;
    }

    wl_list_init(&session->screencast.streams);

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static bool get_restore_data(sd_bus_message *msg, struct screencast_restore_data *restore_data)
{
    int ret;
    ret = sd_bus_message_enter_container(msg, 'v', NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enter VARIANT container for restore_data");
        return false;
    }

    ret = sd_bus_message_enter_container(msg, 'r', "suv");
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enter STRUCT (sua{sv}) container");
        goto exit_variant;
    }

    char *vendor_name;
    ret = sd_bus_message_read(msg, "su", &vendor_name, &restore_data->version);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to read session name or version");
        goto exit_struct;
    }

    restore_data->vendor_name = strdup(vendor_name);
    if (!restore_data->vendor_name) {
        goto exit_struct;
    }

    ret = sd_bus_message_enter_container(msg, 'v', "a{sv}");
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enter VARIANT wrapping the dictionary");
        goto exit_struct;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        sd_bus_message_exit_container(msg);
        goto exit_inner_variant;
    }

    const char *key;
    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        ret = sd_bus_message_read(msg, "s", &key);
        if (ret < 0) {
            break;
        }

        if (strcmp(key, "outputs") == 0) {
            ret = sd_bus_message_enter_container(msg, 'v', "as");
            if (ret < 0) {
                kywc_log(KYWC_ERROR, "Failed to enter VARIANT 'as' for outputs");
                goto exit_entry;
            }
            ret = sd_bus_message_enter_container(msg, 'a', "s");
            if (ret < 0) {
                kywc_log(KYWC_ERROR, "Failed to enter ARRAY 's' for outputs");
                sd_bus_message_exit_container(msg);
                goto exit_entry;
            }

            const char *uuid = NULL;
            while ((ret = sd_bus_message_read(msg, "s", &uuid)) > 0) {
                if (restore_array_add_uuid(&restore_data->outputs, uuid)) {
                    kywc_log(KYWC_DEBUG, "restore output: %s", uuid);
                }
            }

            sd_bus_message_exit_container(msg);
            sd_bus_message_exit_container(msg);
        } else if (strcmp(key, "windows") == 0) {
            ret = sd_bus_message_enter_container(msg, 'v', "as");
            if (ret < 0) {
                kywc_log(KYWC_ERROR, "Failed to enter VARIANT 'as' for windows");
                goto exit_entry;
            }
            ret = sd_bus_message_enter_container(msg, 'a', "s");
            if (ret < 0) {
                kywc_log(KYWC_ERROR, "Failed to enter ARRAY 's' for windows");
                sd_bus_message_exit_container(msg);
                goto exit_entry;
            }

            const char *uuid = NULL;
            while ((ret = sd_bus_message_read(msg, "s", &uuid)) > 0) {
                if (restore_array_add_uuid(&restore_data->toplevels, uuid)) {
                    kywc_log(KYWC_DEBUG, "restore toplevel: %s", uuid);
                }
            }

            sd_bus_message_exit_container(msg);
            sd_bus_message_exit_container(msg);
        } else {
            kywc_log(KYWC_WARN, "SelectDevices: unknown option %s", key);
            sd_bus_message_skip(msg, "v");
        }

    exit_entry:
        ret = sd_bus_message_exit_container(msg);
        if (ret < 0) {
            break;
        }
    }

    sd_bus_message_exit_container(msg);
exit_inner_variant:
    sd_bus_message_exit_container(msg);
exit_struct:
    sd_bus_message_exit_container(msg);
exit_variant:
    sd_bus_message_exit_container(msg);
    return ret >= 0;
}

static int handle_select_sources(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "ScreenCast: SelectSources called");

    char *handle, *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "oos", &handle, &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    // check session
    struct portal_context *ctx = data;
    int session_type = portal_get_session_type(ctx, session_handle);
    if (session_type < 0) {
        kywc_log(KYWC_ERROR, "Tried to select sources on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    uint32_t types = SOURCE_TYPE_MONITOR; // default to monitor
    int multiple = 0;
    uint32_t cursor_mode = CURSOR_MODE_HIDDEN; // The default cursor mode is ‘Hidden’.
    uint32_t persist_mode = SCREENCAST_PERSIST_MODE_NONE;

    int inner_ret = 0;
    char *key;
    struct screencast_restore_data *restore_data = NULL;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "types") == 0) {
            sd_bus_message_read(msg, "v", "u", &types);
            kywc_log(KYWC_DEBUG, "SelectSources: option types: %d", types);
        } else if (strcmp(key, "multiple") == 0) {
            sd_bus_message_read(msg, "v", "b", &multiple);
            kywc_log(KYWC_DEBUG, "SelectSources: option multiple: %d", multiple);
        } else if (strcmp(key, "cursor_mode") == 0) {
            sd_bus_message_read(msg, "v", "u", &cursor_mode);
            kywc_log(KYWC_DEBUG, "SelectSources: option cursor_mode: %d", cursor_mode);
        } else if (strcmp(key, "persist_mode") == 0) {
            sd_bus_message_read(msg, "v", "u", &persist_mode);
            kywc_log(KYWC_DEBUG, "SelectSources: option persist_mode: %d", persist_mode);
        } else if (strcmp(key, "restore_data") == 0) {
            restore_data = calloc(1, sizeof(*restore_data));
            if (restore_data) {
                wl_array_init(&restore_data->outputs);
                wl_array_init(&restore_data->toplevels);
                if (!get_restore_data(msg, restore_data)) {
                    screencast_restore_data_free(restore_data);
                    restore_data = NULL;
                    kywc_log(KYWC_ERROR, "RestoreData: get_restore_data failed");
                }
            }
        } else {
            kywc_log(KYWC_WARN, "SelectSources: unknown option %s", key);
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
             "SelectSources by handle %s session_handle %s app_id %s, types %d multiple %d "
             "cursor_mode %d persist_mode %d",
             handle, session_handle, app_id, types, multiple, cursor_mode, persist_mode);

    if ((types & ctx->screencast.sources) == 0 || (cursor_mode & ctx->screencast.cursors) == 0) {
        kywc_log(KYWC_ERROR, "SelectSources: unsupport source types %u or cursor mode %u", types,
                 cursor_mode);
        return -PORTAL_RESPONSE_CANCELLED;
    }

    if (session_type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        struct portal_session *session =
            portal_session_get(&ctx->remote_desktop.sessions, session_handle);
        session->remote_desktop.screen_share_enabled = true;
        session->remote_desktop.select.multiple = multiple;
        session->remote_desktop.select.source_types = types & ctx->screencast.sources;
        session->remote_desktop.select.cursor_mode = cursor_mode & ctx->screencast.cursors;
        wl_list_init(&session->remote_desktop.streams);
    } else if (session_type == PORTAL_SESSION_TYPE_SCREENCAST) {
        struct portal_session *session =
            portal_session_get(&ctx->screencast.sessions, session_handle);
        session->screencast.select.multiple = multiple;
        session->screencast.select.source_types = types & ctx->screencast.sources;
        session->screencast.select.cursor_mode = cursor_mode & ctx->screencast.cursors;
        session->screencast.restore_data = restore_data;
        session->screencast.restore_data_destroy = screencast_restore_data_free;
        session->screencast.persist_mode = persist_mode;
    }

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static void add_restore_stream(sd_bus_message **reply, struct portal_session *session,
                               uint32_t source_type)
{
    if (source_type == SOURCE_TYPE_MONITOR) {
        sd_bus_message_append_basic(*reply, 's', "outputs");
    } else if (source_type == SOURCE_TYPE_WINDOW) {
        sd_bus_message_append_basic(*reply, 's', "windows");
    }

    sd_bus_message_open_container(*reply, 'v', "as");
    sd_bus_message_open_container(*reply, 'a', "s");

    int count = 0;
    struct portal_stream *stream;
    wl_list_for_each(stream, &session->screencast.streams, link) {
        if (stream->source_type == source_type) {
            count++;
        }
    }

    sd_bus_message_append(*reply, "i", count);

    wl_list_for_each(stream, &session->screencast.streams, link) {
        if (stream->source_type != source_type) {
            continue;
        }

        sd_bus_message_append(*reply, "s", stream->stream->source_uuid);
    }
    sd_bus_message_close_container(*reply);
    sd_bus_message_close_container(*reply);
}

void screencast_restore_send(sd_bus_message **reply, struct portal_session *session)
{
    sd_bus_message_append(*reply, "{sv}", "persist_mode", "u", session->screencast.persist_mode);

    if (session->screencast.persist_mode != SCREENCAST_PERSIST_MODE_NONE) {
        sd_bus_message_open_container(*reply, 'e', "sv");
        sd_bus_message_append_basic(*reply, 's', "restore_data");
        sd_bus_message_open_container(*reply, 'v', "(suv)");
        sd_bus_message_open_container(*reply, 'r', "suv");
        sd_bus_message_append(*reply, "s", "wlcom");
        sd_bus_message_append(*reply, "u", session->ctx->screencast.version);

        sd_bus_message_open_container(*reply, 'v', "a{sv}");
        sd_bus_message_open_container(*reply, 'a', "{sv}");
        sd_bus_message_open_container(*reply, 'e', "sv");

        add_restore_stream(reply, session, SOURCE_TYPE_MONITOR);
        add_restore_stream(reply, session, SOURCE_TYPE_WINDOW);

        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);

        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);
    }
}

static inline enum kywc_stream_cursor_mode get_cursor_mode(uint32_t mode)
{
    return mode == CURSOR_MODE_EMBEDDED ? KYWC_STREAM_CURSOR_MODE_EMBEDDED
                                        : KYWC_STREAM_CURSOR_MODE_HIDDEN;
}

static bool restore_data_check(struct portal_session *session)
{

    struct screencast_restore_data *restore_data = session->screencast.restore_data;
    if (!restore_data) {
        return false;
    }

    if (restore_data->outputs.size == 0 && restore_data->toplevels.size == 0) {
        return false;
    }

    if (restore_data->version != session->ctx->screencast.version) {
        return false;
    }

    if (strcmp(restore_data->vendor_name, "wlcom") != 0) {
        return false;
    }

    char **uuid;
    wl_array_for_each(uuid, &restore_data->outputs) {
        if (!kywc_context_find_output(session->ctx->wayland.kywc, *uuid)) {
            return false;
        }
    }

    wl_array_for_each(uuid, &restore_data->toplevels) {
        if (!kywc_context_find_toplevel(session->ctx->wayland.kywc, *uuid)) {
            return false;
        }
    }

    return true;
}

static int handle_start(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "ScreenCast: Start called");

    char *handle, *session_handle, *app_id, *parent_window;
    int ret = sd_bus_message_read(msg, "ooss", &handle, &session_handle, &app_id, &parent_window);
    if (ret < 0) {
        return ret;
    }

    // check session
    struct portal_context *ctx = data;
    struct portal_session *session = portal_session_get(&ctx->screencast.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to start on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "Start by handle %s session_handle %s app_id %s parent_window %s", handle,
             session_handle, app_id, parent_window);

    if (session->screencast.persist_mode != SCREENCAST_PERSIST_MODE_NONE) {
        if (restore_data_check(session)) {
            kywc_log(KYWC_DEBUG, "screencast restore data check passed");
            if (!screencast_restore_streams_from_data(session, sd_bus_message_ref(msg))) {
                return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
            }
            return 1;
        }
    }

    portal_stream_select(session, app_id, NULL, sd_bus_message_ref(msg));

    return 1;
}

static const sd_bus_vtable screencast_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("AvailableSourceTypes", "u", NULL,
                    offsetof(struct portal_context, screencast.sources),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_PROPERTY("AvailableCursorModes", "u", NULL,
                    offsetof(struct portal_context, screencast.cursors),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_PROPERTY("version", "u", NULL, offsetof(struct portal_context, screencast.version),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_METHOD("CreateSession", "oosa{sv}", "ua{sv}", handle_create_session,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SelectSources", "oosa{sv}", "ua{sv}", handle_select_sources,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Start", "oossa{sv}", "ua{sv}", handle_start, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

bool portal_screencast_init(struct portal_context *ctx)
{
    ctx->screencast.version = 5;
    ctx->screencast.sources = SOURCE_TYPE_MONITOR | SOURCE_TYPE_WINDOW;
    ctx->screencast.cursors = CURSOR_MODE_HIDDEN | CURSOR_MODE_EMBEDDED;
    wl_list_init(&ctx->screencast.sessions);

    int ret = sd_bus_add_object_vtable(ctx->bus, NULL, object_path, interface_name,
                                       screencast_vtable, ctx);
    if (ret < 0) {
        return false;
    }

    return true;
}

void portal_screencast_fini(struct portal_context *ctx)
{
    if (ctx->screencast.version == 0) {
        return;
    }

    struct portal_session *session, *tmp;
    wl_list_for_each_safe(session, tmp, &ctx->screencast.sessions, link) {
        struct screencast_restore_data *restore_data = session->screencast.restore_data;
        if (restore_data) {
            session->screencast.restore_data_destroy(restore_data);
        }
        session->screencast.restore_data = NULL;
        portal_session_destroy(session);
    }
}
