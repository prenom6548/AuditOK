// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <wlr/backend/headless.h>
#include <wlr/backend/multi.h>
#include <wlr/render/swapchain.h>
#include <wlr/types/wlr_output_layout.h>

#include "backend/drm.h"
#include "backend/fbdev.h"
#include "effect/output_transform.h"
#include "output_p.h"
#include "render/profile.h"
#include "util/debug.h"
#include "util/quirks.h"
#include "xwayland.h"

#define FALLBACK_OUTPUT "FALLBACK"

struct output_manager *output_manager = NULL;

struct kywc_output *kywc_output_by_name(const char *name)
{
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (strcmp(name, output->base.name) == 0) {
            return &output->base;
        }
    }

    return NULL;
}

struct kywc_output *kywc_output_by_uuid(const char *uuid)
{
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (strcmp(uuid, output->base.uuid) == 0) {
            return &output->base;
        }
    }

    return NULL;
}

void kywc_output_add_new_listener(struct wl_listener *listener)
{
    wl_signal_add(&output_manager->events.new_output, listener);
}

struct kywc_output *kywc_output_get_primary(void)
{
    return output_manager->primary_output;
}

void kywc_output_add_primary_listener(struct wl_listener *listener)
{
    wl_signal_add(&output_manager->events.primary_output, listener);
}

void kywc_output_effective_geometry(struct kywc_output *kywc_output, struct kywc_box *box)
{
    struct output *output = output_from_kywc_output(kywc_output);
    *box = output->geometry;
}

bool kywc_output_contains_point(struct kywc_output *kywc_output, int x, int y)
{
    struct output *output = output_from_kywc_output(kywc_output);
    return kywc_box_contains_point(&output->geometry, x, y);
}

struct output *output_from_kywc_output(struct kywc_output *kywc_output)
{
    struct output *output = wl_container_of(kywc_output, output, base);
    return output;
}

struct output *output_from_wlr_output(struct wlr_output *wlr_output)
{
    return wlr_output->data;
}

struct output *output_from_resource(struct wl_resource *resource)
{
    struct wlr_output *wlr_output = wl_resource_get_user_data(resource);
    return wlr_output ? output_from_wlr_output(wlr_output) : NULL;
}

void output_schedule_frame(struct wlr_output *wlr_output)
{
    if (output_manager->server->active) {
        wlr_output_schedule_frame(wlr_output);
    }
}

void output_add_update_usable_area_listener(struct kywc_output *kywc_output,
                                            struct wl_listener *listener, bool late)
{
    struct output *output = output_from_kywc_output(kywc_output);

    if (late) {
        wl_signal_add(&output->events.update_late_usable_area, listener);
    } else {
        wl_signal_add(&output->events.update_usable_area, listener);
    }
}

uint32_t output_manager_for_each_output(output_iterator_func_t iterator, bool enabled, void *data)
{
    int index = 0;
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (output->base.state.enabled == enabled) {
            if (iterator(&output->base, index++, data)) {
                break;
            }
        }
    }
    return index;
}

bool output_manager_has_actual_outputs(void)
{
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (&output->base != output_manager->fallback_output) {
            return true;
        }
    }
    return false;
}

float output_manager_get_max_scale(void)
{
    return output_manager->max_scale;
}

void output_manager_add_max_scale_listener(struct wl_listener *listener)
{
    wl_signal_add(&output_manager->events.max_scale, listener);
}

struct kywc_output *output_manager_get_fallback(void)
{
    return output_manager->fallback_output;
}

void output_manager_add_new_enabled_listener(struct wl_listener *listener)
{
    wl_signal_add(&output_manager->events.new_enabled_output, listener);
}

void output_manager_add_configured_listener(struct wl_listener *listener)
{
    wl_signal_add(&output_manager->events.configured, listener);
}

void output_manager_add_layout_damage_listener(struct wl_listener *listener)
{
    wl_signal_add(&output_manager->events.layout_damage, listener);
}

