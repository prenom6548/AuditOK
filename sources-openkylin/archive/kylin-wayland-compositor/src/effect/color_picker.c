// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <math.h>
#include <stdlib.h>

#include <drm_fourcc.h>

#include <kywc/log.h>

#include "effect_p.h"
#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "painter.h"
#include "render/pass.h"
#include "security/render.h"
#include "theme.h"
#include "util/dbus.h"
#include "util/macros.h"
#include "view/view.h"

#define MAGNIFIER_WIDTH (100)
#define MAGNIFIER_HEIGHT (80)
#define MAGNIFIER_OFFSET (10)
#define MAGNIFIER_BORDER (1)
#define MAGNIFIER_SOURCE_SIZE (20)
#define INFO_HEIGHT (30)
#define PREVIEW_SIZE (22)
#define PREVIEW_MARGIN (3)
#define FONT_SIZE (7)
#define RGB_FONT_FACE "Sans"

struct text_cache {
    struct wlr_buffer *buffer; // painter buffer
    struct wlr_texture *texture;
    int width, height;
    float scale;
    uint8_t r, g, b;
};

struct color_picker_effect {
    struct effect *effect;
    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    struct seat *seat;
    struct seat_pointer_grab pointer_grab;
    struct seat_keyboard_grab keyboard_grab;
    struct seat_touch_grab touch_grab;

    struct kywc_box magnifier_geo;
    double cursor_lx, cursor_ly;
    bool cursor_moved;

    uint8_t current_r, current_g, current_b;
    bool picking;

    struct text_cache text_cache;

    struct scene *scene;
    struct wlr_renderer *renderer;

    sd_bus_message *msg;
    struct dbus_object *dbus;
};

static const char *color_picker_dbus_interface = "com.kylin.Wlcom.ColorPicker";
static const char *color_picker_path = "/ColorPicker";

static void text_cache_destroy(struct text_cache *cache)
{
    if (cache->texture) {
        wlr_texture_destroy(cache->texture);
        cache->texture = NULL;
    }
    if (cache->buffer) {
        wlr_buffer_drop(cache->buffer);
        cache->buffer = NULL;
    }
}

static bool color_picker_update_text_texture(struct color_picker_effect *picker,
                                             struct scene_render_target *target)
{
    bool need_rebuild = !picker->text_cache.buffer || picker->text_cache.r != picker->current_r ||
                        picker->text_cache.g != picker->current_g ||
                        picker->text_cache.b != picker->current_b ||
                        picker->text_cache.scale != target->scale;
    if (!need_rebuild) {
        return true;
    }

    text_cache_destroy(&picker->text_cache);

    char text[64];
    snprintf(text, sizeof(text), "R G B\n%d,%d,%d", picker->current_r, picker->current_g,
             picker->current_b);

    struct draw_info info = {
        .width = MAGNIFIER_WIDTH,
        .height = INFO_HEIGHT,
        .scale = target->scale,
        .font_name = RGB_FONT_FACE,
        .font_rgba = (float[4]){ 1, 1, 1, 1 },
        .font_size = FONT_SIZE,
        .text = text,
        .align = TEXT_ALIGN_LEFT,
        .auto_resize = AUTO_RESIZE_ONLY,
        .text_attrs = TEXT_ATTR_MULTILINE,
    };

    picker->text_cache.buffer = painter_draw_buffer(&info);
    if (!picker->text_cache.buffer) {
        return true;
    }

    picker->text_cache.texture =
        wlr_texture_from_buffer(picker->renderer, picker->text_cache.buffer);
    painter_buffer_get_dest_size(picker->text_cache.buffer, &picker->text_cache.width,
                                 &picker->text_cache.height);

    picker->text_cache.r = picker->current_r;
    picker->text_cache.g = picker->current_g;
    picker->text_cache.b = picker->current_b;
    picker->text_cache.scale = target->scale;

    return true;
}

static void color_picker_read_color_from_buffer(struct color_picker_effect *picker,
                                                struct wlr_buffer *buffer,
                                                struct scene_render_target *target)
{
    if (!buffer) {
        return;
    }

    float x = picker->cursor_lx - target->logical.x;
    float y = picker->cursor_ly - target->logical.y;
    scene_render_point(&x, &y, target);

    if (x < 0 || x >= buffer->width || y < 0 || y >= buffer->height) {
        return;
    }

    render_pass_save(target->render_pass);

    wlr_renderer_begin_with_buffer(picker->renderer, buffer);

