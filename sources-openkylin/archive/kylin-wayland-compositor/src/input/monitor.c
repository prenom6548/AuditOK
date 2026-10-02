// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_touch.h>

#include <kywc/log.h>

#include "input_p.h"
#include "output.h"
#include "server.h"

struct input_monitor {
    struct input_manager *input_manager;

    struct wl_listener configured;
    struct wl_listener layout_damage;

    struct wl_listener new_seat;
    struct wl_listener server_destroy;
};

static void seat_rebase_cursor(struct seat *seat, bool initial)
{
    // lx/ly might change when pick output
    double lx = seat->cursor->lx;
    double ly = seat->cursor->ly;
    struct output *output = input_current_output(seat);
    if (output && !output->base.destroying &&
        (!seat->manager->server->start || initial ||
         !kywc_output_contains_point(&output->base, lx, ly))) {
        struct kywc_box geo = output->geometry;
        geo.x += geo.width / 2;
        geo.y += geo.height / 2;

        cursor_move(seat->cursor, NULL, geo.x, geo.y, false, false);
    }

    cursor_rebase(seat->cursor);
}

static void apply_input_mapping(struct input_manager *manager)
{
    /* use primary output as a fallback */
    struct kywc_output *output = kywc_output_get_primary();

    struct input *input;
    wl_list_for_each(input, &manager->inputs, link) {
        if (input->state.mapped_to_output) {
            continue;
        }

        const char *mapped_to_output = NULL;
        if (input->prop.type == WLR_INPUT_DEVICE_POINTER) {
            struct wlr_pointer *pointer = wlr_pointer_from_input_device(input->wlr_input);
            mapped_to_output = pointer->output_name;
        } else if (input->prop.type == WLR_INPUT_DEVICE_TOUCH) {
            struct wlr_touch *touch = wlr_touch_from_input_device(input->wlr_input);
            mapped_to_output = touch->output_name;
            if (!mapped_to_output && output) {
                mapped_to_output = output->name;
            }
        }
        if (!mapped_to_output) {
            continue;
        }

        struct input_state state = input->state;
        state.mapped_to_output = mapped_to_output;
        input_set_state(input, &state);
    }
}

static void handle_configured(struct wl_listener *listener, void *data)
{
    struct configure_event *event = data;
    if (event->type == CONFIGURE_TYPE_NONE) {
        return;
    }

    struct input_monitor *input_monitor = wl_container_of(listener, input_monitor, configured);
    bool init_configure = event->type == CONFIGURE_TYPE_INIT;
    if (init_configure) {
        /* apply input mapped output when initial output configure */
        apply_input_mapping(input_monitor->input_manager);
    }

    struct seat *seat;
    wl_list_for_each(seat, &input_monitor->input_manager->seats, link) {
        seat_rebase_cursor(seat, init_configure);
    }
}

static void handle_layout_damage(struct wl_listener *listener, void *data)
{
    struct input_monitor *input_monitor = wl_container_of(listener, input_monitor, layout_damage);
    pixman_region32_t *damage_region = data;

    struct seat *seat;
    wl_list_for_each(seat, &input_monitor->input_manager->seats, link) {
        /* skip motion when has grab */
        if (seat->pointer_grab && seat->pointer_grab->interface->motion) {
            continue;
        }
        if (seat_is_dragging(seat) || seat->cursor->hold_mode) {
            continue;
        }
        double lx = seat->cursor->lx;
        double ly = seat->cursor->ly;
        if (!pixman_region32_contains_point(damage_region, lx, ly, NULL)) {
            continue;
        }
        // check node in cursor position
        struct scene_node *node = scene_node_at(&seat->scene->tree.node, lx, ly, NULL, NULL);
        if (node != seat->cursor->hover.node) {
            cursor_rebase(seat->cursor);
        }
    }
}

static void handle_seat_idle(struct idle *idle, void *data) {};

static void handle_seat_resume(struct idle *idle, void *data)
{
    output_manager_power_outputs(true);
}

static void handle_new_seat(struct wl_listener *listener, void *data)
{
    struct input_monitor *input_monitor = wl_container_of(listener, input_monitor, new_seat);
    struct seat *seat = data;

    seat_rebase_cursor(seat, true);
    idle_manager_add_idle(seat, false, 0, handle_seat_idle, handle_seat_resume, NULL, NULL);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct input_monitor *input_monitor = wl_container_of(listener, input_monitor, server_destroy);

    wl_list_remove(&input_monitor->server_destroy.link);
    wl_list_remove(&input_monitor->new_seat.link);
    wl_list_remove(&input_monitor->configured.link);
    wl_list_remove(&input_monitor->layout_damage.link);

    free(input_monitor);
}

struct input_monitor *input_monitor_create(struct input_manager *input_manager)
{
    struct input_monitor *input_monitor = calloc(1, sizeof(*input_monitor));
    if (!input_monitor) {
        return NULL;
    }

    input_monitor->input_manager = input_manager;
    input_monitor->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(input_manager->server, &input_monitor->server_destroy);

    input_monitor->configured.notify = handle_configured;
    output_manager_add_configured_listener(&input_monitor->configured);
    input_monitor->layout_damage.notify = handle_layout_damage;
    // output_manager_add_layout_damage_listener(&input_monitor->layout_damage);
    wl_list_init(&input_monitor->layout_damage.link);

    input_monitor->new_seat.notify = handle_new_seat;
    wl_signal_add(&input_manager->events.new_seat, &input_monitor->new_seat);

    return input_monitor;
}