void output_log_state(enum kywc_log_level level, struct output *output, const char *desc,
                      const struct kywc_output_state *state)
{
    if (kywc_log_get_level() < level || !state) {
        return;
    }

    kywc_log(level,
             "%s(%s): mode (%d x %d @ %d) scale %f pos (%d, %d) transform %d %s brightness %d "
             "colortemp %d colorfilter %d %s",
             output->base.name, desc ? desc : "", state->width, state->height, state->refresh,
             state->scale, state->lx, state->ly, state->transform,
             state->enabled ? "enabled" : "disabled", state->brightness, state->color_temp,
             state->color_filter,
             output_manager->pending_primary == &output->base ? "primary" : "");
}

static void output_log_prop(enum kywc_log_level level, struct output *output, const char *desc,
                            const struct kywc_output_prop *prop)
{
    if (kywc_log_get_level() < level || !prop) {
        return;
    }

    static char *backend[] = { "none", "drm", "headless", "fbdev" };

    kywc_log(level, "%s(%s): %s backend, [%s]: [%s, %s, %s] (%d x %d)", output->base.name,
             desc ? desc : "", backend[prop->backend], prop->desc, prop->make, prop->model,
             prop->serial ? prop->serial : "", prop->phys_width, prop->phys_height);
}

static const char *output_get_edid(struct wlr_output *wlr_output)
{
    size_t len = 0;
    const void *edid_raw = wlr_output_get_edid(wlr_output, &len);
    if (!edid_raw) {
        return NULL;
    }

    const char *edid = kywc_identifier_base64_generate(edid_raw, len);
    return edid;
}

static const char *output_get_edid_hash(struct wlr_output *wlr_output)
{
    size_t len = 0;
    const void *edid = wlr_output_get_edid(wlr_output, &len);
    return edid ? kywc_identifier_md5_generate(edid, len)
                : kywc_identifier_md5_generate(wlr_output->name, strlen(wlr_output->name));
}

static void output_get_prop(struct output *output, struct kywc_output_prop *prop)
{
    struct wlr_output *wlr_output = output->wlr_output;
    static char *unknown = " ";

    if (wlr_output_is_drm(wlr_output)) {
        prop->backend = KYWC_OUTPUT_BACKEND_DRM;
    } else if (wlr_output_is_headless(wlr_output)) {
        prop->backend = KYWC_OUTPUT_BACKEND_HEADLESS;
    } else if (wlr_output_is_fbdev(wlr_output)) {
        prop->backend = KYWC_OUTPUT_BACKEND_FBDEV;
    } else {
        prop->backend = KYWC_OUTPUT_BACKEND_NONE;
    }

    prop->phys_width = wlr_output->phys_width;
    prop->phys_height = wlr_output->phys_height;
    prop->make = wlr_output->make ? wlr_output->make : wlr_output->name;
    prop->model = wlr_output->model ? wlr_output->model : unknown;
    prop->serial = wlr_output->serial; // May be NULL
    prop->desc = wlr_output->description ? wlr_output->description : wlr_output->name;
    prop->edid = output_get_edid(wlr_output);
    prop->edid_hash = output_get_edid_hash(wlr_output);

    prop->capabilities = KYWC_OUTPUT_CAPABILITY_POWER;
    if (wlr_output_get_vrr_capable(wlr_output)) {
        prop->capabilities |= KYWC_OUTPUT_CAPABILITY_VRR;
    }
    if (wlr_output_get_rgb_range(wlr_output, &output->base.state.rgb_range)) {
        prop->capabilities |= KYWC_OUTPUT_CAPABILITY_RGB_RANGE;
    }
    if (wlr_output_get_overscan(wlr_output, &output->base.state.overscan)) {
        prop->capabilities |= KYWC_OUTPUT_CAPABILITY_OVERSCAN;
    }
    if (wlr_output_get_scaling_mode(wlr_output, &output->base.state.scaling_mode)) {
        prop->capabilities |= KYWC_OUTPUT_CAPABILITY_SCALING_MODE;
    }
    prop->capabilities |= wlr_output_get_color_capabilities(wlr_output);

    /* fix zero mode in some backend, like wayland */
    if (wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode mode = {
            .width = wlr_output->width,
            .height = wlr_output->height,
            .refresh = wlr_output->refresh,
            .preferred = true,
        };
        output_add_mode(output, &mode, false);
    } else {
        struct wlr_output_mode *mode;
        wl_list_for_each(mode, &wlr_output->modes, link) {
            output_add_mode(output, mode, false);
        }
    }
}

