// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <wlr/types/wlr_matrix.h>
#include <wlr/util/region.h>

#include <kywc/boxes.h>
#include <kywc/log.h>

#include "effect/blur.h"
#include "effect/effect.h"
#include "effect_p.h"
#include "render/opengl.h"
#include "render/profile.h"
#include "scene/scene.h"

struct blur_output_data {
    struct wl_list link;
    pixman_region32_t unaffected_region;
    struct blur_task *blur_task;

    struct scene_output *output;
    struct wl_listener output_destroy;

    struct wlr_buffer *output_buffer;
    struct wl_listener buffer_destroy;
};

static struct blur_data {
    struct blur_effect *effect;
    struct wl_list output_data;
    struct blur_output_data *current_output_data;
    struct opengl_texture *current_output_texture;
} effect_blur_data = { 0 };

struct blur_node {
    struct wl_list link;
    struct scene_node *node;

    struct blur_info blur;
    pixman_region32_t visible_blur;
    pixman_region32_t damaged_blur;
};

struct blur_render_config {
    struct opengl_renderer *renderer;
};

static struct blur_render_config blur_config = {
    .renderer = NULL,
};

struct blur_effect {
    struct effect *effect;
    struct scene *scene;

    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;
};

static void blur_output_data_destroy(struct blur_output_data *data);

static int calculate_blur_radius(int iterations, float offset)
{
    return ceil(pow(2, iterations + 1) * offset);
}

static struct blur_node *create_blur_node(struct scene_node *node, struct blur_info *blur,
                                          pixman_region32_t *visible_blur)
{
    struct blur_node *blur_node = calloc(1, sizeof(*blur_node));
    if (!blur_node) {
        return NULL;
    }

    blur_node->node = node;
    blur_node->blur.offset = blur->offset;
    blur_node->blur.iterations = blur->iterations;
    pixman_region32_init(&blur_node->blur.region);
    pixman_region32_init(&blur_node->damaged_blur);
    pixman_region32_init(&blur_node->visible_blur);
    pixman_region32_copy(&blur_node->blur.region, &blur->region);
    pixman_region32_copy(&blur_node->visible_blur, visible_blur);

    return blur_node;
}

static void destroy_blur_node(struct blur_node *node)
{
    wl_list_remove(&node->link);
    pixman_region32_fini(&node->damaged_blur);
    pixman_region32_fini(&node->blur.region);
    pixman_region32_fini(&node->visible_blur);
    free(node);
}

static void destroy_blur_nodes(struct wl_list *nodes)
{
    struct blur_node *blur_node, *tmp;
    wl_list_for_each_safe(blur_node, tmp, nodes, link) {
        kywc_log(KYWC_DEBUG,
                 "node: %p skiped. when blur calc extend_render_region, the blur region of "
                 "the node may have been overlooked",
                 blur_node->node);
        destroy_blur_node(blur_node);
    }
}

static void blur_data_destroy(struct blur_data *data)
{
    struct blur_output_data *output_data, *tmp;
    wl_list_for_each_safe(output_data, tmp, &data->output_data, link) {
        blur_output_data_destroy(output_data);
    }
}

static void blur_render(struct scene_render_target *target,
                        const struct blur_render_options *options,
                        const pixman_region32_t *damaged_blur_region)
{
    struct blur_data *data = &effect_blur_data;
    if (!data->current_output_data) {
        return;
    }

    struct opengl_render_pass *target_pass =
        opengl_render_pass_from_wlr_render_pass(target->render_pass);

    struct wlr_texture *tmp_texture = NULL;
    struct opengl_texture *in_tex = data->current_output_texture;
    if (!in_tex && target_pass->buffer) {
        tmp_texture = opengl_texture_from_buffer(&blur_config.renderer->wlr_renderer,
                                                 target_pass->buffer->buffer);
        in_tex = tmp_texture ? opengl_texture_from_wlr_texture(tmp_texture) : NULL;
    } else if (!in_tex && target_pass->texture) {
        in_tex = target_pass->texture;
    }
    if (!in_tex) {
        return;
    }

    struct blur_task_render_options task_options = {
        .in_texture = in_tex,
        .out_fbo = opengl_render_pass_get_fbo(target_pass),
        .iterations = options->blur->iterations,
        .offset = options->blur->offset,
        .clip = damaged_blur_region,
        .apply_blur = options,
    };
    blur_task_render(data->current_output_data->blur_task, &task_options);

    if (tmp_texture) {
        wlr_texture_destroy(tmp_texture);
    }
}

