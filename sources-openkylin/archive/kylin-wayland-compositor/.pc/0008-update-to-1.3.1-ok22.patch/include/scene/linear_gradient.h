// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_LINEAR_GRADIENT_H_
#define _SCENE_LINEAR_GRADIENT_H_

#include "scene/scene.h"

struct ky_scene_linear_gradient *
ky_scene_linear_gradient_create(struct ky_scene_tree *parent, int width, int height,
                                const float background_color[static 4]);

struct ky_scene_node *
ky_scene_node_from_linear_gradient(struct ky_scene_linear_gradient *linear_gradient);

struct ky_scene_rect *
ky_scene_rect_from_linear_gradient(struct ky_scene_linear_gradient *linear_gradient);

struct ky_scene_linear_gradient *ky_scene_linear_gradientn_from_node(struct ky_scene_node *node);

void ky_scene_linear_gradient_set_background_color(struct ky_scene_linear_gradient *linear_gradient,
                                                   const float color[static 4]);

void ky_scene_linear_gradient_set_linear(struct ky_scene_linear_gradient *linear_gradient,
                                         float degree);

// local normalized coord point (0~1)
void ky_scene_linear_gradient_set_linear_points(struct ky_scene_linear_gradient *linear_gradient,
                                                float x0, float y0, float x1, float y1);

void ky_scene_linear_gradient_set_color_stop_0(struct ky_scene_linear_gradient *linear_gradient,
                                               float offset, const float color[static 4]);

void ky_scene_linear_gradient_set_color_stop_1(struct ky_scene_linear_gradient *linear_gradient,
                                               float offset, const float color[static 4]);

#endif /* _SCENE_LINEAR_GRADIENT_H_ */
