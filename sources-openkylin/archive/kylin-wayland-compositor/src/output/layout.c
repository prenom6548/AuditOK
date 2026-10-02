// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <wlr/types/wlr_output_layout.h>

#include "output_p.h"
#include "util/macros.h"

static void output_edge_position(struct output *output, enum layout_edge edge, int *lx, int *ly)
{
    struct kywc_box *geo = &output->geometry;

    switch (edge) {
    case LAYOUT_EDGE_TOP:
        *lx = geo->x + geo->width / 2;
        *ly = geo->y - 1;
        break;
    case LAYOUT_EDGE_BOTTOM:
        *lx = geo->x + geo->width / 2;
        *ly = geo->y + geo->height;
        break;
    case LAYOUT_EDGE_LEFT:
        *lx = geo->x - 1;
        *ly = geo->y + geo->height / 2;
        break;
    case LAYOUT_EDGE_RIGHT:
        *lx = geo->x + geo->width;
        *ly = geo->y + geo->height / 2;
        break;
    }
}

bool output_at_layout_edge(struct output *output, enum layout_edge edge)
{
    int lx = 0, ly = 0;
    output_edge_position(output, edge, &lx, &ly);

    struct wlr_output_layout *layout = output->manager->server->layout;
    return !wlr_output_layout_contains_point(layout, NULL, lx, ly);
}

static struct output *output_get_prev_usable_output(struct output *output)
{
    struct output *find_output = NULL;
    wl_list_for_each_reverse(find_output, &output->link, link) {
        if (&find_output->link == &output_manager->outputs) {
            continue;
        }
        if (find_output->base.state.enabled) {
            return find_output;
        }
    }
    return NULL;
}

static struct output *output_get_next_usable_output(struct output *output)
{
    struct output *find_output = NULL;
    wl_list_for_each(find_output, &output->link, link) {
        if (&find_output->link == &output_manager->outputs) {
            continue;
        }
        if (find_output->base.state.enabled) {
            return find_output;
        }
    }
    return NULL;
}

struct output *output_find_specified_output(struct output *output, enum layout_edge edge)
{
    struct output *find_output = NULL;
    switch (edge) {
    case LAYOUT_EDGE_LEFT:
        find_output = output_get_prev_usable_output(output);
        break;
    case LAYOUT_EDGE_RIGHT:
        find_output = output_get_next_usable_output(output);
        break;
    case LAYOUT_EDGE_TOP:
    case LAYOUT_EDGE_BOTTOM:
        break;
    }
    return find_output;
}

bool output_state_is_mirror_mode(void)
{
    if (wl_list_length(&output_manager->outputs) < 2) {
        return false;
    }

    struct kywc_box *geometry = NULL;
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (!output->base.state.enabled) {
            continue;
        }
        if (!geometry) {
            geometry = &output->geometry;
            continue;
        }
        if (!kywc_box_equal(geometry, &output->geometry)) {
            return false;
        }
    }

    return true;
}

struct kywc_output *kywc_output_at_point(double lx, double ly)
{
    struct wlr_output_layout *layout = output_manager->server->layout;
    struct wlr_output *wlr_output = NULL;
    double closest_x, closest_y;

    wlr_output_layout_closest_point(layout, wlr_output, lx, ly, &closest_x, &closest_y);
    wlr_output = wlr_output_layout_output_at(layout, closest_x, closest_y);

    return wlr_output ? &output_from_wlr_output(wlr_output)->base : NULL;
}

void output_layout_get_size(int *width, int *height)
{
    struct wlr_box box;
    wlr_output_layout_get_box(output_manager->server->layout, NULL, &box);

    if (width) {
        *width = box.width;
    }
    if (height) {
        *height = box.height;
    }
}

