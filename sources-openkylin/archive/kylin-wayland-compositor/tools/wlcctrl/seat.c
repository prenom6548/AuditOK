// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "seat.h"

static void seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
    seat *seat = data;

    seat->capabilities = capabilities;
}

static void seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    seat *seat = data;
    seat->name = strdup(name);
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
};

seat *new_seat(struct wl_seat *wl_seat, uint32_t id)
{
    seat *seat = calloc(1, sizeof(*seat));
    if (!seat) {
        return NULL;
    }

    seat->wl_seat = wl_seat;
    seat->id = id;

    wl_seat_add_listener(wl_seat, &seat_listener, seat);

    return seat;
}

void seat_destroy(seat *seat)
{
    if (!seat) {
        return;
    }

    wl_seat_destroy(seat->wl_seat);
    free(seat->name);
    free(seat);
}
