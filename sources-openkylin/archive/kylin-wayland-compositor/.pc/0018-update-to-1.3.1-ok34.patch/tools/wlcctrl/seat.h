// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <wayland-client.h>

typedef struct _seat {
    struct wl_seat *wl_seat;
    struct wl_list link;

    uint32_t id;
    uint32_t capabilities;
    char *name;
} seat;

seat *new_seat(struct wl_seat *wl_seat, uint32_t id);

void seat_destroy(seat *seat);
