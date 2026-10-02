// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_output.h>

#include "effect/animator.h"
#include "effect/translation.h"
#include "effect_p.h"
#include "output.h"
#include "render/pass.h"
#include "render/renderer.h"
#include "scene/render.h"
#include "scene/thumbnail.h"
#include "security/render.h"
#include "util/time.h"
#include "view/view.h"

#define TRANSLATION_ENTITY_COUNT 2

struct translation_entity {
    struct workspace *workspace;
    struct scene_output *scene_output;
    struct animator *automatic_animator;
    struct wlr_buffer *thumbnail_buffer;
    struct wlr_texture *thumbnail_texture;

    int start_offset;
    float current_offset;
    bool is_manual_control;
    struct animation_data current;
    struct kywc_box end_geometry;
    struct kywc_box start_geometry;
};

enum translation_order_type {
    TRANSLATION_AUTOMATIC,
    TRANSLATION_MANUAL,
};

struct translation_order {
    struct wl_list link;

    enum translation_order_type type;
    enum direction direction;
    float offset;

    struct scene_output *output;
    struct workspace *workspace;
    struct wlr_texture *thumbnail_texture;
    struct wlr_buffer *thumbnail_buffer;
};

struct workspace_output {
    struct wl_list link;
    struct scene_output *scene_output;
    struct translation_entity *entities[TRANSLATION_ENTITY_COUNT];

    int order_count;
    int width, height;
    bool is_translating;
    uint32_t start_time;
    uint32_t order_duration;
    struct wl_list orders;

    struct wl_listener output_destroy;
};

#define TRANSLATION_DELTA_MS (100)
#define TRANSLATION_VELOCITY_THRESHOLD (0.00001f)
#define OFFSETS_SIZE (64)

struct translation_offset {
    float offset;
    uint32_t timestamp;
};

struct workspace_translation {
    struct translation_offset offsets[OFFSETS_SIZE];
    /* index of head and tail entry */
    size_t head, tail, count;

    uint32_t duration;
    bool is_manual_control;
    struct workspace *center;

    /* only using for manual control */
    struct workspace *next_workspace;
    struct workspace *displaying_workspace;

    struct wl_list workspace_outputs;
    struct wl_listener new_enabled_output;
};

struct translation_effect {
    struct effect *effect;

    uint32_t duration;
    struct scene *scene;
    struct wlr_renderer *renderer;

    struct wl_listener destroy;
};

static struct translation_effect *translation_effect = NULL;

static void workspace_output_apply_translation_order(struct workspace_output *ws_output);

static void workspace_output_move_translation_entities(struct workspace_output *ws_output,
                                                       uint32_t current_time);

static struct workspace_output *find_workspace_output(struct workspace_translation *data,
                                                      struct scene_output *scene_output);

static void workspace_translation_do_destroy(struct workspace_translation *data);

static enum direction workspace_translation_reverse_direction(enum direction direction)
{
    return direction == DIRECTION_LEFT ? DIRECTION_RIGHT : DIRECTION_LEFT;
}

static enum direction workspace_translation_direction_from_offset(float offset,
                                                                  enum direction default_direction)
{
    if (offset < 0.0f) {
        return DIRECTION_LEFT;
    } else if (offset > 0.0f) {
        return DIRECTION_RIGHT;
    }

    return default_direction;
}

static void translation_effect_entity_destroy(struct effect_entity *entity)
{
    struct workspace_translation *data = entity->user_data;

    workspace_translation_do_destroy(data);
    entity->user_data = NULL;
}

static bool translation_frame_render_pre(struct effect_entity *entity,
                                         struct scene_render_target *target)
{
    struct scene_output *scene_output = target->output;
    struct workspace_translation *data = entity->user_data;
    struct workspace_output *ws_output = find_workspace_output(data, scene_output);
    if (!ws_output) {
        return true;
    }

    if (data->is_manual_control) {
        workspace_output_move_translation_entities(ws_output, 0);
        /* surface maybe push damage */
        scene_output_damage_whole(scene_output);
        return true;
    }

    uint32_t current_time = current_time_msec();
    if (ws_output->start_time + ws_output->order_duration < current_time) {
        if (ws_output->order_count == 0) {
            effect_entity_destroy(entity);
            return true;
        }

        workspace_output_apply_translation_order(ws_output);
    }

    workspace_output_move_translation_entities(ws_output, current_time);
    return true;
}

static void translation_entity_render(struct translation_entity *translation_entity,
                                      struct scene_render_target *target)
{
    if (!translation_entity->thumbnail_texture) {
        return;
    }