    uint8_t pixel_data[4];
    bool ok = wlr_renderer_read_pixels(picker->renderer, DRM_FORMAT_ARGB8888, 4, 1, 1, (int)x,
                                       (int)y, 0, 0, pixel_data);

    wlr_renderer_end(picker->renderer);

    render_pass_restore(target->render_pass);

    if (!ok) {
        kywc_log(KYWC_DEBUG, "color_picker read pixels failed");
        return;
    }

    uint32_t pixel = *(uint32_t *)pixel_data;
    uint8_t r = (pixel >> 16) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = pixel & 0xFF;

    if (picker->current_r != r || picker->current_g != g || picker->current_b != b) {
        picker->current_r = r;
        picker->current_g = g;
        picker->current_b = b;
    }
}

static void color_picker_update_magnifier_geo(struct color_picker_effect *picker)
{
    double pref_x, pref_y;
    int layout_width, layout_height;
    output_layout_get_size(&layout_width, &layout_height);
    int total_height = MAGNIFIER_HEIGHT + INFO_HEIGHT;

    if (picker->cursor_lx + MAGNIFIER_WIDTH + MAGNIFIER_OFFSET <= layout_width) {
        pref_x = ceil(picker->cursor_lx + MAGNIFIER_OFFSET);
    } else {
        pref_x = ceil(picker->cursor_lx - MAGNIFIER_WIDTH - MAGNIFIER_OFFSET);
    }

    if (picker->cursor_ly - total_height - MAGNIFIER_OFFSET >= 0) {
        pref_y = ceil(picker->cursor_ly - total_height - MAGNIFIER_OFFSET);
    } else {
        pref_y = ceil(picker->cursor_ly + MAGNIFIER_OFFSET);
    }

    int x = CLAMP(round(pref_x), 0, layout_width - MAGNIFIER_WIDTH);
    int y = CLAMP(round(pref_y), 0, layout_height - total_height);

    picker->magnifier_geo.x = x;
    picker->magnifier_geo.y = y;
    picker->magnifier_geo.width = MAGNIFIER_WIDTH;
    picker->magnifier_geo.height = total_height;
}

static void color_picker_finish(struct color_picker_effect *picker, bool success, uint8_t r,
                                uint8_t g, uint8_t b)
{
    if (!picker->msg) {
        return;
    }

    if (success) {
        sd_bus_reply_method_return(picker->msg, "ddd", r / 255.0, g / 255.0, b / 255.0);
    } else {
        const sd_bus_error error = SD_BUS_ERROR_MAKE_CONST(
            "com.kylin.Wlcom.ColorPicker.Error.Cancelled", "Color picking was cancelled");
        sd_bus_reply_method_error(picker->msg, &error);
    }

    sd_bus_message_unref(picker->msg);
    picker->msg = NULL;
}

static void color_picker_stop(struct color_picker_effect *picker)
{
    if (!picker->picking) {
        return;
    }

    picker->picking = false;

    seat_end_pointer_grab(picker->seat, &picker->pointer_grab);
    seat_end_keyboard_grab(picker->seat, &picker->keyboard_grab);
    seat_end_touch_grab(picker->seat, &picker->touch_grab);

    cursor_lock_image(picker->seat->cursor, false);
    cursor_set_image(picker->seat->cursor, CURSOR_DEFAULT);

    text_cache_destroy(&picker->text_cache);

    if (picker->msg) {
        color_picker_finish(picker, false, 0, 0, 0);
    }

    view_activate_topmost(false);

    scene_damage_whole(picker->scene);
}

static bool pointer_grab_motion(struct seat_pointer_grab *pointer_grab, uint32_t time, double lx,
                                double ly)
{
    struct color_picker_effect *picker = pointer_grab->data;
    picker->cursor_lx = lx;
    picker->cursor_ly = ly;
    picker->cursor_moved = true;
    return true;
}

static bool pointer_grab_button(struct seat_pointer_grab *pointer_grab, uint32_t time,
                                uint32_t button, bool pressed)
{
    struct color_picker_effect *picker = pointer_grab->data;
    if (!pressed) {
        return true;
    }

    color_picker_finish(picker, true, picker->current_r, picker->current_g, picker->current_b);
    color_picker_stop(picker);
    return true;
}

static bool pointer_grab_axis(struct seat_pointer_grab *pointer_grab, uint32_t time, bool vertical,
                              double value)
{
    return true;
}