void blur_render_with_target(struct scene_render_target *target,
                             const struct blur_render_options *options)
{
    if (!options->blur || options->blur->alpha <= 0.0f || options->blur->offset <= 0 ||
        !blur_config.renderer || !effect_blur_data.effect->effect->enabled ||
        (options->alpha && *options->alpha <= 0.0f)) {
        return;
    }

    /* framebuffer coord */
    if (!pixman_region32_not_empty(&options->blur->region)) {
        return;
    }

    pixman_region32_t damaged_blur_region;
    pixman_region32_init(&damaged_blur_region);
    pixman_region32_intersect(&damaged_blur_region, &options->blur->region, options->clip);
    if (pixman_region32_not_empty(&damaged_blur_region)) {
        blur_render(target, options, &damaged_blur_region);
    }
    pixman_region32_fini(&damaged_blur_region);
}

static void blur_expand_damage(const struct blur_info *blur, pixman_region32_t *damaged_blur,
                               pixman_region32_t *half_expand_damage,
                               struct scene_render_target *target)
{
    int distance = calculate_blur_radius(blur->iterations, blur->offset);
    distance = ceil(distance / target->scale);

    /* blur expand region, region for blur */
    wlr_region_expand(damaged_blur, damaged_blur, distance);
    pixman_region32_intersect(damaged_blur, damaged_blur, &blur->region);
    pixman_region32_union(half_expand_damage, half_expand_damage, damaged_blur);
    /* region for copyback and blur */
    wlr_region_expand(damaged_blur, damaged_blur, distance);
    pixman_region32_intersect(damaged_blur, damaged_blur, &blur->region);
    /* blur expand region add to damage */
    if (target->scale == floor(target->scale)) {
        pixman_region32_union(&target->damage, &target->damage, damaged_blur);
        return;
    }

    /* fractional scaling extends by one more pixel */
    pixman_region32_t add_damage;
    pixman_region32_init(&add_damage);
    wlr_region_expand(&add_damage, damaged_blur, 1);
    pixman_region32_intersect(damaged_blur, &add_damage, &blur->region);
    pixman_region32_union(&target->damage, &target->damage, &add_damage);
    pixman_region32_fini(&add_damage);
}