    struct kywc_box logic_box = translation_entity->current.geometry;
    logic_box.x += target->logical.x;
    logic_box.y += target->logical.y;

    struct kywc_box dst_box = logic_box;
    scene_render_target_box(target, &dst_box);
    /* capture target buffer is transformed, the target don't have transform*/
    enum wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL;
    enum wl_output_transform src_transform = translation_entity->scene_output->output->transform;
    if (target->transform != src_transform) {
        assert(target->transform == WL_OUTPUT_TRANSFORM_NORMAL);
        transform = wlr_output_transform_invert(src_transform);
        transform = wlr_output_transform_compose(transform, target->transform);
    }

    struct wlr_buffer *security_buffer = translation_entity->thumbnail_buffer;
    if (security_check_buffer(security_buffer)) {
        if (target->options & SCENE_RENDER_ENABLE_SECURITY) {
            pixman_region32_t secure;
            pixman_region32_init(&secure);
            if (security_get_region(security_buffer, target->client, &secure)) {
                struct security_render_options options = {
                    .dst_box = logic_box,
                    .secure_region = &secure,
                    .texture = translation_entity->thumbnail_texture,
                    .src_transform = src_transform,
                };
                if (options.texture) {
                    security_render(target, &options);
                }
                pixman_region32_fini(&secure);
                return;
            }
            pixman_region32_fini(&secure);
        } else {
            struct kywc_fbox src_box = { 0, 0, security_buffer->width, security_buffer->height };
            security_target_add_buffer(target, security_buffer, &src_box, src_transform, &dst_box);
        }
    }

    pixman_region32_t render_region;
    pixman_region32_init(&render_region);
    pixman_region32_copy(&render_region, &target->damage);

    pixman_region32_intersect_rect(&render_region, &render_region, logic_box.x, logic_box.y,
                                   logic_box.width, logic_box.height);
    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return;
    }

    scene_render_target_region(target, &render_region);
    struct render_texture_options options = {
        .base = {
            .texture = translation_entity->thumbnail_texture,
            .alpha = &translation_entity->current.alpha,
            .dst_box = (struct wlr_box){dst_box.x, dst_box.y, dst_box.width, dst_box.height},
            .clip = &render_region,
            .transform = transform,
        },
        .rotation_angle = translation_entity->current.angle,
    };
    render_pass_add_texture(target->render_pass, &options);

    pixman_region32_fini(&render_region);
}

static bool translation_frame_render(struct effect_entity *entity,
                                     struct scene_render_target *target)
{
    /* TODO: translation entity maybe be displayed cross output when zooming */
    struct workspace_output *ws_output = find_workspace_output(entity->user_data, target->output);
    if (!ws_output) {
        return false;
    }

    /* clear the target buffer */
    wlr_render_pass_add_rect(target->render_pass, &(struct wlr_render_rect_options){
                                                      .color = { 0, 0, 0, 0 },
                                                      .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                                  });

    for (int i = 0; i < TRANSLATION_ENTITY_COUNT; ++i) {
        if (ws_output->entities[i]) {
            translation_entity_render(ws_output->entities[i], target);
        }
    }
    return true;
}

static bool translation_frame_render_post(struct effect_entity *entity,
                                          struct scene_render_target *target)
{
    struct workspace_translation *data = entity->user_data;
    if (!data->is_manual_control) {
        scene_output_damage_whole(target->output);
    }

    return true;
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&translation_effect->destroy.link);
    free(translation_effect);
    translation_effect = NULL;
}

static bool handle_translation_effect_configure(struct effect *effect,
                                                const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    return false;
}

static bool handle_allow_direct_scanout(struct effect *effect, struct scene_render_target *target)
{
    return !scene_find_effect_entity(translation_effect->scene, effect);
}

static const struct effect_interface effect_impl = {
    .entity_destroy = translation_effect_entity_destroy,
    .frame_render_pre = translation_frame_render_pre,
    .frame_render = translation_frame_render,
    .frame_render_post = translation_frame_render_post,
    .configure = handle_translation_effect_configure,
    .allow_direct_scanout = handle_allow_direct_scanout,
};

bool translation_effect_create(struct effect_manager *manager)
{
    translation_effect = calloc(1, sizeof(*translation_effect));
    if (!translation_effect) {
        return false;
    }

    translation_effect->duration = 400;
    translation_effect->scene = manager->server->scene;
    translation_effect->renderer = manager->server->renderer;

    bool enable = !renderer_is_software(manager->server->renderer);
    translation_effect->effect = effect_create("translation", 105, enable, &effect_impl, NULL);
    if (!translation_effect->effect) {
        free(translation_effect);
        translation_effect = NULL;
        return false;
    }

    translation_effect->effect->category = EFFECT_CATEGORY_SCENE;

    translation_effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&translation_effect->effect->events.destroy, &translation_effect->destroy);
    return true;
}