static void pointer_grab_cancel(struct seat_pointer_grab *pointer_grab)
{
    struct color_picker_effect *picker = pointer_grab->data;
    color_picker_stop(picker);
}

static const struct seat_pointer_grab_interface pointer_grab_impl = {
    .motion = pointer_grab_motion,
    .button = pointer_grab_button,
    .axis = pointer_grab_axis,
    .cancel = pointer_grab_cancel,
};

static bool keyboard_grab_key(struct seat_keyboard_grab *keyboard_grab, struct keyboard *keyboard,
                              uint32_t time, uint32_t key, bool pressed, uint32_t modifiers)
{
    struct color_picker_effect *picker = keyboard_grab->data;
    if (pressed && key == KEY_ESC) {
        color_picker_stop(picker);
        return true;
    }
    return false;
}

static void keyboard_grab_cancel(struct seat_keyboard_grab *keyboard_grab)
{
    struct color_picker_effect *picker = keyboard_grab->data;
    color_picker_stop(picker);
}

static const struct seat_keyboard_grab_interface keyboard_grab_impl = {
    .key = keyboard_grab_key,
    .cancel = keyboard_grab_cancel,
};

static bool touch_grab_touch(struct seat_touch_grab *touch_grab, uint32_t time, bool down,
                             uint32_t serial)
{
    if (down) {
        return true;
    }

    struct color_picker_effect *picker = touch_grab->data;
    color_picker_finish(picker, true, picker->current_r, picker->current_g, picker->current_b);
    color_picker_stop(picker);
    return true;
}

static bool touch_grab_motion(struct seat_touch_grab *touch_grab, uint32_t time, double lx,
                              double ly, uint32_t serial)
{
    struct color_picker_effect *picker = touch_grab->data;
    picker->cursor_lx = lx;
    picker->cursor_ly = ly;
    picker->cursor_moved = true;
    return true;
}

static void touch_grab_cancel(struct seat_touch_grab *touch_grab)
{
    struct color_picker_effect *picker = touch_grab->data;
    color_picker_stop(picker);
}

static const struct seat_touch_grab_interface touch_grab_impl = {
    .touch = touch_grab_touch,
    .motion = touch_grab_motion,
    .cancel = touch_grab_cancel,
};

static bool handle_frame_render_pre(struct effect_entity *entity,
                                    struct scene_render_target *target)
{
    struct color_picker_effect *picker = entity->user_data;
    if (!picker->picking) {
        return true;
    }

    bool need_damage =
        picker->cursor_moved ||
        pixman_region32_contains_point(&target->damage, picker->cursor_lx, picker->cursor_ly, NULL);

    picker->cursor_moved = false;

    struct kywc_box old_geo = picker->magnifier_geo;
    color_picker_update_magnifier_geo(picker);

    if (need_damage) {
        pixman_region32_t region;
        pixman_region32_init_rect(&region, picker->magnifier_geo.x, picker->magnifier_geo.y,
                                  picker->magnifier_geo.width, picker->magnifier_geo.height);
        if (!kywc_box_equal(&old_geo, &picker->magnifier_geo)) {
            pixman_region32_union_rect(&region, &region, old_geo.x, old_geo.y, old_geo.width,
                                       old_geo.height);
        }
        scene_add_damage(picker->scene, &region);
        pixman_region32_fini(&region);
    }

    return true;
}

static void draw_bordered_rect(struct wlr_render_pass *pass, const struct kywc_box *box,
                               int border_width, const float color[4])
{
    pixman_region32_t outer, inner, border;
    pixman_region32_init_rect(&outer, box->x, box->y, box->width, box->height);
    pixman_region32_init_rect(&inner, box->x + border_width, box->y + border_width,
                              box->width - 2 * border_width, box->height - 2 * border_width);
    pixman_region32_init(&border);
    pixman_region32_subtract(&border, &outer, &inner);

    struct render_rect_options options = {
        .base = {
            .box = { box->x, box->y, box->width, box->height },
            .clip = &border,
            .color = { color[0], color[1], color[2], color[3] },
        },
    };
    render_pass_add_rect(pass, &options);

    pixman_region32_fini(&outer);
    pixman_region32_fini(&inner);
    pixman_region32_fini(&border);
}

