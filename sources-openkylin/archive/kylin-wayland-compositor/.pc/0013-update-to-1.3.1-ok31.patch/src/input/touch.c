// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <math.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_touch.h>

#include <kywc/log.h>

#include "backend/libinput.h"
#include "input/seat.h"
#include "input_p.h"
#include "scene/surface.h"
#include "server.h"
#include "util/macros.h"
#include "view/view.h"
#include "xwayland/xwayland.h"

#define TOUCH_HOLD_TIMEOUT (50)
#define TOUCH_FILTER_TIMEOUT (100)

#define TOUCH_DEAD_ZONE (8.f)
#define TOUCH_MOVE_THRESHOLD_MM (4.f)

struct touch_manager {
    struct wl_listener new_input;
    struct wl_listener server_destroy;
};

struct touch {
    struct wlr_touch *wlr_touch;
    struct input *input;
    struct wl_listener input_destroy;

    struct wl_list points;
    uint32_t points_count;

    /* timer for gesture detect filter */
    struct wl_event_source *filter;
    bool filter_enabled;

    /* current gesture state per touch */
    struct gesture_state gestures;
    uint32_t hold_points;
    bool client_cancelled;

    /* hold canceled status */
    uint32_t hold_canceled;

    /* pinch angle */
    double angle;

    /* touch dead zone */
    struct {
        double x1, x2;
        double y1, y2;
    } dead_zone;
};

struct touch_point {
    struct touch *touch;
    struct wl_list link;
    struct wlr_surface *surface;
    struct wl_listener surface_unmap;

    /* timer for hold gesture */
    struct wl_event_source *timer;
    bool hold, moved;

    double abs_x, abs_y;
    double last_x, last_y;
    double dx, dy;
    int32_t touch_id;
    uint32_t directions;

    uint32_t down_serial;
};

static struct touch *touch_from_wlr_touch(struct wlr_touch *wlr_touch)
{
    return wlr_touch->data;
}

static void touch_handle_input_destroy(struct wl_listener *listener, void *data)
{
    struct touch *touch = wl_container_of(listener, touch, input_destroy);
    wl_list_remove(&touch->input_destroy.link);
    gesture_state_finish(&touch->gestures);
    wl_event_source_remove(touch->filter);

    struct touch_point *point, *tmp;
    wl_list_for_each_safe(point, tmp, &touch->points, link) {
        wl_list_remove(&point->link);
        if (point->surface) {
            wl_list_remove(&point->surface_unmap.link);
        }
        if (point->timer) {
            wl_event_source_remove(point->timer);
        }
        free(point);
    }

    free(touch);
}

static enum gesture_edge touch_calc_edge(struct touch *touch)
{
    double abs_avg_x = 0.0;
    double abs_avg_y = 0.0;

    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        /* meet the first free point */
        if (point->touch_id < 0) {
            break;
        }
        abs_avg_x += point->abs_x;
        abs_avg_y += point->abs_y;
    }

    abs_avg_x /= touch->points_count;
    abs_avg_y /= touch->points_count;
    // TODO: edge normalize by physical size of touch

    enum gesture_edge edge = GESTURE_EDGE_NONE;
    if (abs_avg_y < touch->dead_zone.y1) {
        edge = GESTURE_EDGE_TOP;
    }

    if (abs_avg_x < touch->dead_zone.x1) {
        edge = GESTURE_EDGE_LEFT;
    }

    if (abs_avg_x > touch->dead_zone.x2) {
        edge = GESTURE_EDGE_RIGHT;
    }

    if (abs_avg_y > touch->dead_zone.y2) {
        edge = GESTURE_EDGE_BOTTOM;
    }

    return edge;
}

static bool touch_surface_cancelled(struct touch *touch, struct touch_point *target)
{
    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        if (point == target || point->touch_id < 0) {
            break;
        }
        if (point->surface == target->surface) {
            return true;
        }
    }

    return false;
}