static void handle_thumbnail_update(struct thumbnail *thumbnail, struct wlr_buffer *buffer,
                                    bool buffer_changed, void *data)
{
    if (!buffer_changed) {
        return;
    }

    struct translation_order *order = data;
    if (order->thumbnail_texture) {
        wlr_texture_destroy(order->thumbnail_texture);
    }
    order->thumbnail_buffer = buffer;
    order->thumbnail_texture = wlr_texture_from_buffer(translation_effect->renderer, buffer);
}

static const struct thumbnail_interface translation_impl = {
    .update = handle_thumbnail_update,
};

static void translation_order_destroy(struct translation_order *translation_order)
{
    if (translation_order->thumbnail_texture) {
        wlr_texture_destroy(translation_order->thumbnail_texture);
    }

    wl_list_remove(&translation_order->link);
    free(translation_order);
}

static struct translation_order *translation_order_create(struct workspace_output *ws_output,
                                                          enum translation_order_type type,
                                                          struct workspace *workspace,
                                                          enum direction direction, float offset)
{
    struct translation_order *order = calloc(1, sizeof(*order));
    if (!order) {
        return NULL;
    }

    order->type = type;
    order->offset = offset;
    order->workspace = workspace;
    order->direction = direction;
    order->output = ws_output->scene_output;
    wl_list_init(&order->link);

    if (!workspace) {
        return order;
    }

    struct thumbnail *output_thumbnail =
        thumbnail_create_from_output(ws_output->scene_output, NULL, 1.0, &translation_impl, order);
    if (!output_thumbnail) {
        free(order);
        return NULL;
    }

    thumbnail_update(output_thumbnail);
    thumbnail_destroy(output_thumbnail);

    return order;
}

static void translation_entity_destroy(struct translation_entity *translation_entity)
{
    if (translation_entity->thumbnail_texture) {
        wlr_texture_destroy(translation_entity->thumbnail_texture);
    }
    if (translation_entity->automatic_animator) {
        animator_destroy(translation_entity->automatic_animator);
    }

    free(translation_entity);
}

static struct translation_entity *translation_entity_create(struct workspace_output *ws_output,
                                                            struct translation_order *order)
{
    struct translation_entity *translation_entity = calloc(1, sizeof(*translation_entity));
    if (!translation_entity) {
        return NULL;
    }

    translation_entity->start_offset =
        order->offset > 0 ? (int)(order->offset + 0.5) : (int)(order->offset - 0.5);
    translation_entity->current_offset = order->offset;
    translation_entity->workspace = order->workspace;
    translation_entity->scene_output = order->output;
    translation_entity->is_manual_control = order->type == TRANSLATION_MANUAL;
    translation_entity->end_geometry.x = translation_entity->end_geometry.y = 0;
    translation_entity->end_geometry.width = ws_output->width;
    translation_entity->end_geometry.height = ws_output->height;
    translation_entity->start_geometry = translation_entity->end_geometry;

    translation_entity->current.alpha = 1;
    translation_entity->current.angle = 0;
    translation_entity->current.geometry = translation_entity->start_geometry;
    translation_entity->automatic_animator = NULL;
    translation_entity->thumbnail_buffer = order->thumbnail_buffer;
    translation_entity->thumbnail_texture = order->thumbnail_texture;

    return translation_entity;
}

static void workspace_output_move_translation_entities(struct workspace_output *ws_output,
                                                       uint32_t current_time)
{
    for (int i = 0; i < TRANSLATION_ENTITY_COUNT; ++i) {
        struct translation_entity *translation_entity = ws_output->entities[i];
        if (!translation_entity) {
            continue;
        }

        if (translation_entity->is_manual_control) {
            struct kywc_box *current = &translation_entity->current.geometry;
            struct kywc_box *start = &translation_entity->start_geometry;
            struct kywc_box *end = &translation_entity->end_geometry;
            current->x = start->x + (end->x - start->x) * fabs(translation_entity->current_offset -
                                                               translation_entity->start_offset);
            continue;
        }

        if (!translation_entity->automatic_animator) {
            continue;
        }

        const struct animation_data *current_data =
            animator_value(translation_entity->automatic_animator, current_time);
        translation_entity->current = *current_data;
    }
}

