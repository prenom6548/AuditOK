// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <string.h>

#include "effect_p.h"
#include "output.h"
#include "theme.h"
#include "util/color.h"
#include "util/macros.h"
#include "view/view.h"

#define WALLPAPER_ONESHOT_TIMEOUT (5000)

enum wallpaper_mode {
    WALLPAPER_MODE_ONESHOT = 0,
    WALLPAPER_MODE_PERSISTENT,
    WALLPAPER_MODE_WORKAROUND,
};

struct wallpaper_output {
    struct wl_list link;
    struct wallpaper_effect *effect;

    struct output *output;
    struct wl_listener geometry;
    struct wl_listener disable;
    struct wl_listener configure;

    struct scene_buffer *buffer;
    struct scene_rect *rect;

    struct wl_event_source *timer;
    bool raised;
};

struct wallpaper_effect {
    struct effect *effect;
    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    struct wl_list outputs;
    struct wl_listener new_enabled_output;
    struct wl_listener background_update;

    struct wl_event_loop *event_loop;
    enum wallpaper_mode mode;
    int timeout; // for workaround mode
};

static void wallpaper_output_destroy(struct wallpaper_output *output)
{
    wl_list_remove(&output->link);
    wl_list_remove(&output->geometry.link);
    wl_list_remove(&output->disable.link);
    wl_list_remove(&output->configure.link);

    if (output->buffer) {
        scene_node_destroy(&output->buffer->node);
    } else if (output->rect) {
        scene_node_destroy(&output->rect->node);
    }

    wl_event_source_remove(output->timer);
    free(output);
}

static void wallpaper_output_update_buffer(struct wallpaper_output *output)
{
    struct kywc_box *geo = &output->output->geometry;
    struct wlr_fbox dst = { geo->x, geo->y, geo->width, geo->height };
    struct wlr_fbox src = { 0 };

    int width, height;
    output_layout_get_size(&width, &height);
    bool repeated = theme_manager_get_background_box(&dst, &src, width, height);
    scene_buffer_set_repeat_mode(output->buffer, repeated ? RENDER_TEXTURE_REPEAT_NORAML
                                                          : RENDER_TEXTURE_REPEAT_NONE);

    scene_buffer_set_source_box(output->buffer, &src);
    scene_node_set_position(&output->buffer->node, dst.x, dst.y);
    scene_buffer_set_dest_size(output->buffer, dst.width, dst.height);

    pixman_region32_t region;
    pixman_region32_init_rect(&region, 0, 0, dst.width, dst.height);
    scene_buffer_set_opaque_region(output->buffer, &region);
    pixman_region32_fini(&region);
}

static void wallpaper_output_update_rect(struct wallpaper_output *output)
{
    struct kywc_box *geo = &output->output->geometry;
    scene_node_set_position(&output->rect->node, geo->x, geo->y);
    scene_rect_set_size(output->rect, geo->width, geo->height);
}

static void output_handle_geometry(struct wl_listener *listener, void *data)
{
    struct wallpaper_output *output = wl_container_of(listener, output, geometry);

    if (output->buffer) {
        wallpaper_output_update_buffer(output);
    } else if (output->rect) {
        wallpaper_output_update_rect(output);
    }
}

static void output_handle_disable(struct wl_listener *listener, void *data)
{
    struct wallpaper_output *output = wl_container_of(listener, output, disable);
    struct wallpaper_effect *effect = output->effect;

    wallpaper_output_destroy(output);

    // update all outputs if BACKGROUND_OPTION_SPANNED
    wl_list_for_each(output, &effect->outputs, link) {
        if (output->buffer) {
            wallpaper_output_update_buffer(output);
        }
    }
}

static struct scene_tree *wallpaper_output_get_tree(struct wallpaper_output *output)
{
    enum layer layer = output->raised ? LAYER_ON_SCREEN_DISPLAY : LAYER_DESKTOP;
    struct view_layer *view_layer = view_manager_get_layer(layer, false);
    return view_layer->tree;
}

static void wallpaper_output_set_enabled(struct wallpaper_output *output, bool enabled)
{
    if (output->buffer) {
        scene_node_set_enabled(&output->buffer->node, enabled);
    } else if (output->rect) {
        scene_node_set_enabled(&output->rect->node, enabled);
    }
}

static void wallpaper_output_raise_node(struct wallpaper_output *output, bool raise)
{
    output->raised = raise;

    struct scene_node *node = NULL;
    if (output->buffer) {
        node = &output->buffer->node;
    } else if (output->rect) {
        node = &output->rect->node;
    } else {
        return;
    }

    struct scene_tree *tree = wallpaper_output_get_tree(output);
    scene_node_reparent(node, tree);
    if (raise) {
        scene_node_raise_to_top(node);
    } else {
        scene_node_lower_to_bottom(node);
    }
}