static void output_get_state(struct output *output, struct kywc_output_state *state)
{
    struct wlr_output *wlr_output = output->wlr_output;

    state->enabled = wlr_output->enabled;
    state->power = wlr_output->enabled;
    state->width = wlr_output->width;
    state->height = wlr_output->height;
    state->refresh = wlr_output->refresh;
    state->transform = wlr_output->transform;
    state->scale = wlr_output->scale;

    struct wlr_output_layout_output *layout_output;
    layout_output = wlr_output_layout_get(output_manager->server->layout, wlr_output);
    if (layout_output) {
        state->lx = layout_output->x;
        state->ly = layout_output->y;
    } else {
        state->lx = state->ly = -1;
    }

    state->brightness = output->brightness;
    state->color_temp = output->color_temp;
    state->color_filter = output->color_filter;
    state->color_feature = output->color_feature;
    state->vrr_policy = output->vrr_policy;
    wlr_output_get_rgb_range(wlr_output, &state->rgb_range);
    wlr_output_get_overscan(wlr_output, &state->overscan);
    wlr_output_get_scaling_mode(wlr_output, &state->scaling_mode);
}

uint32_t output_get_state_changes(struct output *output, const struct kywc_output_state *state)
{
    struct kywc_output_prop *prop = &output->base.prop;
    struct kywc_output_state *current = &output->base.state;
    uint32_t mask = 0;

    if (current->enabled != state->enabled) {
        mask |= KYWC_OUTPUT_STATE_ENABLE;
    }
    if (prop->capabilities & KYWC_OUTPUT_CAPABILITY_POWER && current->power != state->power) {
        mask |= KYWC_OUTPUT_STATE_POWER;
    }
    if (current->width != state->width || current->height != state->height ||
        current->refresh != state->refresh) {
        mask |= KYWC_OUTPUT_STATE_MODE;
    }
    if (current->transform != state->transform) {
        mask |= KYWC_OUTPUT_STATE_TRANSFORM;
    }
    if (current->scale != state->scale) {
        mask |= KYWC_OUTPUT_STATE_SCALE;
    }
    if (current->vrr_policy != state->vrr_policy) {
        mask |= KYWC_OUTPUT_STATE_VRR_POLICY;
    }
    if (current->lx != state->lx || current->ly != state->ly) {
        mask |= KYWC_OUTPUT_STATE_POSITION;
    }
    if (prop->capabilities & KYWC_OUTPUT_CAPABILITY_BRIGHTNESS &&
        current->brightness != state->brightness) {
        mask |= KYWC_OUTPUT_STATE_BRIGHTNESS;
    }
    if (prop->capabilities & KYWC_OUTPUT_CAPABILITY_COLOR_TEMP &&
        current->color_temp != state->color_temp) {
        mask |= KYWC_OUTPUT_STATE_COLOR_TEMP;
    }
    if (current->overscan != state->overscan) {
        mask |= KYWC_OUTPUT_STATE_OVERSCAN;
    }
    if (current->color_feature != state->color_feature) {
        mask |= KYWC_OUTPUT_STATE_COLOR_FEATURE;
    }
    if (prop->capabilities & KYWC_OUTPUT_CAPABILITY_COLOR_FILTER &&
        current->color_filter != state->color_filter) {
        mask |= KYWC_OUTPUT_STATE_COLOR_FILTER;
    }
    if (current->scaling_mode != state->scaling_mode) {
        mask |= KYWC_OUTPUT_STATE_SCALING_MODE;
    }
    if (current->rgb_range != state->rgb_range) {
        mask |= KYWC_OUTPUT_STATE_RGB_RANGE;
    }

    return mask;
}

static void output_manager_update_max_scale(struct kywc_output *kywc_output)
{
    float scale = 1.0;
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output->destroying || !kywc_output->state.enabled) {
            continue;
        }
        if (kywc_output->state.scale > scale) {
            scale = kywc_output->state.scale;
        }
    }

    if (scale != output_manager->max_scale) {
        output_manager->max_scale = scale;
        wl_signal_emit_mutable(&output_manager->events.max_scale, kywc_output);
    }
}

bool output_use_hardware_color(struct output *output)
{
    return output->hardware_color;
}