static void workspace_output_reset_entities_animator(struct workspace_output *ws_output)
{
    uint32_t current_time = current_time_msec();
    uint32_t end_time = current_time + ws_output->order_duration;
    ws_output->start_time = current_time;
    struct translation_entity *exist_entity;
    for (int i = 0; i < TRANSLATION_ENTITY_COUNT; ++i) {
        exist_entity = ws_output->entities[i];
        if (!exist_entity) {
            continue;
        }

        if (exist_entity->automatic_animator) {
            animator_set_time_ex(exist_entity->automatic_animator, current_time, end_time);
            animator_set_position(exist_entity->automatic_animator, exist_entity->end_geometry.x,
                                  exist_entity->end_geometry.y);
            continue;
        }

        struct animation_group group = {
            .geometry = animation_manager_get(ANIMATION_TYPE_EASE_OUT),
        };
        exist_entity->automatic_animator =
            animator_create(&exist_entity->current, &group, current_time, end_time);
        if (!exist_entity->automatic_animator) {
            continue;
        }

        animator_set_position(exist_entity->automatic_animator, exist_entity->end_geometry.x,
                              exist_entity->end_geometry.y);
    }
}

static struct translation_entity *
workspace_output_create_translation_entity(struct workspace_output *ws_output,
                                           struct translation_order *order)
{
    struct translation_entity *translation_entity = translation_entity_create(ws_output, order);
    if (!translation_entity) {
        return NULL;
    }

    order->thumbnail_texture = NULL;

    struct translation_entity *exist_entity = ws_output->entities[0];
    if (!exist_entity) {
        ws_output->entities[0] = translation_entity;
        ws_output->is_translating = true;
        return translation_entity;
    }

    struct workspace *workspace = order->workspace;
    enum direction direct = order->direction;
    if (direct == DIRECTION_LEFT) {
        translation_entity->start_geometry.x =
            exist_entity->current.geometry.x - exist_entity->current.geometry.width;
        if (!workspace) {
            translation_entity->end_geometry.x =
                translation_entity->start_geometry.x + ws_output->width * 0.2;
        }
    } else if (direct == DIRECTION_RIGHT) {
        translation_entity->start_geometry.x =
            exist_entity->current.geometry.x + exist_entity->current.geometry.width;
        if (!workspace) {
            translation_entity->end_geometry.x =
                translation_entity->start_geometry.x - ws_output->width * 0.2;
        }
    }
    exist_entity->end_geometry.x = exist_entity->current.geometry.x +
                                   translation_entity->end_geometry.x -
                                   translation_entity->start_geometry.x;
    translation_entity->current.geometry = translation_entity->start_geometry;

    ws_output->entities[1] = translation_entity;
    if (order->type == TRANSLATION_AUTOMATIC) {
        workspace_output_reset_entities_animator(ws_output);
    }
    return translation_entity;
}

static struct translation_entity *
workspace_output_add_translation_entity(struct workspace_output *ws_output,
                                        struct workspace *workspace, enum direction direct)
{
    struct translation_order *order =
        translation_order_create(ws_output, TRANSLATION_AUTOMATIC, workspace, direct, 0);
    if (!order) {
        return NULL;
    }

    if (ws_output->entities[0] && ws_output->entities[1]) {
        wl_list_insert(&ws_output->orders, &order->link);
        ws_output->order_count++;
        return NULL;
    }

    struct translation_entity *new_entity =
        workspace_output_create_translation_entity(ws_output, order);

    translation_order_destroy(order);

    return new_entity;
}

static struct translation_entity *
workspace_output_add_manaul_translation_entity(struct workspace_output *ws_output,
                                               struct workspace *workspace,
                                               enum direction direction, float offset)
{
    struct translation_entity *already_exsit = NULL;
    if (ws_output->entities[0] && ws_output->entities[0]->workspace == workspace) {
        already_exsit = ws_output->entities[0];
    } else if (ws_output->entities[1] && ws_output->entities[1]->workspace == workspace) {
        already_exsit = ws_output->entities[1];
    }

    if (already_exsit) {
        already_exsit->current_offset = offset;
        return already_exsit;
    }

