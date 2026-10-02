// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <kywc/log.h>
#include <wayland-util.h>

#include "effect/output_transform.h"
#include "effect_p.h"
#include "output.h"
#include "render/opengl.h"
#include "scene/thumbnail.h"
#include "security/render.h"
#include "util/anim.h"
#include "util/macros.h"
#include "util/time.h"

#define SCALE_ANIM_SIZE (0.9)

#define ROTATE_ANIM_CH_ANGLE (0)
#define ROTATE_ANIM_CH_ALPHA (1)
#define ROTATE_ANIM_CH_WIDTH (2)
#define ROTATE_ANIM_CH_HEIGHT (3)

// output - output_entry - effect_entity. life cycle: output_entry = effect_entity
struct output_entry {
    struct output_transform_effect *effect;
    struct effect_entity *effect_entity;
    struct scene_output *output;
    struct wl_listener destroy;
    enum output_transform_effect_type type;
    enum wl_output_transform last_transform;
    // animate
    float width, height, angle, alpha;
    uint32_t start_time;
    struct animtr animtr_scale;
    struct anim anim_rotate;
    struct animtr animtr_rotate;
    // output screenshot
    struct thumbnail *old_thumbnail;
    struct wlr_texture *old_thumbnail_texture;
    struct wlr_buffer *old_thumbnail_buffer;
    struct opengl_texture *old_security_texture;
    struct thumbnail *new_thumbnail;
    struct wlr_texture *new_thumbnail_texture;
    struct wlr_buffer *new_thumbnail_buffer;
    struct opengl_texture *new_security_texture;
    enum wl_output_transform new_thumbnail_transform;
};

static void anim_alpha_callback(void *target, float value)
{
    struct output_entry *output = target;
    output->alpha = value;
}

static float anim_scale_alpha_easing(float t)
{
    const float alpha_mid = 0.5f;

    if (t > alpha_mid) {
        return 1.0f;
    } else {
        float x = t / alpha_mid;
        return x * x;
    }
}

static float anim_scale_easing(float t)
{
    const float phase_first = 0.4f;
    const float phase_middle = 0.2f;
    const float phase_last = 1.0f - phase_first - phase_middle;
    const float first_end = phase_first;
    const float middle_end = phase_first + phase_middle;

    if (t < first_end) {
        float u = t / first_end;
        float factor = 1.0f - u * u * u;
        return SCALE_ANIM_SIZE + (1.0f - SCALE_ANIM_SIZE) * factor;
    } else if (t < middle_end) {
        return SCALE_ANIM_SIZE;
    } else {
        float u = (t - middle_end) / phase_last;
        float factor = u * u * u;
        return SCALE_ANIM_SIZE + (1.0f - SCALE_ANIM_SIZE) * factor;
    }
}

static void anim_scale_callback(void *target, float value)
{
    struct output_entry *output = target;
    struct wlr_output *wlr_output = output->output->output;
    output->width = wlr_output->width * value;
    output->height = wlr_output->height * value;
}

static struct anim anim_scale = {
    .channels_num = 2,
    .channels =
        (struct anim_channel[]){
            {
                .start = 0.0f,
                .end = 1.0f,
                .easing = { .easing.function = anim_scale_easing },
                .callback = anim_scale_callback,
            },
            {
                .start = 0.0f,
                .end = 1.0f,
                .easing = { .easing.function = anim_scale_alpha_easing },
                .callback = anim_alpha_callback,
            },
        },
};

static float anim_rotate_alpha_easing(float t)
{
    return CLAMP((t - 0.5f) * 2.0f, 0.0f, 1.0f);
}

static void anim_ch_angle_callback(void *target, float value)
{
    struct output_entry *output = target;
    output->angle = value;
}

static void anim_ch_width_callback(void *target, float value)
{
    struct output_entry *output = target;
    output->width = value;
}

static void anim_ch_height_callback(void *target, float value)
{
    struct output_entry *output = target;
    output->height = value;
}

struct output_transform_effect {
    struct effect *effect;
    struct wl_listener destroy;
    struct effect_manager *manager;
    uint32_t anim_scale_duration;
    uint32_t anim_rotate_duration;
};

static struct output_transform_effect *transform_effect = NULL;

