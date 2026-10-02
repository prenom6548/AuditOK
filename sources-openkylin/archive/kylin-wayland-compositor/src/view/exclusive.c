// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "output.h"
#include "scene/surface.h"
#include "view/view.h"

struct exclusive_zone_output {
    struct wl_list link;
    struct exclusive_zone *zone;
    struct kywc_output *output;
    struct wl_listener update_usable_area;
};

struct exclusive_zone {
    struct wl_list outputs;

    struct wlr_surface *surface;
    struct wl_listener output_enter;
    struct wl_listener output_leave;

    struct view *view;
    struct wl_listener view_unmap;
    struct wl_listener view_minimize;
    struct wl_listener view_size;
    struct wl_listener view_position;

    enum kywc_edges edge;
    bool late_update, enabled;
};

static void exclusive_zone_update_edge(struct exclusive_zone *zone)
{
    struct view *view = zone->view;
    struct kywc_output *output = view->output;

    struct kywc_box geo;
    kywc_output_effective_geometry(output, &geo);

    struct kywc_box *view_geo = &view->base.geometry;
    int mid_x = view_geo->x + view_geo->width / 2;
    int mid_y = view_geo->y + view_geo->height / 2;

    if (view_geo->width > view_geo->height) {
        if (mid_y - geo.y < geo.y + geo.height - mid_y) {
            zone->edge = KYWC_EDGE_TOP;
        } else {
            zone->edge = KYWC_EDGE_BOTTOM;
        }
    } else {
        if (mid_x - geo.x < geo.x + geo.width - mid_x) {
            zone->edge = KYWC_EDGE_LEFT;
        } else {
            zone->edge = KYWC_EDGE_RIGHT;
        }
    }
}

static void exclusive_zone_output_destroy(struct exclusive_zone_output *output)
{
    wl_list_remove(&output->link);
    wl_list_remove(&output->update_usable_area.link);
    output_update_usable_area(output->output);
    free(output);
}

static void handle_output_update_usable_area(struct wl_listener *listener, void *data)
{
    struct exclusive_zone_output *output = wl_container_of(listener, output, update_usable_area);
    struct exclusive_zone *zone = output->zone;
    struct kywc_box *usable_area = data;

    if (zone->view->impl->update_usable_area) {
        zone->view->impl->update_usable_area(zone->view, output->output, usable_area, zone->edge);
    }
}

static struct exclusive_zone_output *exclusive_zone_output_create(struct exclusive_zone *zone,
                                                                  struct kywc_output *kywc_output)
{
    struct exclusive_zone_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return NULL;
    }

    output->zone = zone;
    wl_list_insert(&zone->outputs, &output->link);

    output->output = kywc_output;
    output->update_usable_area.notify = handle_output_update_usable_area;
    output_add_update_usable_area_listener(kywc_output, &output->update_usable_area,
                                           zone->late_update);

    output_update_usable_area(output->output);

    return output;
}

static struct exclusive_zone_output *exclusive_zone_get_output(struct exclusive_zone *zone,
                                                               struct kywc_output *kywc_output)
{
    struct exclusive_zone_output *output;
    wl_list_for_each(output, &zone->outputs, link) {
        if (output->output == kywc_output) {
            return output;
        }
    }
    return NULL;
}

static void handle_output_enter(struct wl_listener *listener, void *data)
{
    struct exclusive_zone *zone = wl_container_of(listener, zone, output_enter);
    struct scene_output *scene_output = data;
    struct kywc_output *kywc_output = &output_from_wlr_output(scene_output->output)->base;

    if (exclusive_zone_get_output(zone, kywc_output)) {
        kywc_log(KYWC_ERROR, "%s enter output multiple times", zone->view->base.app_id);
        return;
    }

    assert(zone->view->base.mapped);
    exclusive_zone_output_create(zone, kywc_output);
}

static void handle_output_leave(struct wl_listener *listener, void *data)
{
    struct exclusive_zone *zone = wl_container_of(listener, zone, output_leave);
    struct scene_output *scene_output = data;
    struct kywc_output *kywc_output = &output_from_wlr_output(scene_output->output)->base;

    struct exclusive_zone_output *output = exclusive_zone_get_output(zone, kywc_output);
    if (!output) {
        kywc_log(KYWC_ERROR, "%s leave an output nerver entered", zone->view->base.app_id);
        return;
    }

    exclusive_zone_output_destroy(output);
}

