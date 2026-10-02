// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <float.h>
#include <stdlib.h>
#include <string.h>

#include <kywc/log.h>

#include "effect_p.h"
#include "input/cursor.h"
#include "input/function.h"
#include "input/seat.h"
#include "output.h"
#include "render/opengl.h"
#include "render/pass.h"
#include "render/pixman.h"
#include "security/render.h"
#include "util/dbus.h"
#include "util/macros.h"
#include "util/spawn.h"

static const char *zoom_dbus_interface = "org.ukui.Magnifier";
static const char *zoom_path = "/Magnifier";
static const char *command = "kylin-magnifier";

#define MAX_SCALE (16)
#define WINDOW_WIDTH (320)
#define WINDOW_HEIGHT (180)
#define WINDOW_BORDER (2)

enum zoom_type {
    ZOOM_OUTPUT = 0,
    ZOOM_WINDOW,
};

struct zoom_output {
    struct wl_list link;
    struct output *output;
    struct wl_listener disable;
};

struct zoom_output_info {
    double offset_x, offset_y;
    pixman_region32_t viewport_region;
};

struct seat_cursor {
    struct wl_list link;
    struct zoom_effect *effect;

    struct seat *seat;
    struct wl_listener seat_destroy;
    struct wl_listener cursor_motion;

    struct kywc_box geo;
    double lx, ly;
    bool moved;
};

struct zoom_effect {
    struct effect *effect;
    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    struct wl_list outputs;
    struct zoom_output_info output_info;
    struct wl_listener new_enabled_output;

    struct wl_list seat_cursors;
    struct wl_listener new_seat;

    enum zoom_type type;
    int scale;
    bool enabled;
    bool support_zoom_window;

    struct wl_client *client;
    struct wl_listener client_destroy;

    struct server *server;
    struct scene *scene;
    struct dbus_object *dbus;
};

static struct zoom_effect *zoom = NULL;

static struct shortcut {
    char *keybind;
    char *desc;
    enum function_id func_id;
} shortcuts[] = {
    { "Win+Equal:no", "zoom_in", FUNC_ID_ZOOM_IN },
    { "Win+Minus:no", "zoom_out", FUNC_ID_ZOOM_OUT },
    { "Win+KP_Add:no", "zoom_in_1", FUNC_ID_ZOOM_IN },
    { "Win+KP_Subtract:no", "zoom_out_1", FUNC_ID_ZOOM_OUT },
    { "Win+Escape:no", "stop_zoom", FUNC_ID_ZOOM_STOP },
};

static void handle_cursor_motion(struct wl_listener *listener, void *data)
{
    struct seat_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
    struct seat_cursor_motion_event *event = data;
    cursor->lx = event->lx;
    cursor->ly = event->ly;
    cursor->moved = true;
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    if (strcmp(option->key, "type") == 0) {
        int type = option->value.num;
        if (type != ZOOM_OUTPUT && type != ZOOM_WINDOW) {
            kywc_log(KYWC_WARN, "Invalid type");
            return false;
        }

        zoom->type = type;
        return true;
    }

    if (strcmp(option->key, "scale") == 0) {
        if (option->value.num < 1 || option->value.num > MAX_SCALE) {
            kywc_log(KYWC_WARN, "Invalid scale");
            return false;
        }

        zoom->scale = option->value.num;
        return true;
    }

    return false;
}

static void seat_cursor_destroy(struct seat_cursor *cursor)
{
    wl_list_remove(&cursor->seat_destroy.link);
    wl_list_remove(&cursor->cursor_motion.link);
    wl_list_remove(&cursor->link);
    free(cursor);
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct seat_cursor *cursor = wl_container_of(listener, cursor, seat_destroy);
    seat_cursor_destroy(cursor);
}

