// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdint.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>

#include <kywc/log.h>

#include "effect/action.h"
#include "effect/animator.h"
#include "effect_p.h"
#include "server.h"
#include "ukui-effect-v1-protocol.h"
#include "view/view.h"

#define UKUI_FLOAT_PRECISION 100.0f

enum ukui_effect_stage {
    UKUI_EFFECT_STAGE_UNKNOW = 0,
    UKUI_EFFECT_STAGE_MAP = 1 << 0,
    UKUI_EFFECT_STAGE_UNMAP = 1 << 1,
    UKUI_EFFECT_STAGE_ALL = (1 << 2) - 1,
};

enum effect_options_mask {
    EFFECT_OPTION_NONE = 0,
    EFFECT_OPTION_XY = 1 << 0,
    EFFECT_OPTION_TYPE = 1 << 1,
    EFFECT_OPTION_SIZE = 1 << 2,
    EFFECT_OPTION_ALPHA = 1 << 3,
    EFFECT_OPTION_OFFSET = 1 << 4,
    EFFECT_OPTION_DURATION = 1 << 5,
    EFFECT_OPTION_ANIMATION = 1 << 6,
    EFFECT_OPTION_ALL = (1 << 7) - 1,
};

struct ukui_effect {
    struct wl_global *global;

    struct wl_list resources;
    struct wl_list builtin_animations;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct effect_animation {
    struct wl_list link;

    struct ukui_effect *effect;
    struct animation *base_animation;
    struct wl_resource *resource;
    struct wl_list clients; // effect_animation_client.link
};

struct effect_animation_client {
    struct wl_resource *resource;
    struct wl_list link;
};

struct ukui_effect_surface {
    struct wlr_surface *surface;
    struct wl_list effect_entities;

    struct wlr_addon addon;
};

struct ukui_effect_entities {
    uint32_t stages;
    uint32_t options_mask;
    enum action_effect_type type;
    struct action_effect_options options;

    struct wl_list link;
    struct wl_resource *resource;
};

static struct effect_animation_client *
find_effect_animation_client(struct effect_animation *animation, struct wl_client *wl_client)
{
    struct effect_animation_client *client;
    wl_list_for_each(client, &animation->clients, link) {
        if (wl_client == wl_resource_get_client(client->resource)) {
            return client;
        }
    }
    return NULL;
}

static void effect_animation_resource_destroy(struct wl_resource *resource)
{
    struct effect_animation *animation = wl_resource_get_user_data(resource);
    if (!animation) {
        return;
    }

    if (wl_list_empty(&animation->clients)) {
        kywc_log(KYWC_DEBUG, "custom animation's clients is empty, destroy animation: %p",
                 animation->base_animation);
        animation_manager_destroy_animation(animation->base_animation);
        free(animation);
        return;
    }

    /* builtin_animations */
    struct effect_animation_client *effect_client =
        find_effect_animation_client(animation, wl_resource_get_client(resource));
    if (!effect_client) {
        return;
    }

    wl_list_remove(&effect_client->link);
    free(effect_client);
}

static void animation_curve_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct animation_curve_v1_interface animation_curve_impl = {
    .destroy = animation_curve_handle_destroy,
};

static struct effect_animation_client *
create_effect_animation_client_for_resource(struct effect_animation *animation,
                                            struct wl_resource *effect_client_resource)
{
    struct effect_animation_client *animation_client = calloc(1, sizeof(*animation_client));
    if (!animation_client) {
        return NULL;
    }

    struct wl_client *client = wl_resource_get_client(effect_client_resource);
    struct wl_resource *animation_client_resource = wl_resource_create(
        client, &animation_curve_v1_interface, wl_resource_get_version(effect_client_resource), 0);
    if (!animation_client_resource) {
        wl_client_post_no_memory(client);
        free(animation_client);
        return NULL;
    }

    animation_client->resource = animation_client_resource;
    wl_list_insert(&animation->clients, &animation_client->link);

    wl_resource_set_implementation(animation_client_resource, &animation_curve_impl, animation,
                                   effect_animation_resource_destroy);

    ukui_effect_v1_send_builtin_animation_curves(
        effect_client_resource, animation_client->resource, animation->base_animation->p1.x * 100,
        animation->base_animation->p1.y * 100, animation->base_animation->p2.x * 100,
        animation->base_animation->p2.y * 100);

