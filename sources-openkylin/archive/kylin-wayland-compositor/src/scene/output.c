// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/backend.h>
#include <wlr/render/swapchain.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/region.h>

#include "backend/backend.h"
#include "backend/drm.h"
#include "effect/effect.h"
#include "output.h"
#include "render/drm_syncobj.h"
#include "render/pass.h"
#include "render/renderer.h"
#include "scene/surface.h"
#include "scene_p.h"
#include "util/debug.h"
#include "util/quirks.h"

void scene_output_damage_whole(struct scene_output *scene_output)
{
    wlr_damage_ring_add_whole(&scene_output->damage_ring);
    output_schedule_frame(scene_output->output);
}

static void scene_output_update_geometry(struct scene_output *scene_output, bool force_update)
{
    struct output *output = output_from_wlr_output(scene_output->output);
    wlr_damage_ring_set_bounds(&scene_output->damage_ring, output->geometry.width,
                               output->geometry.height);

    wlr_damage_ring_add_whole(&scene_output->damage_ring);
    output_schedule_frame(scene_output->output);

    scene_node_update_outputs(&scene_output->scene->tree.node, &scene_output->scene->outputs, NULL,
                              force_update ? scene_output : NULL);
}

static void scene_output_handle_destroy(struct wlr_addon *addon)
{
    struct scene_output *scene_output = wl_container_of(addon, scene_output, addon);
    scene_output_destroy(scene_output);
}

static const struct wlr_addon_interface output_addon_impl = {
    .name = "scene_output",
    .destroy = scene_output_handle_destroy,
};

static void scene_output_handle_configure(struct wl_listener *listener, void *data)
{
    struct scene_output *scene_output = wl_container_of(listener, scene_output, output_configure);
    struct output *output = output_from_wlr_output(scene_output->output);
    struct output_configure_event *event = data;
    /* already called in scene_output_create */
    if (!output->state_usable) {
        return;
    }

    bool force_update = event->changes & (KYWC_OUTPUT_STATE_TRANSFORM | KYWC_OUTPUT_STATE_SCALE);
    if (force_update || event->changes & (KYWC_OUTPUT_STATE_MODE | KYWC_OUTPUT_STATE_POSITION |
                                          KYWC_OUTPUT_STATE_POWER)) {
        scene_output->geometry = output->geometry;
        scene_output_update_geometry(scene_output, force_update);
    }
}

static void scene_output_handle_damage(struct wl_listener *listener, void *data)
{
    struct scene_output *scene_output = wl_container_of(listener, scene_output, output_damage);
    struct wlr_output_event_damage *event = data;

    pixman_region32_t damage;
    pixman_region32_init(&damage);
    wlr_region_scale(&damage, event->damage, 1 / event->output->scale);

    if (wlr_damage_ring_add(&scene_output->damage_ring, &damage)) {
        output_schedule_frame(scene_output->output);
    }

    pixman_region32_fini(&damage);
}

static void scene_output_handle_needs_frame(struct wl_listener *listener, void *data)
{
    struct scene_output *scene_output = wl_container_of(listener, scene_output, output_needs_frame);
    output_schedule_frame(scene_output->output);
}

struct scene_output *scene_output_create(struct scene *scene, struct wlr_output *output)
{
    struct scene_output *scene_output = calloc(1, sizeof(*scene_output));
    if (!scene_output) {
        return NULL;
    }

    scene_output->output = output;
    scene_output->scene = scene;
    wlr_addon_init(&scene_output->addon, &output->addons, scene, &output_addon_impl);
    wlr_addon_set_init(&scene_output->addons);

    wlr_damage_ring_init(&scene_output->damage_ring);
    pixman_region32_init(&scene_output->collected_damage);
    pixman_region32_init(&scene_output->frame_damage);

    int prev_output_index = -1;
    struct wl_list *prev_output_link = &scene->outputs;

    struct scene_output *current_output;
    wl_list_for_each(current_output, &scene->outputs, link) {
        if (prev_output_index + 1 != current_output->index) {
            break;
        }

        prev_output_index = current_output->index;
        prev_output_link = &current_output->link;
    }

    scene_output->index = prev_output_index + 1;
    assert(scene_output->index < 64);
    wl_list_insert(prev_output_link, &scene_output->link);