static void seat_cursor_create(struct zoom_effect *effect, struct seat *seat)
{
    struct seat_cursor *seat_cursor = calloc(1, sizeof(*seat_cursor));
    if (!seat_cursor) {
        return;
    }

    seat_cursor->effect = effect;
    seat_cursor->lx = seat->cursor->lx;
    seat_cursor->ly = seat->cursor->ly;
    wl_list_insert(&effect->seat_cursors, &seat_cursor->link);

    seat_cursor->seat = seat;
    seat_cursor->seat_destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &seat_cursor->seat_destroy);
    seat_cursor->cursor_motion.notify = handle_cursor_motion;
    wl_signal_add(&seat->events.cursor_motion, &seat_cursor->cursor_motion);
}

static void handle_new_seat(struct wl_listener *listener, void *data)
{
    struct zoom_effect *effect = wl_container_of(listener, effect, new_seat);
    struct seat *seat = data;
    seat_cursor_create(effect, seat);
}

static bool handle_seat(struct seat *seat, int index, void *data)
{
    struct zoom_effect *effect = data;
    seat_cursor_create(effect, seat);
    return false;
}

static void zoom_output_cursor_outside_viewport_region(struct zoom_effect *effect,
                                                       struct scene_render_target *target)
{
    struct seat_cursor *seat_cursor;
    struct cursor *cursor;
    wl_list_for_each(seat_cursor, &effect->seat_cursors, link) {
        cursor = seat_cursor->seat->cursor;
        if (!pixman_region32_contains_point(&effect->output_info.viewport_region, cursor->lx,
                                            cursor->ly, NULL)) {
            double lx = target->logical.x + target->logical.width * 0.5;
            double ly = target->logical.y + target->logical.height * 0.5;
            cursor_move(cursor, NULL, lx, ly, false, false);
        }
    }
}

static void zoom_output_calc_closest_point(struct zoom_effect *effect, double lx, double ly,
                                           double *dest_x, double *dest_y)
{
    double distance_x, distance_y, distance;
    double min_x = lx, min_y = ly, min_distance = DBL_MAX;
    int rects_len;
    pixman_box32_t *rects =
        pixman_region32_rectangles(&effect->output_info.viewport_region, &rects_len);
    for (int i = 0; i < rects_len; i++) {
        struct wlr_box box;
        pixman_box32_t *rect = &rects[i];
        box.x = rect->x1;
        box.y = rect->y1;
        box.width = rect->x2 - rect->x1;
        box.height = rect->y2 - rect->y1;
        wlr_box_closest_point(&box, lx, ly, &distance_x, &distance_y);
        // calculate squared distance suitable for comparison
        distance = (lx - distance_x) * (lx - distance_x) + (ly - distance_y) * (ly - distance_y);
        if (!isfinite(distance)) {
            distance = DBL_MAX;
        }

        if (distance < min_distance) {
            min_x = distance_x;
            min_y = distance_y;
            min_distance = distance;
        }
    }

    *dest_x = min_x;
    *dest_y = min_y;
}

static void zoom_output_update_viewport_region(struct zoom_effect *effect)
{
    pixman_region32_clear(&effect->output_info.viewport_region);
    int trans_width, trans_height = 0;
    struct scene_output *scene_output;
    wl_list_for_each(scene_output, &effect->scene->outputs, link) {
        wlr_output_transformed_resolution(scene_output->output, &trans_width, &trans_height);
        float scale = effect->scale * scene_output->output->scale;
        struct kywc_box logic_box = {
            .x = (int)(scene_output->geometry.x / effect->scale) + effect->output_info.offset_x,
            .y = (int)(scene_output->geometry.y / effect->scale) + effect->output_info.offset_y,
            .width = trans_width / scale,
            .height = trans_height / scale,
        };

        pixman_region32_union_rect(&effect->output_info.viewport_region,
                                   &effect->output_info.viewport_region, logic_box.x, logic_box.y,
                                   logic_box.width, logic_box.height);
    }
}