    return animation_client;
}

static bool check_value(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2)
{
    return x1 > 100 || x2 > 100 || y1 > 100 || y2 > 100;
}

static struct effect_animation *create_effect_animation(struct ukui_effect *effect,
                                                        struct wl_resource *resource,
                                                        struct animation *base_animation)
{
    struct effect_animation *animation = calloc(1, sizeof(*animation));
    if (!animation) {
        return NULL;
    }

    animation->base_animation = base_animation;
    animation->resource = resource;
    animation->effect = effect;
    wl_list_init(&animation->clients);

    return animation;
}

static void handle_create_animation(struct wl_client *client, struct wl_resource *effect_resource,
                                    uint32_t id, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
    if (check_value(x1, x2, y1, y2)) {
        wl_resource_post_error(effect_resource, UKUI_EFFECT_V1_ERROR_OUT_OF_RANGE,
                               "value is out of range");
        return;
    }

    int version = wl_resource_get_version(effect_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &animation_curve_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &animation_curve_impl, NULL,
                                   effect_animation_resource_destroy);

    struct animation *base_animation = animation_manager_create_animation(
        (struct point){ x1 / 100.0f, y1 / 100.0f }, (struct point){ x2 / 100.0f, y2 / 100.0f });
    if (!base_animation) {
        return;
    }

    struct ukui_effect *effect = wl_resource_get_user_data(effect_resource);
    struct effect_animation *animation = create_effect_animation(effect, resource, base_animation);
    if (!animation) {
        animation_manager_destroy_animation(base_animation);
        return;
    }

    wl_resource_set_user_data(resource, animation);
}

static struct ukui_effect_entities *find_ukui_effect(struct ukui_effect_surface *surface,
                                                     uint32_t stages)
{
    struct ukui_effect_entities *effect;
    wl_list_for_each(effect, &surface->effect_entities, link) {
        if (effect->stages & stages) {
            return effect;
        }
    }

    return NULL;
}

static void ukui_effect_entities_destroy(struct ukui_effect_entities *effect)
{
    wl_list_remove(&effect->link);
    free(effect);
}

static struct ukui_effect_entities *create_ukui_effect_entities(struct ukui_effect_surface *surface,
                                                                struct wl_resource *resource,
                                                                uint32_t stages,
                                                                enum action_effect_type type)
{
    struct ukui_effect_entities *effect_entities = calloc(1, sizeof(*effect_entities));
    if (!effect_entities) {
        return NULL;
    }

    effect_entities->type = type;
    effect_entities->stages = stages;
    effect_entities->resource = resource;
    wl_list_insert(&surface->effect_entities, &effect_entities->link);

    return effect_entities;
}

static void ukui_effect_surface_destroy(struct wlr_addon *addon)
{
    struct ukui_effect_surface *effect_surface = wl_container_of(addon, effect_surface, addon);

    wlr_addon_finish(&effect_surface->addon);

    struct ukui_effect_entities *effect_entities, *tmp;
    wl_list_for_each_safe(effect_entities, tmp, &effect_surface->effect_entities, link) {
        wl_list_remove(&effect_entities->link);
        wl_list_init(&effect_entities->link);
    }

    free(effect_surface);
}

static const struct wlr_addon_interface ukui_effect_surface_impl = {
    .name = "ukui_effect_surface",
    .destroy = ukui_effect_surface_destroy,
};

static struct ukui_effect_surface *create_ukui_effect_surface(struct wlr_surface *wlr_surface)
{
    struct ukui_effect_surface *effect_surface = calloc(1, sizeof(*effect_surface));
    if (!effect_surface) {
        return NULL;
    }

    effect_surface->surface = wlr_surface;
    wl_list_init(&effect_surface->effect_entities);
    wlr_addon_init(&effect_surface->addon, &wlr_surface->addons, wlr_surface,
                   &ukui_effect_surface_impl);

    return effect_surface;
}

static void fade_effect_resource_destroy(struct wl_resource *resource)
{
    struct ukui_effect_entities *effect = wl_resource_get_user_data(resource);
    if (effect) {
        ukui_effect_entities_destroy(effect);
    }
}

static void fade_effect_handle_set_animation(struct wl_client *client, struct wl_resource *resource,
                                             struct wl_resource *geometry,
                                             struct wl_resource *opacity)
{
    struct ukui_effect_entities *surface_effect = wl_resource_get_user_data(resource);
    if (!surface_effect) {
        return;
    }

