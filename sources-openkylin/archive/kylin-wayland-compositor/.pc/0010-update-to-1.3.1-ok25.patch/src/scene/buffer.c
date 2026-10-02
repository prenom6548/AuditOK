// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/region.h>

#include "render/pass.h"
#include "render/profile.h"
#include "render/renderer.h"
#include "scene/render.h"
#include "scene/surface.h"
#include "scene_p.h"
#include "security.h"

struct scene_buffer *scene_buffer_from_node(struct scene_node *node)
{
    assert(node->type == SCENE_NODE_BUFFER);
    struct scene_buffer *scene_buffer = wl_container_of(node, scene_buffer, node);
    return scene_buffer;
}

static void buffer_get_dest_size(struct scene_buffer *scene_buffer, int *width, int *height)
{
    if (scene_buffer->dst_width > 0 && scene_buffer->dst_height > 0) {
        *width = scene_buffer->dst_width;
        *height = scene_buffer->dst_height;
    } else if (scene_buffer->buffer) {
        if (scene_buffer->transform & WL_OUTPUT_TRANSFORM_90) {
            *height = scene_buffer->buffer->width;
            *width = scene_buffer->buffer->height;
        } else {
            *width = scene_buffer->buffer->width;
            *height = scene_buffer->buffer->height;
        }
    } else {
        *width = *height = 0;
    }
}

static void scene_buffer_get_opaque_region(struct scene_buffer *scene_buffer, int width, int height,
                                           pixman_region32_t *region)
{
    if (!scene_buffer->buffer || scene_buffer->opacity != 1) {
        return;
    }

    if (render_buffer_is_opaque(scene_buffer->buffer)) {
        pixman_region32_init_rect(region, 0, 0, width, height);
    } else if (pixman_region32_not_empty(&scene_buffer->opaque_region)) {
        pixman_region32_intersect_rect(region, &scene_buffer->opaque_region, 0, 0, width, height);
    }
}

static void buffer_get_opaque_region(struct scene_node *node, pixman_region32_t *opaque)
{
    pixman_region32_clear(opaque);
    if (!node->enabled) {
        return;
    }

    int width, height;
    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    buffer_get_dest_size(scene_buffer, &width, &height);
    scene_buffer_get_opaque_region(scene_buffer, width, height, opaque);
}

static struct scene_node *buffer_accept_input(struct scene_node *node, int lx, int ly, double px,
                                              double py, double *rx, double *ry)
{
    /* skip disabled or input bypassed nodes */
    if (!node->enabled || node->input_bypassed) {
        return NULL;
    }

    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    struct kywc_box box = { floor(px), floor(py), 1, 1 };
    struct kywc_box node_box = { .x = lx, .y = ly };
    buffer_get_dest_size(scene_buffer, &node_box.width, &node_box.height);

    if (!kywc_box_intersects(&node_box, &box)) {
        return NULL;
    }

    *rx = px - lx;
    *ry = py - ly;

    if (pixman_region32_not_empty(&node->input_region) &&
        !pixman_region32_contains_point(&node->input_region, box.x - lx, box.y - ly, NULL)) {
        return NULL;
    }

    /* check buffer_point_accepts_input */
    if (scene_buffer->point_accepts_input &&
        !scene_buffer->point_accepts_input(scene_buffer, rx, ry)) {
        return NULL;
    }

    return node;
}

static uint32_t region_area(pixman_region32_t *region)
{
    uint32_t area = 0;

    int nrects;
    pixman_box32_t *rects = pixman_region32_rectangles(region, &nrects);
    for (int i = 0; i < nrects; ++i) {
        area += (rects[i].x2 - rects[i].x1) * (rects[i].y2 - rects[i].y1);
    }

    return area;
}

