// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _STREAM_H_
#define _STREAM_H_

#include "portal.h"

enum source_type {
    SOURCE_TYPE_MONITOR = 1,
    SOURCE_TYPE_WINDOW = 2,
    SOURCE_TYPE_VIRTUAL = 4,
};

enum cursor_mode {
    CURSOR_MODE_HIDDEN = 1,
    CURSOR_MODE_EMBEDDED = 2,
    CURSOR_MODE_METADATA = 4,
};

struct screencast_restore_data {
    char *vendor_name;
    uint32_t version;
    struct wl_array outputs;
    struct wl_array toplevels;
};

void portal_stream_select(struct portal_session *session, char *app_id,
                          struct portal_request *request, sd_bus_message *msg);

bool screencast_restore_streams_from_data(struct portal_session *session, sd_bus_message *msg);

void portal_stream_destroy(struct portal_session *session);

void append_streams_entry(struct wl_list *streams, sd_bus_message **reply);

void remote_desktop_restore_send(sd_bus_message **reply, struct portal_session *session);

void screencast_restore_send(sd_bus_message **reply, struct portal_session *session);

struct portal_stream *portal_stream_get_from_remote_desktop(struct portal_session *session,
                                                            uint32_t node);

#endif /* _STREAM_H_ */