static int output_handle_timeout(void *data)
{
    struct wallpaper_output *output = data;
    wallpaper_output_set_enabled(output, false);
    wallpaper_output_raise_node(output, false);
    return 0;
}

static void output_handle_configure(struct wl_listener *listener, void *data)
{
    struct wallpaper_output *output = wl_container_of(listener, output, configure);
    struct output_configure_event *event = data;
    /* skip if output is disabled */
    if (!event->output->base.state.enabled) {
        return;
    }
    /* we have a transform effect */
    if (event->changes & KYWC_OUTPUT_STATE_TRANSFORM) {
        return;
    }
    /* skip if geometry is not changed */
    uint32_t changes = event->changes & (KYWC_OUTPUT_STATE_POSITION | KYWC_OUTPUT_STATE_SCALE |
                                         KYWC_OUTPUT_STATE_MODE);
    if (changes == 0 || !event->geometry_changed) {
        return;
    }

    wallpaper_output_raise_node(output, true);
    wallpaper_output_set_enabled(output, true);
    wl_event_source_timer_update(output->timer, output->effect->timeout);
}

static void wallpaper_output_update_node(struct wallpaper_output *output, struct wlr_buffer *buffer,
                                         int32_t color)
{
    if (buffer) {
        if (output->rect) {
            scene_node_destroy(&output->rect->node);
            output->rect = NULL;
        }
        if (!output->buffer) {
            struct scene_tree *tree = wallpaper_output_get_tree(output);
            output->buffer = scene_buffer_create(tree, buffer);
            scene_node_set_input_bypassed(&output->buffer->node, true);
            scene_node_set_role(&output->buffer->node, SCENE_ROLE_WALLPAPER, NULL, output);
        } else {
            scene_buffer_set_buffer(output->buffer, buffer);
        }
    } else if (color >= 0) {
        if (output->buffer) {
            scene_node_destroy(&output->buffer->node);
            output->buffer = NULL;
        }
        float solid[4];
        color_uint24_to_float(solid, color);
        if (!output->rect) {
            struct scene_tree *tree = wallpaper_output_get_tree(output);
            output->rect = scene_rect_create(tree, 1, 1, solid);
            scene_node_set_input_bypassed(&output->rect->node, true);
            scene_node_set_role(&output->rect->node, SCENE_ROLE_WALLPAPER, NULL, output);
        } else {
            scene_rect_set_color(output->rect, solid);
        }
    } else if (output->rect) {
        scene_node_destroy(&output->rect->node);
        output->rect = NULL;
    } else if (output->buffer) {
        scene_node_destroy(&output->buffer->node);
        output->buffer = NULL;
    }

    wallpaper_output_raise_node(output, output->raised);
}

static void wallpaper_output_apply_mode(struct wallpaper_output *output)
{
    struct wallpaper_effect *effect = output->effect;

    wl_list_remove(&output->configure.link);
    wl_list_init(&output->configure.link);
    wl_event_source_timer_update(output->timer, 0);
    wallpaper_output_raise_node(output, false);

    if (effect->mode == WALLPAPER_MODE_ONESHOT) {
        wallpaper_output_set_enabled(output, true);
        wl_event_source_timer_update(output->timer, WALLPAPER_ONESHOT_TIMEOUT);
    } else if (effect->mode == WALLPAPER_MODE_WORKAROUND) {
        wallpaper_output_set_enabled(output, false);
        wl_signal_add(&output->output->events.configure, &output->configure);
    } else if (effect->mode == WALLPAPER_MODE_PERSISTENT) {
        wallpaper_output_set_enabled(output, true);
    }
}

static void wallpaper_output_create(struct wallpaper_effect *effect,
                                    struct kywc_output *kywc_output)
{
    struct wallpaper_output *output = calloc(1, sizeof(*output));
    if (!output) {
        return;
    }

    output->timer = wl_event_loop_add_timer(effect->event_loop, output_handle_timeout, output);
    if (!output->timer) {
        free(output);
        return;
    }

    output->effect = effect;
    wl_list_insert(&effect->outputs, &output->link);

    output->output = output_from_kywc_output(kywc_output);
    output->geometry.notify = output_handle_geometry;
    wl_signal_add(&output->output->events.geometry, &output->geometry);
    output->disable.notify = output_handle_disable;
    wl_signal_add(&output->output->events.disable, &output->disable);
    output->configure.notify = output_handle_configure;
    wl_list_init(&output->configure.link);

