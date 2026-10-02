// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_output.h>

#include "effect_p.h"
#include "output.h"
#include "painter.h"
#include "scene/scene.h"
#include "util/macros.h"
#include "view/view.h"

#define MAX_FRAMES (1024)
#define FRAME_MASK (MAX_FRAMES - 1)
#define OFFSET_X (5)
#define OFFSET_Y (5)

struct frame_output {
    struct wl_list link;
    struct showfps_effect *effect;

    struct scene_output *output;
    struct wl_listener present; // wlr_output
    struct wl_listener destroy;

    struct wlr_buffer *buffer; // painter buffer
    struct wlr_texture *texture;
    int dst_width, dst_height;
    int shown_fps;

    struct kywc_box damage;

    uint32_t frames[MAX_FRAMES];
    int head, tail, count, fps;
};

struct showfps_effect {
    struct wl_list outputs;
    struct wl_listener new_enabled_output;

    struct effect *effect;
    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    struct effect_manager *manager;
    /* always schedule frame by default */
    bool always_repaint;
    bool allow_direct_scanout;
};

static void frame_output_destroy(struct frame_output *output)
{
    wl_list_remove(&output->link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->present.link);

    wlr_texture_destroy(output->texture);
    wlr_buffer_drop(output->buffer);

    free(output);
}

static void output_handle_destroy(struct wl_listener *listener, void *data)
{
    struct frame_output *output = wl_container_of(listener, output, destroy);
    frame_output_destroy(output);
}

static void output_handle_present(struct wl_listener *listener, void *data)
{
    struct frame_output *output = wl_container_of(listener, output, present);
    struct wlr_output_event_present *event = data;
    /* no event->when when event->presented is false */
    if (!event->presented) {
        return;
    }

    uint32_t now = (uint32_t)((uint64_t)event->when->tv_sec * 1000ULL +
                              (uint64_t)event->when->tv_nsec / 1000000ULL);

    while (output->count > 0) {
        if (now - output->frames[output->tail] >= 1000) {
            output->tail = (output->tail + 1) & FRAME_MASK;
            output->count--;
        } else {
            break;
        }
    }

    output->frames[output->head] = now;
    output->head = (output->head + 1) & FRAME_MASK;
    output->count++;

    if (output->count > MAX_FRAMES) {
        output->tail = (output->tail + 1) & FRAME_MASK;
        output->count = MAX_FRAMES;
    }

    output->fps = output->count;
}

static void frame_output_create(struct showfps_effect *effect, struct scene_output *scene_output)
{
    struct frame_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return;
    }

    output->effect = effect;
    wl_list_insert(&effect->outputs, &output->link);

    output->output = scene_output;
    output->present.notify = output_handle_present;
    wl_signal_add(&scene_output->output->events.present, &output->present);
    output->destroy.notify = output_handle_destroy;
    wl_signal_add(&scene_output->events.destroy, &output->destroy);

    output_schedule_frame(output->output->output);
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct showfps_effect *effect = wl_container_of(listener, effect, new_enabled_output);
    struct kywc_output *output = data;
    frame_output_create(effect, output_from_kywc_output(output)->scene_output);
}

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    struct showfps_effect *effect = wl_container_of(listener, effect, enable);
    assert(wl_list_empty(&effect->outputs));

    struct scene *scene = effect->manager->server->scene;
    struct scene_output *output;
    wl_list_for_each(output, &scene->outputs, link) {
        frame_output_create(effect, output);
    }

    effect->new_enabled_output.notify = handle_new_enabled_output;
    output_manager_add_new_enabled_listener(&effect->new_enabled_output);
}

static void frame_output_add_damage(struct frame_output *output)
{
    pixman_region32_t damage;
    pixman_region32_init_rect(&damage, output->damage.x, output->damage.y, output->damage.width,
                              output->damage.height);
    scene_add_damage(output->effect->manager->server->scene, &damage);
    pixman_region32_fini(&damage);
}

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    struct showfps_effect *effect = wl_container_of(listener, effect, disable);
    wl_list_remove(&effect->new_enabled_output.link);
    wl_list_init(&effect->new_enabled_output.link);

    struct frame_output *output, *tmp;
    wl_list_for_each_safe(output, tmp, &effect->outputs, link) {
        if (!effect->always_repaint) {
            frame_output_add_damage(output);
        }
        frame_output_destroy(output);
    }
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct showfps_effect *effect = wl_container_of(listener, effect, destroy);
    assert(wl_list_empty(&effect->outputs));
    wl_list_remove(&effect->destroy.link);
    wl_list_remove(&effect->enable.link);
    wl_list_remove(&effect->disable.link);
    free(effect);
}

static bool frame_output_draw_fps(struct frame_output *output)
{
    if (output->texture && output->shown_fps == output->fps) {
        return true;
    }

    wlr_texture_destroy(output->texture);
    wlr_buffer_drop(output->buffer);

    struct wlr_output *wlr_output = output->output->output;
    char text[4];
    snprintf(text, sizeof(text), "%3d", output->fps);

    struct draw_info info = {
        .width = 500,
        .height = 200,
        .scale = wlr_output->scale,

        .font_name = "monospace",
        .font_rgba = (float[4]){ 1, 0, 0, 1 },
        .font_size = 20,
        .text = text,
        .align = TEXT_ALIGN_CENTER,
        .auto_resize = AUTO_RESIZE_ONLY,
    };

    output->buffer = painter_draw_buffer(&info);
    if (!output->buffer) {
        output->texture = NULL;
        return false;
    }

    output->texture = wlr_texture_from_buffer(wlr_output->renderer, output->buffer);
    painter_buffer_get_dest_size(output->buffer, &output->dst_width, &output->dst_height);
    output->shown_fps = output->fps;

    return true;
}