static void node_for_each_blur_region(struct scene_node *node, struct scene_render_target *target,
                                      int lx, int ly, struct wl_list *nodes,
                                      pixman_region32_t *half_expand_damage)
{
    if (!scene_node_is_visible(node)) {
        return;
    }

    if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each_reverse(child, &tree->children, link) {
            node_for_each_blur_region(child, target, lx + child->x, ly + child->y, nodes,
                                      half_expand_damage);
        }
        return;
    }

    if (!node->has_blur) {
        return;
    }

    struct blur_info blur_info;
    pixman_region32_init(&blur_info.region);
    scene_node_get_blur_info(node, &blur_info);
    if (!pixman_region32_not_empty(&blur_info.region)) {
        pixman_region32_fini(&blur_info.region);
        kywc_log(KYWC_DEBUG, "node has blur, but region is empty.");
        return;
    }

    pixman_region32_translate(&blur_info.region, lx, ly);
    pixman_region32_intersect_rect(&blur_info.region, &blur_info.region, target->logical.x,
                                   target->logical.y, target->logical.width,
                                   target->logical.height);

    pixman_region32_t visible_blur;
    pixman_region32_init(&visible_blur);
    pixman_region32_intersect(&visible_blur, &blur_info.region, &node->visible_region);
    struct blur_node *blur_node = create_blur_node(node, &blur_info, &visible_blur);
    if (!blur_node) {
        pixman_region32_fini(&blur_info.region);
        pixman_region32_fini(&visible_blur);
        return;
    }

    /* blur in damage and visible */
    pixman_region32_t damaged_visible_blur;
    pixman_region32_init(&damaged_visible_blur);
    pixman_region32_intersect(&damaged_visible_blur, &visible_blur, &target->damage);
    pixman_region32_fini(&visible_blur);
    if (!pixman_region32_not_empty(&damaged_visible_blur)) {
        wl_list_insert(nodes, &blur_node->link);
        pixman_region32_fini(&blur_info.region);
        pixman_region32_fini(&damaged_visible_blur);
        return;
    }

    /* node damaged blur region */
    pixman_region32_t new_half_expand_damage;
    pixman_region32_init(&new_half_expand_damage);
    pixman_region32_copy(&new_half_expand_damage, half_expand_damage);
    blur_expand_damage(&blur_info, &damaged_visible_blur, half_expand_damage, target);
    pixman_region32_subtract(&new_half_expand_damage, half_expand_damage, &new_half_expand_damage);

    pixman_region32_copy(&blur_node->damaged_blur, &damaged_visible_blur);
    pixman_region32_fini(&damaged_visible_blur);
    pixman_region32_fini(&blur_info.region);

    /**
     * expand region maybe out the damaged blur region of up node
     * and in the visible blur region of up node. the region like the damage
     * region from blur's backgroud.
     */
    pixman_region32_t node_expand_blur_damage;
    pixman_region32_init(&node_expand_blur_damage);
    struct blur_node *up_blur_node;
    wl_list_for_each(up_blur_node, nodes, link) {
        pixman_region32_intersect(&node_expand_blur_damage, &up_blur_node->visible_blur,
                                  &new_half_expand_damage);
        if (!pixman_region32_not_empty(&node_expand_blur_damage)) {
            continue;
        }

        blur_expand_damage(&up_blur_node->blur, &node_expand_blur_damage, &new_half_expand_damage,
                           target);
        pixman_region32_union(&up_blur_node->damaged_blur, &up_blur_node->damaged_blur,
                              &node_expand_blur_damage);
        pixman_region32_union(half_expand_damage, half_expand_damage, &new_half_expand_damage);
    }

    wl_list_insert(nodes, &blur_node->link);
    pixman_region32_fini(&new_half_expand_damage);
    pixman_region32_fini(&node_expand_blur_damage);
    return;
}

static void node_for_each_visible_region(struct scene_node *node, int lx, int ly,
                                         struct wl_list *nodes, pixman_region32_t *damaged_blur)
{
    pixman_region32_clear(&node->extend_render_region);
    if (!node->enabled) {
        return;
    }

    if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each_reverse(child, &tree->children, link) {
            if (child->has_blur) {
                struct blur_node *blur_node, *tmp;
                wl_list_for_each_safe(blur_node, tmp, nodes, link) {
                    if (blur_node->node != child) {
                        continue;
                    }

                    pixman_region32_union(damaged_blur, damaged_blur, &blur_node->damaged_blur);
                    destroy_blur_node(blur_node);
                    break;
                }
            }
            node_for_each_visible_region(child, lx + child->x, ly + child->y, nodes, damaged_blur);
        }
        return;
    }

    /* the blur damage must be refresh */
    if (!pixman_region32_not_empty(damaged_blur)) {
        return;
    }

    struct wlr_box box;
    node->impl.get_bounding_box(node, &box);

    pixman_region32_t blur_update_region;
    pixman_region32_init_rect(&blur_update_region, 0, 0, box.width, box.height);
    if (pixman_region32_not_empty(&node->clip_region)) {
        pixman_region32_intersect(&blur_update_region, &blur_update_region, &node->clip_region);
    }
    pixman_region32_translate(&blur_update_region, box.x + lx, box.y + ly);
    pixman_region32_intersect(&blur_update_region, &blur_update_region, damaged_blur);
    pixman_region32_copy(&node->extend_render_region, &blur_update_region);
    pixman_region32_fini(&blur_update_region);

    pixman_region32_t node_opaque;
    pixman_region32_init(&node_opaque);
    node->impl.get_opaque_region(node, &node_opaque);
    if (!pixman_region32_not_empty(&node_opaque)) {
        pixman_region32_fini(&node_opaque);
        return;
    }

    pixman_region32_translate(&node_opaque, lx, ly);
    pixman_region32_subtract(damaged_blur, damaged_blur, &node_opaque);
    pixman_region32_fini(&node_opaque);
}