static int output_transform_to_angle(enum wl_output_transform transform)
{
    if (transform == WL_OUTPUT_TRANSFORM_90) {
        return 90;
    } else if (transform == WL_OUTPUT_TRANSFORM_180) {
        return 180;
    } else if (transform == WL_OUTPUT_TRANSFORM_270) {
        return 270;
    } else if (transform == WL_OUTPUT_TRANSFORM_FLIPPED) {
        return 360;
    } else if (transform == WL_OUTPUT_TRANSFORM_FLIPPED_90) {
        return 90;
    } else if (transform == WL_OUTPUT_TRANSFORM_FLIPPED_180) {
        return 180;
    } else if (transform == WL_OUTPUT_TRANSFORM_FLIPPED_270) {
        return 270;
    } else {
        return 0;
    }
}

static enum wl_output_transform diff_angle_to_new_thumbnail_transform(int angle)
{
    if (angle == 90) {
        return WL_OUTPUT_TRANSFORM_90;
    } else if (angle == 180) {
        return WL_OUTPUT_TRANSFORM_180;
    } else if (angle == -90) {
        return WL_OUTPUT_TRANSFORM_270;
    } else if (angle == -180) {
        return WL_OUTPUT_TRANSFORM_180;
    } else {
        return WL_OUTPUT_TRANSFORM_NORMAL;
    }
}

static void output_entry_start_animate(struct output_entry *output)
{
    output->start_time = current_time_msec();

    if (output->type == OUTPUT_TRANSFORM_EFFECT_SCALE) {
        // static animation template. Dont modify prototype duration, control play speed instead
        double speed = 1.0 / MAX(0.1, effect_manager_get_scale_time());
        animtr_start(&output->animtr_scale, &anim_scale, output, (float)speed);
        return;
    }

    if (!output->anim_rotate.channels) {
        return;
    }

    struct anim_channel *anim_ch_angle = &output->anim_rotate.channels[ROTATE_ANIM_CH_ANGLE];
    struct anim_channel *anim_ch_alpha = &output->anim_rotate.channels[ROTATE_ANIM_CH_ALPHA];
    struct anim_channel *anim_ch_width = &output->anim_rotate.channels[ROTATE_ANIM_CH_WIDTH];
    struct anim_channel *anim_ch_height = &output->anim_rotate.channels[ROTATE_ANIM_CH_HEIGHT];

    float start_angle = (float)output_transform_to_angle(output->last_transform);
    float end_angle = (float)output_transform_to_angle(output->output->output->transform);
    end_angle = start_angle - end_angle;
    // reduce rotate angle
    if (end_angle > 180.0f) {
        end_angle -= 360.0f;
    } else if (end_angle < -180.0f) {
        end_angle += 360.0f;
    }
    // keep angle not equal zero
    if (fabsf(end_angle) < 0.01f) {
        end_angle = 360.0f;
    }
    anim_ch_angle->start = 0.0f;
    anim_ch_angle->end = end_angle;
    output->angle = 0.0f;

    anim_ch_alpha->start = 0.0f;
    anim_ch_alpha->end = 1.0f;
    output->alpha = 0.0f;

    int diff_angle = (int)end_angle;
    // vertical rotation need resize animation
    if (diff_angle == 90 || diff_angle == -90) {
        output->anim_rotate.channels_num = 4;
        anim_ch_width->start = output->output->output->width;
        anim_ch_height->start = output->output->output->height;
        anim_ch_width->end = anim_ch_height->start;
        anim_ch_height->end = anim_ch_width->start;
        output->width = anim_ch_width->start;
        output->height = anim_ch_height->start;
    } else {
        output->anim_rotate.channels_num = 2;
    }

    animtr_start(&output->animtr_rotate, &output->anim_rotate, output, 1.0f);

    output->new_thumbnail_transform = diff_angle_to_new_thumbnail_transform(diff_angle);
}