    /* remove entity who doesn't refresh offset */
    if (ws_output->entities[0] && ws_output->entities[1]) {
        assert(ws_output->entities[0]->current_offset != FLT_MAX ||
               ws_output->entities[1]->current_offset != FLT_MAX);
        for (int i = 0; i < TRANSLATION_ENTITY_COUNT; i++) {
            if (ws_output->entities[i]->current_offset == FLT_MAX) {
                translation_entity_destroy(ws_output->entities[i]);
                ws_output->entities[i] = NULL;
            } else {
                ws_output->entities[i]->start_offset =
                    offset > 0 ? (int)(offset + 0.5) : (int)(offset - 0.5);
                ws_output->entities[i]->end_geometry =
                    (struct kywc_box){ 0, 0, ws_output->width, ws_output->height };
                ws_output->entities[i]->start_geometry = ws_output->entities[i]->end_geometry;
                ws_output->entities[i]->current.geometry = ws_output->entities[i]->end_geometry;
            }
        }

        if (!ws_output->entities[0]) {
            ws_output->entities[0] = ws_output->entities[1];
            ws_output->entities[1] = NULL;
        }
    }

    struct translation_order *order =
        translation_order_create(ws_output, TRANSLATION_MANUAL, workspace, direction, offset);
    if (!order) {
        return NULL;
    }

    struct translation_entity *ret = workspace_output_create_translation_entity(ws_output, order);

    translation_order_destroy(order);
    return ret;
}

static void workspace_output_apply_translation_order(struct workspace_output *ws_output)
{
    assert(ws_output->order_count);
    if (ws_output->entities[1]) {
        if (ws_output->entities[0]) {
            translation_entity_destroy(ws_output->entities[0]);
        }

        ws_output->entities[0] = ws_output->entities[1];
        ws_output->entities[1] = NULL;
    }

    struct translation_order *order, *tmp;
    wl_list_for_each_reverse_safe(order, tmp, &ws_output->orders, link) {
        workspace_output_create_translation_entity(ws_output, order);
        wl_list_remove(&order->link);
        ws_output->order_count--;
        free(order);
        break;
    }
}

static void workspace_output_destroy(struct workspace_output *ws_output)
{
    wl_list_remove(&ws_output->output_destroy.link);

    struct translation_entity *exist_entity;
    for (int i = 0; i < TRANSLATION_ENTITY_COUNT; ++i) {
        exist_entity = ws_output->entities[i];
        if (!exist_entity) {
            continue;
        }

        translation_entity_destroy(exist_entity);
    }

    struct translation_order *pos, *tmp;
    wl_list_for_each_safe(pos, tmp, &ws_output->orders, link) {
        translation_order_destroy(pos);
    }

    wl_list_remove(&ws_output->link);
    free(ws_output);
}

static struct workspace_output *workspace_output_create(struct scene_output *output, int duration)
{
    struct workspace_output *workspace_output = calloc(1, sizeof(*workspace_output));
    if (!workspace_output) {
        return NULL;
    }

    workspace_output->order_count = 0;
    workspace_output->is_translating = false;
    workspace_output->order_duration = duration;
    workspace_output->scene_output = output;
    wlr_output_effective_resolution(output->output, &workspace_output->width,
                                    &workspace_output->height);

    wl_list_init(&workspace_output->orders);
    wl_list_init(&workspace_output->link);
    return workspace_output;
}

static struct workspace_output *find_workspace_output(struct workspace_translation *data,
                                                      struct scene_output *output)
{
    struct workspace_output *ws_output;
    wl_list_for_each(ws_output, &data->workspace_outputs, link) {
        if (ws_output->scene_output == output) {
            return ws_output;
        }
    }
    return NULL;
}

static void workspace_translation_do_destroy(struct workspace_translation *data)
{
    wl_list_remove(&data->new_enabled_output.link);

    struct workspace_output *ws_output, *tmp;
    wl_list_for_each_safe(ws_output, tmp, &data->workspace_outputs, link) {
        workspace_output_destroy(ws_output);
    }
    free(data);
}

static void workspace_output_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct workspace_output *ws_output = wl_container_of(listener, ws_output, output_destroy);
    workspace_output_destroy(ws_output);
}

static void workspace_translation_add_output(struct workspace_translation *data,
                                             struct scene_output *output)
{
    struct workspace_output *ws_output = workspace_output_create(output, data->duration);
    if (!ws_output) {
        return;
    }

    ws_output->output_destroy.notify = workspace_output_handle_output_destroy;
    wl_signal_add(&output->events.destroy, &ws_output->output_destroy);
    wl_list_insert(&data->workspace_outputs, &ws_output->link);
}

static void workspace_translation_handle_new_output(struct wl_listener *listener, void *data)
{
    struct workspace_translation *translation_data =
        wl_container_of(listener, translation_data, new_enabled_output);
    struct scene_output *output = output_from_kywc_output(data)->scene_output;
    struct workspace_output *ws_output = find_workspace_output(translation_data, output);
    if (ws_output) {
        return;
    }

    workspace_translation_add_output(translation_data, output);
}