    int32_t color = -1;
    struct wlr_buffer *buffer = theme_manager_get_background(&color);
    wallpaper_output_update_node(output, buffer, color);
    if (output->buffer) {
        // update all outputs if BACKGROUND_OPTION_SPANNED
        struct wallpaper_output *woutput;
        wl_list_for_each(woutput, &effect->outputs, link) {
            wallpaper_output_update_buffer(woutput);
        }
    } else if (output->rect) {
        wallpaper_output_update_rect(output);
    }

    wallpaper_output_apply_mode(output);
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct wallpaper_effect *effect = wl_container_of(listener, effect, new_enabled_output);
    struct kywc_output *output = data;
    wallpaper_output_create(effect, output);
}

static void handle_background_update(struct wl_listener *listener, void *data)
{
    int32_t color = -1;
    struct wlr_buffer *buffer = theme_manager_get_background(&color);
    struct wallpaper_effect *effect = wl_container_of(listener, effect, background_update);
    struct wallpaper_output *output;
    wl_list_for_each(output, &effect->outputs, link) {
        wallpaper_output_update_node(output, buffer, color);
        if (output->buffer) {
            wallpaper_output_update_buffer(output);
        } else if (output->rect) {
            wallpaper_output_update_rect(output);
        }
    }
}

static bool create_request(struct kywc_output *kywc_output, int index, void *data)
{
    struct wallpaper_effect *effect = data;
    wallpaper_output_create(effect, kywc_output);
    return false;
}

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    struct wallpaper_effect *effect = wl_container_of(listener, effect, enable);
    /* get current enabled output */
    output_manager_for_each_output(create_request, true, effect);

    output_manager_add_new_enabled_listener(&effect->new_enabled_output);
    theme_manager_add_background_update_listener(&effect->background_update);
}

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    struct wallpaper_effect *effect = wl_container_of(listener, effect, disable);
    wl_list_remove(&effect->new_enabled_output.link);
    wl_list_init(&effect->new_enabled_output.link);
    wl_list_remove(&effect->background_update.link);
    wl_list_init(&effect->background_update.link);

    struct wallpaper_output *output, *tmp;
    wl_list_for_each_safe(output, tmp, &effect->outputs, link) {
        wallpaper_output_destroy(output);
    }
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct wallpaper_effect *effect = wl_container_of(listener, effect, destroy);
    wl_list_remove(&effect->new_enabled_output.link);
    wl_list_remove(&effect->background_update.link);
    wl_list_remove(&effect->destroy.link);
    wl_list_remove(&effect->enable.link);
    wl_list_remove(&effect->disable.link);
    free(effect);
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    struct wallpaper_effect *wallpaper = effect->user_data;

    if (strcmp(option->key, "mode") == 0) {
        enum wallpaper_mode mode =
            CLAMP(option->value.num, WALLPAPER_MODE_ONESHOT, WALLPAPER_MODE_WORKAROUND);
        if (wallpaper->mode != mode) {
            wallpaper->mode = mode;
            struct wallpaper_output *output;
            wl_list_for_each(output, &wallpaper->outputs, link) {
                wallpaper_output_apply_mode(output);
            }
        }
        return true;
    }

    if (strcmp(option->key, "timeout") == 0) {
        wallpaper->timeout = MAX(option->value.num, 200);
        return true;
    }

    return false;
}

static const struct effect_interface wallpaper_effect_impl = {
    .configure = handle_effect_configure,
};

bool wallpaper_effect_create(struct effect_manager *manager)
{
    struct wallpaper_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return false;
    }

    effect->effect = effect_create("wallpaper", 0, false, &wallpaper_effect_impl, effect);
    if (!effect->effect) {
        free(effect);
        return false;
    }

    effect->event_loop = manager->server->event_loop;
    effect->mode = CLAMP(effect_get_option_int(effect->effect, "mode", WALLPAPER_MODE_ONESHOT),
                         WALLPAPER_MODE_ONESHOT, WALLPAPER_MODE_WORKAROUND);
    effect->timeout = MAX(effect_get_option_int(effect->effect, "timeout", 500), 200);

    effect->enable.notify = handle_effect_enable;
    wl_signal_add(&effect->effect->events.enable, &effect->enable);
    effect->disable.notify = handle_effect_disable;
    wl_signal_add(&effect->effect->events.disable, &effect->disable);
    effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&effect->effect->events.destroy, &effect->destroy);

    wl_list_init(&effect->outputs);
    effect->new_enabled_output.notify = handle_new_enabled_output;
    wl_list_init(&effect->new_enabled_output.link);

    effect->background_update.notify = handle_background_update;
    wl_list_init(&effect->background_update.link);

    if (effect->effect->enabled) {
        handle_effect_enable(&effect->enable, NULL);
    }

    return true;
}