static void buffer_update_outputs(struct scene_node *node, int lx, int ly, struct wl_list *outputs,
                                  struct scene_output *ignore, struct scene_output *force)
{
    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);

    uint32_t largest_overlap = 0;
    struct scene_output *old_primary_output = scene_buffer->primary_output;
    scene_buffer->primary_output = NULL;

    size_t count = 0;
    uint64_t active_outputs = 0;
    int width, height;

    // let's update the outputs in two steps:
    //  - the primary outputs
    //  - the enter/leave signals
    // This ensures that the enter/leave signals can rely on the primary output
    // to have a reasonable value. Otherwise, they may get a value that's in
    // the middle of a calculation.
    struct scene_output *scene_output;
    wl_list_for_each(scene_output, outputs, link) {
        if (scene_output == ignore) {
            continue;
        }

        if (!scene_output->output->enabled) {
            continue;
        }

        pixman_region32_t intersection, node_region;
        pixman_region32_init(&intersection);
        buffer_get_dest_size(scene_buffer, &width, &height);
        pixman_region32_init_rect(&node_region, lx, ly, width, height);
        pixman_region32_intersect_rect(&intersection, &node_region, scene_output->geometry.x,
                                       scene_output->geometry.y, scene_output->geometry.width,
                                       scene_output->geometry.height);

        if (pixman_region32_not_empty(&intersection)) {
            uint32_t overlap = region_area(&intersection);
            if (overlap > largest_overlap) {
                largest_overlap = overlap;
                scene_buffer->primary_output = scene_output;
            }

            active_outputs |= 1ull << scene_output->index;
            count++;
        }

        pixman_region32_fini(&intersection);
        pixman_region32_fini(&node_region);
    }

    uint64_t old_active = scene_buffer->active_outputs;
    scene_buffer->active_outputs = active_outputs;

    wl_list_for_each(scene_output, outputs, link) {
        uint64_t mask = 1ull << scene_output->index;
        bool intersects = active_outputs & mask;
        bool intersects_before = old_active & mask;

        if (intersects && !intersects_before) {
            wl_signal_emit_mutable(&scene_buffer->events.output_enter, scene_output);
        } else if (!intersects && intersects_before) {
            wl_signal_emit_mutable(&scene_buffer->events.output_leave, scene_output);
        }
    }

    // if there are active outputs on this node, we should always have a primary output
    assert(!scene_buffer->active_outputs || scene_buffer->primary_output);

    // Skip output update event if nothing was updated
    if (old_active == active_outputs && (!force || ((1ull << force->index) & ~active_outputs)) &&
        old_primary_output == scene_buffer->primary_output) {
        return;
    }

    struct scene_output *outputs_array[64];
    struct scene_outputs_update_event event = {
        .primary = scene_buffer->primary_output,
        .active = outputs_array,
        .size = count,
    };

    size_t i = 0;
    wl_list_for_each(scene_output, outputs, link) {
        if (~active_outputs & (1ull << scene_output->index)) {
            continue;
        }

        assert(i < count);
        outputs_array[i++] = scene_output;
    }

    wl_signal_emit_mutable(&scene_buffer->events.outputs_update, &event);
}

static void buffer_collect_invisible(struct scene_buffer *scene_buffer, struct kywc_box *box,
                                     pixman_region32_t *invisible)
{
    pixman_region32_t region;
    pixman_region32_init(&region);
    scene_buffer_get_opaque_region(scene_buffer, box->width, box->height, &region);
    if (!pixman_region32_not_empty(&region)) {
        pixman_region32_fini(&region);
        return;
    }

    struct scene_node *node = &scene_buffer->node;
    if (pixman_region32_not_empty(&node->clip_region)) {
        pixman_region32_intersect(&region, &region, &node->clip_region);
    }

    /* subtract (round corners + anti-aliasing edge) */
    pixman_region32_t corner;
    pixman_region32_init(&corner);
    scene_corner_region(&corner, box->width, box->height, node->radius);
    pixman_region32_subtract(&region, &region, &corner);
    pixman_region32_fini(&corner);

    pixman_region32_translate(&region, box->x, box->y);
    pixman_region32_union(invisible, invisible, &region);
    pixman_region32_fini(&region);
}