static void touch_cancel_client_points(struct touch *touch, uint32_t retain_serial)
{
    if (!touch->points_count || touch->client_cancelled) {
        return;
    }

    struct seat *seat = touch->input->seat;
    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        if (point->touch_id < 0) {
            break;
        }
        if (!point->surface || point->down_serial == retain_serial ||
            touch_surface_cancelled(touch, point)) {
            continue;
        }
        kywc_log(KYWC_DEBUG, "Send touch cancel to client for gesture binding: surface: %p",
                 point->surface);
        wlr_seat_touch_notify_cancel(seat->wlr_seat, point->surface);
    }

    if (seat->cursor->touch_simulation_pointer) {
        seat->cursor->pointer_touch_up = true;
        seat->cursor->pointer_touch_id = -1;
    }

    touch->client_cancelled = true;
}

static bool touch_gesture_begin(struct touch *touch, enum gesture_type gesture, uint8_t fingers)
{
    struct gesture_state *state = &touch->gestures;
    if (state->type == gesture && state->fingers == fingers) {
        return false;
    }
    /* cancel current gesture and enter the new hold */
    if (state->type != GESTURE_TYPE_NONE) {
        gesture_state_end(state, state->type, state->device, true);
    }
    if (gesture == GESTURE_TYPE_NONE) {
        return false;
    }

    // ignore one finger
    if (fingers != 1 && kywc_gesture_binding_check(gesture, GESTURE_DEVICE_TOUCHSCREEN, fingers)) {
        touch_cancel_client_points(touch, 0);
    }

    enum gesture_edge edge = touch_calc_edge(touch);
    gesture_state_begin(state, touch->input->seat, gesture, GESTURE_DEVICE_TOUCHSCREEN, edge,
                        fingers);

    return true;
}

static void touch_gesture_detect(struct touch *touch)
{
    if (touch->points_count == 0) {
        return;
    }

    bool all_points_moved = true;
    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        /* meet the first free point */
        if (point->touch_id < 0) {
            break;
        }
        if (!point->moved) {
            all_points_moved = false;
            break;
        }
    }

    if (!all_points_moved) {
        /* hold gesture if all hold points not moved */
        if (touch->hold_points && !touch->hold_canceled) {
            if (!touch_gesture_begin(touch, GESTURE_TYPE_HOLD, touch->hold_points)) {
                /* cancel the hold gesture when touch point is moved,
                 * except for there is only one touch point
                 */
                if (touch->hold_points != 1) {
                    gesture_state_end(&touch->gestures, GESTURE_TYPE_HOLD,
                                      GESTURE_DEVICE_TOUCHSCREEN, true);
                }
                touch->hold_canceled = true;
            }
        }
        return;
    }

    /* moved when one touch point swipe from edge */
    if (touch->points_count == 1) {
        touch_gesture_begin(touch, GESTURE_TYPE_SWIPE, 1);
        return;
    }

    /* swipre or pinch check */
    wl_list_for_each(point, &touch->points, link) {
        /* meet the first free point */
        if (point->touch_id < 0) {
            break;
        }
        if (!point->moved) {
            all_points_moved = false;
            break;
        }
    }

    bool one_direction = true;
    uint32_t directions = GESTURE_DIRECTION_NONE;
    wl_list_for_each(point, &touch->points, link) {
        /* meet the first free point */
        if (point->touch_id < 0) {
            break;
        }
        if (directions == GESTURE_DIRECTION_NONE) {
            directions = point->directions;
        } else if (directions != point->directions) {
            one_direction = false;
            break;
        }
    }

    touch_gesture_begin(touch, one_direction ? GESTURE_TYPE_SWIPE : GESTURE_TYPE_PINCH,
                        touch->points_count);
}

static void touch_filter_enable(struct touch *touch, bool enabled)
{
    wl_event_source_timer_update(touch->filter, enabled ? TOUCH_FILTER_TIMEOUT : 0);
    touch->filter_enabled = enabled;
}

static int touch_handle_timer(void *data)
{
    struct touch *touch = data;

    touch->filter_enabled = false;
    touch_gesture_detect(touch);
    return 0;
}