static void entity_destroy(struct effect_entity *entity)
{
    struct output_entry *output = entity->user_data;
    if (!output) {
        return;
    }

    if (output->anim_rotate.channels) {
        free(output->anim_rotate.channels);
    }

    if (output->old_security_texture) {
        opengl_texture_destroy(output->old_security_texture);
    }
    if (output->new_security_texture) {
        opengl_texture_destroy(output->new_security_texture);
    }
    if (output->old_thumbnail_texture) {
        wlr_texture_destroy(output->old_thumbnail_texture);
    }
    if (output->new_thumbnail_texture) {
        wlr_texture_destroy(output->new_thumbnail_texture);
    }
    if (output->old_thumbnail) {
        thumbnail_destroy(output->old_thumbnail);
    }
    if (output->new_thumbnail) {
        thumbnail_destroy(output->new_thumbnail);
    }

    wl_list_remove(&output->destroy.link);
    free(output);
}

static void handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct output_entry *output = wl_container_of(listener, output, destroy);
    if (output->effect_entity) {
        effect_entity_destroy(output->effect_entity);
    }
}

static struct output_entry *output_entry_create(struct output_transform_effect *effect,
                                                struct scene_output *scene_output,
                                                enum output_transform_effect_type type,
                                                enum wl_output_transform last_transform)
{
    struct output_entry *output = calloc(1, sizeof(*output));
    if (!output) {
        return NULL;
    }

    output->effect = effect;
    output->last_transform = last_transform;
    output->output = scene_output;

    output->type = type;
    if (type == OUTPUT_TRANSFORM_EFFECT_ROTATE) {
        output->anim_rotate.channels = calloc(1, 4 * sizeof(struct anim_channel));
        if (output->anim_rotate.channels) {
            uint32_t duration = effect_manager_scale_time(output->effect->anim_rotate_duration);

            anim_channel_set_easing_fn(&output->anim_rotate.channels[ROTATE_ANIM_CH_ANGLE],
                                       anim_easing_quad_in, duration, 0.f, 1.f,
                                       anim_ch_angle_callback);

            anim_channel_set_easing_fn(&output->anim_rotate.channels[ROTATE_ANIM_CH_ALPHA],
                                       anim_rotate_alpha_easing, duration, 0.f, 1.f,
                                       anim_alpha_callback);

            anim_channel_set_easing_fn(&output->anim_rotate.channels[ROTATE_ANIM_CH_WIDTH],
                                       anim_easing_quad_in, duration, 0.f, 1.f,
                                       anim_ch_width_callback);

            anim_channel_set_easing_fn(&output->anim_rotate.channels[ROTATE_ANIM_CH_HEIGHT],
                                       anim_easing_quad_in, duration, 0.f, 1.f,
                                       anim_ch_height_callback);
        }
    }

    output->destroy.notify = handle_output_destroy;
    wl_signal_add(&scene_output->events.destroy, &output->destroy);

    return output;
}

static void handle_old_thumbnail_update(struct thumbnail *thumbnail, struct wlr_buffer *buffer,
                                        bool buffer_changed, void *data)
{
    if (!buffer_changed) {
        return;
    }

    struct output_entry *output = data;
    if (output->old_thumbnail_texture) {
        wlr_texture_destroy(output->old_thumbnail_texture);
    }
    struct wlr_renderer *renderer = output->effect->manager->server->renderer;
    output->old_thumbnail_texture = wlr_texture_from_buffer(renderer, buffer);
    output->old_thumbnail_buffer = buffer;
}

static void handle_old_thumbnail_cancel(struct thumbnail *thumbnail, void *data)
{
    struct output_entry *output = data;
    output->old_thumbnail_buffer = NULL;
    output->old_thumbnail = NULL;
}

static const struct thumbnail_interface old_thumbnail_impl = {
    .update = handle_old_thumbnail_update,
    .cancel = handle_old_thumbnail_cancel,
};

static void handle_new_thumbnail_update(struct thumbnail *thumbnail, struct wlr_buffer *buffer,
                                        bool buffer_changed, void *data)
{
    if (!buffer_changed) {
        return;
    }

    struct output_entry *output = data;
    if (output->new_thumbnail_texture) {
        wlr_texture_destroy(output->new_thumbnail_texture);
    }
    struct wlr_renderer *renderer = output->effect->manager->server->renderer;
    output->new_thumbnail_texture = wlr_texture_from_buffer(renderer, buffer);
    output->new_thumbnail_buffer = buffer;
}