bool output_state_attempt_vrr(struct output *output, struct wlr_output_state *state,
                              bool fullscreen)
{
    if (!(output->base.prop.capabilities & KYWC_OUTPUT_CAPABILITY_VRR)) {
        return false;
    }

    if (output->vrr_policy == KYWC_OUTPUT_VRR_POLICY_NEVER ||
        (output->vrr_policy == KYWC_OUTPUT_VRR_POLICY_AUTO && !fullscreen)) {
        if (output->wlr_output->adaptive_sync_status) {
            wlr_output_state_set_adaptive_sync_enabled(state, false);
        }
        return true;
    }

    if (!output->wlr_output->adaptive_sync_status) {
        wlr_output_state_set_adaptive_sync_enabled(state, true);
    }

    return true;
}

bool output_state_attempt_tearing(struct output *output, struct wlr_output_state *state,
                                  bool is_tearing)
{
    if (!is_tearing) {
        return false;
    }

    state->tearing_page_flip = true;
    if (!wlr_output_test_state(output->wlr_output, state)) {
        state->tearing_page_flip = false;
        return false;
    }

    return true;
}

static void output_state_get_geometry(struct kywc_output_state *state, struct kywc_box *box)
{
    if (state->transform % 2 == 0) {
        box->width = state->width;
        box->height = state->height;
    } else {
        box->width = state->height;
        box->height = state->width;
    }

    box->x = state->lx;
    box->y = state->ly;
    box->width = ceil(box->width / state->scale);
    box->height = ceil(box->height / state->scale);
}

static void output_do_update_usable_area(struct output *output, struct kywc_box *usable)
{
    *usable = output->geometry;
    wl_signal_emit_mutable(&output->events.update_usable_area, usable);
    wl_signal_emit_mutable(&output->events.update_late_usable_area, usable);
}

static void output_emit_usable_area(struct output *output)
{
    if (!output->state_usable) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Output %s usable area is (%d, %d) %d x %d", output->base.name,
             output->usable_area.x, output->usable_area.y, output->usable_area.width,
             output->usable_area.height);

    xwayland_update_workarea();

    wl_signal_emit_mutable(&output->events.usable_area, NULL);
}