static struct frame_output *frame_output_get(struct showfps_effect *effect,
                                             struct scene_output *scene_output)
{
    struct frame_output *output;
    wl_list_for_each(output, &effect->outputs, link) {
        if (output->output == scene_output) {
            return output;
        }
    }
    return NULL;
}

static bool handle_frame_render_begin(struct effect_entity *entity,
                                      struct scene_render_target *target)
{
    struct showfps_effect *effect = entity->user_data;
    struct frame_output *output = frame_output_get(effect, target->output);
    if (!output) {
        return true;
    }

    int width = output->dst_width;
    int height = output->dst_height;

    if (!frame_output_draw_fps(output)) {
        return true;
    }

    int x = target->logical.x + OFFSET_X;
    int y = target->logical.y + OFFSET_Y;
    output->damage = (struct kywc_box){ x, y, output->dst_width, output->dst_height };

    /* dmamage will be added in post to next frame */
    if (effect->always_repaint) {
        return true;
    }

    width = MAX(width, output->dst_width);
    height = MAX(height, output->dst_height);
    pixman_region32_union_rect(&target->damage, &target->damage, x, y, width, height);
    /* tricks to add damage to buffer damage */
    pixman_region32_t *current = &target->output->damage_ring.current;
    pixman_region32_union_rect(current, current, OFFSET_X, OFFSET_Y, width, height);

    return true;
}

static bool handle_frame_render_end(struct effect_entity *entity,
                                    struct scene_render_target *target)
{
    struct showfps_effect *effect = entity->user_data;
    struct frame_output *output = frame_output_get(effect, target->output);
    if (!output || !output->texture) {
        return true;
    }

    struct wlr_box dst_box = { OFFSET_X, OFFSET_Y, output->dst_width, output->dst_height };
    scene_render_box(&dst_box, target);

    struct wlr_render_texture_options options = {
        .texture = output->texture,
        .dst_box = dst_box,
        .transform = target->transform,
    };

    wlr_render_pass_add_texture(target->render_pass, &options);

    return true;
}

static bool handle_frame_render_post(struct effect_entity *entity,
                                     struct scene_render_target *target)
{
    struct showfps_effect *effect = entity->user_data;
    if (!effect->always_repaint) {
        return true;
    }

    struct frame_output *output = frame_output_get(effect, target->output);
    if (!output || !output->texture) {
        return true;
    }

    frame_output_add_damage(output);

    return true;
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    struct showfps_effect *showfps = effect->user_data;

    if (strcmp(option->key, "always_repaint") == 0) {
        if (showfps->always_repaint == option->value.boolean) {
            return false;
        }
        showfps->always_repaint = option->value.boolean;
        /* schedule all outputs if always_repaint changes to true */
        if (showfps->always_repaint) {
            struct frame_output *output;
            wl_list_for_each(output, &showfps->outputs, link) {
                output_schedule_frame(output->output->output);
            }
        }
    } else if (strcmp(option->key, "allow_direct_scanout") == 0) {
        if (showfps->allow_direct_scanout == option->value.boolean) {
            return false;
        }
        showfps->allow_direct_scanout = option->value.boolean;
    } else {
        return false;
    }

    return true;
}

static bool handle_allow_direct_scanout(struct effect *effect, struct scene_render_target *target)
{
    struct showfps_effect *showfps = effect->user_data;
    return showfps->allow_direct_scanout;
}

static const struct effect_interface showfps_effect_impl = {
    .frame_render_begin = handle_frame_render_begin,
    .frame_render_end = handle_frame_render_end,
    .frame_render_post = handle_frame_render_post,
    .configure = handle_effect_configure,
    .allow_direct_scanout = handle_allow_direct_scanout,
};

bool showfps_effect_create(struct effect_manager *manager)
{
    struct showfps_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return false;
    }

    effect->effect = effect_create("showfps", 112, false, &showfps_effect_impl, effect);
    if (!effect->effect) {
        free(effect);
        return false;
    }

    struct effect_entity *entity = scene_add_effect(manager->server->scene, effect->effect);
    if (!entity) {
        effect_destroy(effect->effect);
        free(effect);
        return false;
    }

    entity->user_data = effect;
    effect->manager = manager;
    effect->effect->category = EFFECT_CATEGORY_UTILS;
    effect->always_repaint = effect_get_option_boolean(effect->effect, "always_repaint", true);
    effect->allow_direct_scanout =
        effect_get_option_boolean(effect->effect, "allow_direct_scanout", false);

    wl_list_init(&effect->outputs);
    wl_list_init(&effect->new_enabled_output.link);

    effect->enable.notify = handle_effect_enable;
    wl_signal_add(&effect->effect->events.enable, &effect->enable);
    effect->disable.notify = handle_effect_disable;
    wl_signal_add(&effect->effect->events.disable, &effect->disable);
    effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&effect->effect->events.destroy, &effect->destroy);

    if (effect->effect->enabled) {
        handle_effect_enable(&effect->enable, NULL);
    }

    return true;
}