static void exclusive_zone_set_enabled(struct exclusive_zone *zone, bool enabled)
{
    if (zone->enabled == enabled) {
        return;
    }
    zone->enabled = enabled;

    if (enabled) {
        struct scene_buffer *buffer = scene_buffer_try_from_surface(zone->view->surface);
        assert(buffer);
        wl_signal_add(&buffer->events.output_enter, &zone->output_enter);
        wl_signal_add(&buffer->events.output_leave, &zone->output_leave);
        wl_signal_add(&zone->view->base.events.size, &zone->view_size);
        wl_signal_add(&zone->view->base.events.position, &zone->view_position);

        exclusive_zone_update_edge(zone);

        // add all outputs to zone
        struct wlr_surface_output *surface_output;
        wl_list_for_each(surface_output, &zone->surface->current_outputs, link) {
            exclusive_zone_output_create(zone,
                                         &output_from_wlr_output(surface_output->output)->base);
        }
        return;
    }

    wl_list_remove(&zone->output_enter.link);
    wl_list_init(&zone->output_enter.link);
    wl_list_remove(&zone->output_leave.link);
    wl_list_init(&zone->output_leave.link);
    wl_list_remove(&zone->view_size.link);
    wl_list_init(&zone->view_size.link);
    wl_list_remove(&zone->view_position.link);
    wl_list_init(&zone->view_position.link);

    struct exclusive_zone_output *output, *tmp;
    wl_list_for_each_safe(output, tmp, &zone->outputs, link) {
        exclusive_zone_output_destroy(output);
    }
}

static void exclusive_zone_destroy(struct exclusive_zone *zone)
{
    wl_list_remove(&zone->view_unmap.link);
    wl_list_remove(&zone->view_minimize.link);

    exclusive_zone_set_enabled(zone, false);

    free(zone);
}

static void handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct exclusive_zone *zone = wl_container_of(listener, zone, view_unmap);
    zone->view->exclusive_zone = NULL;
    exclusive_zone_destroy(zone);
}

static void exclusive_zone_update(struct exclusive_zone *zone)
{
    exclusive_zone_update_edge(zone);

    struct exclusive_zone_output *output;
    wl_list_for_each(output, &zone->outputs, link) {
        output_update_usable_area(output->output);
    }
}

static void handle_view_size(struct wl_listener *listener, void *data)
{
    struct exclusive_zone *zone = wl_container_of(listener, zone, view_size);
    exclusive_zone_update(zone);
}

static void handle_view_position(struct wl_listener *listener, void *data)
{
    struct exclusive_zone *zone = wl_container_of(listener, zone, view_position);
    exclusive_zone_update(zone);
}

static void handle_view_minimize(struct wl_listener *listener, void *data)
{
    struct exclusive_zone *zone = wl_container_of(listener, zone, view_minimize);
    struct kywc_view *view = &zone->view->base;
    exclusive_zone_set_enabled(zone, !view->minimized);
}

static struct exclusive_zone *exclusive_zone_create(struct view *view)
{
    if (!view->base.mapped) {
        kywc_log(KYWC_ERROR, "add unmapped view(%s) to exclusive zone", view->base.app_id);
        return NULL;
    }

    struct exclusive_zone *zone = calloc(1, sizeof(*zone));
    if (!zone) {
        return NULL;
    }

    zone->late_update = false;

    zone->surface = view->surface;
    wl_list_init(&zone->outputs);
    zone->output_enter.notify = handle_output_enter;
    wl_list_init(&zone->output_enter.link);
    zone->output_leave.notify = handle_output_leave;
    wl_list_init(&zone->output_leave.link);

    zone->view = view;
    zone->view_unmap.notify = handle_view_unmap;
    wl_signal_add(&view->base.events.unmap, &zone->view_unmap);
    zone->view_minimize.notify = handle_view_minimize;
    wl_signal_add(&view->base.events.minimize, &zone->view_minimize);
    zone->view_size.notify = handle_view_size;
    zone->view_position.notify = handle_view_position;
    wl_list_init(&zone->view_size.link);
    wl_list_init(&zone->view_position.link);

    handle_view_minimize(&zone->view_minimize, NULL);

    return zone;
}

void view_set_exclusive(struct view *view, bool exclusive)
{
    if (exclusive == !!view->exclusive_zone) {
        return;
    }

    if (exclusive) {
        view->exclusive_zone = exclusive_zone_create(view);
    } else {
        exclusive_zone_destroy(view->exclusive_zone);
        view->exclusive_zone = NULL;
    }
}
