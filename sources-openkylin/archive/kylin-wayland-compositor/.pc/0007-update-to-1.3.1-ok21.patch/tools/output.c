// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "output.h"

static void output_handle_geometry(void *data, struct wl_output *wl_output, int x, int y,
                                   int physical_width, int physical_height, int subpixel,
                                   const char *make, const char *model, int transform)
{
    output *output = data;
    output->x = x;
    output->y = y;
    output->transform = transform;
}

static void output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags, int width,
                               int height, int refresh)
{
    output *output = data;
    output->width = width;
    output->height = height;
}

static void output_handle_done(void *data, struct wl_output *wl_output) {}

static void output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {}

static void output_handle_name(void *data, struct wl_output *wl_output, const char *name)
{
    output *output = data;
    output->name = strdup(name);
}

static void output_handle_description(void *data, struct wl_output *wl_output,
                                      const char *description)
{
}

static struct wl_output_listener output_listener = {
    .geometry = output_handle_geometry,
    .mode = output_handle_mode,
    .done = output_handle_done,
    .scale = output_handle_scale,
    .name = output_handle_name,
    .description = output_handle_description,
};

output *output_new(struct wl_output *wl_output, int id)
{
    output *output = calloc(1, sizeof(*output));
    if (!output) {
        return NULL;
    }

    output->wl_output = wl_output;
    output->id = id;

    wl_output_add_listener(output->wl_output, &output_listener, output);

    return output;
}

void output_destroy(output *output)
{
    wl_output_release(output->wl_output);
    free(output->name);
    free(output);
}

output *output_find_by_id(struct wl_list *list, uint32_t id)
{
    output *output;
    wl_list_for_each(output, list, link) {
        if (output->id == id) {
            return output;
        }
    }

    return NULL;
}

output *output_find_by_name(struct wl_list *list, const char *name)
{
    output *output;
    wl_list_for_each(output, list, link) {
        if (output->name && strcmp(output->name, name) == 0) {
            return output;
        }
    }

    return NULL;
}

void output_list_destroy(struct wl_list *list)
{
    output *output, *tmp;
    wl_list_for_each_safe(output, tmp, list, link) {
        wl_list_remove(&output->link);
        output_destroy(output);
    }
}

void output_transform_coord(const output *output, uint32_t src_x, uint32_t src_y, uint32_t *dst_x,
                            uint32_t *dst_y)
{
    switch (output->transform) {
    case WL_OUTPUT_TRANSFORM_NORMAL:
        *dst_x = src_x;
        *dst_y = src_y;
        break;
    case WL_OUTPUT_TRANSFORM_90:
        *dst_x = src_y;
        *dst_y = output->height - src_x;
        break;
    case WL_OUTPUT_TRANSFORM_180:
        *dst_x = output->width - src_x;
        *dst_y = output->height - src_y;
        break;
    case WL_OUTPUT_TRANSFORM_270:
        *dst_x = output->width - src_y;
        *dst_y = src_x;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED:
        *dst_x = output->width - src_x;
        *dst_y = src_y;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        *dst_x = src_y;
        *dst_y = src_x;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        *dst_x = src_x;
        *dst_y = output->height - src_y;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        *dst_x = output->width - src_y;
        *dst_y = output->height - src_x;
        break;
    default:
        break;
    }
}