static void draw_magnifier_crossline(struct wlr_render_pass *pass, const struct kywc_box *dst_box)
{
    int center_x = dst_box->x + dst_box->width / 2;
    int center_y = dst_box->y + dst_box->height / 2;

    struct theme *theme = theme_manager_get_theme();
    render_pass_add_rect(pass, &(struct render_rect_options){
                                   .base = {
                                       .box = { dst_box->x + 1, center_y - MAGNIFIER_BORDER / 2,
                                                dst_box->width - 2, MAGNIFIER_BORDER },
                                       .color = { theme->accent_color[0], theme->accent_color[1],
                                                  theme->accent_color[2], theme->accent_color[3] },
                                   } });

    render_pass_add_rect(pass, &(struct render_rect_options){
                                   .base = {
                                       .box = { center_x - MAGNIFIER_BORDER / 2, dst_box->y + 1,
                                                MAGNIFIER_BORDER, dst_box->height - 2 },
                                       .color = { theme->accent_color[0], theme->accent_color[1],
                                                  theme->accent_color[2], theme->accent_color[3] },
                                   } });
}

static bool handle_frame_render_end(struct effect_entity *entity,
                                    struct scene_render_target *target)
{
    struct color_picker_effect *picker = entity->user_data;
    if (!picker->picking) {
        return true;
    }

    if (!kywc_box_intersects(&picker->magnifier_geo, &target->logical)) {
        return true;
    }

    struct kywc_box dst_box = {
        .x = picker->magnifier_geo.x,
        .y = picker->magnifier_geo.y,
        .width = MAGNIFIER_WIDTH,
        .height = MAGNIFIER_HEIGHT,
    };
    scene_render_target_box(target, &dst_box);

    float cursor_buf_x = picker->cursor_lx - target->logical.x;
    float cursor_buf_y = picker->cursor_ly - target->logical.y;
    scene_render_point(&cursor_buf_x, &cursor_buf_y, target);

    double half_source = (MAGNIFIER_SOURCE_SIZE * 0.5) * target->scale;
    double src_x = cursor_buf_x - half_source;
    double src_y = cursor_buf_y - half_source;
    double src_width = MAGNIFIER_SOURCE_SIZE * target->scale;
    double src_height = MAGNIFIER_SOURCE_SIZE * target->scale;

    src_x = fmax(0, fmin(src_x, target->buffer->width - src_width));
    src_y = fmax(0, fmin(src_y, target->buffer->height - src_height));

    /* check if the panel (magnifier + info bar + border) intersects damage */
    struct kywc_box panel_box = picker->magnifier_geo;
    scene_render_target_box(target, &panel_box);
    panel_box.x -= MAGNIFIER_BORDER;
    panel_box.y -= MAGNIFIER_BORDER;
    panel_box.width += 2 * MAGNIFIER_BORDER;
    panel_box.height += 2 * MAGNIFIER_BORDER;
    if (!(target->options & SCENE_RENDER_ENABLE_SECURITY) &&
        security_check_buffer(target->buffer)) {
        security_target_add_buffer(target, target->buffer,
                                   &(struct kywc_fbox){ src_x, src_y, src_width, src_height },
                                   target->transform, &dst_box);

        pixman_region32_t info_region, zoom_region;
        pixman_region32_init_rect(
            &zoom_region, dst_box.x - MAGNIFIER_BORDER, dst_box.y - MAGNIFIER_BORDER,
            dst_box.width + 2 * MAGNIFIER_BORDER, dst_box.height + 2 * MAGNIFIER_BORDER);
        pixman_region32_init_rect(&info_region, panel_box.x, panel_box.y, panel_box.width,
                                  panel_box.height);
        pixman_region32_subtract(&info_region, &info_region, &zoom_region);
        security_target_add_buffer_with_region(target, target->buffer, &info_region);
        pixman_region32_fini(&info_region);
        pixman_region32_fini(&zoom_region);
    }

    pixman_region32_t panel_region;
    pixman_region32_init(&panel_region);
    pixman_region32_copy(&panel_region, &target->damage);
    scene_render_target_region(target, &panel_region);
    pixman_region32_intersect_rect(&panel_region, &panel_region, panel_box.x, panel_box.y,
                                   panel_box.width, panel_box.height);
    bool panel_damaged = pixman_region32_not_empty(&panel_region);
    pixman_region32_fini(&panel_region);