static void touch_update_dead_zone(struct touch *touch)
{
    int width, height;
    input_get_mapped_size(touch->input, &width, &height);
    touch->dead_zone.x1 = TOUCH_DEAD_ZONE / width;
    touch->dead_zone.x2 = 1 - touch->dead_zone.x1;
    touch->dead_zone.y1 = TOUCH_DEAD_ZONE / height;
    touch->dead_zone.y2 = 1 - touch->dead_zone.y1;
    kywc_log(KYWC_DEBUG, "touch dead zone %lf %lf %lf %lf", touch->dead_zone.x1,
             touch->dead_zone.x2, touch->dead_zone.y1, touch->dead_zone.y2);
}

static void handle_new_input(struct wl_listener *listener, void *data)
{
    struct touch_manager *manager = wl_container_of(listener, manager, new_input);
    struct input *input = data;

    /* input has been configured, only care about touch */
    if (input->prop.type != WLR_INPUT_DEVICE_TOUCH) {
        return;
    }

    struct touch *touch = calloc(1, sizeof(*touch));
    if (!touch) {
        return;
    }

    struct wl_display *display = input->seat->wlr_seat->display;
    struct wl_event_loop *loop = wl_display_get_event_loop(display);
    touch->filter = wl_event_loop_add_timer(loop, touch_handle_timer, touch);
    if (!touch->filter) {
        free(touch);
        return;
    }

    touch->input = input;
    touch->input_destroy.notify = touch_handle_input_destroy;
    wl_signal_add(&input->events.destroy, &touch->input_destroy);

    touch->wlr_touch = wlr_touch_from_input_device(input->wlr_input);
    touch->wlr_touch->data = touch;
    wl_list_init(&touch->points);

    touch_update_dead_zone(touch);

    gesture_state_init(&touch->gestures, display);
}

void touch_reset_gesture(struct input_manager *input_manager)
{
    struct input *input = NULL;
    wl_list_for_each(input, &input_manager->inputs, link) {
        if (input->prop.type != WLR_INPUT_DEVICE_TOUCH) {
            continue;
        }

        struct wlr_touch *wlr_touch = wlr_touch_from_input_device(input->wlr_input);
        struct touch *touch = wlr_touch->data;
        touch_filter_enable(touch, false);

        struct touch_point *point;
        wl_list_for_each(point, &touch->points, link) {
            /* meet the first free point */
            if (point->touch_id < 0) {
                break;
            }
            if (point->timer) {
                wl_event_source_timer_update(point->timer, 0);
            }
        }
        gesture_state_end(&touch->gestures, touch->gestures.type, GESTURE_DEVICE_TOUCHSCREEN, true);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct touch_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->new_input.link);
    free(manager);
}

bool touch_manager_create(struct input_manager *input_manager)
{
    struct touch_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->new_input.notify = handle_new_input;
    input_add_new_listener(&manager->new_input);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(input_manager->server, &manager->server_destroy);

    return true;
}

static struct wlr_surface *touch_get_surface(struct touch *touch, double *sx, double *sy,
                                             struct wlr_surface **toplevel)
{
    struct seat *seat = touch->input->seat;
    struct cursor *cursor = seat->cursor;

    struct scene_node *node =
        scene_node_at(&seat->scene->tree.node, cursor->lx, cursor->ly, sx, sy);
    if (!node) {
        return NULL;
    }

    if (toplevel) {
        *toplevel = input_event_node_toplevel(input_event_node_from_node(node));
    }
    return wlr_surface_try_from_node(node);
}

static int touch_point_handle_timer(void *data)
{
    struct touch_point *point = data;
    point->hold = true;
    point->touch->hold_points++;

    if (point->touch->filter_enabled) {
        return 0;
    }

    if (point->touch->gestures.type == GESTURE_TYPE_NONE) {
        touch_filter_enable(point->touch, true);
        return 0;
    }

    /* keep hold gesture */
    if (point->touch->gestures.type == GESTURE_TYPE_HOLD) {
        touch_gesture_begin(point->touch, GESTURE_TYPE_HOLD, point->touch->hold_points);
    }

    return 0;
}

static void touch_point_handle_surface_unmap(struct wl_listener *listener, void *data)
{
    struct touch_point *point = wl_container_of(listener, point, surface_unmap);
    wl_list_remove(&point->surface_unmap.link);
    point->surface = NULL;
}