static void workspace_translation_insert_offset(struct workspace_translation *ws_translation,
                                                float offset)
{
    uint32_t current_time = current_time_msec();

    /* if ring buffer is full, overwrite the oldest (tail) */
    if (ws_translation->count >= OFFSETS_SIZE) {
        ws_translation->tail = (ws_translation->tail + 1) % OFFSETS_SIZE;
        ws_translation->count--;
    }

    size_t head_idx = ws_translation->head;
    ws_translation->offsets[head_idx].offset = offset;
    ws_translation->offsets[head_idx].timestamp = current_time;
    ws_translation->head = (ws_translation->head + 1) % OFFSETS_SIZE;
    ws_translation->count++;
}

static struct workspace_translation *workspace_translation_create(struct scene *scene,
                                                                  struct workspace *center,
                                                                  int duration,
                                                                  bool is_manual_control)
{
    struct workspace_translation *data = calloc(1, sizeof(*data));
    if (!data) {
        return NULL;
    }

    data->center = center;
    data->is_manual_control = is_manual_control;
    data->duration = duration;
    data->head = 0;
    data->tail = 0;
    data->count = 0;
    workspace_translation_insert_offset(data, 0);

    wl_list_init(&data->workspace_outputs);

    data->new_enabled_output.notify = workspace_translation_handle_new_output;
    output_manager_add_new_enabled_listener(&data->new_enabled_output);

    struct scene_output *output;
    wl_list_for_each(output, &scene->outputs, link) {
        workspace_translation_add_output(data, output);
    }

    return data;
}

static struct workspace_translation *workspace_add_translation_effect(struct workspace *center,
                                                                      bool is_manual_control)
{
    if (!translation_effect || !translation_effect->effect->enabled) {
        return NULL;
    }

    struct effect *effect = translation_effect->effect;
    struct scene *scene = translation_effect->scene;
    struct effect_entity *entity = scene_find_effect_entity(scene, effect);
    if (entity) {
        return entity->user_data;
    }

    /**
     * When switching workspaces, the damage region only contains
     * the damage regions generated by the view in the workspace layer.
     */
    scene_damage_whole(scene);

    entity = scene_add_effect(scene, effect);
    if (!entity) {
        return NULL;
    }

    struct workspace_translation *data = workspace_translation_create(
        scene, center, effect_manager_scale_time(translation_effect->duration), is_manual_control);
    if (!data) {
        effect_entity_destroy(entity);
        return NULL;
    }

    entity->user_data = data;
    return data;
}

bool workspace_add_automatic_translation_effect(struct workspace *current, struct workspace *next,
                                                enum direction direction)
{
    if (direction != DIRECTION_LEFT && direction != DIRECTION_RIGHT) {
        return false;
    }

    struct workspace_translation *ws_translation = workspace_add_translation_effect(NULL, false);
    if (!ws_translation) {
        return false;
    }

    if (ws_translation->is_manual_control) {
        return false;
    }

    next = next == current ? NULL : next;

    struct workspace_output *ws_output;
    wl_list_for_each(ws_output, &ws_translation->workspace_outputs, link) {
        if (!ws_output->is_translating) {
            workspace_output_add_translation_entity(ws_output, current, direction);
        }
    }

    if (next) {
        workspace_activate(next);
    }

    wl_list_for_each(ws_output, &ws_translation->workspace_outputs, link) {
        if (ws_output->order_count >= 10) {
            continue;
        }

        if (!wl_list_empty(&ws_output->orders)) {
            struct translation_order *last_order =
                wl_container_of(ws_output->orders.next, last_order, link);
            enum direction round_trip_direction =
                workspace_translation_reverse_direction(direction);
            if (next == NULL && last_order->direction == round_trip_direction &&
                last_order->workspace == current) {
                continue;
            }
        } else if (next == NULL && ws_output->entities[0]->workspace == next &&
                   ws_output->entities[1]->workspace == current) {
            continue;
        }

        workspace_output_add_translation_entity(ws_output, next, direction);
        /* if current workspace is last workspace, need a short round trip */
        if (!next) {
            workspace_output_add_translation_entity(
                ws_output, current, workspace_translation_reverse_direction(direction));
        }
    }

    return true;
}

struct workspace_translation *workspace_create_manual_translation_effect(struct workspace *center)
{
    if (!center) {
        return NULL;
    }