    wl_signal_init(&scene_output->events.viewport);
    wl_signal_init(&scene_output->events.frame);
    wl_signal_init(&scene_output->events.destroy);

    scene_output->output_damage.notify = scene_output_handle_damage;
    wl_signal_add(&output->events.damage, &scene_output->output_damage);

    scene_output->output_needs_frame.notify = scene_output_handle_needs_frame;
    wl_signal_add(&output->events.needs_frame, &scene_output->output_needs_frame);

    struct output *_output = output_from_wlr_output(output);
    char *env = getenv("KYWC_DISABLE_DIRECT_SCANOUT");
    scene_output->direct_scanout = !(env && strcmp(env, "true") == 0) &&
                                   !(_output->quirks & QUIRKS_MASK_DISABLE_DIRECT_SCANOUT);

    scene_output->geometry = _output->geometry;
    scene_output->output_configure.notify = scene_output_handle_configure;
    wl_signal_add(&_output->events.configure, &scene_output->output_configure);

    scene_output_update_geometry(scene_output, false);

    int drm_fd = wlr_backend_get_drm_fd(output->backend);
    if (drm_fd >= 0 && renderer_support_timeline(output->renderer) &&
        backend_support_timeline(output->backend)) {
        scene_output->in_timeline = drm_syncobj_timeline_create(drm_fd);
        scene_output->out_timeline = drm_syncobj_timeline_create(drm_fd);
        if (scene_output->in_timeline == NULL || scene_output->out_timeline == NULL) {
            drm_syncobj_timeline_unref(scene_output->in_timeline);
            drm_syncobj_timeline_unref(scene_output->out_timeline);
            scene_output->in_timeline = NULL;
            scene_output->out_timeline = NULL;
            kywc_log(KYWC_ERROR, "Failed to create DRM syncobj timeline, falling back to implicit sync");
        }
    }

    return scene_output;
}

void scene_output_destroy(struct scene_output *scene_output)
{
    if (!scene_output) {
        return;
    }

    wl_signal_emit_mutable(&scene_output->events.destroy, NULL);
    wlr_addon_set_finish(&scene_output->addons);
    assert(wl_list_empty(&scene_output->events.viewport.listener_list));
    assert(wl_list_empty(&scene_output->events.frame.listener_list));
    assert(wl_list_empty(&scene_output->events.destroy.listener_list));

    scene_node_update_outputs(&scene_output->scene->tree.node, &scene_output->scene->outputs,
                              scene_output, NULL);

    wlr_addon_finish(&scene_output->addon);
    wlr_damage_ring_finish(&scene_output->damage_ring);
    pixman_region32_fini(&scene_output->collected_damage);
    pixman_region32_fini(&scene_output->frame_damage);
    wlr_buffer_unlock(scene_output->buffer);

    if (scene_output->in_timeline != NULL) {
        drm_syncobj_timeline_signal(scene_output->in_timeline, UINT64_MAX);
        drm_syncobj_timeline_unref(scene_output->in_timeline);
    }
    if (scene_output->out_timeline != NULL) {
        drm_syncobj_timeline_signal(scene_output->out_timeline, UINT64_MAX);
        drm_syncobj_timeline_unref(scene_output->out_timeline);
    }

    wl_list_remove(&scene_output->link);
    wl_list_remove(&scene_output->output_damage.link);
    wl_list_remove(&scene_output->output_needs_frame.link);
    wl_list_remove(&scene_output->output_configure.link);

    free(scene_output);
}

struct scene_output *scene_get_scene_output(struct scene *scene, struct wlr_output *output)
{
    struct wlr_addon *addon = wlr_addon_find(&output->addons, scene, &output_addon_impl);
    if (!addon) {
        return NULL;
    }
    struct scene_output *scene_output = wl_container_of(addon, scene_output, addon);
    return scene_output;
}

struct output *scene_get_first_enabled_output(struct scene *scene)
{
    struct scene_output *scene_output;
    wl_list_for_each(scene_output, &scene->outputs, link) {
        if (scene_output->output->enabled) {
            return output_from_wlr_output(scene_output->output);
        }
    }

    return NULL;
}