static struct touch_point *touch_point_create(struct touch *touch, int32_t touch_id)
{
    struct touch_point *point, *free_point = NULL;
    wl_list_for_each(point, &touch->points, link) {
        if (point->touch_id == touch_id) {
            return point;
        }
        if (!free_point && point->touch_id < 0) {
            free_point = point;
        }
    }

    /* not found, reuse the first free one */
    if (free_point) {
        free_point->touch_id = touch_id;
        return free_point;
    }

    /* alloc one if all in used */
    point = calloc(1, sizeof(*point));
    if (!point) {
        return NULL;
    }

    point->touch_id = touch_id;
    point->touch = touch;
    point->surface_unmap.notify = touch_point_handle_surface_unmap;
    wl_list_insert(touch->points.prev, &point->link);

    struct wl_display *display = touch->input->seat->wlr_seat->display;
    struct wl_event_loop *loop = wl_display_get_event_loop(display);
    point->timer = wl_event_loop_add_timer(loop, touch_point_handle_timer, point);

    return point;
}

static struct touch_point *touch_point_from_id(struct touch *touch, int32_t touch_id)
{
    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        if (point->touch_id == touch_id) {
            return point;
        }
    }
    return NULL;
}

static void touch_point_reset(struct touch_point *point, bool cancelled)
{
    struct gesture_state *state = &point->touch->gestures;
    if (state->type != GESTURE_TYPE_NONE) {
        /* touch point up to set hold canceled in hold state,
         * and cancels gesture when touch point count is more than 1
         */
        if (state->type == GESTURE_TYPE_HOLD) {
            point->touch->hold_canceled = true;
            if (!cancelled && point->touch->points_count > 1) {
                cancelled = true;
            }
        }
        if (cancelled || point->touch->points_count <= 1) {
            gesture_state_end(state, state->type, state->device, cancelled);
        }
    }

    point->down_serial = 0;
    point->touch_id = -1;
    point->touch->points_count--;

    if (point->surface) {
        point->surface = NULL;
        wl_list_remove(&point->surface_unmap.link);
    }

    if (point->timer) {
        wl_event_source_timer_update(point->timer, 0);
    }

    if (point->hold) {
        point->touch->hold_points--;
        point->hold = false;
    }

    /* last touch point up reset the hold canceled state */
    if (!point->touch->points_count) {
        point->touch->hold_canceled = false;
        point->touch->client_cancelled = false;
    }
    /* reinsert to tail */
    wl_list_remove(&point->link);
    wl_list_insert(point->touch->points.prev, &point->link);
}

