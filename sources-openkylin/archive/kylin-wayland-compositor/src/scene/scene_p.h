// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_P_H_
#define _SCENE_P_H_

#include <kywc/log.h>

#include "scene/scene.h"

void scene_node_init(struct scene_node *node, struct scene_tree *parent);

void scene_rect_init(struct scene_rect *rect, struct scene_tree *parent, int width, int height,
                     const float color[static 4]);

void scene_buffer_init(struct scene_buffer *scene_buffer, struct scene_tree *parent);

/**
 * update output states for buffer node in the tree or the single buffer node, when
 * 1. scene buffer state:
 *      position: set_position, reparent
 *      dest_size
 *      create with buffer
 *      destroy: direct emit output_leave
 * 2. scene output state:
 *      position
 *      mode
 *      create/destroy
 *      scale/transform/subpixel: force update
 */
void scene_node_update_outputs(struct scene_node *node, struct wl_list *outputs,
                               struct scene_output *ignore, struct scene_output *force);

/**
 * collect current damage and distribute to outputs
 */
void scene_collect_damage(struct scene *scene);

/**
 * use to calculate opaque/invisible region
 * include anti-aliasing non-opaque edge region
 */
void scene_corner_region(pixman_region32_t *region, int width, int height,
                         const int radius[static 4]);

void scene_log_region(enum kywc_log_level level, const char *desc, const pixman_region32_t *region);

void scene_log_box(enum kywc_log_level level, const char *desc, struct kywc_box *box);

void scene_log_fbox(enum kywc_log_level level, const char *desc, struct kywc_fbox *box);

bool scene_surface_is_tearing_allowed(struct scene *scene, struct wlr_surface *surface);

bool scene_is_tearing_needed(struct scene *scene);

void scene_surface_presented_on_output(struct wlr_surface *surface, struct wlr_output *output,
                                       bool zero_copy);

#endif /* _SCENE_P_H_ */