    struct workspace_translation *ws_translation = workspace_add_translation_effect(center, true);
    if (!ws_translation) {
        return NULL;
    }

    if (ws_translation->center != center || !ws_translation->is_manual_control) {
        return NULL;
    }

    return ws_translation;
}

static void get_workspace_index(struct workspace *center_workspace, uint32_t workspace_count,
                                double offset, int64_t *displaying, int64_t *next)
{
    int32_t center = center_workspace->position;
    if (offset > 0) {
        *displaying = center + floor(offset);
        *next = center + ceil(offset);
    } else {
        *displaying = center - floor(-offset);
        *next = center + (int)(offset - 1);
    }

    if (*displaying <= 0 && *next <= 0) {
        *displaying = 0;
        *next = -1;
    } else if (*displaying >= workspace_count && *next >= workspace_count) {
        *displaying = (int64_t)workspace_count - 1;
        *next = workspace_count;
    }
}

static bool update_translation_workspace(struct workspace_translation *ws_translation,
                                         struct workspace *displaying1,
                                         struct workspace *displaying2, bool *order_reversed)
{
    *order_reversed = false;
    if (!ws_translation->displaying_workspace && !ws_translation->next_workspace) {
        ws_translation->displaying_workspace = displaying1;
        ws_translation->next_workspace = displaying2;
        return true;
    }

    bool is_displaying1_exist = ws_translation->displaying_workspace == displaying1 ||
                                ws_translation->next_workspace == displaying1;
    bool is_displaying2_exist = ws_translation->displaying_workspace == displaying2 ||
                                ws_translation->next_workspace == displaying2;
    if (is_displaying1_exist && is_displaying2_exist) {
        return true;
    }

    if (is_displaying1_exist) {
        ws_translation->displaying_workspace = displaying1;
        ws_translation->next_workspace = displaying2;
        return true;
    }

    if (is_displaying2_exist) {
        ws_translation->displaying_workspace = displaying2;
        ws_translation->next_workspace = displaying1;
        *order_reversed = true;
        return true;
    }

    return false;
}

bool workspace_translation_manual(struct workspace_translation *ws_translation,
                                  enum direction direction, float offset)
{
    if (!ws_translation->is_manual_control ||
        (direction != DIRECTION_LEFT && direction != DIRECTION_RIGHT)) {
        return false;
    }

    assert(ws_translation->count > 0);
    size_t head_idx = (ws_translation->head == 0) ? OFFSETS_SIZE - 1 : ws_translation->head - 1;
    float last_offset_val = ws_translation->offsets[head_idx].offset;
    if (offset < 0) {
        offset = fmax(fmax(offset, last_offset_val - 0.01f), -2.0f);
    } else {
        offset = fmin(fmin(offset, last_offset_val + 0.01f), 2.0f);
    }

    workspace_translation_insert_offset(ws_translation, offset);
    if (last_offset_val == offset) {
        return false;
    }

    direction = workspace_translation_direction_from_offset(offset, direction);

    int64_t displaying1_index, displaying2_index;
    get_workspace_index(ws_translation->center, workspace_manager_get_count(), offset,
                        &displaying1_index, &displaying2_index);

    struct workspace *displaying1_workspace =
        displaying1_index < 0 ? NULL : workspace_by_position(displaying1_index);
    struct workspace *displaying2_workspace =
        displaying2_index < 0 ? NULL : workspace_by_position(displaying2_index);

    bool order_reversed = false;
    if (!update_translation_workspace(ws_translation, displaying1_workspace, displaying2_workspace,
                                      &order_reversed)) {
        /* workspace1 and workspace2 didn't display */
        return false;
    }

    struct workspace *next = ws_translation->next_workspace;
    struct workspace *current = ws_translation->displaying_workspace;
    enum direction next_direction =
        order_reversed ? workspace_translation_reverse_direction(direction) : direction;
    bool current_exist = false, next_exist = false;
    /* reset offset */
    struct workspace_output *ws_output;
    wl_list_for_each(ws_output, &ws_translation->workspace_outputs, link) {
        for (int i = 0; i < TRANSLATION_ENTITY_COUNT; i++) {
            if (!ws_output->entities[i]) {
                continue;
            }

            ws_output->entities[i]->current_offset = FLT_MAX;
            if (ws_output->entities[i]->workspace == current) {
                current_exist = true;
            } else if (ws_output->entities[i]->workspace == next) {
                next_exist = true;
            }
        }

        /* don't know which entity show be deleted, if current workspace don't in array */
        if (!current_exist && ws_output->entities[0] && ws_output->entities[1]) {
            return false;
        }
    }