static void buffer_collect_damage(struct scene_node *node, int lx, int ly, bool parent_enabled,
                                  uint32_t damage_type, pixman_region32_t *damage,
                                  pixman_region32_t *invisible, pixman_region32_t *affected)
{
    bool node_enabled = parent_enabled && node->enabled;

    /* node is still disabled, skip it */
    if (!node_enabled && !node->last_enabled) {
        node->damage_type = SCENE_DAMAGE_NONE;
        return;
    }

    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    int width, height;
    buffer_get_dest_size(scene_buffer, &width, &height);
    // if node state is changed, it must in the affected region
    if (width > 0 && height > 0 &&
        pixman_region32_contains_rectangle(
            affected, &(pixman_box32_t){ lx, ly, lx + width, ly + height }) == PIXMAN_REGION_OUT) {
        if (node_enabled) {
            buffer_collect_invisible(scene_buffer, &(struct kywc_box){ lx, ly, width, height },
                                     invisible);
        }
        return;
    }

    bool no_damage = node->last_enabled && node_enabled && damage_type == SCENE_DAMAGE_NONE;
    if (!no_damage) {
        /* node last visible region is added to damgae */
        if (node->last_enabled && (!node_enabled || (damage_type & SCENE_DAMAGE_HARMFUL))) {
            pixman_region32_union(damage, damage, &node->visible_region);
        }
    }

    bool visible = node_enabled && scene_buffer->opacity != 0 && scene_buffer->buffer;
    pixman_region32_clear(&node->visible_region);

    if (visible) {
        bool has_clip_region = pixman_region32_not_empty(&node->clip_region);
        if (has_clip_region) {
            pixman_region32_intersect_rect(&node->visible_region, &node->clip_region, 0, 0, width,
                                           height);
            pixman_region32_translate(&node->visible_region, lx, ly);
        } else {
            pixman_region32_init_rect(&node->visible_region, lx, ly, width, height);
        }
        pixman_region32_subtract(&node->visible_region, &node->visible_region, invisible);

        if (!no_damage) {
            pixman_region32_union(damage, damage, &node->visible_region);
        }

        buffer_collect_invisible(scene_buffer, &(struct kywc_box){ lx, ly, width, height },
                                 invisible);
    }

    node->last_enabled = node_enabled;
    node->damage_type = SCENE_DAMAGE_NONE;
}

struct wlr_texture *scene_buffer_get_texture(struct scene_buffer *scene_buffer,
                                             struct wlr_renderer *renderer)
{
    struct wlr_client_buffer *client_buffer = wlr_client_buffer_get(scene_buffer->buffer);
    if (client_buffer != NULL) {
        return client_buffer->texture;
    }

    if (scene_buffer->texture != NULL) {
        return scene_buffer->texture;
    }

    scene_buffer->texture = wlr_texture_from_buffer(renderer, scene_buffer->buffer);
    return scene_buffer->texture;
}

static void buffer_render(struct scene_node *node, int lx, int ly,
                          struct scene_render_target *target)
{
    if (!node->enabled) {
        return;
    }

    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    if (!scene_buffer->buffer || scene_buffer->opacity == 0) {
        return;
    }

    if (target->options & SCENE_RENDER_ENABLE_SECURITY &&
        security_check_node(&scene_buffer->node)) {
        return;
    }

    bool render_with_visibility = !(target->options & SCENE_RENDER_DISABLE_VISIBILITY);
    if (render_with_visibility && !pixman_region32_not_empty(&node->visible_region) &&
        !pixman_region32_not_empty(&node->extend_render_region)) {
        return;
    }

    int width, height;
    buffer_get_dest_size(scene_buffer, &width, &height);