void output_update_usable_area(struct kywc_output *kywc_output)
{
    /* no need to update usable area when disabled or destroyed */
    if (!kywc_output || kywc_output->destroying || !kywc_output->state.enabled) {
        return;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    struct kywc_box usable_area;

    output_do_update_usable_area(output, &usable_area);
    if (kywc_box_equal(&output->usable_area, &usable_area)) {
        return;
    }

    output->usable_area = usable_area;
    output_emit_usable_area(output);
}

void output_update_state(struct output *output)
{
    struct kywc_output *kywc_output = &output->base;
    /* update current state */
    struct kywc_output_state *current = &kywc_output->state;
    struct kywc_output_state old = kywc_output->state;
    output_get_state(output, current);

    // fix current.enabled for dpms power
    current->enabled = output->pending_state.enabled;
    if (!kywc_output->destroying) {
        output_log_state(KYWC_INFO, output, "current", current);
    }

    uint32_t changes = output_get_state_changes(output, &old);
    /* update geometry and usable area before all signals */
    bool geometry_changed = false;
    bool usable_area_changed = false;

    if (current->enabled) {
        struct kywc_box geo = output->geometry;
        output_state_get_geometry(&output->base.state, &output->geometry);
        geometry_changed = !kywc_box_equal(&geo, &output->geometry);
        /* only update usable area when geometry changed */
        if (!old.enabled || geometry_changed) {
            output_do_update_usable_area(output, &geo);
            if (!kywc_box_equal(&geo, &output->usable_area)) {
                output->usable_area = geo;
                usable_area_changed = true;
            }
        }
    }

    /* usable area may changed in scene_node_update_outputs */
    struct kywc_box usable_area = output->usable_area;

    struct server *server = output->manager->server;
    if (output->layout_output && !output->scene_output) {
        output->scene_output = scene_output_create(server->scene, output->wlr_output);
    } else if (!output->layout_output && output->scene_output) {
        scene_output_destroy(output->scene_output);
        output->scene_output = NULL;
    }
    assert(!!output->scene_output == !!output->layout_output);

    /* sort output manager outputs */
    if (geometry_changed || current->width != old.width || current->height != old.height ||
        current->transform != old.transform || current->scale != old.scale ||
        current->lx != old.lx || current->ly != old.ly) {
        output_manager_sort_outputs();
    }

    if (changes & (KYWC_OUTPUT_STATE_ENABLE | KYWC_OUTPUT_STATE_SCALE)) {
        output_manager_update_max_scale(kywc_output);
    }

    if (!kywc_output->destroying) {
        output_write_config(output);
    }

    struct output_configure_event event = {
        .output = output,
        .changes = changes,
        .geometry_changed = geometry_changed,
    };
    wl_signal_emit_mutable(&output->events.configure, &event);

    usable_area_changed |= !kywc_box_equal(&usable_area, &output->usable_area);
    /* used to sync signals */
    output->state_usable = current->enabled;

    /* check state changes */
    if (changes & KYWC_OUTPUT_STATE_ENABLE) {
        if (!current->enabled) {
            wl_signal_emit_mutable(&kywc_output->events.off, NULL);
            wl_signal_emit_mutable(&output->events.disable, NULL);
            return;
        } else {
            wl_signal_emit_mutable(&kywc_output->events.on, NULL);
            wl_signal_emit_mutable(&output_manager->events.new_enabled_output, kywc_output);
        }
    }

    if (geometry_changed) {
        kywc_log(KYWC_DEBUG, "Output %s geometry is (%d, %d) %d x %d", output->base.name,
                 output->geometry.x, output->geometry.y, output->geometry.width,
                 output->geometry.height);
        wl_signal_emit_mutable(&output->events.geometry, NULL);
    }

    if (usable_area_changed) {
        output_emit_usable_area(output);
    }

    if (changes & KYWC_OUTPUT_STATE_POWER) {
        wl_signal_emit_mutable(&kywc_output->events.power, NULL);
    }

    if (changes & KYWC_OUTPUT_STATE_BRIGHTNESS) {
        wl_signal_emit_mutable(&kywc_output->events.brightness, NULL);
    }

    if (changes & KYWC_OUTPUT_STATE_COLOR_TEMP) {
        wl_signal_emit_mutable(&kywc_output->events.color_temp, NULL);
    }

    if (changes & KYWC_OUTPUT_CAPABILITY_COLOR_FILTER) {
        wl_signal_emit_mutable(&kywc_output->events.color_filter, NULL);
    }

    if (changes & KYWC_OUTPUT_STATE_MODE) {
        wl_signal_emit_mutable(&kywc_output->events.mode, NULL);
    }

    if (changes & KYWC_OUTPUT_STATE_TRANSFORM) {
        output_add_transform_effect(kywc_output, &old, current);
        wl_signal_emit_mutable(&kywc_output->events.transform, NULL);
    }

    if (changes & KYWC_OUTPUT_STATE_SCALE) {
        wl_signal_emit_mutable(&kywc_output->events.scale, NULL);
    }

    if (changes & KYWC_OUTPUT_STATE_POSITION) {
        wl_signal_emit_mutable(&kywc_output->events.position, NULL);
    }
}

static void output_init_quirks(struct output *output)
{
    if (!wlr_output_is_drm(output->wlr_output)) {
        return;
    }

    int drm_fd = drm_backend_get_non_master_fd(output->wlr_output->backend);
    output->quirks = quirks_by_backend(drm_fd);

    /*  using software curosr, depending on the quirks mask */
    if (output->quirks & QUIRKS_MASK_SOFTWARE_CURSOR) {
        wlr_output_lock_software_cursors(output->wlr_output, true);
    }

    close(drm_fd);
}

static void output_uuid_generate(struct kywc_output *kywc_output)
{
    char description[128];
    snprintf(description, sizeof(description), "%s %s%s%s (%s)", kywc_output->prop.make,
             kywc_output->prop.model, kywc_output->prop.serial ? " " : "",
             kywc_output->prop.serial ? kywc_output->prop.serial : "", kywc_output->name);
    kywc_output->uuid = kywc_identifier_md5_generate_uuid((void *)description, strlen(description));
}

static void handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->present.link);
    wl_list_remove(&output->commit.link);
    wl_list_remove(&output->request_state.link);

    struct kywc_output *kywc_output = &output->base;
    wl_list_remove(&output->link);
    kywc_output->destroying = true;
    /* get configure outputs */
    output_manager->impl.get_configures(true);
    /* disable output if output is enabled */
    if (kywc_output->state.enabled) {
        output->pending_state.enabled = false;
        output_manager_add_pending_state(output, &output->pending_state);
    }
    output_manager_configure_outputs(output);
    wl_list_remove(&output->configure_link);

    wl_signal_emit_mutable(&output->events.disable, NULL);
    wl_signal_emit_mutable(&kywc_output->events.destroy, NULL);
    assert(wl_list_empty(&output->events.configure.listener_list));
    assert(wl_list_empty(&output->events.disable.listener_list));
    assert(wl_list_empty(&output->events.geometry.listener_list));
    assert(wl_list_empty(&output->events.usable_area.listener_list));
    assert(wl_list_empty(&output->events.update_usable_area.listener_list));
    assert(wl_list_empty(&output->events.update_late_usable_area.listener_list));
    assert(wl_list_empty(&kywc_output->events.on.listener_list));
    assert(wl_list_empty(&kywc_output->events.off.listener_list));
    assert(wl_list_empty(&kywc_output->events.scale.listener_list));
    assert(wl_list_empty(&kywc_output->events.transform.listener_list));
    assert(wl_list_empty(&kywc_output->events.mode.listener_list));
    assert(wl_list_empty(&kywc_output->events.position.listener_list));
    assert(wl_list_empty(&kywc_output->events.power.listener_list));
    assert(wl_list_empty(&kywc_output->events.brightness.listener_list));
    assert(wl_list_empty(&kywc_output->events.color_temp.listener_list));
    assert(wl_list_empty(&kywc_output->events.destroy.listener_list));

    struct kywc_output_mode *mode, *tmp_mode;
    wl_list_for_each_safe(mode, tmp_mode, &kywc_output->prop.modes, link) {
        wl_list_remove(&mode->link);
        free(mode);
    }

    free((void *)kywc_output->prop.edid);
    free((void *)kywc_output->prop.edid_hash);
    free((void *)kywc_output->uuid);
    free(output);
}