    /**
     * When switching workspaces, the damage region only contains
     * the damage regions generated by the view in the workspace layer.
     */
    scene_damage_whole(translation_effect->scene);

    if (!current_exist && current) {
        workspace_activate(current);
    }

    wl_list_for_each(ws_output, &ws_translation->workspace_outputs, link) {
        workspace_output_add_manaul_translation_entity(ws_output, current, direction, offset);
    }

    if (!next_exist && next) {
        workspace_activate(next);
    }

    wl_list_for_each(ws_output, &ws_translation->workspace_outputs, link) {
        workspace_output_add_manaul_translation_entity(ws_output, next, next_direction, offset);
    }

    if (ws_translation->center) {
        workspace_activate(ws_translation->center);
    }

    return true;
}

struct workspace *workspace_translation_destroy(struct workspace_translation *ws_translation,
                                                float continue_switching_percent)
{
    if (!ws_translation->is_manual_control) {
        return NULL;
    }

    scene_damage_whole(translation_effect->scene);

    assert(ws_translation->count > 0);
    uint32_t time = current_time_msec();

    /* last: most recent entry is at head - 1 */
    size_t last_idx = (ws_translation->head == 0) ? OFFSETS_SIZE - 1 : ws_translation->head - 1;
    float last_offset_val = ws_translation->offsets[last_idx].offset;
    uint32_t last_timestamp = ws_translation->offsets[last_idx].timestamp;

    /* prev: scan from newest to oldest for first entry older than TRANSLATION_DELTA_MS */
    float prev_offset_val = ws_translation->offsets[ws_translation->tail].offset;
    uint32_t prev_timestamp = ws_translation->offsets[ws_translation->tail].timestamp;
    for (size_t i = 0; i < ws_translation->count; i++) {
        size_t idx = (last_idx + OFFSETS_SIZE - i) % OFFSETS_SIZE;
        if (time - ws_translation->offsets[idx].timestamp > TRANSLATION_DELTA_MS) {
            prev_offset_val = ws_translation->offsets[idx].offset;
            prev_timestamp = ws_translation->offsets[idx].timestamp;
            break;
        }
    }

    /* clear all offsets */
    ws_translation->head = 0;
    ws_translation->tail = 0;
    ws_translation->count = 0;

    /* determine if we should switch based on velocity or offset percentage */
    double velocity =
        last_timestamp - prev_timestamp == 0
            ? 0
            : (last_offset_val - prev_offset_val) / (double)(last_timestamp - prev_timestamp);
    bool switch_by_velocity = fabs(velocity) > TRANSLATION_VELOCITY_THRESHOLD;
    bool workspace_continue_switched = false;
    struct workspace_output *ws_output;
    wl_list_for_each(ws_output, &ws_translation->workspace_outputs, link) {
        bool is_can_continue_switching = ws_output->entities[0] && ws_output->entities[1] &&
                                         ws_output->entities[0]->workspace &&
                                         ws_output->entities[1]->workspace;
        for (int i = 0; i < TRANSLATION_ENTITY_COUNT; i++) {
            if (!ws_output->entities[i]) {
                continue;
            }

            ws_output->entities[i]->is_manual_control = false;
            if (!is_can_continue_switching) {
                /* back to start geometry */
                ws_output->entities[i]->end_geometry = ws_output->entities[i]->start_geometry;
                continue;
            }

            if (switch_by_velocity) {
                /* fast swipe: check the directions of velocity and last_offset_val */
                /* back to start geometry */
                if (velocity * last_offset_val < 0.0f) {
                    ws_output->entities[i]->end_geometry = ws_output->entities[i]->start_geometry;
                    continue;
                }
                workspace_continue_switched = true;
            } else {
                /* slow swipe: check offset percentage threshold */
                float current_percent = fabs(ws_output->entities[i]->current_offset -
                                             ws_output->entities[i]->start_offset);
                if (current_percent < continue_switching_percent) {
                    /* back to start geometry */
                    ws_output->entities[i]->end_geometry = ws_output->entities[i]->start_geometry;
                    continue;
                }
                workspace_continue_switched = true;
            }
        }
        workspace_output_reset_entities_animator(ws_output);
    }

    struct workspace *final_show = workspace_continue_switched
                                       ? ws_translation->next_workspace
                                       : ws_translation->displaying_workspace;

    ws_translation->is_manual_control = false;
    ws_translation->displaying_workspace = NULL;
    ws_translation->next_workspace = NULL;

    return final_show;
}