static void zoom_constrain_viewport_offset(struct zoom_effect *effect)
{
    int layout_width, layout_height;
    int layout_x = 0, layout_y = 0;
    int dx = 0, dy = 0;
    output_layout_get_size(&layout_width, &layout_height);
    pixman_box32_t *extents = pixman_region32_extents(&effect->output_info.viewport_region);
    int vport_x = extents->x1;
    int vport_y = extents->y1;
    int vport_w = extents->x2 - extents->x1;
    int vport_h = extents->y2 - extents->y1;
    // left right
    if (vport_x < layout_x) {
        dx = layout_x - vport_x;
    } else if (vport_x + vport_w > layout_x + layout_width) {
        dx = (layout_x + layout_width) - (vport_x + vport_w);
    }
    // top bottom
    if (vport_y < layout_y) {
        dy = layout_y - vport_y;
    } else if (vport_y + vport_h > layout_y + layout_height) {
        dy = (layout_y + layout_height) - (vport_y + vport_h);
    }

    if (dx != 0 || dy != 0) {
        effect->output_info.offset_x += dx;
        effect->output_info.offset_y += dy;
        zoom_output_update_viewport_region(effect);
    }
}

static void zoom_output_calc_viewport_region(struct zoom_effect *effect)
{
    zoom_output_update_viewport_region(effect);
    zoom_constrain_viewport_offset(effect);
}

static void zoom_scale_changed(struct zoom_effect *effect, struct scene_render_target *target,
                               int new_scale, int old_scale, double cursor_x, double cursor_y)
{
    if (new_scale == 1) {
        effect->output_info.offset_x = 0;
        effect->output_info.offset_y = 0;
        pixman_region32_clear(&effect->output_info.viewport_region);
        return;
    }

    double old_viewport_x =
        (int)(target->output->geometry.x / old_scale) + effect->output_info.offset_x;
    double old_viewport_y =
        (int)(target->output->geometry.y / old_scale) + effect->output_info.offset_y;
    double rel_cx = cursor_x - old_viewport_x;
    double rel_cy = cursor_y - old_viewport_y;
    double new_viewport_x = cursor_x - rel_cx * ((double)old_scale / new_scale);
    double new_viewport_y = cursor_y - rel_cy * ((double)old_scale / new_scale);

    effect->output_info.offset_x = new_viewport_x - (int)(target->output->geometry.x / new_scale);
    effect->output_info.offset_y = new_viewport_y - (int)(target->output->geometry.y / new_scale);

    zoom_output_calc_viewport_region(effect);
}

static bool handle_output_frame_render_pre(struct effect_entity *entity,
                                           struct scene_render_target *target)
{
    struct zoom_effect *effect = entity->user_data;
    if (wl_list_empty(&effect->seat_cursors)) {
        return true;
    }

    struct seat_cursor *cursor = wl_container_of(effect->seat_cursors.next, cursor, link);
    static int last_scale = 1;
    if (effect->scale != last_scale) {
        zoom_scale_changed(effect, target, effect->scale, last_scale, cursor->lx, cursor->ly);
        scene_damage_whole(zoom->scene);
        last_scale = effect->scale;
    }

    /* check if the mouse is in the area */
    bool need_check = !pixman_region32_not_empty(&effect->output_info.viewport_region);
    if (cursor->moved || need_check) {
        if (!pixman_region32_contains_point(&effect->output_info.viewport_region, cursor->lx,
                                            cursor->ly, NULL)) {
            double closest_x = 0, closest_y = 0;
            zoom_output_calc_closest_point(effect, cursor->lx, cursor->ly, &closest_x, &closest_y);
            int distance_x = cursor->lx - closest_x;
            int distance_y = cursor->ly - closest_y;

            effect->output_info.offset_x += distance_x;
            effect->output_info.offset_y += distance_y;
            zoom_output_calc_viewport_region(effect);
        }
        scene_damage_whole(zoom->scene);
        cursor->moved = false;
    }

    /* calc target logical */
    float output_scale = target->output->output->scale;
    target->scale = effect->scale * output_scale;
    target->logical.width = target->logical.width / effect->scale;
    target->logical.height = target->logical.height / effect->scale;
    target->logical.x =
        (int)(target->output->geometry.x / effect->scale) + effect->output_info.offset_x;
    target->logical.y =
        (int)(target->output->geometry.y / effect->scale) + effect->output_info.offset_y;