static bool blur_frame_render_begin(struct effect_entity *entity,
                                    struct scene_render_target *target)
{
    struct blur_data *data = entity->user_data;

    struct opengl_render_pass *gl_pass =
        opengl_render_pass_from_wlr_render_pass(target->render_pass);
    struct wlr_texture *src_tex =
        opengl_texture_from_buffer(&blur_config.renderer->wlr_renderer, gl_pass->buffer->buffer);
    data->current_output_texture = opengl_texture_from_wlr_texture(src_tex);

    struct blur_output_data *output_data = NULL, *exits_data;
    wl_list_for_each(exits_data, &data->output_data, link) {
        if (exits_data->output == target->output) {
            output_data = exits_data;
            break;
        }
    }

    data->current_output_data = output_data;
    if (!output_data) {
        return true;
    }

    pixman_region32_clear(&output_data->unaffected_region);

    pixman_region32_t all_blur, damaged_blur, half_expand_damage;
    pixman_region32_init(&all_blur);
    pixman_region32_init(&damaged_blur);
    pixman_region32_init(&half_expand_damage);
    pixman_region32_copy(&half_expand_damage, &target->damage);

    struct wl_list blur_nodes;
    wl_list_init(&blur_nodes);
    struct scene_node *node = &data->effect->scene->tree.node;
    node_for_each_blur_region(node, target, 0, 0, &blur_nodes, &half_expand_damage);
    node_for_each_visible_region(node, 0, 0, &blur_nodes, &damaged_blur);
    destroy_blur_nodes(&blur_nodes);

    /* the target must be output in render begin */
    pixman_region32_intersect_rect(&target->damage, &target->damage, target->logical.x,
                                   target->logical.y, target->logical.width,
                                   target->logical.height);
    pixman_region32_subtract(&output_data->unaffected_region, &target->damage, &half_expand_damage);

    pixman_region32_fini(&all_blur);
    pixman_region32_fini(&damaged_blur);
    pixman_region32_fini(&half_expand_damage);
    return true;
}

static bool blur_frame_render_end(struct effect_entity *entity, struct scene_render_target *target)
{
    struct blur_data *data = entity->user_data;

    if (data->current_output_texture) {
        wlr_texture_destroy(&data->current_output_texture->wlr_texture);
        data->current_output_texture = NULL;
    }

    struct blur_output_data *output_data = data->current_output_data;
    if (!output_data || !output_data->output_buffer ||
        !pixman_region32_not_empty(&output_data->unaffected_region)) {
        return true;
    }

    /**
     * if sub the unaffected region from target damage,
     * the new target damage (to framebuffer coord) union (+) unaffected region (to framebuffer
     * coord) maybe not equal (!=) target old framebuffer damage (to framebuffer coord), when output
     * scale is not integer.
     */
    pixman_region32_union(&target->excluded_damage, &target->excluded_damage,
                          &output_data->unaffected_region);

    pixman_region32_translate(&output_data->unaffected_region, -target->logical.x,
                              -target->logical.y);
    scene_render_region(&output_data->unaffected_region, target);

    PROFILE_RENDER_ZONE(&blur_config.renderer->wlr_renderer, gzone, __func__);
    struct wlr_texture *texture =
        wlr_texture_from_buffer(target->output->output->renderer, output_data->output_buffer);
    struct wlr_render_texture_options options = {
        .texture = texture,
        .clip = &output_data->unaffected_region,
        .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
    };
    wlr_render_pass_add_texture(target->render_pass, &options);
    wlr_texture_destroy(texture);
    PROFILE_RENDER_ZONE_END(&blur_config.renderer->wlr_renderer);

    return true;
}

static void blur_output_data_buffer_destroy(struct blur_output_data *data)
{
    if (!data->output_buffer) {
        return;
    }
    wl_list_remove(&data->buffer_destroy.link);
    wl_list_init(&data->buffer_destroy.link);
    data->output_buffer = NULL;
}