    /* if panel itself is not damaged, check whether the sampled screen area changed */
    bool src_damaged = false;
    if (!panel_damaged) {
        struct kywc_box src_box = {
            .x = picker->cursor_lx - (MAGNIFIER_SOURCE_SIZE * 0.5),
            .y = picker->cursor_ly - (MAGNIFIER_SOURCE_SIZE * 0.5),
            .width = MAGNIFIER_SOURCE_SIZE,
            .height = MAGNIFIER_SOURCE_SIZE,
        };
        pixman_region32_t src_region;
        pixman_region32_init_rect(&src_region, src_box.x, src_box.y, src_box.width, src_box.height);
        pixman_region32_intersect(&src_region, &src_region, &target->damage);
        src_damaged = pixman_region32_not_empty(&src_region);
        pixman_region32_fini(&src_region);
    }

    if (!panel_damaged && !src_damaged) {
        return true;
    }

    /* read color from buffer */
    color_picker_read_color_from_buffer(picker, target->buffer, target);

    struct wlr_texture *wlr_texture = wlr_texture_from_buffer(picker->renderer, target->buffer);
    if (!wlr_texture) {
        return true;
    }

    struct render_texture_options options = {
                .base = {
                    .texture = wlr_texture,
                    .src_box = { src_x, src_y, src_width, src_height },
                    .dst_box = { dst_box.x, dst_box.y, dst_box.width, dst_box.height },
                    .transform = WL_OUTPUT_TRANSFORM_NORMAL,
                    .filter_mode = WLR_SCALE_FILTER_NEAREST,
                },
            };
    render_pass_add_texture(target->render_pass, &options);
    wlr_texture_destroy(wlr_texture);

    /* add two crossline */
    draw_magnifier_crossline(target->render_pass, &dst_box);

    /* add info bar background */
    struct kywc_box info_bg_box = {
        .x = picker->magnifier_geo.x,
        .y = picker->magnifier_geo.y + MAGNIFIER_HEIGHT,
        .width = MAGNIFIER_WIDTH,
        .height = INFO_HEIGHT,
    };
    scene_render_target_box(target, &info_bg_box);
    /* shrink info bg box to avoid covering the panel border */
    info_bg_box.x += MAGNIFIER_BORDER;
    info_bg_box.width -= 2 * MAGNIFIER_BORDER;

    struct render_rect_options bg_rect = {
        .base = {
            .box = { info_bg_box.x, info_bg_box.y, info_bg_box.width, info_bg_box.height },
            .color = { 0.15f, 0.15f, 0.15f, 1.0f },
        },
    };
    render_pass_add_rect(target->render_pass, &bg_rect);

    /* add preview square */
    struct kywc_box preview_box = {
        .x = picker->magnifier_geo.x + PREVIEW_MARGIN,
        .y = picker->magnifier_geo.y + MAGNIFIER_HEIGHT + PREVIEW_MARGIN + 1,
        .width = PREVIEW_SIZE,
        .height = PREVIEW_SIZE,
    };
    scene_render_target_box(target, &preview_box);

    struct render_rect_options preview_rect = {
        .base = {
            .box = { preview_box.x, preview_box.y, preview_box.width, preview_box.height },
            .color = { picker->current_r / 255.0f, picker->current_g / 255.0f,
                       picker->current_b / 255.0f, 1.0f },
        },
    };
    render_pass_add_rect(target->render_pass, &preview_rect);

    /* add preview square border */
    const float preview_border_color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    draw_bordered_rect(target->render_pass, &preview_box, 1, preview_border_color);

    /* text: "R G B\nR,G,B" */
    color_picker_update_text_texture(picker, target);
    if (picker->text_cache.texture) {
        struct kywc_box text_box = {
            .x = picker->magnifier_geo.x + PREVIEW_SIZE + 2 * PREVIEW_MARGIN,
            .y = picker->magnifier_geo.y + MAGNIFIER_HEIGHT + 2,
            .width = picker->text_cache.width,
            .height = picker->text_cache.height,
        };
        scene_render_target_box(target, &text_box);

        struct render_texture_options text_options = {
            .base = {
                .texture = picker->text_cache.texture,
                .src_box = { 0, 0, picker->text_cache.buffer->width,
                            picker->text_cache.buffer->height },
                .dst_box = { text_box.x, text_box.y, text_box.width, text_box.height },
                .transform = target->transform,
            },
        };
        render_pass_add_texture(target->render_pass, &text_options);
    }