    scene_output_set_viewport_source_box(target->output, &target->logical);

    /* force rendering cursor */
    target->options |= SCENE_RENDER_ENABLE_CURSORS;

    if (need_check) {
        zoom_output_cursor_outside_viewport_region(effect, target);
    }

    return true;
}

static void zoom_window_update_geo(struct seat_cursor *cursor, struct scene_render_target *target)
{
    int width = (target->transform & WL_OUTPUT_TRANSFORM_90) ? WINDOW_HEIGHT : WINDOW_WIDTH;
    int height = (target->transform & WL_OUTPUT_TRANSFORM_90) ? WINDOW_WIDTH : WINDOW_HEIGHT;
    int logical_width = (int)ceil(width / (float)target->scale);
    int logical_height = (int)ceil(height / (float)target->scale);

    cursor->geo.x = ceil(cursor->lx - logical_width * 0.5);
    cursor->geo.y = ceil(cursor->ly - logical_height * 0.5);
    cursor->geo.width = logical_width;
    cursor->geo.height = logical_height;
}

static void zoom_window_add_damage(struct kywc_box *geo, struct scene *scene)
{
    pixman_region32_t region;
    pixman_region32_init_rect(&region, geo->x, geo->y, geo->width, geo->height);
    scene_add_damage(scene, &region);
    pixman_region32_fini(&region);
}

static bool handle_window_frame_render_pre(struct effect_entity *entity,
                                           struct scene_render_target *target)
{
    struct zoom_effect *effect = entity->user_data;
    struct scene *scene = effect->server->scene;
    struct seat_cursor *cursor;
    wl_list_for_each(cursor, &effect->seat_cursors, link) {
        struct kywc_box old_geo = cursor->geo;
        zoom_window_update_geo(cursor, target);
        if (!kywc_box_equal(&old_geo, &cursor->geo)) {
            zoom_window_add_damage(&old_geo, scene);
            zoom_window_add_damage(&cursor->geo, scene);
            return true;
        }

        if (pixman_region32_not_empty(&scene->collected_damage) ||
            pixman_region32_not_empty(&scene->pushed_damage)) {
            if (!kywc_box_intersects(&target->logical, &cursor->geo)) {
                continue;
            }

            zoom_window_add_damage(&cursor->geo, scene);
        }
    }

    return true;
}

static bool handle_frame_render_pre(struct effect_entity *entity,
                                    struct scene_render_target *target)
{
    struct zoom_effect *effect = entity->user_data;
    if (!effect->enabled) {
        return true;
    }

    if (effect->type == ZOOM_OUTPUT) {
        return handle_output_frame_render_pre(entity, target);
    } else if (effect->type == ZOOM_WINDOW) {
        return handle_window_frame_render_pre(entity, target);
    }

    return true;
}

static bool handle_frame_render_end(struct effect_entity *entity,
                                    struct scene_render_target *target)
{
    struct zoom_effect *effect = entity->user_data;
    if (!effect->enabled || effect->type == ZOOM_OUTPUT || effect->scale == 1) {
        return true;
    }

