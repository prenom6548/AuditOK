// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _PRIMARY_SELECTION_H_
#define _PRIMARY_SELECTION_H_

#include <wlr/types/wlr_seat.h>

#include "util/client.h"

struct seat;

/**
 * A source is the sending side of a selection.
 */
struct primary_selection_source {
    const struct primary_selection_source_impl *impl;

    // source metadata
    struct wl_array mime_types;

    // the owner client, filled at init
    struct client client;

    struct {
        struct wl_signal destroy;
    } events;
};

/**
 * A data source implementation. Only the `send` function is mandatory.
 */
struct primary_selection_source_impl {
    void (*send)(struct primary_selection_source *source, const char *mime_type, int fd);
    void (*destroy)(struct primary_selection_source *source);

    /* called at init to fill the client info of the source */
    void (*client)(struct primary_selection_source *source, struct client *client);
};

void primary_selection_source_init(struct primary_selection_source *source,
                                   const struct primary_selection_source_impl *impl);

void primary_selection_source_destroy(struct primary_selection_source *source);

void primary_selection_source_send(struct primary_selection_source *source, const char *mime_type,
                                   int fd);

/* return true if the destination client is allowed to read the source */
bool primary_selection_source_allow_read(struct seat *seat, struct primary_selection_source *source,
                                         struct client *destination);

/* return true if the source owner is allowed to write the primary selection */
bool primary_selection_source_allow_write(struct seat *seat,
                                          struct primary_selection_source *source);

void seat_request_set_primary_selection(struct seat *seat, struct wlr_seat_client *client,
                                        struct primary_selection_source *source, uint32_t serial);

void seat_set_primary_selection(struct seat *seat, struct primary_selection_source *source,
                                uint32_t serial);

#endif /* _PRIMARY_SELECTION_H_ */