    pixman_region32_t render_region;
    if (render_with_visibility) {
        pixman_region32_init(&render_region);
        pixman_region32_union(&render_region, &node->visible_region, &node->extend_render_region);
        pixman_region32_intersect(&render_region, &render_region, &target->damage);
    } else {
        pixman_region32_init_rect(&render_region, 0, 0, width, height);
        if (pixman_region32_not_empty(&node->clip_region)) {
            pixman_region32_intersect(&render_region, &render_region, &node->clip_region);
        }
        pixman_region32_translate(&render_region, lx, ly);
        pixman_region32_intersect(&render_region, &render_region, &target->damage);
    }

    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return;
    }

    struct wlr_texture *texture =
        scene_buffer_get_texture(scene_buffer, target->output->output->renderer);
    if (texture == NULL) {
        pixman_region32_fini(&render_region);
        return;
    }

    struct wlr_box dst_box = {
        .x = lx - target->logical.x,
        .y = ly - target->logical.y,
        .width = width,
        .height = height,
    };
    scene_render_box(&dst_box, target);

    pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);

    pixman_region32_t opaque;
    pixman_region32_init(&opaque);
    scene_buffer_get_opaque_region(scene_buffer, width, height, &opaque);
    pixman_region32_translate(&opaque, lx - target->logical.x, ly - target->logical.y);
    pixman_region32_subtract(&opaque, &render_region, &opaque);

    scene_render_region(&render_region, target);

    enum wl_output_transform transform = wlr_output_transform_invert(scene_buffer->transform);
    transform = wlr_output_transform_compose(transform, target->transform);

    PROFILE_RENDER_ZONE(render_pass_get_renderer(target->render_pass), gzone, __func__);
    bool render_with_radius = !(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER);
    struct render_texture_options options = {
        .base = {
            .texture = texture,
            .src_box = scene_buffer->src_box,
            .dst_box = dst_box,
            .transform = transform,
            .alpha = &scene_buffer->opacity,
            .clip = &render_region,
            .blend_mode = pixman_region32_not_empty(&opaque) ?
                WLR_RENDER_BLEND_MODE_PREMULTIPLIED : WLR_RENDER_BLEND_MODE_NONE,
        },
        .radius = {
            .rb = render_with_radius ? node->radius[0] * target->scale : 0,
            .rt = render_with_radius ? node->radius[1] * target->scale : 0,
            .lb = render_with_radius ? node->radius[2] * target->scale : 0,
            .lt = render_with_radius ? node->radius[3] * target->scale : 0,
        },
        .repeated = scene_buffer->repeated,
        .enable_force_opaque = target->options & SCENE_RENDER_ENABLE_FORCE_OPAQUE,
    };

    if (scene_buffer->repeated) {
        struct wlr_fbox *box = &options.base.src_box;
        if (wlr_fbox_empty(box)) {
            box->x = box->y = 0;
            box->width = scene_buffer->buffer->width;
            box->height = scene_buffer->buffer->height;
        }
        box->x *= target->scale;
        box->y *= target->scale;
        box->width *= target->scale;
        box->height *= target->scale;
    }

    if (!(target->options & SCENE_RENDER_DISABLE_BLUR)) {
        struct scene_render_blur_options opts = {
            .node_lx = lx,
            .node_ly = ly,
            .round_mask = {
                .boundary = &(struct kywc_box){ dst_box.x, dst_box.y, dst_box.width, dst_box.height,},
                .radius = &options.radius,
            },
            .clip = &render_region,
            .blur = node->has_blur ? &node->blur : NULL,
        };
        scene_render_target_add_blur(target, &opts);
    }

    render_pass_add_texture(target->render_pass, &options);

    pixman_region32_fini(&render_region);
    pixman_region32_fini(&opaque);

    PROFILE_RENDER_ZONE_END(render_pass_get_renderer(target->render_pass));

    if (target->options & SCENE_RENDER_ENABLE_PRESENTATION) {
        struct scene_output_sample_event sample_event = {
            .output = target->output,
            .direct_scanout = false,
        };
        wl_signal_emit_mutable(&scene_buffer->events.output_sample, &sample_event);
    }

    if ((target->options & SCENE_RENDER_ENABLE_PRESENTATION) &&
        scene_buffer->primary_output == target->output && !node->sent_dmabuf_feedback) {
        struct wlr_linux_dmabuf_feedback_v1_init_options options = {
            .main_renderer = target->output->output->renderer,
            .scanout_primary_output = NULL,
        };

        scene_buffer_send_dmabuf_feedback(target->output->scene, scene_buffer, &options);
    }
}