    struct seat_cursor *cursor;
    wl_list_for_each(cursor, &effect->seat_cursors, link) {
        struct kywc_box preferred_dst_box = cursor->geo;
        scene_render_target_box(target, &preferred_dst_box);

        int src_width = preferred_dst_box.width / effect->scale;
        int src_height = preferred_dst_box.height / effect->scale;
        struct kywc_box buffer_src_box = {
            .x = preferred_dst_box.x + (preferred_dst_box.width - src_width) / 2.0f,
            .y = preferred_dst_box.y + (preferred_dst_box.height - src_height) / 2.0f,
            .width = src_width,
            .height = src_height,
        };

        struct kywc_box src_box;
        struct kywc_box target_box = { 0, 0, target->buffer->width, target->buffer->height };
        if (!kywc_box_intersection(&src_box, &buffer_src_box, &target_box)) {
            continue;
        }

        /* adjust dst_box to match src_box when clipped at edges */
        struct kywc_box dst_box = preferred_dst_box;
        if (src_box.width != buffer_src_box.width || src_box.height != buffer_src_box.height) {
            float scale_x = (float)src_box.width / buffer_src_box.width;
            float scale_y = (float)src_box.height / buffer_src_box.height;
            dst_box.width = preferred_dst_box.width * scale_x;
            dst_box.height = preferred_dst_box.height * scale_y;
            dst_box.x = preferred_dst_box.x + (preferred_dst_box.width - dst_box.width) / 2;
            dst_box.y = preferred_dst_box.y + (preferred_dst_box.height - dst_box.height) / 2;
        }

        pixman_region32_t render_region;
        pixman_region32_init(&render_region);
        pixman_region32_copy(&render_region, &target->damage);
        pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);
        scene_render_region(&render_region, target);
        pixman_region32_intersect_rect(&render_region, &render_region, dst_box.x, dst_box.y,
                                       dst_box.width, dst_box.height);
        if (!pixman_region32_not_empty(&render_region)) {
            pixman_region32_fini(&render_region);
            continue;
        }
        pixman_region32_fini(&render_region);

        struct wlr_texture *wlr_texture =
            wlr_texture_from_buffer(effect->server->renderer, target->buffer);
        if (!wlr_texture) {
            continue;
        }

        if (!(target->options & SCENE_RENDER_ENABLE_SECURITY) &&
            security_check_buffer(target->buffer)) {
            security_target_add_buffer(
                target, target->buffer,
                &(struct kywc_fbox){ src_box.x, src_box.y, src_box.width, src_box.height },
                target->transform, &dst_box);
        }

        struct render_texture_options options = {
            .base = {
                .texture = wlr_texture,
                .src_box = { src_box.x, src_box.y, src_box.width, src_box.height },
                .dst_box = { dst_box.x, dst_box.y, dst_box.width, dst_box.height },
            },
        };

        render_pass_add_texture(target->render_pass, &options);
        wlr_texture_destroy(wlr_texture);

        pixman_region32_t clip_region;
        pixman_region32_init_rect(&clip_region, dst_box.x, dst_box.y, dst_box.width,
                                  dst_box.height);
        struct wlr_box inner_box = {
            .x = dst_box.x + WINDOW_BORDER,
            .y = dst_box.y + WINDOW_BORDER,
            .width = dst_box.width - 2 * WINDOW_BORDER,
            .height = dst_box.height - 2 * WINDOW_BORDER,
        };
        pixman_region32_t inner_region;
        pixman_region32_init_rect(&inner_region, inner_box.x, inner_box.y, inner_box.width,
                                  inner_box.height);
        pixman_region32_subtract(&clip_region, &clip_region, &inner_region);

        struct render_rect_options border = {
            .base = {
                .box = { dst_box.x, dst_box.y, dst_box.width, dst_box.height },
                .clip = &clip_region,
                .color = { 1, 1, 1, 1.0 },
            },
        };

        render_pass_add_rect(target->render_pass, &border);
        pixman_region32_fini(&clip_region);
        pixman_region32_fini(&inner_region);
    }

    return true;
}

static void zoom_output_destroy(struct zoom_output *output)
{
    wlr_output_lock_software_cursors(output->output->wlr_output, false);

    wl_list_remove(&output->link);
    wl_list_remove(&output->disable.link);
    free(output);
}

static void output_handle_disable(struct wl_listener *listener, void *data)
{
    struct zoom_output *output = wl_container_of(listener, output, disable);
    zoom_output_destroy(output);
}

