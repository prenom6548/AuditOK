// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/render/swapchain.h>

#include "backend/drm.h"
#include "effect/effect.h"
#include "output_p.h"
#include "render/pass.h"
#include "theme.h"
#include "util/macros.h"

#define COLORTEMP_CLAMP(val) ((val) < 1000 ? 6500 : ((val) > 25000 ? 25000 : (val)))
#define BRIGHTNESS_CLAMP(val) ((val) > 100 ? 100 : (val))

static struct kywc_output_mode *output_preferred_mode(struct kywc_output *kywc_output)
{
    struct kywc_output_mode *mode;
    wl_list_for_each(mode, &kywc_output->prop.modes, link) {
        if (mode->preferred) {
            return mode;
        }
    }
    // No preferred mode, choose the first one
    return wl_container_of(kywc_output->prop.modes.prev, mode, link);
}

static float get_screen_factor(float phys_size_inch)
{
    if (phys_size_inch < 12.0) { // Tablets (screnn_size < 12 inches)
        return 20.0;
    } else if (phys_size_inch < 20.0) { // Laptop internal displays (12 ≤ screen_size < 20 inches)
        return 24.5;
    } else {
        return 28.0; // External monitors (screen_size ≥ 20 inches)
    }
}

static float get_scale_level(float scale_factor, int width, int height)
{
    if (scale_factor < 1.2) {
        return 1.0;
    } else if (scale_factor < 1.43) {
        return 1.25;
    } else if (scale_factor < 1.78) {
        return (height <= 1200 || width <= 1920) ? 1.25 : 1.5;
    } else {
        return 2.0;
    }
}

float output_preferred_scale(struct kywc_output *kywc_output, int width, int height)
{
    float phys_width = kywc_output->prop.phys_width;
    float phys_height = kywc_output->prop.phys_height;

    if (phys_width == 0 || phys_height == 0) {
        return width >= 2160 ? 1.25 : (width >= 2000 ? 1.25 : 1.0);
    }

    float phys_inch = sqrtf(pow(phys_width, 2) + pow(phys_height, 2)) / 25.4;
    float factor = get_screen_factor(phys_inch);
    float ppi = sqrtf(pow(width, 2) + pow(height, 2)) / phys_inch;

    float scale_factor = ppi * factor / (96.0 * 28.0);
    double scale = get_scale_level(scale_factor, width, height);

    kywc_log(KYWC_DEBUG, "Output %s(%d x %d %f inch %f ppi) scale = %f", kywc_output->name, width,
             height, phys_inch, ppi, scale_factor);

    return scale;
}

void output_fill_preferred_state(struct output *output)
{
    struct kywc_output *kywc_output = &output->base;
    struct kywc_output_state *pending = &output->pending_state;
    *pending = output->base.state;

    pending->enabled = pending->power = true;
    /* use the preferred mode */
    struct kywc_output_mode *mode = output_preferred_mode(kywc_output);
    pending->width = mode->width;
    pending->height = mode->height;
    pending->refresh = mode->refresh;
    /* use the preferred scale */
    pending->scale = output_preferred_scale(kywc_output, pending->width, pending->height);

    /* fallback output position is (0, 0) */
    if (output_manager->fallback_output == kywc_output) {
        pending->lx = pending->ly = 0;
    } else { /* use auto position from layout */
        pending->lx = pending->ly = -1;
    }

    pending->transform = WL_OUTPUT_TRANSFORM_NORMAL;
    pending->brightness = pending->brightness == 0 ? 100 : CLAMP(pending->brightness, 10, 100);
    pending->color_temp = pending->color_temp == 0 ? 6500 : CLAMP(pending->color_temp, 1000, 25000);
}

