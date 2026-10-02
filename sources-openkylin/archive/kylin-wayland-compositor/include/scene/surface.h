// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_SURFACE_H_
#define _SCENE_SURFACE_H_

#include "scene.h"

struct scene_surface {
    struct scene_buffer *buffer;
    struct wlr_surface *surface;

    struct wlr_addon addon;
    struct wlr_addon node_addon;

    struct wl_listener outputs_update;
    struct wl_listener output_enter;
    struct wl_listener output_leave;
    struct wl_listener output_sample;
    struct wl_listener frame_done;
    struct wl_listener surface_destroy;
    struct wl_listener surface_commit;
    struct wl_listener surface_map;

    struct scene_node_interface buffer_default_impl;
};

struct scene_tree *scene_subsurface_tree_create(struct scene_tree *parent,
                                                struct wlr_surface *surface);

struct scene_surface *scene_surface_create(struct scene_tree *parent,
                                           struct wlr_surface *wlr_surface);

struct scene_surface *scene_surface_try_from_buffer(struct scene_buffer *scene_buffer);

struct wlr_surface *wlr_surface_try_from_node(struct scene_node *node);

struct scene_buffer *scene_buffer_try_from_surface(struct wlr_surface *wlr_surface);

/**
 * Get the scaling factor(buffer width / logic width) of a surface.
 * Prioritize using the scaling factor of the surface.
 * If the scaling factor is set to the default 1.0,
 * use the ratio of buffer width to logical width as the scaling factor
 * (Actual scaling under fractional scaling protocol).
 * If the scaling factor is still 1.0 or less than the output factor,
 * use the output scaling factor as the scaling factor.
 * Because regardless of the size of the surface buffer,
 * it will ultimately be rendered in a buffer of size (output scale multiplied by logical size),
 * the fractional scaling protocol also uses the output scaling factor.
 */
float scene_surface_get_scale(struct wlr_surface *wlr_surface);

#endif /* _SCENE_SURFACE_H_ */