void output_layout_get_workarea(struct wlr_box *box)
{
    wlr_output_layout_get_box(output_manager->server->layout, NULL, box);

    /* layout edges */
    int layout_left = MAX(0, box->x);
    int layout_right = MAX(0, box->x + box->width);
    int layout_top = MAX(0, box->y);
    int layout_bottom = MAX(0, box->y + box->height);

    /* workarea edges */
    int workarea_left = layout_left;
    int workarea_right = layout_right;
    int workarea_top = layout_top;
    int workarea_bottom = layout_bottom;

    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (!output->base.state.enabled) {
            continue;
        }

        /* output edges */
        int output_left = output->geometry.x;
        int output_right = output_left + output->geometry.width;
        int output_top = output->geometry.y;
        int output_bottom = output_top + output->geometry.height;

        /* output usable edges */
        int usable_left = output->usable_area.x;
        int usable_right = usable_left + output->usable_area.width;
        int usable_top = output->usable_area.y;
        int usable_bottom = usable_top + output->usable_area.height;

        /**
         * Only adjust workarea edges for output edges that are
         * aligned with outer edges of layout
         */
        if (output_left == layout_left) {
            workarea_left = MAX(workarea_left, usable_left);
        }
        if (output_right == layout_right) {
            workarea_right = MIN(workarea_right, usable_right);
        }
        if (output_top == layout_top) {
            workarea_top = MAX(workarea_top, usable_top);
        }
        if (output_bottom == layout_bottom) {
            workarea_bottom = MIN(workarea_bottom, usable_bottom);
        }
    }

    box->x = workarea_left;
    box->y = workarea_top;
    box->width = workarea_right - workarea_left;
    box->height = workarea_bottom - workarea_top;
}

static bool output_manager_is_sorted(void)
{
    int prev_x = 0, prev_y = 0;
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        int distance_x = output->geometry.x - prev_x;
        int distance_y = output->geometry.y - prev_y;
        if (distance_x < 0 || (distance_x == 0 && distance_y < 0)) {
            return false;
        }
        prev_x = output->geometry.x;
        prev_y = output->geometry.y;
    }
    return true;
}

void output_manager_sort_outputs(void)
{
    if (output_manager_is_sorted()) {
        return;
    }
    struct output *output, *prev_output, *tmp;
    wl_list_for_each_safe(output, tmp, &output_manager->outputs, link) {
        wl_list_for_each_reverse(prev_output, &output->link, link) {
            if (&prev_output->link == &output_manager->outputs) {
                wl_list_remove(&output->link);
                wl_list_insert(&output_manager->outputs, &output->link);
                break;
            }
            // sort
            int distance_x = output->geometry.x - prev_output->geometry.x;
            int distance_y = output->geometry.y - prev_output->geometry.y;
            if (distance_x < 0 || (distance_x == 0 && distance_y < 0)) {
                continue;
            }
            if (distance_x > 0 || (distance_x == 0 && distance_y > 0)) {
                wl_list_remove(&output->link);
                wl_list_insert(&prev_output->link, &output->link);
                break;
            }
        }
    }
}

void output_set_layout(struct output *output)
{
    struct server *server = output_manager->server;
    struct wlr_output *wlr_output = output->wlr_output;
    struct kywc_output_state *state = &output->pending_state;

    /* if output disabled, skip output_layout_add */
    if (!state->enabled) {
        if (output->layout_output) {
            wlr_output_layout_remove(server->layout, wlr_output);
            output->layout_output = NULL;
        }
        return;
    }

    /* force to reconfigure even if have loutput */
    if (state->lx == -1 || state->ly == -1) {
        output->layout_output = wlr_output_layout_add_auto(server->layout, wlr_output);
        if (output->layout_output) {
            output->layout_output->auto_configured = false;
        }
    } else {
        if (!output->layout_output || output->layout_output->x != state->lx ||
            output->layout_output->y != state->ly) {
            output->layout_output =
                wlr_output_layout_add(server->layout, wlr_output, state->lx, state->ly);
        }
    }
}