static bool fallback_output_fill_state(bool enabled, struct output *mirror)
{
    struct kywc_output *kywc_output = output_manager->fallback_output;
    /* no fallback output or still be disabled */
    if (!kywc_output || (!kywc_output->state.enabled && !enabled)) {
        return false;
    }

    /* early return if disable the fallback output */
    struct output *output = output_from_kywc_output(kywc_output);
    if (!enabled) {
        output->pending_state.enabled = false;
        return true;
    }

    if (!mirror) {
        output_fill_preferred_state(output);
        return true;
    }

    struct kywc_output_state *state = &output->pending_state;
    *state = mirror->base.state;
    /* fixup the state */
    state->enabled = state->power = true;
    state->lx = state->ly = 0;
    /* try to add this mode to fallback output */
    struct wlr_output_mode mode = {
        .width = state->width,
        .height = state->height,
        .refresh = state->refresh,
    };
    output_add_mode(output, &mode, true);

    return true;
}

static void output_set_primary(struct kywc_output *kywc_output)
{
    assert(kywc_output);
    if (output_manager->primary_output == kywc_output) {
        return;
    }

    output_manager->primary_output = kywc_output;
    kywc_log(KYWC_INFO, "Primary output is changed to %s", kywc_output->name);
    wl_signal_emit_mutable(&output_manager->events.primary_output, kywc_output);
}

void output_manager_set_pending_primary(struct output *output)
{
    struct kywc_output *kywc_output = &output->base;
    output_manager->pending_primary = kywc_output;
}

void output_manager_add_pending_state(struct output *output, struct kywc_output_state *state)
{
    if (state != &output->pending_state) {
        output->pending_state = *state;
    }

    wl_list_remove(&output->configure_link);
    wl_list_insert(&output_manager->configure_outputs, &output->configure_link);

    if (!output->base.destroying) {
        output_log_state(KYWC_INFO, output, "pending", state);
    }
}

static void clear_pending_configures(void)
{
    struct output *output, *tmp;
    wl_list_for_each_safe(output, tmp, &output_manager->configure_outputs, configure_link) {
        wl_list_remove(&output->configure_link);
        wl_list_init(&output->configure_link);
    }
    assert(wl_list_empty(&output_manager->configure_outputs));

    output_manager->pending_primary = NULL;
}

static void output_manager_emit_configured(enum configure_type type)
{
    struct configure_event event = { .type = type };
    wl_signal_emit_mutable(&output_manager->events.configured, &event);
}

static void output_find_best_mode(struct wlr_output *wlr_output, int32_t width, int32_t height,
                                  int32_t refresh, struct wlr_output_mode **best)
{
    struct wlr_output_mode *m;
    wl_list_for_each(m, &wlr_output->modes, link) {
        if (width != m->width || height != m->height) {
            continue;
        }
        if (refresh == m->refresh) {
            *best = m;
            break;
        }
        if (!*best || m->refresh > (*best)->refresh) {
            *best = m;
        }
    }
}

static void output_state_set_mode(struct output *output, struct wlr_output_state *wlr_state)
{
    struct kywc_output_state *state = &output->pending_state;
    struct wlr_output *wlr_output = output->wlr_output;

    struct wlr_output_mode *best = NULL;
    if (state->width <= 0 || state->height <= 0) {
        kywc_log(KYWC_INFO, "Set preferred mode as no config found");
        best = wlr_output_preferred_mode(wlr_output);
    } else {
        output_find_best_mode(wlr_output, state->width, state->height, state->refresh, &best);
    }

    if (best) {
        wlr_output_state_set_mode(wlr_state, best);
    } else {
        wlr_output_state_set_custom_mode(wlr_state, state->width, state->height, state->refresh);
    }

    if (wlr_output_test_state(wlr_output, wlr_state)) {
        return;
    }

    kywc_log(KYWC_ERROR, "Mode rejected, falling back to another mode");
    struct wlr_output_mode *mode;
    wl_list_for_each(mode, &wlr_output->modes, link) {
        if (best && best == mode) {
            continue;
        }

        wlr_output_state_set_mode(wlr_state, mode);
        if (wlr_output_test_state(wlr_output, wlr_state)) {
            break;
        }
    }
}