bool touch_handle_down(struct wlr_touch_down_event *event)
{
    struct touch *touch = touch_from_wlr_touch(event->touch);
    if (!touch) {
        return false;
    }

    struct seat *seat = touch->input->seat;
    cursor_move(seat->cursor, &event->touch->base, event->x, event->y, false, true);

    if (seat->touch_grab && seat->touch_grab->interface->touch &&
        seat->touch_grab->interface->touch(seat->touch_grab, event->time_msec, true, 0)) {
        return true;
    }

    struct touch_point *point = touch_point_create(touch, event->touch_id);
    if (point->timer) {
        wl_event_source_timer_update(point->timer, TOUCH_HOLD_TIMEOUT);
    }
    touch->points_count++;
    point->abs_x = point->last_x = event->x;
    point->abs_y = point->last_y = event->y;
    point->moved = false;
    touch->angle = 0.0;

    if (wlr_seat_touch_has_grab(seat->wlr_seat)) {
        double sx, sy;
        struct wlr_surface *surface = touch_get_surface(touch, &sx, &sy, NULL);
        if (!surface) {
            return false;
        }

        if (xwayland_check_client(wl_resource_get_client(surface->resource))) {
            sx = xwayland_scale(sx);
            sy = xwayland_scale(sy);
        }
        wlr_seat_touch_notify_down(seat->wlr_seat, surface, event->time_msec, event->touch_id, sx,
                                   sy);
        return true;
    }

    if (touch->client_cancelled) {
        return true;
    }

    touch_update_dead_zone(touch);
    enum gesture_edge edges = kywc_gesture_binding_check_edge(GESTURE_DEVICE_TOUCHSCREEN);
    if ((event->x < touch->dead_zone.x1 && (edges & GESTURE_EDGE_LEFT)) ||
        (event->x > touch->dead_zone.x2 && (edges & GESTURE_EDGE_RIGHT)) ||
        (event->y < touch->dead_zone.y1 && (edges & GESTURE_EDGE_TOP)) ||
        (event->y > touch->dead_zone.y2 && (edges & GESTURE_EDGE_BOTTOM))) {
        return true;
    }

    struct wlr_surface *toplevel = NULL;
    double sx, sy;
    struct wlr_surface *surface = touch_get_surface(touch, &sx, &sy, &toplevel);
    if (!surface || !wlr_surface_accepts_touch(seat->wlr_seat, surface)) {
        return false;
    }

    point->surface = surface;
    wl_signal_add(&surface->events.unmap, &point->surface_unmap);

    if (xwayland_check_client(wl_resource_get_client(point->surface->resource))) {
        sx = xwayland_scale(sx);
        sy = xwayland_scale(sy);

        int major_minimum, major_maximum, major_resolution;
        int minor_minimum, minor_maximum, minor_resolution;
        if (wlr_libinput_get_touch_size_abs(point->touch->wlr_touch, &major_minimum, &major_maximum,
                                            &major_resolution, &minor_minimum, &minor_maximum,
                                            &minor_resolution)) {
            xwayland_send_touch_info(touch->input, major_minimum, major_maximum, major_resolution,
                                     minor_minimum, minor_maximum, minor_resolution);
        }

        double major, minor;
        if (wlr_libinput_get_touch_size_value(point->touch->wlr_touch, point->touch_id, &major,
                                              &minor)) {
            xwayland_send_touch_size(touch->input, point->touch_id, major, minor);
        }
    }

    point->down_serial = wlr_seat_touch_notify_down(seat->wlr_seat, surface, event->time_msec,
                                                    event->touch_id, sx, sy);

    /* activate and focus the toplevel surface */
    if (toplevel) {
        struct view *view = view_try_from_wlr_surface(toplevel);
        if (view) {
            kywc_view_activate(&view->base);
            view_set_focus(view, seat);
        } else {
            seat_focus_surface(seat, toplevel);
        }
    }

    return true;
}

static uint32_t touch_point_calc_directions(struct touch_point *point, double dx, double dy)
{
    uint32_t directions = GESTURE_DIRECTION_NONE;

    if (fabs(dx) > fabs(dy)) {
        if (dx > 0) {
            directions |= GESTURE_DIRECTION_RIGHT;
        } else {
            directions |= GESTURE_DIRECTION_LEFT;
        }
    } else {
        if (dy > 0) {
            directions |= GESTURE_DIRECTION_DOWN;
        } else {
            directions |= GESTURE_DIRECTION_UP;
        }
    }

    return directions;
}

static bool touch_point_has_moved(struct touch_point *point, double dx, double dy)
{
    struct wlr_touch *wlr_touch = point->touch->wlr_touch;
    if (wlr_touch->width_mm <= 0.0 || wlr_touch->height_mm <= 0.0) {
        return fabs(dx) > 0.01f || fabs(dy) > 0.01f;
    }

    double dx_mm = dx * wlr_touch->width_mm;
    double dy_mm = dy * wlr_touch->height_mm;
    return hypot(dx_mm, dy_mm) > TOUCH_MOVE_THRESHOLD_MM;
}

static void touch_calc_average_delta(struct touch_point *point, double *dx, double *dy)
{
    struct touch *touch = point->touch;
    *dx = point->dx / touch->points_count;
    *dy = point->dy / touch->points_count;
}