static void buffer_get_bounding_box(struct scene_node *node, struct wlr_box *box)
{
    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    box->x = box->y = 0;
    buffer_get_dest_size(scene_buffer, &box->width, &box->height);
}

static void buffer_destroy(struct scene_node *node)
{
    if (!node) {
        return;
    }

    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    scene_buffer_set_buffer(scene_buffer, NULL);

    if (node->last_enabled) {
        struct scene *scene = scene_from_node(node);
        scene_add_damage(scene, &node->visible_region);
    }

    pixman_region32_fini(&scene_buffer->opaque_region);
    scene_buffer->node_destroy(node);
}

static void scene_buffer_update_buffer(struct scene_buffer *scene_buffer, struct wlr_buffer *buffer)
{
    if (scene_buffer->buffer == buffer) {
        return;
    }

    wlr_buffer_unlock(scene_buffer->buffer);
    scene_buffer->buffer = NULL;
    if (!buffer) {
        return;
    }

    scene_buffer->buffer = wlr_buffer_lock(buffer);
}

void scene_buffer_init(struct scene_buffer *scene_buffer, struct scene_tree *parent)
{
    *scene_buffer = (struct scene_buffer){
        .opacity = 1,
    };
    scene_node_init(&scene_buffer->node, parent);

    scene_buffer->node.type = SCENE_NODE_BUFFER;

    scene_buffer->node_destroy = scene_buffer->node.impl.destroy;
    scene_buffer->node.impl.destroy = buffer_destroy;

    scene_buffer->node.impl.accept_input = buffer_accept_input;
    scene_buffer->node.impl.update_outputs = buffer_update_outputs;
    scene_buffer->node.impl.get_opaque_region = buffer_get_opaque_region;
    scene_buffer->node.impl.collect_damage = buffer_collect_damage;
    scene_buffer->node.impl.render = buffer_render;
    scene_buffer->node.impl.get_bounding_box = buffer_get_bounding_box;

    wl_signal_init(&scene_buffer->events.outputs_update);
    wl_signal_init(&scene_buffer->events.output_enter);
    wl_signal_init(&scene_buffer->events.output_leave);
    wl_signal_init(&scene_buffer->events.output_sample);
    wl_signal_init(&scene_buffer->events.frame_done);

    pixman_region32_init(&scene_buffer->opaque_region);
}

struct scene_buffer *scene_buffer_create(struct scene_tree *parent, struct wlr_buffer *buffer)
{
    struct scene_buffer *scene_buffer = calloc(1, sizeof(*scene_buffer));
    if (!scene_buffer) {
        return NULL;
    }

    scene_buffer_init(scene_buffer, parent);

    if (buffer) {
        scene_buffer_update_buffer(scene_buffer, buffer);
        scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_HARMFUL, NULL);
        scene_node_update_outputs(&scene_buffer->node, NULL, NULL, NULL);
    }

    return scene_buffer;
}

static void buffer_get_scene_damage(struct scene_buffer *scene_buffer, pixman_region32_t *dst,
                                    const pixman_region32_t *src)
{
    struct wlr_buffer *buffer = scene_buffer->buffer;
    struct wlr_fbox box = scene_buffer->src_box;
    if (wlr_fbox_empty(&box)) {
        box = (struct wlr_fbox){ 0, 0, buffer->width, buffer->height };
    }

    wlr_fbox_transform(&box, &box, scene_buffer->transform, buffer->width, buffer->height);

    float scale_x, scale_y;
    int width, height;
    buffer_get_dest_size(scene_buffer, &width, &height);
    scale_x = width / box.width;
    scale_y = height / box.height;

    wlr_region_transform(dst, src, scene_buffer->transform, buffer->width, buffer->height);
    pixman_region32_intersect_rect(dst, dst, box.x, box.y, box.width, box.height);
    pixman_region32_translate(dst, -box.x, -box.y);
    wlr_region_scale_xy(dst, dst, scale_x, scale_y);
}