static void handle_new_thumbnail_cancel(struct thumbnail *thumbnail, void *data)
{
    struct output_entry *output = data;
    output->new_thumbnail_buffer = NULL;
    output->new_thumbnail = NULL;
}

static const struct thumbnail_interface new_thumbnail_impl = {
    .update = handle_new_thumbnail_update,
    .cancel = handle_new_thumbnail_cancel,
};

// direct call from output.c not from output transform event
bool output_add_transform_effect(struct kywc_output *kywc_output,
                                 enum output_transform_effect_type type,
                                 struct kywc_output_state *old_state,
                                 struct kywc_output_state *new_state)
{
    if (!transform_effect || !old_state->enabled || !new_state->enabled) {
        return false;
    }

    struct scene_output *scene_output = output_from_kywc_output(kywc_output)->scene_output;
    if (!scene_output) {
        return false;
    }

    // prevent duplicate add
    struct scene *scene = transform_effect->manager->server->scene;
    struct effect_entity *entity = scene_find_effect_entity(scene, transform_effect->effect);
    if (entity) {
        struct output_entry *output = entity->user_data;
        if (output->output == scene_output) {
            return false;
        }
    }

    entity = scene_add_effect(scene, transform_effect->effect);
    if (!entity) {
        return false;
    }

    struct output_entry *output =
        output_entry_create(transform_effect, scene_output, type, old_state->transform);
    if (!output) {
        effect_entity_destroy(entity);
        return false;
    }

    output->effect_entity = entity;
    entity->user_data = output;

    output->old_thumbnail =
        thumbnail_create_from_output(scene_output, old_state, 1.0f, &old_thumbnail_impl, output);
    if (!output->old_thumbnail) {
        effect_entity_destroy(entity);
        return false;
    }

    output->new_thumbnail =
        thumbnail_create_from_output(scene_output, new_state, 1.0f, &new_thumbnail_impl, output);
    if (!output->new_thumbnail) {
        effect_entity_destroy(entity);
        return false;
    }

    // screenshot before transform, call once update and remove
    thumbnail_update(output->old_thumbnail);
    thumbnail_destroy(output->old_thumbnail);
    output->old_thumbnail = NULL;

    // create animate
    output_entry_start_animate(output);

    // trigger render event
    scene_output_damage_whole(scene_output);

    return true;
}

static bool frame_render_pre(struct effect_entity *entity, struct scene_render_target *target)
{
    struct output_entry *output = entity->user_data;
    if (output->output != target->output) {
        return true;
    }

    uint32_t elapsed_time = current_time_msec() - output->start_time;
    if (output->type == OUTPUT_TRANSFORM_EFFECT_SCALE) {
        animtr_update(&output->animtr_scale, elapsed_time);
        if (animtr_is_end(&output->animtr_scale)) {
            effect_entity_destroy(output->effect_entity);
        }
    } else if (output->type == OUTPUT_TRANSFORM_EFFECT_ROTATE) {
        animtr_update(&output->animtr_rotate, elapsed_time);
        if (animtr_is_end(&output->animtr_rotate)) {
            effect_entity_destroy(output->effect_entity);
        }
    }

    return true;
}

static bool frame_render_post(struct effect_entity *entity, struct scene_render_target *target)
{
    struct output_entry *output = entity->user_data;
    if (output->output != target->output) {
        return true;
    }

    // add damage to trigger render event
    scene_output_damage_whole(output->output);
    return true;
}

static void output_security_render(const struct scene_render_target *target,
                                   const pixman_region32_t *security_region,
                                   struct wlr_texture *in_tex, struct opengl_texture *out_texture)
{
    struct opengl_render_pass *target_pass =
        opengl_render_pass_from_wlr_render_pass(target->render_pass);
    opengl_render_pass_save(target_pass);
    /* render in_tex to out_texture with security */
    struct opengl_render_pass *tmp_pass = opengl_begin_texture_pass(out_texture, NULL);
    if (!tmp_pass) {
        opengl_render_pass_restore(target_pass);
        return;
    }

    struct kywc_box dst_box = { 0, 0, out_texture->wlr_texture.width,
                                out_texture->wlr_texture.height };
    render_pass_add_rect(
        &tmp_pass->base,
        &(struct render_rect_options){
            .base = {
                .box = (struct wlr_box){ dst_box.x, dst_box.y, dst_box.width, dst_box.height },
                .color = { .r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0 },
            } });