void scene_output_set_viewport_source_box(struct scene_output *scene_output,
                                          const struct kywc_box *src_box)
{
    if (kywc_box_equal(&scene_output->viewport.src, src_box)) {
        return;
    }

    if (src_box) {
        scene_output->viewport.src = *src_box;
    } else {
        scene_output->viewport.src = (struct kywc_box){ 0 };
    }

    scene_output_damage_whole(scene_output);
    wl_signal_emit_mutable(&scene_output->events.viewport, NULL);
}

static void scene_node_send_frame_done(struct scene_node *node, struct scene_output *scene_output,
                                       struct timespec *now)
{
    if (!node->enabled && node->force_damage_event == 0) {
        return;
    }

    if (node->type == SCENE_NODE_BUFFER) {
        struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
        if (scene_buffer->primary_output == scene_output) {
            wl_signal_emit_mutable(&scene_buffer->events.frame_done, now);
        }
    } else if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *scene_tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each(child, &scene_tree->children, link) {
            if (child->type != SCENE_NODE_RECT) {
                scene_node_send_frame_done(child, scene_output, now);
            }
        }
    }
}

void scene_output_send_frame_done(struct scene_output *scene_output, struct timespec *now)
{
    scene_node_send_frame_done(&scene_output->scene->tree.node, scene_output, now);
}

static bool scene_output_do_render(struct scene_output *scene_output,
                                   struct wlr_output_state *state,
                                   struct scene_render_target *target)
{
    PROFILE_ZONE(zone, __func__);

    struct wlr_output *wlr_output = scene_output->output;
    if (!wlr_output_configure_primary_swapchain(wlr_output, state, &wlr_output->swapchain)) {
        PROFILE_ZONE_END(zone);
        return false;
    }

    int buffer_age;
    struct wlr_buffer *buffer = wlr_swapchain_acquire(wlr_output->swapchain, &buffer_age);
    if (buffer == NULL) {
        PROFILE_ZONE_END(zone);
        return false;
    }

    scene_output->in_point++;
    struct wlr_buffer_pass_options options = { .signal_timeline = scene_output->in_timeline,
                                               .signal_point = scene_output->in_point };
    struct wlr_render_pass *render_pass =
        wlr_renderer_begin_buffer_pass(wlr_output->renderer, buffer, &options);
    if (render_pass == NULL) {
        wlr_buffer_unlock(buffer);

        PROFILE_ZONE_END(zone);
        return false;
    }

    target->buffer = buffer;
    target->render_pass = render_pass;

    pixman_region32_t frame_damage, acc_damage;
    pixman_region32_init(&frame_damage);
    pixman_region32_init(&acc_damage);

    // target damage is accumulated damage in the output, but in layout coord
    wlr_damage_ring_get_buffer_damage(&scene_output->damage_ring, buffer_age, &target->damage);

    if (pixman_region32_not_empty(&target->damage)) {
        // translate to scene layout coord
        pixman_region32_translate(&target->damage, target->logical.x, target->logical.y);
        pixman_region32_copy(&frame_damage, &scene_output->damage_ring.current);
        pixman_region32_copy(&acc_damage, &target->damage);

        // render all nodes and effects
        scene_render_damage_in_target(scene_output->scene,
                                      &scene_output->scene->collected_invisible, target);

        // update target excluded_buffer_damage from excluded_damage
        pixman_region32_copy(&target->excluded_buffer_damage, &target->excluded_damage);
        pixman_region32_translate(&target->excluded_buffer_damage, -target->logical.x,
                                  -target->logical.y);
        scene_render_region(&target->excluded_buffer_damage, target);

        // trying to render software cursors
        scene_render_target_add_software_cursors(target);

        /**
         * damage extended in render, translate to output coord
         * the expansion area consists of two parts, one is the two expansion areas of blur,
         * and the other is the damage area formed effects rendering ( in target expand_damage).
         * all expansion areas don't add to current damage ring. the damage area
         * formed effects rendering add to the next frame, it will be in next damage ring.
         */
        pixman_region32_t expand_damage;
        pixman_region32_init(&expand_damage);
        pixman_region32_subtract(&expand_damage, &target->damage, &acc_damage);
        // subtract the excluded damage
        pixman_region32_subtract(&expand_damage, &expand_damage, &target->excluded_damage);
        pixman_region32_union(&expand_damage, &expand_damage, &target->expand_damage);
        pixman_region32_translate(&expand_damage, -target->logical.x, -target->logical.y);
        pixman_region32_intersect_rect(&expand_damage, &expand_damage, 0, 0,
                                       scene_output->damage_ring.width,
                                       scene_output->damage_ring.height);

        pixman_region32_translate(&acc_damage, -target->logical.x, -target->logical.y);
        pixman_region32_union(&acc_damage, &acc_damage, &expand_damage);
        pixman_region32_union(&frame_damage, &frame_damage, &expand_damage);
        pixman_region32_fini(&expand_damage);
    }