void scene_buffer_set_buffer_with_damage(struct scene_buffer *scene_buffer,
                                         struct wlr_buffer *buffer, const pixman_region32_t *damage)
{
    assert(buffer || !damage);
    /* do nothing when still no buffer */
    if (!scene_buffer->buffer && !buffer) {
        return;
    }

    int old_width, old_height, new_width, new_height;
    bool get_or_lost_buffer = !scene_buffer->buffer || !buffer;

    wlr_texture_destroy(scene_buffer->texture);
    scene_buffer->texture = NULL;

    buffer_get_dest_size(scene_buffer, &old_width, &old_height);
    scene_buffer_update_buffer(scene_buffer, buffer);
    buffer_get_dest_size(scene_buffer, &new_width, &new_height);

    /* return early if the scene buffer output no need to update */
    if (old_width == new_width && old_height == new_height) {
        pixman_region32_t region;
        pixman_region32_init_rect(&region, 0, 0, new_width, new_height);
        if (get_or_lost_buffer) {
            scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_BOTH, &region);
        } else if (damage && pixman_region32_not_empty(damage)) {
            buffer_get_scene_damage(scene_buffer, &region, damage);
            scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_HARMLESS, &region);
        } else {
            scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_HARMLESS, &region);
        }
        pixman_region32_fini(&region);
        return;
    }

    if (old_width > new_width || old_height > new_height) {
        pixman_region32_t region;
        pixman_region32_init_rect(&region, 0, 0, old_width, old_height);
        scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_BOTH, &region);
        pixman_region32_fini(&region);
    }
    if (old_width < new_width || old_height < new_height) {
        scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_BOTH, NULL);
    }

    // buffer update outputs, leave active outputs when no buffer
    scene_node_update_outputs(&scene_buffer->node, NULL, NULL, NULL);
}

void scene_buffer_set_buffer(struct scene_buffer *scene_buffer, struct wlr_buffer *buffer)
{
    scene_buffer_set_buffer_with_damage(scene_buffer, buffer, NULL);
}

void scene_buffer_set_opaque_region(struct scene_buffer *scene_buffer,
                                    const pixman_region32_t *region)
{
    if (pixman_region32_equal(&scene_buffer->opaque_region, region)) {
        return;
    }

    pixman_region32_copy(&scene_buffer->opaque_region, region);
    scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_HARMFUL, NULL);
}

void scene_buffer_set_source_box(struct scene_buffer *scene_buffer, const struct wlr_fbox *box)
{
    if (wlr_fbox_equal(&scene_buffer->src_box, box)) {
        return;
    }

    if (box != NULL) {
        scene_buffer->src_box = *box;
    } else {
        scene_buffer->src_box = (struct wlr_fbox){ 0 };
    }

    scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_HARMLESS, NULL);
}

void scene_buffer_set_dest_size(struct scene_buffer *scene_buffer, int width, int height)
{
    if (scene_buffer->dst_width == width && scene_buffer->dst_height == height) {
        return;
    }

    bool update_later = false;
    if (scene_buffer->dst_width > width || scene_buffer->dst_height > height) {
        scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_BOTH, NULL);
    }
    if (scene_buffer->dst_width < width || scene_buffer->dst_height < height) {
        update_later = true;
    }

    scene_buffer->dst_width = width;
    scene_buffer->dst_height = height;

    if (update_later) {
        scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_BOTH, NULL);
    }

    scene_node_update_outputs(&scene_buffer->node, NULL, NULL, NULL);
}