static void handle_output_present(struct wl_listener *listener, void *data)
{
    PROFILE_ZONE(zone, __func__);

    struct output *output = wl_container_of(listener, output, present);
    output->scene_commit = false;

    PROFILE_RENDER_COLLECT(output->wlr_output->renderer);

    if (output->has_pending) {
        output_manager_add_pending_state(output, &output->pending_state);
        /* has_pending is reset in handle_output_frame */
        if (!output_manager_configure_outputs(NULL)) {
            output->has_pending = false;
        }
    }

    PROFILE_ZONE_END(zone);
}

static void handle_output_frame(struct wl_listener *listener, void *data)
{
    if (!output_manager->server->active) {
        return;
    }

    struct output *output = wl_container_of(listener, output, frame);
    if (output_manager->primary_output == &output->base) {
        PROFILE_FRAME();
    }
    PROFILE_FRAME_NAME(output->wlr_output->name);
    PROFILE_ZONE(zone, __func__);

    /* skip rendering if has_pending, otherwise drm may report busy */
    if (output->has_pending) {
        output->wlr_output->frame_pending = true;
        output->has_pending = false;
        PROFILE_ZONE_END(zone);
        return;
    }

    output->scene_commit = scene_output_commit(output->scene_output, NULL);

    struct timespec now = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    scene_output_send_frame_done(output->scene_output, &now);

    PROFILE_ZONE_END(zone);
}

static void handle_output_commit(struct wl_listener *listener, void *data)
{
    PROFILE_ZONE(zone, __func__);

    struct output *output = wl_container_of(listener, output, commit);
    struct wlr_output_event_commit *event = data;
    bool has_damage = event->state->committed & WLR_OUTPUT_STATE_DAMAGE &&
                      pixman_region32_not_empty(&output->scene_output->frame_damage);
    if (has_damage) {
        wl_signal_emit_mutable(&output_manager->events.layout_damage,
                               &output->scene_output->frame_damage);
    }

    PROFILE_ZONE_END(zone);
}

