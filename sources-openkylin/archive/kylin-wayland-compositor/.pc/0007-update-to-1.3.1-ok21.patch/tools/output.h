// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <wayland-client.h>

typedef struct _output {
    struct wl_list link;
    struct wl_output *wl_output;
    char *name;
    uint32_t id;
    int x, y, width, height;
    enum wl_output_transform transform;
} output;

output *output_new(struct wl_output *wl_output, int id);

void output_destroy(output *output);

output *output_find_by_id(struct wl_list *list, uint32_t id);

output *output_find_by_name(struct wl_list *list, const char *name);

void output_list_destroy(struct wl_list *list);

void output_transform_coord(const output *output, uint32_t src_x, uint32_t src_y, uint32_t *dst_x,
                            uint32_t *dst_y);

#endif