static void blur_output_data_destroy(struct blur_output_data *data)
{
    wl_list_remove(&data->output_destroy.link);

    blur_output_data_buffer_destroy(data);

    blur_task_destroy(data->blur_task);
    pixman_region32_fini(&data->unaffected_region);
    wl_list_remove(&data->link);
    free(data);
}

static void handle_output_buffer_destroy(struct wl_listener *listener, void *data)
{
    struct blur_output_data *_data = wl_container_of(listener, _data, buffer_destroy);
    blur_output_data_buffer_destroy(_data);
}

static void handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct blur_output_data *_data = wl_container_of(listener, _data, output_destroy);
    blur_output_data_destroy(_data);
}

static bool blur_frame_render_post(struct effect_entity *entity, struct scene_render_target *target)
{
    struct blur_data *data = entity->user_data;
    struct wlr_buffer *buffer = target->buffer;
    struct blur_output_data *output_data = NULL, *exits_data;
    wl_list_for_each(exits_data, &data->output_data, link) {
        if (exits_data->output == target->output) {
            output_data = exits_data;
            break;
        }
    }
    if (output_data) {
        if (output_data->output_buffer == buffer) {
            return true;
        }
        wl_list_remove(&output_data->buffer_destroy.link);
        output_data->output_buffer = buffer;
        wl_signal_add(&buffer->events.destroy, &output_data->buffer_destroy);
        return true;
    }

    output_data = calloc(1, sizeof(*output_data));
    if (!output_data) {
        return true;
    }
    wl_list_insert(&data->output_data, &output_data->link);

    pixman_region32_init(&output_data->unaffected_region);
    output_data->blur_task = blur_task_create();
    output_data->output = target->output;
    output_data->output_destroy.notify = handle_output_destroy;
    wl_signal_add(&target->output->events.destroy, &output_data->output_destroy);

    output_data->output_buffer = buffer;
    output_data->buffer_destroy.notify = handle_output_buffer_destroy;
    wl_signal_add(&buffer->events.destroy, &output_data->buffer_destroy);
    return true;
}

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    struct blur_effect *effect = wl_container_of(listener, effect, enable);
    scene_damage_whole(effect->scene);
}

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    struct blur_effect *effect = wl_container_of(listener, effect, disable);
    scene_damage_whole(effect->scene);
}

static bool handle_blur_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    return false;
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct blur_effect *effect = wl_container_of(listener, effect, destroy);
    wl_list_remove(&effect->destroy.link);
    wl_list_remove(&effect->enable.link);
    wl_list_remove(&effect->disable.link);
    blur_data_destroy(&effect_blur_data);
    free(effect);
}

static const struct effect_interface blur_impl = {
    .frame_render_begin = blur_frame_render_begin,
    .frame_render_end = blur_frame_render_end,
    .frame_render_post = blur_frame_render_post,
    .configure = handle_blur_effect_configure,
};

bool blur_effect_create(struct effect_manager *effect_manager)
{
    /* check blur if opengl renderer support */
    struct wlr_renderer *renderer = effect_manager->server->renderer;
    if (!wlr_renderer_is_opengl(renderer)) {
        blur_config.renderer = NULL;
        return false;
    }
    blur_config.renderer = opengl_renderer_from_wlr_renderer(renderer);

    struct blur_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return false;
    }
    /* the priority very high, ensure correct display before other effect paint. */
    effect->effect = effect_create("blur", 999, true, &blur_impl, NULL);
    if (!effect->effect) {
        free(effect);
        return false;
    }

    effect->effect->category = EFFECT_CATEGORY_STYLE;
    effect->scene = effect_manager->server->scene;

    effect->enable.notify = handle_effect_enable;
    wl_signal_add(&effect->effect->events.enable, &effect->enable);
    effect->disable.notify = handle_effect_disable;
    wl_signal_add(&effect->effect->events.disable, &effect->disable);
    effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&effect->effect->events.destroy, &effect->destroy);

    effect_blur_data.effect = effect;
    wl_list_init(&effect_blur_data.output_data);

    struct effect_entity *entity = scene_add_effect(effect->scene, effect->effect);
    if (!entity) {
        effect_destroy(effect->effect);
        return false;
    }
    entity->user_data = &effect_blur_data;

    return true;
}