static void handle_request_state(struct wl_listener *listener, void *data)
{
    struct output *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    const struct wlr_output_state *state = event->state;

    /* only support mode */
    if (!(state->committed & WLR_OUTPUT_STATE_MODE)) {
        return;
    }

    output->pending_state = output->base.state;
    if (state->mode) {
        output->pending_state.width = state->mode->width;
        output->pending_state.height = state->mode->height;
        output->pending_state.refresh = state->mode->refresh;
    } else {
        output->pending_state.width = state->custom_mode.width;
        output->pending_state.height = state->custom_mode.height;
        output->pending_state.refresh = state->custom_mode.refresh;
    }

    output_manager_add_pending_state(output, &output->pending_state);
    output_manager_configure_outputs(NULL);
}

static void handle_new_output(struct wl_listener *listener, void *data)
{
    struct server *server = output_manager->server;
    struct wlr_output *wlr_output = data;

    if (wlr_output->non_desktop) {
        kywc_log(KYWC_WARN, "Not configuring non-desktop output");
        return;
    }

    if (!wlr_output_init_render(wlr_output, server->allocator, server->renderer)) {
        kywc_log(KYWC_ERROR, "Unable to init output renderer");
        return;
    }

    struct output *output = calloc(1, sizeof(*output));
    if (!output) {
        return;
    }

    output->wlr_output = wlr_output;
    wlr_output->data = output;
    output->present.notify = handle_output_present;
    wl_signal_add(&wlr_output->events.present, &output->present);
    output->frame.notify = handle_output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->destroy.notify = handle_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);
    output->commit.notify = handle_output_commit;
    wl_signal_add(&wlr_output->events.commit, &output->commit);
    output->request_state.notify = handle_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    struct kywc_output *kywc_output = &output->base;
    kywc_output->name = wlr_output->name;

    wl_signal_init(&kywc_output->events.on);
    wl_signal_init(&kywc_output->events.off);
    wl_signal_init(&kywc_output->events.scale);
    wl_signal_init(&kywc_output->events.transform);
    wl_signal_init(&kywc_output->events.mode);
    wl_signal_init(&kywc_output->events.position);
    wl_signal_init(&kywc_output->events.power);
    wl_signal_init(&kywc_output->events.brightness);
    wl_signal_init(&kywc_output->events.color_temp);
    wl_signal_init(&kywc_output->events.color_filter);
    wl_signal_init(&kywc_output->events.destroy);

    wl_signal_init(&output->events.configure);
    wl_signal_init(&output->events.disable);
    wl_signal_init(&output->events.geometry);
    wl_signal_init(&output->events.usable_area);
    wl_signal_init(&output->events.update_usable_area);
    wl_signal_init(&output->events.update_late_usable_area);

    wl_list_init(&output->configure_link);
    output->manager = output_manager;
    wl_list_insert(&output_manager->outputs, &output->link);

    output->brightness = 100;
    output->color_temp = 6500;
    output_init_quirks(output);
    /* add modes for eDP/LVDS panel */
    output_add_custom_modes(output);

    /* get props */
    wl_list_init(&kywc_output->prop.modes);
    output_get_prop(output, &kywc_output->prop);
    output_log_prop(KYWC_DEBUG, output, "prop", &kywc_output->prop);

    if (kywc_output->prop.backend == KYWC_OUTPUT_BACKEND_HEADLESS &&
        strcmp(kywc_output->name, FALLBACK_OUTPUT) == 0) {
        output_manager->fallback_output = kywc_output;
    }

    output_get_state(output, &kywc_output->state);
    output_log_state(KYWC_DEBUG, output, "state", &kywc_output->state);

    output_uuid_generate(kywc_output);
    kywc_log(KYWC_INFO, "New output %s: %s", kywc_output->name, kywc_output->uuid);
    assert(!kywc_output->state.enabled);
    wl_signal_emit_mutable(&output_manager->events.new_output, kywc_output);

    /* configure the output */
    output_manager->impl.get_configures(false);
    output_manager_configure_outputs(NULL);
}

static void handle_server_start(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_INFO, "Configure outputs on server start");
    output_manager->impl.get_configures(false);
    output_manager_configure_outputs(NULL);
    assert(output_manager->primary_output);
    output_manager->initial_configured = true;
}