static void output_acquire_buffer(struct output *output, struct wlr_output_state *wlr_state)
{
    struct wlr_output *wlr_output = output->wlr_output;
    struct wlr_buffer *buffer = wlr_swapchain_acquire(wlr_output->swapchain, NULL);
    if (!buffer) {
        return;
    }

    char *env = getenv("KYWC_OUTPUT_FILL_WALLPAPER");
    bool filled_with_wallpaper = env && strcmp(env, "1") == 0;
    int32_t color = 0;
    struct wlr_buffer *image = NULL;

    if (filled_with_wallpaper) {
        image = theme_manager_get_background(&color);
        if (!image && color < 0) {
            color = 0; // fallback to empty
        } else {
            struct effect *effect = effect_by_name("wallpaper");
            if (effect) {
                struct effect_option option = { .key = "mode", .value.num = 0 };
                effect_set_option(effect, EFFECT_OPTION_TYPE_INT, &option);
                effect_set_enabled(effect, true);
            }
        }
    }

    struct wlr_render_pass *pass =
        wlr_renderer_begin_buffer_pass(wlr_output->renderer, buffer, NULL);
    if (!pass) {
        wlr_buffer_unlock(buffer);
        return;
    }

    if (image) {
        struct wlr_texture *tex = wlr_texture_from_buffer(wlr_output->renderer, image);
        render_pass_add_texture(pass, &(struct render_texture_options){
                                          .base = {
                                              .dst_box = { 0, 0, wlr_output->swapchain->width,
                                                           wlr_output->swapchain->height },
                                              .texture = tex,
                                              .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                          } });
        wlr_texture_destroy(tex);
    } else {
        render_pass_add_rect(
            pass, &(struct render_rect_options){
                      .base = {
                          .color = { ((color >> 16) & 0xff) / 255.0, ((color >> 8) & 0xff) / 255.0,
                                     (color & 0xff) / 255.0, 1.0 },
                          .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                      } });
    }

    render_pass_submit(pass, output->quirks);
    wlr_output_state_set_buffer(wlr_state, buffer);
    wlr_buffer_unlock(buffer);
}