    struct scene_render_target tmp_target = {
        .logical = dst_box,
        .scale = 1.0f,
        .buffer = NULL,
        .transform = WL_OUTPUT_TRANSFORM_NORMAL,
        .trans_width = out_texture->wlr_texture.width,
        .trans_height = out_texture->wlr_texture.height,
        .output = target->output,
        .render_pass = &tmp_pass->base,
        .options = SCENE_RENDER_ENABLE_PRESENTATION | SCENE_RENDER_ENABLE_SECURITY,
    };
    scene_render_target_init_regions(&tmp_target);
    pixman_region32_init_rect(&tmp_target.damage, tmp_target.logical.x, tmp_target.logical.y,
                              tmp_target.logical.width, tmp_target.logical.height);
    security_target_reset(&tmp_target);

    struct security_render_options options = {
        .dst_box = dst_box,
        .secure_region = security_region,
        .texture = in_tex,
        .src_transform = WL_OUTPUT_TRANSFORM_NORMAL,
    };
    if (options.texture) {
        security_render(&tmp_target, &options);
    }

    render_pass_submit(&tmp_pass->base, 0);
    scene_render_target_finish_regions(&tmp_target);
    opengl_render_pass_restore(target_pass);
}

static void output_security_render_check(struct scene_render_target *target,
                                         struct wlr_buffer *security_buffer,
                                         struct wlr_texture *security_texture,
                                         struct opengl_texture **out_texture)
{
    if (!security_buffer || !security_check_buffer(security_buffer)) {
        return;
    }

    pixman_region32_t secure;
    pixman_region32_init(&secure);
    if (target->options & SCENE_RENDER_ENABLE_SECURITY) {
        struct opengl_render_pass *target_pass =
            opengl_render_pass_from_wlr_render_pass(target->render_pass);
        if (security_get_region(security_buffer, target->client, &secure)) {
            if (!*out_texture) {
                *out_texture = opengl_texture_create_with_fbo(
                    target_pass->renderer, security_texture->width, security_texture->height,
                    GL_BGRA_EXT, GL_LINEAR);
            }

            if (*out_texture && ((*out_texture)->wlr_texture.height != security_texture->height ||
                                 (*out_texture)->wlr_texture.width != security_texture->width)) {
                opengl_texture_destroy(*out_texture);
                *out_texture = opengl_texture_create_with_fbo(
                    target_pass->renderer, security_texture->width, security_texture->height,
                    GL_BGRA_EXT, GL_LINEAR);
            }

            if (!*out_texture) {
                pixman_region32_fini(&secure);
                return;
            }

            output_security_render(target, &secure, security_texture, *out_texture);
        }
    } else {
        pixman_region32_init_rect(&secure, 0, 0, security_buffer->width, security_buffer->height);
        security_target_add_buffer_with_region(target, security_buffer, &secure);
    }
    pixman_region32_fini(&secure);
}