    /* add border for the whole panel (magnifier + info bar) */
    const float border_color[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
    struct kywc_box total_box = {
        .x = picker->magnifier_geo.x,
        .y = picker->magnifier_geo.y,
        .width = MAGNIFIER_WIDTH,
        .height = MAGNIFIER_HEIGHT + INFO_HEIGHT,
    };
    scene_render_target_box(target, &total_box);
    draw_bordered_rect(target->render_pass, &total_box, MAGNIFIER_BORDER, border_color);

    return true;
}

static int color_picker_pick_color(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct color_picker_effect *picker = userdata;
    if (picker->picking) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_FAILED, "Color picker is already active");
        return sd_bus_reply_method_error(msg, &error);
    }

    struct seat *seat = input_manager_get_default_seat();
    if (!seat) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_FAILED, "No seat available");
        return sd_bus_reply_method_error(msg, &error);
    }

    picker->seat = seat;
    picker->cursor_lx = seat->cursor->lx;
    picker->cursor_ly = seat->cursor->ly;

    picker->picking = true;
    picker->cursor_moved = true;
    picker->msg = sd_bus_message_ref(msg);
    picker->current_r = picker->current_g = picker->current_b = 0;

    seat_focus_surface(seat, NULL);

    cursor_set_image(seat->cursor, CURSOR_CROSSHAIR);
    cursor_lock_image(seat->cursor, true);

    picker->pointer_grab = (struct seat_pointer_grab){ &pointer_grab_impl, seat, picker };
    seat_start_pointer_grab(seat, &picker->pointer_grab);
    picker->keyboard_grab = (struct seat_keyboard_grab){ &keyboard_grab_impl, seat, picker };
    seat_start_keyboard_grab(seat, &picker->keyboard_grab);
    picker->touch_grab = (struct seat_touch_grab){ &touch_grab_impl, seat, picker, 0 };
    seat_start_touch_grab(seat, &picker->touch_grab);

    scene_damage_whole(picker->scene);

    return 1;
}

static const sd_bus_vtable color_picker_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("PickColor", "", "ddd", color_picker_pick_color, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    struct color_picker_effect *picker = wl_container_of(listener, picker, enable);
    picker->dbus = dbus_register_object(NULL, color_picker_path, color_picker_dbus_interface,
                                        color_picker_vtable, picker);
}

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    struct color_picker_effect *picker = wl_container_of(listener, picker, disable);
    color_picker_stop(picker);
    dbus_unregister_object(picker->dbus);
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct color_picker_effect *picker = wl_container_of(listener, picker, destroy);

    text_cache_destroy(&picker->text_cache);

    wl_list_remove(&picker->destroy.link);
    wl_list_remove(&picker->enable.link);
    wl_list_remove(&picker->disable.link);

    free(picker);
}

static bool handle_allow_direct_scanout(struct effect *effect, struct scene_render_target *target)
{
    struct color_picker_effect *picker = effect->user_data;
    if (!picker->picking) {
        return true;
    }

    if (kywc_box_intersects(&target->logical, &picker->magnifier_geo)) {
        return false;
    }

    return true;
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }
    return false;
}

static const struct effect_interface color_picker_impl = {
    .frame_render_pre = handle_frame_render_pre,
    .frame_render_end = handle_frame_render_end,
    .configure = handle_effect_configure,
    .allow_direct_scanout = handle_allow_direct_scanout,
};

bool color_picker_effect_create(struct effect_manager *effect_manager)
{
    struct color_picker_effect *color_picker = calloc(1, sizeof(*color_picker));
    if (!color_picker) {
        return false;
    }

    color_picker->effect =
        effect_create("color_picker", 116, true, &color_picker_impl, color_picker);
    if (!color_picker->effect) {
        free(color_picker);
        return false;
    }

    struct effect_entity *entity =
        scene_add_effect(effect_manager->server->scene, color_picker->effect);
    if (!entity) {
        effect_destroy(color_picker->effect);
        free(color_picker);
        return false;
    }

    entity->user_data = color_picker;
    color_picker->renderer = effect_manager->server->renderer;
    color_picker->scene = effect_manager->server->scene;

    color_picker->enable.notify = handle_effect_enable;
    wl_signal_add(&color_picker->effect->events.enable, &color_picker->enable);
    color_picker->disable.notify = handle_effect_disable;
    wl_signal_add(&color_picker->effect->events.disable, &color_picker->disable);
    color_picker->destroy.notify = handle_effect_destroy;
    wl_signal_add(&color_picker->effect->events.destroy, &color_picker->destroy);

    if (color_picker->effect->enabled) {
        handle_effect_enable(&color_picker->enable, color_picker);
    }

    return true;
}