    surface_effect->options_mask |= EFFECT_OPTION_ANIMATION;
    struct effect_animation *geometry_animation =
        geometry ? wl_resource_get_user_data(geometry) : NULL;
    struct effect_animation *opacity_animation =
        opacity ? wl_resource_get_user_data(opacity) : NULL;

    surface_effect->options.animations.geometry =
        geometry_animation ? geometry_animation->base_animation : NULL;
    surface_effect->options.animations.alpha =
        opacity_animation ? opacity_animation->base_animation : NULL;
}

static void fade_effect_handle_set_duration(struct wl_client *client, struct wl_resource *resource,
                                            uint32_t duration)
{
    struct ukui_effect_entities *surface_effect = wl_resource_get_user_data(resource);
    if (!surface_effect) {
        return;
    }

    surface_effect->options_mask |= EFFECT_OPTION_DURATION;
    surface_effect->options.duration = duration;
}

static void fade_effect_handle_set_opacity(struct wl_client *client, struct wl_resource *resource,
                                           uint32_t opacity)
{
    struct ukui_effect_entities *surface_effect = wl_resource_get_user_data(resource);
    if (!surface_effect) {
        return;
    }

    surface_effect->options_mask |= EFFECT_OPTION_ALPHA;
    surface_effect->options.alpha = opacity / UKUI_FLOAT_PRECISION;
}

static void fade_effect_handle_set_scale(struct wl_client *client, struct wl_resource *resource,
                                         uint32_t scale_type, uint32_t width_scale,
                                         uint32_t height_scale)
{
    struct ukui_effect_entities *surface_effect = wl_resource_get_user_data(resource);
    if (!surface_effect) {
        return;
    }

    surface_effect->options_mask |= EFFECT_OPTION_TYPE;
    surface_effect->options_mask |= EFFECT_OPTION_SIZE;
    surface_effect->options.style = scale_type;
    surface_effect->options.width_scale = width_scale / UKUI_FLOAT_PRECISION;
    surface_effect->options.height_scale = height_scale / UKUI_FLOAT_PRECISION;
}

static void fade_effect_handle_set_offset(struct wl_client *client, struct wl_resource *resource,
                                          int32_t x, int32_t y)
{
    struct ukui_effect_entities *surface_effect = wl_resource_get_user_data(resource);
    if (!surface_effect) {
        return;
    }

    surface_effect->options_mask |= EFFECT_OPTION_OFFSET;
    surface_effect->options.x_offset = x;
    surface_effect->options.y_offset = y;
}

static void fade_effect_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct fade_effect_v1_interface fade_effect_impl = {
    .set_animation = fade_effect_handle_set_animation,
    .set_duration = fade_effect_handle_set_duration,
    .set_opacity = fade_effect_handle_set_opacity,
    .set_scale = fade_effect_handle_set_scale,
    .set_offset = fade_effect_handle_set_offset,
    .destroy = fade_effect_handle_destroy,
};

static void handle_create_fade_effect(struct wl_client *client, struct wl_resource *effect_resource,
                                      uint32_t id, struct wl_resource *surface, uint32_t stage)
{
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface);

    struct ukui_effect_surface *effect_surface = NULL;
    struct wlr_addon *ukui_addon =
        wlr_addon_find(&wlr_surface->addons, wlr_surface, &ukui_effect_surface_impl);
    if (ukui_addon) {
        effect_surface = wl_container_of(ukui_addon, effect_surface, addon);
    }

    if (!effect_surface) {
        effect_surface = create_ukui_effect_surface(wlr_surface);
    } else if (find_ukui_effect(effect_surface, stage)) {
        wl_resource_post_error(effect_resource, UKUI_EFFECT_V1_ERROR_EFFECT_EXIST,
                               "effect has existed on the same stage");
        return;
    }

    if (!effect_surface) {
        wl_client_post_no_memory(client);
        return;
    }

