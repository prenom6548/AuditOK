// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_LINEAR_GRADIENT_H_
#define _SCENE_LINEAR_GRADIENT_H_

#include "scene/scene.h"

struct scene_linear_gradient *scene_linear_gradient_create(struct scene_tree *parent, int width,
                                                           int height,
                                                           const float background_color[static 4]);

struct scene_node *scene_node_from_linear_gradient(struct scene_linear_gradient *linear_gradient);

struct scene_rect *scene_rect_from_linear_gradient(struct scene_linear_gradient *linear_gradient);

struct scene_linear_gradient *scene_linear_gradient_from_node(struct scene_node *node);

void scene_linear_gradient_set_background_color(struct scene_linear_gradient *linear_gradient,
                                                const float color[static 4]);

void scene_linear_gradient_set_linear(struct scene_linear_gradient *linear_gradient, float degree);

// local logical coord. typically full rect: x0 = 0, y0 = 0, x1 = rect_width, y1 = rect_height
void scene_linear_gradient_set_linear_points(struct scene_linear_gradient *linear_gradient,
                                             float x0, float y0, float x1, float y1);

void scene_linear_gradient_set_color_stop_0(struct scene_linear_gradient *linear_gradient,
                                            float offset, const float color[static 4]);

void scene_linear_gradient_set_color_stop_1(struct scene_linear_gradient *linear_gradient,
                                            float offset, const float color[static 4]);

#endif /* _SCENE_LINEAR_GRADIENT_H_ */