static void zoom_output_create(struct zoom_effect *effect, struct kywc_output *kywc_output)
{
    struct zoom_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return;
    }

    output->output = output_from_kywc_output(kywc_output);
    output->disable.notify = output_handle_disable;
    wl_signal_add(&output->output->events.disable, &output->disable);

    wl_list_insert(&effect->outputs, &output->link);

    wlr_output_lock_software_cursors(output->output->wlr_output, true);
}

static bool output_create_request(struct kywc_output *kywc_output, int index, void *data)
{
    zoom_output_create(zoom, kywc_output);
    return false;
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct kywc_output *kywc_output = data;
    zoom_output_create(zoom, kywc_output);
}

static void zoom_init_state(struct zoom_effect *zoom)
{
    input_manager_for_each_seat(handle_seat, zoom);
    seat_add_new_listener(&zoom->new_seat);

    if (zoom->type == ZOOM_OUTPUT) {
        pixman_region32_init(&zoom->output_info.viewport_region);
        output_manager_for_each_output(output_create_request, true, zoom);
        zoom->new_enabled_output.notify = handle_new_enabled_output;
        output_manager_add_new_enabled_listener(&zoom->new_enabled_output);
    }
}

static void zoom_reset_state(struct zoom_effect *effect, uint32_t old_type)
{
    wl_list_remove(&effect->new_seat.link);
    wl_list_init(&effect->new_seat.link);

    struct seat_cursor *cursor, *cursor_tmp;
    wl_list_for_each_safe(cursor, cursor_tmp, &effect->seat_cursors, link) {
        seat_cursor_destroy(cursor);
    }

    if (old_type == ZOOM_OUTPUT) {
        pixman_region32_fini(&effect->output_info.viewport_region);

        wl_list_remove(&effect->new_enabled_output.link);
        wl_list_init(&effect->new_enabled_output.link);

        struct zoom_output *output, *output_tmp;
        wl_list_for_each_safe(output, output_tmp, &effect->outputs, link) {
            zoom_output_destroy(output);
        }

        struct scene_output *scene_output;
        wl_list_for_each(scene_output, &effect->scene->outputs, link) {
            scene_output_set_viewport_source_box(scene_output, NULL);
        }
    }
}

static void zoom_client_destory(void)
{
    if (!zoom->client) {
        return;
    }

    wl_list_remove(&zoom->client_destroy.link);
    wl_client_destroy(zoom->client);
    zoom->client = NULL;
}

static void handle_client_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&zoom->client_destroy.link);
    zoom->client = NULL;
}

static void zoom_client_create(void)
{
    zoom_client_destory();

    zoom->client = spawn_client(zoom->server->display, command);
    if (zoom->client) {
        zoom->client_destroy.notify = handle_client_destroy;
        wl_client_add_destroy_listener(zoom->client, &zoom->client_destroy);
    }
}

static int start_zoom_effect(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t type, scale;
    CK(sd_bus_message_read(m, "uu", &type, &scale));
    if (type != ZOOM_OUTPUT && type != ZOOM_WINDOW) {
        const sd_bus_error error = SD_BUS_ERROR_MAKE_CONST(
            "org.ukui.KWin.Zoom.Error.Cancelled", "Invalid zoom type, 0 = output, 1 = window");
        return sd_bus_reply_method_error(m, &error);
    }

    if (scale < 1 || scale > 16) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Scale must be between 1 to 16 ");
        return sd_bus_reply_method_error(m, &error);
    }

    if (zoom->enabled) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    if (!zoom->support_zoom_window && type == ZOOM_WINDOW) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    zoom->enabled = true;
    zoom->type = type;
    zoom->scale = scale;
    zoom_client_create();
    zoom_init_state(zoom);
    scene_damage_whole(zoom->scene);

    dbus_emit_signal(zoom_path, zoom_dbus_interface, "zoomStart", "");

    return sd_bus_reply_method_return(m, "b", true);
}