static bool output_set_state(struct output *output)
{
    struct wlr_output *wlr_output = output->wlr_output;
    struct kywc_output_state *state = &output->pending_state;

    if (output->base.destroying) {
        output_set_layout(output);
        output_update_state(output);
        return true;
    }

    uint32_t changes = output_get_state_changes(output, state);
    bool state_changed = (changes & (KYWC_OUTPUT_STATE_ENABLE | KYWC_OUTPUT_STATE_POWER)) ||
                         !output->manager->initial_configured;
    bool mode_changed = changes & KYWC_OUTPUT_STATE_MODE;
    bool params_changed = changes & (KYWC_OUTPUT_STATE_TRANSFORM | KYWC_OUTPUT_STATE_VRR_POLICY |
                                     KYWC_OUTPUT_STATE_SCALE | KYWC_OUTPUT_STATE_RGB_RANGE |
                                     KYWC_OUTPUT_STATE_OVERSCAN | KYWC_OUTPUT_STATE_SCALING_MODE);
    bool enabled = state->enabled && state->power;

    if (state_changed || mode_changed || params_changed) {
        struct wlr_output_state wlr_state;
        wlr_output_state_init(&wlr_state);
        if (state_changed) {
            wlr_output_state_set_enabled(&wlr_state, enabled);
        }

        if (enabled) {
            struct wlr_swapchain *old_swapchain = wlr_output->swapchain;

            if (mode_changed) {
                output_state_set_mode(output, &wlr_state);
            } else if (output->wlr_output->enabled && wlr_output_is_drm(wlr_output) &&
                       output->wlr_output->width == state->width &&
                       output->wlr_output->height == state->height && output->scene_commit) {
                output->has_pending = true;
                kywc_log(KYWC_WARN, "Drm output commit need waiting for pageflip");
                return true;
            }

            wlr_output_configure_primary_swapchain(wlr_output, &wlr_state, &wlr_output->swapchain);
            if (old_swapchain != wlr_output->swapchain && wlr_output->swapchain) {
                output_acquire_buffer(output, &wlr_state);
            }

            wlr_output_set_color_changed(wlr_output, enabled);
            wlr_output_state_set_transform(&wlr_state, state->transform);
            wlr_output_state_set_scale(&wlr_state, state->scale);
            wlr_output_set_rgb_range(wlr_output, state->rgb_range);
            wlr_output_set_overscan(wlr_output, state->overscan);
            wlr_output_set_scaling_mode(wlr_output, state->scaling_mode);
        }

        if (output->scene_commit) {
            output->has_pending = false;
        }
        if (!wlr_output_commit_state(wlr_output, &wlr_state)) {
            kywc_log_ratelimited(KYWC_ERROR, "Failed to commit output: %s", wlr_output->name);
            wlr_output_state_finish(&wlr_state);
            return false;
        }
        wlr_output_state_finish(&wlr_state);
    }

    bool color_feature_changed =
        changes & (KYWC_OUTPUT_STATE_POWER | KYWC_OUTPUT_STATE_COLOR_FEATURE);
    bool color_changed = color_feature_changed ||
                         changes & (KYWC_OUTPUT_STATE_POWER | KYWC_OUTPUT_STATE_COLOR_TEMP |
                                    KYWC_OUTPUT_STATE_BRIGHTNESS | KYWC_OUTPUT_STATE_COLOR_FILTER);
    if (enabled && color_changed) {
        wlr_output_set_color_changed(wlr_output, color_changed);
        bool hardware_color = wlr_output_is_hardware_color_allowed(output->wlr_output, state);
        if (color_feature_changed || output->hardware_color != hardware_color || !hardware_color) {
            if (output->scene_output) {
                scene_output_damage_whole(output->scene_output);
            }
        } else {
            output_schedule_frame(output->wlr_output);
        }
        output->hardware_color = hardware_color;
    }

    output->color_temp = state->color_temp;
    output->brightness = state->brightness;
    output->color_feature = state->color_feature;
    output->color_filter = state->color_filter;
    output->vrr_policy = state->vrr_policy;

    output_set_layout(output);
    output_update_state(output);

    return true;
}

static inline bool output_is_actual_valid(struct kywc_output *output)
{
    return output && !output->destroying && output->state.enabled &&
           output != output_manager->fallback_output;
}