    wlr_damage_ring_rotate(&scene_output->damage_ring);
    /* damage in post is added in next frame */
    scene_output_render_post(target);

    struct output *output = output_from_wlr_output(wlr_output);
    if (!render_pass_submit(render_pass, output->quirks)) {
        wlr_buffer_unlock(buffer);
        pixman_region32_fini(&acc_damage);
        pixman_region32_fini(&frame_damage);
        wlr_damage_ring_add_whole(&scene_output->damage_ring);

        PROFILE_ZONE_END(zone);
        return false;
    }

    if (pixman_region32_not_empty(&frame_damage)) {
        pixman_region32_copy(&scene_output->frame_damage, &frame_damage);
        pixman_region32_translate(&scene_output->frame_damage, scene_output->geometry.x,
                                  scene_output->geometry.y);
        scene_render_region(&frame_damage, target);
    }
    if (pixman_region32_not_empty(&acc_damage)) {
        scene_render_region(&acc_damage, target);
    }
    /* set damage even if frame/acc damage is empty */
    wlr_output_set_buffer_damage(wlr_output, &acc_damage);
    wlr_output_state_set_damage(state, &frame_damage);
    wlr_output_state_set_buffer(state, buffer);

    wlr_buffer_unlock(buffer);
    pixman_region32_fini(&acc_damage);
    pixman_region32_fini(&frame_damage);

    if (scene_output->in_timeline != NULL) {
        wlr_output_set_wait_timeline(wlr_output, scene_output->in_timeline, scene_output->in_point);
        scene_output->out_point++;
        wlr_output_set_signal_timeline(wlr_output, scene_output->out_timeline,
                                       scene_output->out_point);
    }

    PROFILE_ZONE_END(zone);
    return true;
}

static bool scene_get_fullscreen_buffer(struct scene_node *node, int lx, int ly,
                                        const struct kywc_box *box, struct scene_buffer **buffer)
{
    if (!scene_node_is_visible(node)) {
        return false;
    }

    if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each_reverse(child, &tree->children, link) {
            if (scene_get_fullscreen_buffer(child, lx + child->x, ly + child->y, box, buffer)) {
                return true;
            }
        }
        return false;
    }

    struct kywc_box bound = { 0 };
    node->impl.get_affected_bounding_box(node, SCENE_BOUNDING_WITHOUT_EFFECT, &bound);
    bound.x += lx, bound.y += ly;

    struct kywc_box dest;
    if (!kywc_box_intersection(&dest, &bound, box)) {
        return false;
    }

    if (node->type == SCENE_NODE_RECT) {
        return true;
    }

    if (dest.width != box->width || dest.height != box->height) {
        return true;
    }

    struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
    pixman_region32_t opaque_region;
    pixman_region32_init(&opaque_region);
    node->impl.get_opaque_region(node, &opaque_region);
    if (!pixman_region32_not_empty(&opaque_region)) {
        pixman_region32_fini(&opaque_region);
        return true;
    }

    if (pixman_region32_contains_rectangle(&opaque_region,
                                           &(pixman_box32_t){ 0, 0, box->width, box->height }) ==
        PIXMAN_REGION_IN) {
        *buffer = scene_buffer;
    }

    pixman_region32_fini(&opaque_region);

    return true;
}

static struct scene_buffer *scene_output_get_fullscreen_buffer(struct scene_output *scene_output)
{
    struct scene_node *root = &scene_output->scene->tree.node;
    int lx = root->x, ly = root->y;

    struct scene_buffer *buffer = NULL;
    scene_get_fullscreen_buffer(root, lx, ly, &scene_output->geometry, &buffer);
    return buffer;
}

