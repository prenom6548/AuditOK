// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_DECORATION_H_
#define _SCENE_DECORATION_H_

#include "scene/scene.h"

enum decoration_mask {
    DECORATION_MASK_NONE = 0,
    DECORATION_MASK_TOP_LEFT = 1 << 0,
    DECORATION_MASK_TOP = 1 << 1,
    DECORATION_MASK_TOP_RIGHT = 1 << 2,
    DECORATION_MASK_RIGHT = 1 << 3,
    DECORATION_MASK_BOTTOM_RIGHT = 1 << 4,
    DECORATION_MASK_BOTTOM = 1 << 5,
    DECORATION_MASK_BOTTOM_LEFT = 1 << 6,
    DECORATION_MASK_LEFT = 1 << 7,
    DECORATION_MASK_ALL = (1 << 8) - 1,
};

enum decoration_border_type {
    DECORATION_BORDER_TYPE_OUTER = 0,
    DECORATION_BORDER_TYPE_INNER = 1,
};

struct scene_decoration *scene_decoration_create(struct scene_tree *parent);

struct scene_decoration *
scene_decoration_create_with_border_type(struct scene_tree *parent,
                                         enum decoration_border_type border_type);

struct scene_node *scene_node_from_decoration(struct scene_decoration *decoration);

struct scene_decoration *scene_decoration_from_node(struct scene_node *node);

void scene_decoration_set_surface_size(struct scene_decoration *decoration, int width, int height);

void scene_decoration_set_surface_color(struct scene_decoration *decoration,
                                        const float color[static 4]);

// 0=right-bottom, 1=right-top, 2=left-bottom, 3=left-top
void scene_decoration_set_round_corner_radius(struct scene_decoration *decoration,
                                              const int round_corner_radius[static 4]);

void scene_decoration_set_margin(struct scene_decoration *decoration, int title_height,
                                 int border_thickness);

void scene_decoration_set_margin_color(struct scene_decoration *decoration,
                                       const float title_color[static 4],
                                       const float border_color[static 4]);

void scene_decoration_set_mask(struct scene_decoration *decoration, uint32_t mask);

void scene_decoration_set_shadow_count(struct scene_decoration *decoration, int size);

void scene_decoration_set_shadow(struct scene_decoration *decoration, int index, int offset_x,
                                 int offset_y, int spread, int blur, const float color[static 4]);

void scene_decoration_set_resize_width(struct scene_decoration *decoration, int resize_with);

void scene_decoration_set_surface_blurred(struct scene_decoration *decoration, bool blurred);

void scene_decoration_set_opacity(struct scene_decoration *decoration, float opacity);

#endif /* _SCENE_DECORATION_H_ */