bool output_manager_configure_outputs(struct output *destroying_output)
{
    if (wl_list_empty(&output_manager->configure_outputs)) {
        return true;
    }

    if (output_manager->server->terminate) {
        if (destroying_output && destroying_output->base.state.enabled) {
            output_set_state(destroying_output);
        }
        clear_pending_configures();
        return false;
    }

    /* 1. check configs */
    if (!output_manager->pending_primary &&
        output_is_actual_valid(output_manager->primary_output)) {
        output_manager->pending_primary = output_manager->primary_output;
        kywc_log(KYWC_DEBUG, "Keep primary output to %s", output_manager->primary_output->name);
    }

    struct kywc_output *kywc_output = NULL;
    struct output *output = NULL, *pending_output = NULL;

    /* primary output may be disabled, fixup it */
    if (output_manager->pending_primary) {
        output = output_from_kywc_output(output_manager->pending_primary);
        bool has_configure = !wl_list_empty(&output->configure_link);
        if ((!has_configure && !output->base.state.enabled) ||
            (has_configure && !output->pending_state.enabled)) {
            output_manager->pending_primary = NULL;
        }
    }

    /* if all outputs will be disabled in config, find others not in config */
    bool have_enabled_output = false, have_zero_coord = false;
    bool have_actual_output = output_manager_has_actual_outputs();

    wl_list_for_each(output, &output_manager->outputs, link) {
        kywc_output = &output->base;
        /* skip fallback output if has actual output */
        if (have_actual_output && kywc_output == output_manager->fallback_output) {
            continue;
        }

        if (!wl_list_empty(&output->configure_link)) {
            pending_output = output;
            have_enabled_output |= output->pending_state.enabled;
            have_zero_coord |= output->pending_state.enabled && output->pending_state.lx == 0 &&
                               output->pending_state.ly == 0;
        } else {
            have_enabled_output |= kywc_output->state.enabled;
            have_zero_coord |= kywc_output->state.enabled && kywc_output->state.lx == 0 &&
                               kywc_output->state.ly == 0;
        }

        /* don't set primary to fallback output if has actaul enabled output */
        if (have_actual_output && have_enabled_output &&
            output_manager->pending_primary == output_manager->fallback_output) {
            output_manager->pending_primary = NULL;
        }

        /* fixup primary output to the (pending)enabled output */
        if (have_enabled_output && !output_manager->pending_primary) {
            kywc_log(KYWC_DEBUG, "Fixup primary output to %s", output->base.name);
            output_manager->pending_primary = kywc_output;
        }

        if (have_enabled_output && have_zero_coord) {
            break;
        }
    }

    /* have one actual output and coord is not zero */
    if (!have_zero_coord && pending_output) {
        int output_num = wl_list_length(&output_manager->outputs);
        if (output_num == 1 || (output_num == 2 && output_manager->fallback_output)) {
            kywc_log(KYWC_DEBUG, "Fixup output %s coord to zero", pending_output->base.name);
            pending_output->pending_state.lx = pending_output->pending_state.ly = 0;
        }
    }

    bool ret = false;

    /* don't reject when the only one output plug-out */
    if (have_actual_output && !have_enabled_output) {
        kywc_log(KYWC_WARN, "All outputs will be disabled, reject this configuration");
        goto finish;
    }

    /* 2. config outputs */
    /* call kywc_output_set_state in all pending outputs, enable first and disable outputs later
     */
    wl_list_for_each(output, &output_manager->configure_outputs, configure_link) {
        if (!output->pending_state.enabled) {
            continue;
        }
        if (!output->base.state.enabled) {
            continue;
        }
        if (!output_set_state(output)) {
            goto finish;
        }
    }

    wl_list_for_each(output, &output_manager->configure_outputs, configure_link) {
        if (!output->pending_state.enabled) {
            continue;
        }
        if (output->base.state.enabled) {
            continue;
        }
        if (have_actual_output && output_manager->fallback_output == &output->base) {
            kywc_log(KYWC_INFO, "Reject enable fallback output if have actual output");
            continue;
        }
        if (!output_set_state(output)) {
            goto finish;
        }
    }

    /* should enable fallback output if still no primary output */
    if (!output_manager->pending_primary && fallback_output_fill_state(true, destroying_output)) {
        output_set_state(output_from_kywc_output(output_manager->fallback_output));
        output_manager->pending_primary = output_manager->fallback_output;
        kywc_log(KYWC_DEBUG, "Auto enable fallback output");
    }

    /* switch the primary before output disable */
    output_set_primary(output_manager->pending_primary);

    wl_list_for_each(output, &output_manager->configure_outputs, configure_link) {
        if (output->pending_state.enabled) {
            continue;
        }
        if (!output->base.state.enabled && output->manager->initial_configured) {
            continue;
        }
        if (!output_set_state(output)) {
            goto finish;
        }
    }

    ret = true;

finish:
    /* make sure primary output is configured even if above set_state failed */
    if (!output_manager->pending_primary || !output_manager->pending_primary->state.enabled) {
        output_manager->pending_primary = NULL;
        if (output_is_actual_valid(output_manager->primary_output)) {
            output_manager->pending_primary = output_manager->primary_output;
        } else {
            wl_list_for_each(output, &output_manager->outputs, link) {
                kywc_output = &output->base;
                if (output_is_actual_valid(kywc_output)) {
                    output_manager->pending_primary = kywc_output;
                    break;
                }
            }
        }

        if (!output_manager->pending_primary &&
            fallback_output_fill_state(true, destroying_output)) {
            output_set_state(output_from_kywc_output(output_manager->fallback_output));
            output_manager->pending_primary = output_manager->fallback_output;
            kywc_log(KYWC_DEBUG, "Auto enable fallback output");
        }
    }

    output_set_primary(output_manager->pending_primary);

    /* disable the fallback when an actual enabled output exists */
    if (output_manager->primary_output != output_manager->fallback_output &&
        fallback_output_fill_state(false, NULL)) {
        output_set_state(output_from_kywc_output(output_manager->fallback_output));
        kywc_log(KYWC_DEBUG, "Auto disable fallback output");
    }

    /* make sure destroying output is disabled even if above set_state failed */
    if (destroying_output && destroying_output->base.state.enabled) {
        output_set_state(destroying_output);
    }

    output_manager_emit_configured(output_manager->initial_configured ? CONFIGURE_TYPE_UPDATE
                                                                      : CONFIGURE_TYPE_INIT);
    clear_pending_configures();
    return ret;
}