    int version = wl_resource_get_version(effect_resource);
    struct wl_resource *effect_entities_resource =
        wl_resource_create(client, &fade_effect_v1_interface, version, id);
    if (!effect_entities_resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(effect_entities_resource, &fade_effect_impl, NULL,
                                   fade_effect_resource_destroy);

    struct ukui_effect_entities *effect_entities = create_ukui_effect_entities(
        effect_surface, effect_entities_resource, stage, ACTION_EFFECT_FADE);
    if (!effect_entities) {
        return;
    }

    wl_resource_set_user_data(effect_entities_resource, effect_entities);
}

static void slide_effect_resource_destroy(struct wl_resource *resource)
{
    struct ukui_effect_entities *effect_entities = wl_resource_get_user_data(resource);
    if (effect_entities) {
        ukui_effect_entities_destroy(effect_entities);
    }
}

static void slide_effect_handle_set_duration(struct wl_client *client, struct wl_resource *resource,
                                             uint32_t duration)
{
    struct ukui_effect_entities *effect_entities = wl_resource_get_user_data(resource);
    if (!effect_entities) {
        return;
    }

    effect_entities->options_mask |= EFFECT_OPTION_DURATION;
    effect_entities->options.duration = duration;
}

static void slide_effect_handle_set_pos(struct wl_client *client, struct wl_resource *resource,
                                        uint32_t location, int32_t offset)
{
    struct ukui_effect_entities *effect_entities = wl_resource_get_user_data(resource);
    if (!effect_entities) {
        return;
    }

    effect_entities->options_mask |= EFFECT_OPTION_XY;
    effect_entities->options.location = location;
    effect_entities->options.y_offset = offset;
}

static void slide_effect_handle_set_opacity(struct wl_client *client, struct wl_resource *resource,
                                            uint32_t opacity)
{
    struct ukui_effect_entities *effect_entities = wl_resource_get_user_data(resource);
    if (!effect_entities) {
        return;
    }

    effect_entities->options_mask |= EFFECT_OPTION_ALPHA;
    effect_entities->options.alpha = opacity / UKUI_FLOAT_PRECISION;
}

static void slide_effect_handle_set_type(struct wl_client *client, struct wl_resource *resource,
                                         uint32_t type)
{
    struct ukui_effect_entities *effect_entities = wl_resource_get_user_data(resource);
    if (!effect_entities) {
        return;
    }

    effect_entities->options_mask |= EFFECT_OPTION_TYPE;
    effect_entities->options.style = type;
}

static void slide_effect_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct slide_effect_v1_interface slide_effect_impl = {
    .set_duration = slide_effect_handle_set_duration,
    .set_opacity = slide_effect_handle_set_opacity,
    .set_position = slide_effect_handle_set_pos,
    .set_type = slide_effect_handle_set_type,
    .destroy = slide_effect_handle_destroy,
};

static void handle_create_slide_effect(struct wl_client *client,
                                       struct wl_resource *effect_resource, uint32_t id,
                                       struct wl_resource *surface, uint32_t stage)
{
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface);

    struct ukui_effect_surface *effect_surface = NULL;
    struct wlr_addon *ukui_addon =
        wlr_addon_find(&wlr_surface->addons, wlr_surface, &ukui_effect_surface_impl);
    if (ukui_addon) {
        effect_surface = wl_container_of(ukui_addon, effect_surface, addon);
    }

    if (!effect_surface) {
        effect_surface = create_ukui_effect_surface(wlr_surface);
    } else if (find_ukui_effect(effect_surface, stage)) {
        wl_resource_post_error(effect_resource, UKUI_EFFECT_V1_ERROR_EFFECT_EXIST,
                               "effect has existed on the same stage");
        return;
    }

    if (!effect_surface) {
        wl_client_post_no_memory(client);
        return;
    }

