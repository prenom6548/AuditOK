// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_BOX_H_
#define _SCENE_BOX_H_

#include "scene.h"

struct scene_stroke_rect *scene_stroke_rect_create(struct scene_tree *parent, int width, int height,
                                                   const float color[static 4], int stroke_width);

struct scene_node *scene_node_from_stroke_rect(struct scene_stroke_rect *stroke_rect);

struct scene_rect *scene_rect_from_stroke_rect(struct scene_stroke_rect *stroke_rect);

struct scene_stroke_rect *scene_stroke_rect_from_node(struct scene_node *node);

void scene_stroke_rect_set_color(struct scene_stroke_rect *stroke_rect,
                                 const float color[static 4]);

void scene_stroke_rect_set_size(struct scene_stroke_rect *stroke_rect, int width, int height);

void scene_stroke_rect_set_stroke_width(struct scene_stroke_rect *stroke_rect, int width);

// 0=right-bottom, 1=right-top, 2=left-bottom, 3=left-top
void scene_stroke_rect_set_radius(struct scene_stroke_rect *stroke_rect,
                                  const int radius[static 4]);

#endif /* _SCENE_BOX_H_ */