bool output_set_color_feature(struct kywc_output *kywc_output,
                              enum kywc_output_color_feature color_feature)
{
    if (!kywc_output->state.enabled) {
        return false;
    }

    if (kywc_output->state.color_feature == color_feature) {
        return true;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    output->pending_state = output->base.state;
    output->pending_state.color_feature = color_feature;
    output_set_state(output);
    return true;
}

bool output_set_color_filter(struct kywc_output *kywc_output,
                             enum kywc_output_color_filter color_filter)
{
    if (!kywc_output->state.enabled) {
        return false;
    }

    if (kywc_output->state.color_filter == color_filter) {
        return true;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    output->pending_state = output->base.state;
    output->pending_state.color_filter = color_filter;
    output_set_state(output);
    return true;
}

bool output_set_color_temp(struct kywc_output *kywc_output, uint32_t color_temp)
{
    if (!kywc_output->state.enabled) {
        return false;
    }

    color_temp = COLORTEMP_CLAMP(color_temp);
    if (kywc_output->state.color_temp == color_temp) {
        return true;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    output->pending_state = output->base.state;
    output->pending_state.color_temp = color_temp;
    output_set_state(output);
    return true;
}

bool output_set_brightness(struct kywc_output *kywc_output, uint32_t brightness)
{
    if (!kywc_output->state.enabled) {
        return false;
    }

    brightness = BRIGHTNESS_CLAMP(brightness);
    if (kywc_output->state.brightness == brightness) {
        return true;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    output->pending_state = output->base.state;
    output->pending_state.brightness = brightness;
    output_set_state(output);
    return true;
}

bool output_set_power(struct kywc_output *kywc_output, bool power)
{
    if (!kywc_output->state.enabled) {
        return false;
    }
    if (kywc_output->state.power == power) {
        return true;
    }

    struct output *output = output_from_kywc_output(kywc_output);
    output->pending_state = output->base.state;
    output->pending_state.power = power;
    output_set_state(output);
    output_manager_emit_configured(CONFIGURE_TYPE_NONE);
    return true;
}

void output_manager_power_outputs(bool power)
{
    bool have_powered_output = false;

    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        if (!output->base.state.enabled || output->base.state.power == power) {
            continue;
        }
        output->pending_state = output->base.state;
        output->pending_state.power = power;
        output_set_state(output);
        have_powered_output = true;
    }

    if (have_powered_output) {
        output_manager_emit_configured(CONFIGURE_TYPE_NONE);
    }
}