static double touch_calc_average_scale(struct touch *touch)
{
    double start_min = 1.0, current_min = 1.0;
    double start_max = 0.0, current_max = 0.0;

    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        /* meet the first free point */
        if (point->touch_id < 0) {
            break;
        }
        if (start_min == 1.0 && start_max == 0.0) {
            start_min = start_max = point->abs_x;
            current_min = current_max = point->last_x;
        } else {
            start_min = MIN(start_min, point->abs_x);
            start_max = MAX(start_max, point->abs_x);
            current_min = MIN(current_min, point->last_x);
            current_max = MAX(current_max, point->last_x);
        }
    }
    double start_width = start_max - start_min;
    double current_width = current_max - current_min;
    if (start_width == 0.0) {
        kywc_log(KYWC_WARN, "Start(max = %f, min = %f), Current(max = %f, min = %f)", start_max,
                 start_min, current_max, current_min);
        return 1.0;
    }

    return current_width / start_width;
}

static double touch_calc_average_angle_delta(struct touch *touch)
{
    double dx = 0.0;
    double dy = 0.0;

    struct touch_point *point;
    wl_list_for_each(point, &touch->points, link) {
        /* meet the first free point */
        if (point->touch_id < 0) {
            break;
        }
        dx = point->last_x - dx;
        dy = point->last_y - dy;
    }

    double tangle = atan2(dy, dx) * 180.0 / 3.14;
    if (touch->angle == 0.0) {
        touch->angle = tangle;
    }

    double angle_delta = tangle - touch->angle;
    if (angle_delta > 180.0) {
        angle_delta -= 360.0;
    } else if (angle_delta < -180.0) {
        angle_delta += 360.0;
    }

    touch->angle = tangle;

    return angle_delta;
}

void touch_handle_motion(struct wlr_touch_motion_event *event, bool handle)
{
    struct touch *touch = touch_from_wlr_touch(event->touch);
    if (!touch) {
        return;
    }

    struct seat *seat = touch->input->seat;
    cursor_move(seat->cursor, &event->touch->base, event->x, event->y, false, true);

    struct touch_point *point = touch_point_from_id(touch, event->touch_id);
    if (!point) {
        return;
    }

    point->dx = event->x - point->last_x;
    point->dy = event->y - point->last_y;
    point->last_x = event->x;
    point->last_y = event->y;

    double dx = event->x - point->abs_x;
    double dy = event->y - point->abs_y;
    if (!point->moved && !touch_point_has_moved(point, dx, dy)) {
        return;
    }

    point->moved = true;

    if (seat->touch_grab && seat->touch_grab->interface->motion &&
        seat->touch_grab->interface->motion(seat->touch_grab, event->time_msec, seat->cursor->lx,
                                            seat->cursor->ly, point->down_serial)) {
        return;
    }

    if (wlr_seat_touch_has_grab(seat->wlr_seat)) {
        double sx, sy;
        struct wlr_surface *surface = touch_get_surface(touch, &sx, &sy, NULL);
        if (!surface) {
            return;
        }

        if (xwayland_check_client(wl_resource_get_client(surface->resource))) {
            sx = xwayland_scale(sx);
            sy = xwayland_scale(sy);
        }

        if ((int32_t)seat->wlr_seat->touch_state.grab_id == point->touch_id) {
            wlr_seat_touch_point_focus(seat->wlr_seat, surface, event->time_msec, event->touch_id,
                                       sx, sy);
            selection_handle_cursor_move(seat, seat->cursor->lx, seat->cursor->ly);
        }
        wlr_seat_touch_notify_motion(seat->wlr_seat, event->time_msec, event->touch_id, sx, sy);
        return;
    }

    /* calc point motion directions */
    point->directions = touch_point_calc_directions(point, dx, dy);

    if (!touch->filter_enabled) {
        if (!touch->gestures.handled && !touch->gestures.triggered) {
            touch_gesture_detect(touch);
        }
        if (touch->gestures.type == GESTURE_TYPE_SWIPE) {
            double avg_dx = 0, avg_dy = 0;
            touch_calc_average_delta(point, &avg_dx, &avg_dy);
            gesture_state_update(&touch->gestures, GESTURE_TYPE_SWIPE, GESTURE_DEVICE_TOUCHSCREEN,
                                 avg_dx, avg_dy, NAN, NAN);
        } else if (touch->gestures.type == GESTURE_TYPE_PINCH) {
            double avg_dx = 0, avg_dy = 0;
            touch_calc_average_delta(point, &avg_dx, &avg_dy);
            double scale = touch_calc_average_scale(touch);
            double angle_delta = touch_calc_average_angle_delta(touch);
            gesture_state_update(&touch->gestures, GESTURE_TYPE_PINCH, GESTURE_DEVICE_TOUCHSCREEN,
                                 avg_dx, avg_dy, scale, angle_delta);
        }
    }

    if (!handle || touch->client_cancelled || !point->surface) {
        return;
    }

    double sx, sy;
    struct wlr_surface *surface = touch_get_surface(touch, &sx, &sy, NULL);
    struct wlr_surface *target = point->surface;

    /* if motion out of point->surface */
    if (surface != point->surface) {
        if (!seat_is_dragging(seat)) {
            int lx, ly;
            struct scene_buffer *scene_buffer = scene_buffer_try_from_surface(point->surface);
            scene_node_coords(&scene_buffer->node, &lx, &ly);
            sx = seat->cursor->lx - lx;
            sy = seat->cursor->ly - ly;
        } else if (surface) {
            target = surface;
        }
    }

    if (xwayland_check_client(wl_resource_get_client(target->resource))) {
        sx = xwayland_scale(sx);
        sy = xwayland_scale(sy);
        double major, minor;
        if (wlr_libinput_get_touch_size_value(point->touch->wlr_touch, point->touch_id, &major,
                                              &minor)) {
            xwayland_send_touch_size(touch->input, point->touch_id, major, minor);
        }
    }

    if (target == surface) {
        wlr_seat_touch_point_focus(seat->wlr_seat, surface, event->time_msec, event->touch_id, sx,
                                   sy);
    }

    selection_handle_cursor_move(seat, seat->cursor->lx, seat->cursor->ly);
    wlr_seat_touch_notify_motion(seat->wlr_seat, event->time_msec, event->touch_id, sx, sy);
}