static bool scene_output_direct_scanout_is_allowed(struct scene_render_target *target)
{
    struct scene_output *scene_output = target->output;
    if (!scene_output->direct_scanout) {
        return false;
    }

    struct wlr_output *output = scene_output->output;
    if (!wlr_output_is_direct_scanout_allowed(output)) {
        return false;
    }

    if (!output_use_hardware_color(output_from_wlr_output(output))) {
        return false;
    }

    if (!effect_manager_allow_direct_scanout(target)) {
        return false;
    }

    return true;
}

static bool scene_try_direct_scanout(struct scene_output *scene_output,
                                     struct wlr_output_state *state,
                                     struct scene_buffer *scene_buffer)
{
    if (!scene_buffer) {
        return false;
    }

    struct wlr_output *output = scene_output->output;
    if (scene_buffer->transform != output->transform) {
        return false;
    }

    struct wlr_fbox default_box = { 0 };
    if (scene_buffer->transform & WL_OUTPUT_TRANSFORM_90) {
        default_box.width = scene_buffer->buffer->height;
        default_box.height = scene_buffer->buffer->width;
    } else {
        default_box.width = scene_buffer->buffer->width;
        default_box.height = scene_buffer->buffer->height;
    }

    if (!wlr_fbox_empty(&scene_buffer->src_box) &&
        !wlr_fbox_equal(&scene_buffer->src_box, &default_box)) {
        return false;
    }

    if (scene_buffer->buffer->width != output->width ||
        scene_buffer->buffer->height != output->height) {
        kywc_log(KYWC_DEBUG, "Scene buffer size mismatch");
        return false;
    }

    if (scene_buffer->primary_output == scene_output) {
        struct wlr_linux_dmabuf_feedback_v1_init_options options = {
            .main_renderer = scene_output->output->renderer,
            .scanout_primary_output = scene_output->output,
        };

        scene_buffer_send_dmabuf_feedback(scene_output->scene, scene_buffer, &options);
        scene_buffer->node.sent_dmabuf_feedback = true;
    }

    struct wlr_output_state pending;
    wlr_output_state_init(&pending);
    if (!wlr_output_state_copy(&pending, state)) {
        return false;
    }

    struct wlr_buffer *wlr_buffer = scene_buffer->buffer;
    struct wlr_client_buffer *client_buffer = wlr_client_buffer_get(wlr_buffer);
    if (client_buffer != NULL && client_buffer->source != NULL &&
        client_buffer->source->n_locks > 0) {
        wlr_buffer = client_buffer->source;
    }
    wlr_output_state_set_buffer(&pending, wlr_buffer);

    if (scene_buffer->wait_timeline != NULL) {
        wlr_output_set_wait_timeline(output, scene_buffer->wait_timeline, scene_buffer->wait_point);
    }

    if (scene_output->out_timeline) {
        scene_output->out_point++;
        wlr_output_set_signal_timeline(output, scene_output->out_timeline, scene_output->out_point);
    }

    if (!wlr_output_test_state(scene_output->output, &pending)) {
        wlr_output_state_finish(&pending);
        return false;
    }

    wlr_output_state_copy(state, &pending);
    wlr_output_state_finish(&pending);

    struct scene_output_sample_event sample_event = {
        .output = scene_output,
        .direct_scanout = true,
        .release_timeline = scene_output->out_timeline,
        .release_point = scene_output->out_point,
    };
    wl_signal_emit_mutable(&scene_buffer->events.output_sample, &sample_event);

    return true;
}

static bool is_tearing_allowed(struct scene *scene, struct scene_buffer *buffer)
{
    if (!buffer) {
        return false;
    }

    struct wlr_surface *surface = wlr_surface_try_from_node(&buffer->node);
    if (!surface) {
        return false;
    }

    return scene_surface_is_tearing_allowed(scene, surface);
}