void scene_buffer_set_transform(struct scene_buffer *scene_buffer,
                                enum wl_output_transform transform)
{
    if (scene_buffer->transform == transform) {
        return;
    }

    /* size not changed when has dest size */
    bool use_dest_size = scene_buffer->dst_width > 0 && scene_buffer->dst_height > 0;
    scene_node_push_damage(&scene_buffer->node,
                           use_dest_size ? SCENE_DAMAGE_HARMLESS : SCENE_DAMAGE_BOTH, NULL);
    scene_buffer->transform = transform;
    if (!use_dest_size) {
        scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_BOTH, NULL);
    }
}

void scene_buffer_set_opacity(struct scene_buffer *scene_buffer, float opacity)
{
    if (scene_buffer->opacity == opacity) {
        return;
    }

    bool harmful =
        scene_buffer->opacity == 0 || opacity == 0 || scene_buffer->opacity == 1 || opacity == 1;
    scene_buffer->opacity = opacity;
    /* push damage return early when has no buffer */
    scene_node_push_damage(&scene_buffer->node, harmful ? SCENE_DAMAGE_BOTH : SCENE_DAMAGE_HARMLESS,
                           NULL);
}

void scene_buffer_set_repeated(struct scene_buffer *scene_buffer, bool repeated)
{
    if (scene_buffer->repeated == repeated) {
        return;
    }

    scene_buffer->repeated = repeated;
    scene_node_push_damage(&scene_buffer->node, SCENE_DAMAGE_HARMLESS, NULL);
}

void scene_buffer_send_dmabuf_feedback(
    const struct scene *scene, struct scene_buffer *scene_buffer,
    const struct wlr_linux_dmabuf_feedback_v1_init_options *options)
{
    if (!scene->linux_dmabuf_v1) {
        return;
    }

    struct scene_surface *surface = scene_surface_try_from_buffer(scene_buffer);
    if (!surface) {
        return;
    }

    // compare to the previous options so that we don't send duplicate feedback events.
    if (memcmp(options, &scene_buffer->prev_feedback_options, sizeof(*options)) == 0) {
        return;
    }

    scene_buffer->prev_feedback_options = *options;

    struct wlr_linux_dmabuf_feedback_v1 feedback = { 0 };
    if (!wlr_linux_dmabuf_feedback_v1_init_with_options(&feedback, options)) {
        return;
    }

    wlr_linux_dmabuf_v1_set_surface_feedback(scene->linux_dmabuf_v1, surface->surface, &feedback);

    wlr_linux_dmabuf_feedback_v1_finish(&feedback);
}

bool scene_buffer_is_clipped(struct scene_buffer *scene_buffer)
{
    struct wlr_buffer *buffer = scene_buffer->buffer;
    if (!buffer) {
        return false;
    }

    struct wlr_fbox fbox = { 0, 0, buffer->width, buffer->height };
    if (!wlr_fbox_empty(&scene_buffer->src_box) && !wlr_fbox_equal(&scene_buffer->src_box, &fbox)) {
        return true;
    }

    if (!pixman_region32_not_empty(&scene_buffer->node.clip_region)) {
        return false;
    }

    int width, height;
    buffer_get_dest_size(scene_buffer, &width, &height);

    pixman_region32_t hide_region;
    pixman_region32_init_rect(&hide_region, 0, 0, width, height);
    pixman_region32_subtract(&hide_region, &hide_region, &scene_buffer->node.clip_region);
    if (pixman_region32_not_empty(&hide_region)) {
        pixman_region32_fini(&hide_region);
        return true;
    }

    pixman_region32_fini(&hide_region);
    return false;
}

void scene_node_update_outputs(struct scene_node *node, struct wl_list *outputs,
                               struct scene_output *ignore, struct scene_output *force)
{
    if (!outputs) {
        outputs = &scene_from_node(node)->outputs;
    }

    int x, y;
    scene_node_coords(node, &x, &y);

    node->impl.update_outputs(node, x, y, outputs, ignore, force);
}