void touch_handle_up(struct wlr_touch_up_event *event, bool handle)
{
    struct touch *touch = touch_from_wlr_touch(event->touch);
    if (!touch) {
        return;
    }

    uint32_t serial = 0;
    struct touch_point *point = touch_point_from_id(touch, event->touch_id);
    if (point) {
        serial = point->down_serial;
        touch_point_reset(point, false);
    }

    struct seat *seat = touch->input->seat;
    if (seat->touch_grab && seat->touch_grab->interface->touch &&
        seat->touch_grab->interface->touch(seat->touch_grab, event->time_msec, false, serial)) {
        return;
    }

    if (!point) {
        return;
    }

    if (handle) {
        wlr_seat_touch_notify_up(seat->wlr_seat, event->time_msec, event->touch_id);
    }
}

void touch_handle_cancel(struct wlr_touch_cancel_event *event, bool handle)
{
    struct touch *touch = touch_from_wlr_touch(event->touch);
    if (!touch) {
        return;
    }

    uint32_t serial = 0;
    bool client_cancelled = touch->client_cancelled;
    struct touch_point *point = touch_point_from_id(touch, event->touch_id);
    if (point) {
        serial = point->down_serial;
        touch_point_reset(point, true);
    }

    struct seat *seat = touch->input->seat;
    if (seat->touch_grab && seat->touch_grab->interface->touch &&
        seat->touch_grab->interface->touch(seat->touch_grab, event->time_msec, false, serial)) {
        return;
    }

    if (point && handle && !client_cancelled && point->surface) {
        struct seat *seat = touch->input->seat;
        wlr_seat_touch_notify_cancel(seat->wlr_seat, point->surface);
    }
}

void seat_cancel_touch_points(struct seat *seat, uint32_t retain_serial)
{
    struct input *input;
    wl_list_for_each(input, &seat->inputs, seat_link) {
        if (input->prop.type == WLR_INPUT_DEVICE_TOUCH) {
            struct wlr_touch *wlr_touch = wlr_touch_from_input_device(input->wlr_input);
            struct touch *touch = touch_from_wlr_touch(wlr_touch);
            touch_cancel_client_points(touch, retain_serial);
        }
    }
}