bool scene_output_commit(struct scene_output *scene_output,
                         const struct scene_output_state_options *options)
{
    /* make sure something is done before commit */
    wl_signal_emit_mutable(&scene_output->events.frame, NULL);

    struct wlr_output *wlr_output = scene_output->output;
    struct scene_render_target target = {
        .transform = wlr_output->transform,
        .scale = wlr_output->scale,
        .logical = { .x = scene_output->geometry.x, .y = scene_output->geometry.y },
        .output = scene_output,
        .options = SCENE_RENDER_ENABLE_PRESENTATION,
    };
    wlr_output_transformed_resolution(wlr_output, &target.trans_width, &target.trans_height);
    target.logical.width = ceil(target.trans_width / wlr_output->scale);
    target.logical.height = ceil(target.trans_height / wlr_output->scale);

    scene_output_render_pre(&target);

    pixman_region32_t damage;
    // current scene damage in the output box
    pixman_region32_init_rect(&damage, target.logical.x, target.logical.y, target.logical.width,
                              target.logical.height);
    scene_collect_damage(scene_output->scene);
    pixman_region32_intersect(&damage, &damage, &scene_output->collected_damage);
    pixman_region32_clear(&scene_output->collected_damage);

    // union all damage in the output layout box
    pixman_region32_translate(&damage, -target.logical.x, -target.logical.y);
    if (floor(target.scale) != target.scale) {
        wlr_region_expand(&damage, &damage, 1);
    }
    /* union damage from output damage event */
    wlr_damage_ring_add(&scene_output->damage_ring, &damage);
    pixman_region32_fini(&damage);

    if (!wlr_output->needs_frame &&
        !pixman_region32_not_empty(&scene_output->damage_ring.current)) {
        return false;
    }

    struct scene_buffer *fullscreen = NULL;
    bool is_tearing = false;
    if (scene_output_direct_scanout_is_allowed(&target) ||
        scene_is_tearing_needed(scene_output->scene)) {
        fullscreen = scene_output_get_fullscreen_buffer(scene_output);
        is_tearing = is_tearing_allowed(scene_output->scene, fullscreen);
    }

    struct output *output = output_from_wlr_output(wlr_output);
    struct wlr_output_state state;
    wlr_output_state_init(&state);
    output_state_attempt_vrr(output, &state, fullscreen != NULL);

    bool scanout = scene_try_direct_scanout(scene_output, &state, fullscreen);
    if (scene_output->prev_scanout != scanout) {
        scene_output->prev_scanout = scanout;
        kywc_log(KYWC_INFO, "%s: Direct scan-out %s", wlr_output->name,
                 scanout ? "enabled" : "disabled");
        if (!scanout) {
            // When exiting direct scan-out, damage everything
            wlr_damage_ring_add_whole(&scene_output->damage_ring);
        }
    }
    if (scanout) {
        pixman_region32_t frame_damage;
        pixman_region32_init(&frame_damage);
        if (pixman_region32_not_empty(&scene_output->damage_ring.current)) {
            pixman_region32_copy(&frame_damage, &scene_output->damage_ring.current);
            scene_render_region(&frame_damage, &target);
            /* translate to layout coord */
            pixman_region32_copy(&scene_output->frame_damage, &scene_output->damage_ring.current);
            pixman_region32_translate(&scene_output->frame_damage, scene_output->geometry.x,
                                      scene_output->geometry.y);
        }
        wlr_output_state_set_damage(&state, &frame_damage);
        pixman_region32_fini(&frame_damage);

        output_state_attempt_tearing(output, &state, is_tearing);
        wlr_damage_ring_rotate(&scene_output->damage_ring);
        bool ok = wlr_output_commit_state(wlr_output, &state);
        wlr_output_state_finish(&state);
        pixman_region32_clear(&scene_output->frame_damage);
        return ok;
    }

    scene_render_target_init_regions(&target);
    // scene_log_region(KYWC_ERROR, "frame damage", &scene_output->damage_ring.current);

    bool ok = false;
    if (scene_output_do_render(scene_output, &state, &target)) {
        output_state_attempt_tearing(output, &state, is_tearing);
        ok = wlr_output_commit_state(wlr_output, &state);
        wlr_buffer_unlock(scene_output->buffer);
        scene_output->buffer = wlr_buffer_lock(target.buffer);
        if (!ok) {
            wlr_swapchain_set_buffer_submitted(wlr_output->swapchain, state.buffer);
            output_schedule_frame_timer(scene_output->output);
        }
    }

    wlr_output_state_finish(&state);
    pixman_region32_clear(&scene_output->frame_damage);
    scene_render_target_finish_regions(&target);

    return ok;
}