static int set_zoom_type(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t new_type;
    CK(sd_bus_message_read(m, "u", &new_type));
    if (new_type != ZOOM_OUTPUT && new_type != ZOOM_WINDOW) {
        const sd_bus_error error = SD_BUS_ERROR_MAKE_CONST(
            SD_BUS_ERROR_INVALID_ARGS, "Invalid zoom type, 0 = output, 1 = window");
        return sd_bus_reply_method_error(m, &error);
    }

    if (zoom->type == new_type) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    if (!zoom->support_zoom_window && new_type == ZOOM_WINDOW) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    uint32_t old_type = zoom->type;
    zoom->type = new_type;
    effect_set_option_int(zoom->effect, "type", new_type);

    if (zoom->enabled) {
        zoom_reset_state(zoom, old_type);
        zoom_init_state(zoom);
        scene_damage_whole(zoom->scene);
    }

    return sd_bus_reply_method_return(m, "b", true);
}

static int stop_zoom_effect(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    if (!zoom->enabled) {
        return sd_bus_reply_method_return(m, NULL);
    }

    zoom->enabled = false;
    zoom_reset_state(zoom, zoom->type);
    scene_damage_whole(zoom->scene);
    zoom_client_destory();
    dbus_emit_signal(zoom_path, zoom_dbus_interface, "zoomStop", "");

    return sd_bus_reply_method_return(m, NULL);
}

static int get_zoom_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_reply_method_return(m, "b", zoom->enabled);
}

static int get_zoom_scale(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_reply_method_return(m, "i", zoom->scale);
}

static int get_zoom_type(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_reply_method_return(m, "i", zoom->type);
}

static int set_zoom_out(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    if (!zoom->enabled || zoom->scale <= 1) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    zoom->scale--;
    scene_damage_whole(zoom->scene);
    dbus_emit_signal(zoom_path, zoom_dbus_interface, "scaleChanged", "u", zoom->scale);

    return sd_bus_reply_method_return(m, "b", true);
}

static int set_zoom_in(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    if (!zoom->enabled || zoom->scale >= MAX_SCALE) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    zoom->scale++;
    scene_damage_whole(zoom->scene);
    dbus_emit_signal(zoom_path, zoom_dbus_interface, "scaleChanged", "u", zoom->scale);

    return sd_bus_reply_method_return(m, "b", true);
}

static const sd_bus_vtable zoom_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("StartZoom", "uu", "b", start_zoom_effect, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SetMagnifierMode", "u", "b", set_zoom_type, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("ZoomIn", "", "b", set_zoom_in, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("ZoomOut", "", "b", set_zoom_out, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("IsZooming", "", "b", get_zoom_enabled, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetScale", "", "i", get_zoom_scale, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetType", "", "i", get_zoom_type, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("StopZoom", "", "", stop_zoom_effect, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

void effect_zoom_in(void)
{
    if (!zoom) {
        return;
    }

    if (!zoom->enabled) {
        if (zoom->type == ZOOM_OUTPUT || (zoom->type == ZOOM_WINDOW && zoom->support_zoom_window)) {
            zoom->enabled = true;
            zoom_client_create();
            zoom_init_state(zoom);
            scene_damage_whole(zoom->scene);
            dbus_emit_signal(zoom_path, zoom_dbus_interface, "zoomStart", "");
        }
        return;
    }

    if (zoom->scale < MAX_SCALE) {
        zoom->scale++;
        scene_damage_whole(zoom->scene);
        dbus_emit_signal(zoom_path, zoom_dbus_interface, "scaleChanged", "u", zoom->scale);
    }
}

void effect_zoom_out(void)
{
    if (!zoom) {
        return;
    }

    if (!zoom->enabled || zoom->scale <= 1) {
        return;
    }

    zoom->scale--;
    scene_damage_whole(zoom->scene);
    dbus_emit_signal(zoom_path, zoom_dbus_interface, "scaleChanged", "u", zoom->scale);
}

void effect_zoom_exit(void)
{
    if (!zoom) {
        return;
    }

    if (!zoom->enabled) {
        return;
    }

    zoom->enabled = false;
    zoom_reset_state(zoom, zoom->type);
    scene_damage_whole(zoom->scene);
    zoom_client_destory();
    dbus_emit_signal(zoom_path, zoom_dbus_interface, "zoomStop", "");
}

static void zoom_register_shortcuts(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(shortcuts); i++) {
        struct shortcut *shortcut = &shortcuts[i];
        struct key_binding *binding =
            kywc_key_binding_create(shortcut->keybind, shortcut->desc, shortcut->func_id, 0);
        if (!binding) {
            continue;
        }

        if (!kywc_key_binding_register(binding, function_entry_get_type(shortcut->func_id),
                                       function_manager_handle_key_binding, NULL)) {
            kywc_key_binding_destroy(binding);
            continue;
        }
    }
}

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    zoom->dbus = dbus_register_object(NULL, zoom_path, zoom_dbus_interface, zoom_vtable, zoom);
}

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    dbus_unregister_object(zoom->dbus);

    if (!zoom->enabled) {
        return;
    }

    zoom->enabled = false;
    zoom_reset_state(zoom, zoom->type);
    scene_damage_whole(zoom->scene);
    zoom_client_destory();
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&zoom->destroy.link);
    wl_list_remove(&zoom->enable.link);
    wl_list_remove(&zoom->disable.link);
    wl_list_remove(&zoom->new_seat.link);
    free(zoom);
    zoom = NULL;
}