static bool frame_render(struct effect_entity *entity, struct scene_render_target *target)
{
    struct output_entry *output = entity->user_data;
    if (output->output != target->output) {
        return false;
    }

    if (!output->old_thumbnail_texture || !output->new_thumbnail_texture) {
        return false;
    }

    // old_texture update once
    if (!output->old_security_texture) {
        output_security_render_check(target, output->old_thumbnail_buffer,
                                     output->old_thumbnail_texture, &output->old_security_texture);
    }
    struct wlr_texture *old_texture =
        output->old_security_texture && (target->options & SCENE_RENDER_ENABLE_SECURITY)
            ? &output->old_security_texture->wlr_texture
            : output->old_thumbnail_texture;

    // new_texture need update every frame
    output_security_render_check(target, output->new_thumbnail_buffer,
                                 output->new_thumbnail_texture, &output->new_security_texture);
    struct wlr_texture *new_texture =
        output->new_security_texture && (target->options & SCENE_RENDER_ENABLE_SECURITY)
            ? &output->new_security_texture->wlr_texture
            : output->new_thumbnail_texture;

    struct wlr_box dst_box = {
        .x = 0,
        .y = 0,
        .width = output->output->output->width,
        .height = output->output->output->height,
    };

    // background color
    wlr_render_pass_add_rect(target->render_pass,
                             &(struct wlr_render_rect_options){
                                 .box = dst_box,
                                 .color = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
                             });

    if (output->type == OUTPUT_TRANSFORM_EFFECT_SCALE) {
        // pivot align center
        dst_box.x = (dst_box.width - output->width) / 2;
        dst_box.y = (dst_box.height - output->height) / 2;
        dst_box.width = output->width;
        dst_box.height = output->height;

        opengl_render_pass_add_texture(
            target->render_pass,
            &(struct render_texture_options){
                .base = { .texture = old_texture, .alpha = &(float){ 1.0f }, .dst_box = dst_box },
            });

        opengl_render_pass_add_texture(
            target->render_pass,
            &(struct render_texture_options){
                .base = { .texture = new_texture, .alpha = &output->alpha, .dst_box = dst_box },
            });

    } else if (output->type == OUTPUT_TRANSFORM_EFFECT_ROTATE) {
        // size transformed
        if (output->anim_rotate.channels_num > 2) {
            dst_box.x = (dst_box.width - output->width) / 2;
            dst_box.y = (dst_box.height - output->height) / 2;
            dst_box.width = output->width;
            dst_box.height = output->height;
        }

        opengl_render_pass_add_texture(
            target->render_pass,
            &(struct render_texture_options){
                .base = { .texture = old_texture, .alpha = &(float){ 1.0f }, .dst_box = dst_box },
                .rotation_angle = output->angle,
            });

        opengl_render_pass_add_texture(target->render_pass,
                                       &(struct render_texture_options){
                                           .base = { .texture = new_texture,
                                                     .transform = output->new_thumbnail_transform,
                                                     .alpha = &output->alpha,
                                                     .dst_box = dst_box },
                                           .rotation_angle = output->angle,
                                       });
    }

    return true;
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&transform_effect->destroy.link);
    free(transform_effect);
    transform_effect = NULL;
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (transform_effect == NULL) {
        return false;
    }

    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    if (strcmp(option->key, "scale_duration") == 0) {
        int scale_duration = option->value.num;
        transform_effect->anim_scale_duration = scale_duration;
        anim_scale.channels[0].duration = (float)transform_effect->anim_scale_duration;
        anim_scale.channels[1].duration = (float)transform_effect->anim_scale_duration;
        return true;
    }

    if (strcmp(option->key, "rotate_duration") == 0) {
        int rotate_duration = option->value.num;
        transform_effect->anim_rotate_duration = rotate_duration;
        return true;
    }

    return false;
}

static bool handle_allow_direct_scanout(struct effect *effect, struct scene_render_target *target)
{
    struct scene *scene = transform_effect->manager->server->scene;
    return !scene_find_effect_entity(scene, transform_effect->effect);
}

static const struct effect_interface effect_impl = {
    .entity_destroy = entity_destroy,
    .frame_render_pre = frame_render_pre,
    .frame_render = frame_render,
    .frame_render_post = frame_render_post,
    .configure = handle_effect_configure,
    .allow_direct_scanout = handle_allow_direct_scanout,
};

bool output_transform_effect_create(struct effect_manager *manager)
{
    if (!wlr_renderer_is_opengl(manager->server->renderer)) {
        return false;
    }

    transform_effect = calloc(1, sizeof(*transform_effect));
    if (!transform_effect) {
        return false;
    }

    transform_effect->effect = effect_create("output_transform", 105, true, &effect_impl, NULL);
    if (!transform_effect->effect) {
        free(transform_effect);
        transform_effect = NULL;
        return false;
    }

    transform_effect->effect->category = EFFECT_CATEGORY_SCENE;

    transform_effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&transform_effect->effect->events.destroy, &transform_effect->destroy);
    transform_effect->manager = manager;

    transform_effect->anim_scale_duration =
        effect_get_option_int(transform_effect->effect, "scale_duration", 1400);
    anim_scale.channels[0].duration = (float)transform_effect->anim_scale_duration;
    anim_scale.channels[1].duration = (float)transform_effect->anim_scale_duration;

    transform_effect->anim_rotate_duration =
        effect_get_option_int(transform_effect->effect, "rotate_duration", 500);

    return true;
}