static void handle_server_suspend(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_DEBUG, "Handle server D-Bus suspend");
    // output_manager_power_outputs(false);
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct wlr_output *wlr_output = output->wlr_output;
        if (wlr_output->swapchain) {
            wlr_swapchain_destroy(wlr_output->swapchain);
            wlr_output->swapchain = NULL;
        }
    }
}

static void handle_server_hibernate(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_DEBUG, "Handle server D-Bus hibernate");
    output_manager_power_outputs(false);
}

static void handle_server_resume(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_DEBUG, "Handle server D-Bus resume");
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (!kywc_output->state.enabled) {
            continue;
        }

        if (kywc_output->state.power) {
            scene_output_damage_whole(output->scene_output);
        } else {
            output_set_power(kywc_output, true);
        }
    }
}

static void handle_backend_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&output_manager->backend_destroy.link);
    wl_list_remove(&output_manager->new_output.link);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&output_manager->server_destroy.link);
    wl_list_remove(&output_manager->server_suspend.link);
    wl_list_remove(&output_manager->server_hibernate.link);
    wl_list_remove(&output_manager->server_resume.link);
    wl_list_remove(&output_manager->server_start.link);
    wl_list_remove(&output_manager->configured.link);

    assert(wl_list_empty(&output_manager->events.new_output.listener_list));
    assert(wl_list_empty(&output_manager->events.new_enabled_output.listener_list));
    assert(wl_list_empty(&output_manager->events.primary_output.listener_list));
    assert(wl_list_empty(&output_manager->events.configured.listener_list));
    assert(wl_list_empty(&output_manager->events.layout_damage.listener_list));
    assert(wl_list_empty(&output_manager->events.max_scale.listener_list));

    free(output_manager);
    output_manager = NULL;
}

struct output_manager *output_manager_create(struct server *server)
{
    output_manager = calloc(1, sizeof(*output_manager));
    if (!output_manager) {
        return NULL;
    }

    output_manager->max_scale = 1.0;
    wl_list_init(&output_manager->outputs);
    wl_list_init(&output_manager->configure_outputs);
    wl_list_init(&output_manager->configured.link);
    wl_signal_init(&output_manager->events.new_output);
    wl_signal_init(&output_manager->events.new_enabled_output);
    wl_signal_init(&output_manager->events.primary_output);
    wl_signal_init(&output_manager->events.configured);
    wl_signal_init(&output_manager->events.layout_damage);
    wl_signal_init(&output_manager->events.max_scale);

    output_manager->server = server;
    output_manager->backend_destroy.notify = handle_backend_destroy;
    wl_signal_add(&server->backend->events.destroy, &output_manager->backend_destroy);
    output_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &output_manager->server_destroy);
    output_manager->server_suspend.notify = handle_server_suspend;
    wl_signal_add(&server->events.suspend, &output_manager->server_suspend);
    output_manager->server_hibernate.notify = handle_server_hibernate;
    wl_signal_add(&server->events.hibernate, &output_manager->server_hibernate);
    output_manager->server_resume.notify = handle_server_resume;
    wl_signal_add(&server->events.resume, &output_manager->server_resume);
    output_manager->server_start.notify = handle_server_start;
    wl_signal_add(&server->events.start, &output_manager->server_start);

    struct wlr_backend *headless = wlr_headless_backend_create(server->display);
    assert(headless);
    wlr_multi_backend_add(server->backend, headless);
    struct wlr_output *wlr_output = wlr_headless_add_output(headless, 1920, 1080);
    wlr_output_set_name(wlr_output, "FALLBACK");

    output_manager->new_output.notify = handle_new_output;
    wl_signal_add(&server->backend->events.new_output, &output_manager->new_output);

    char *env = getenv("KYWC_USE_LAYOUT_MANAGER");
    output_manager->has_layout_manager = env && strcmp(env, "1") == 0;
    output_manager_config_init(output_manager);
    ukui_output_config_init(output_manager);

    xdg_output_manager_v1_create(server);
    ky_output_manager_create(server);
    kde_output_management_create(server);
    wlr_output_management_create(server);
    ukui_output_management_create(server);

    return output_manager;
}