static bool handle_allow_direct_scanout(struct effect *effect, struct scene_render_target *target)
{
    if (!zoom->enabled) {
        return true;
    }

    if (zoom->type == ZOOM_OUTPUT) {
        return !scene_find_effect_entity(zoom->scene, zoom->effect);
    }

    struct seat_cursor *cursor;
    wl_list_for_each(cursor, &zoom->seat_cursors, link) {
        if (kywc_box_intersects(&target->logical, &cursor->geo)) {
            return false;
        }
    }
    return true;
}

static const struct effect_interface zoom_impl = {
    .frame_render_pre = handle_frame_render_pre,
    .frame_render_end = handle_frame_render_end,
    .configure = handle_effect_configure,
    .allow_direct_scanout = handle_allow_direct_scanout,
};

bool zoom_effect_create(struct effect_manager *effect_manager)
{
    zoom = calloc(1, sizeof(*zoom));
    if (!zoom) {
        return false;
    }

    zoom->effect = effect_create("zoom", 122, false, &zoom_impl, zoom);
    if (!zoom->effect) {
        free(zoom);
        zoom = NULL;
        return false;
    }

    struct effect_entity *entity = scene_add_effect(effect_manager->server->scene, zoom->effect);
    if (!entity) {
        effect_destroy(zoom->effect);
        free(zoom);
        zoom = NULL;
        return false;
    }

    entity->user_data = zoom;
    zoom->server = effect_manager->server;
    zoom->scene = effect_manager->server->scene;
    /* vulkan renderer is not support */
    zoom->support_zoom_window = wlr_renderer_is_opengl(effect_manager->server->renderer) ||
                                wlr_renderer_is_pixman_renderer(effect_manager->server->renderer);

    zoom->enabled = false;
    zoom->scale = effect_get_option_int(zoom->effect, "scale", 2);
    zoom->type = effect_get_option_int(zoom->effect, "type", ZOOM_WINDOW);

    wl_list_init(&zoom->seat_cursors);
    wl_list_init(&zoom->outputs);

    zoom->enable.notify = handle_effect_enable;
    wl_signal_add(&zoom->effect->events.enable, &zoom->enable);
    zoom->disable.notify = handle_effect_disable;
    wl_signal_add(&zoom->effect->events.disable, &zoom->disable);
    zoom->destroy.notify = handle_effect_destroy;
    wl_signal_add(&zoom->effect->events.destroy, &zoom->destroy);
    zoom->new_seat.notify = handle_new_seat;
    wl_list_init(&zoom->new_seat.link);

    if (zoom->effect->enabled) {
        handle_effect_enable(&zoom->enable, zoom);
    }

    zoom_register_shortcuts();

    return true;
}