    int version = wl_resource_get_version(effect_resource);
    struct wl_resource *entities_resource =
        wl_resource_create(client, &slide_effect_v1_interface, version, id);
    if (!entities_resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(entities_resource, &slide_effect_impl, NULL,
                                   slide_effect_resource_destroy);
    struct ukui_effect_entities *effect_entities =
        create_ukui_effect_entities(effect_surface, entities_resource, stage, ACTION_EFFECT_SLIDE);
    if (!effect_entities) {
        return;
    }

    wl_resource_set_user_data(entities_resource, effect_entities);
}

static void handle_ukui_effect_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ukui_effect_v1_interface ukui_effect_impl = {
    .create_animation_curve = handle_create_animation,
    .create_fade_effect = handle_create_fade_effect,
    .create_slide_effect = handle_create_slide_effect,
    .destroy = handle_ukui_effect_destroy,
};

static void ukui_effect_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void ukui_effect_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct ukui_effect *effect = data;
    struct wl_resource *effect_client_resource =
        wl_resource_create(client, &ukui_effect_v1_interface, version, id);
    if (!effect_client_resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(effect_client_resource, &ukui_effect_impl, effect,
                                   ukui_effect_resource_destroy);
    wl_list_insert(&effect->resources, wl_resource_get_link(effect_client_resource));

    /* send all builtin_animations */
    struct effect_animation *animation;
    wl_list_for_each(animation, &effect->builtin_animations, link) {
        create_effect_animation_client_for_resource(animation, effect_client_resource);
    }
    ukui_effect_v1_send_done(effect_client_resource);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_effect *effect = wl_container_of(listener, effect, display_destroy);
    wl_list_remove(&effect->display_destroy.link);
    wl_global_destroy(effect->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct ukui_effect *effect = wl_container_of(listener, effect, server_destroy);
    wl_list_remove(&effect->server_destroy.link);

    struct effect_animation *animation, *temp;
    wl_list_for_each_safe(animation, temp, &effect->builtin_animations, link) {
        wl_list_remove(&animation->link);
        free(animation);
    }

    free(effect);
}

static bool animation_manager_builtin_animation_callback(struct animation *base_animation,
                                                         void *data)
{
    struct ukui_effect *effect = data;
    struct effect_animation *animation = create_effect_animation(effect, NULL, base_animation);
    if (!animation) {
        return false;
    }

    wl_list_insert(&effect->builtin_animations, &animation->link);
    return false;
}

static void apply_options(struct action_effect_options *ukui_options, uint32_t options_mask,
                          struct action_effect_options *options)
{
    if (options_mask & EFFECT_OPTION_XY) {
        options->y_offset = ukui_options->y_offset;
        options->location = ukui_options->location;
    }

    if (options_mask & EFFECT_OPTION_TYPE) {
        options->style = ukui_options->style;
    }

    if (options_mask & EFFECT_OPTION_SIZE) {
        options->width_scale = ukui_options->width_scale;
        options->height_scale = ukui_options->height_scale;
    }

    if (options_mask & EFFECT_OPTION_ANIMATION) {
        options->animations = ukui_options->animations;
    }

    if (options_mask & EFFECT_OPTION_DURATION) {
        options->duration = ukui_options->duration;
    }

    if (options_mask & EFFECT_OPTION_ALPHA) {
        options->alpha = ukui_options->alpha;
    }

    if (options_mask & EFFECT_OPTION_OFFSET) {
        options->x_offset = ukui_options->x_offset;
        options->y_offset = ukui_options->y_offset;
    }
}

static enum ukui_effect_stage action_to_ukui_stage(enum effect_action action)
{
    switch (action) {
    case EFFECT_ACTION_MAP:
        return UKUI_EFFECT_STAGE_MAP;
    case EFFECT_ACTION_UNMAP:
        return UKUI_EFFECT_STAGE_UNMAP;
    default:
        return UKUI_EFFECT_STAGE_UNKNOW;
    }
}

static void action_effect_options_process(enum action_effect_options_step step,
                                          enum effect_action action,
                                          struct action_effect_options *options, void *user_data)
{
    struct wlr_surface *surface = options->surface;
    if (!surface) {
        return;
    }

    struct wlr_addon *ukui_addon =
        wlr_addon_find(&surface->addons, surface, &ukui_effect_surface_impl);
    if (!ukui_addon) {
        return;
    }

    struct ukui_effect_surface *effect_surface = wl_container_of(ukui_addon, effect_surface, addon);
    struct ukui_effect_entities *effect =
        find_ukui_effect(effect_surface, action_to_ukui_stage(action));
    if (!effect) {
        return;
    }

    struct action_effect *action_effect = action_effect_manager_get_effect(effect->type);
    if (!action_effect) {
        kywc_log(KYWC_WARN, "effect type: %d not found.", effect->type);
        return;
    }

    if (!action_effect_init_options(action_effect, options)) {
        options->effect_type = effect->type;
        kywc_log(KYWC_WARN, "effect type: %d havn't default options.", effect->type);
    }

    apply_options(&effect->options, effect->options_mask, options);
}

bool ukui_effect_create(struct server *server)
{
    struct ukui_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return false;
    }

    effect->global =
        wl_global_create(server->display, &ukui_effect_v1_interface, 1, effect, ukui_effect_bind);
    if (!effect->global) {
        kywc_log(KYWC_WARN, "surface effect manager create failed");
        free(effect);
        return false;
    }

    wl_list_init(&effect->resources);
    wl_list_init(&effect->builtin_animations);

    effect->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &effect->server_destroy);
    effect->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &effect->display_destroy);

    animation_manager_for_each_builtin_animation(animation_manager_builtin_animation_callback,
                                                 effect);

    view_manager_set_effect_options_impl(action_effect_options_process);
    return true;
}
