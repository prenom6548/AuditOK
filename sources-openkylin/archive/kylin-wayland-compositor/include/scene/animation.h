// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_ANIMATION_H_
#define _SCENE_ANIMATION_H_

#include "effect/animator.h"

void scene_node_set_position_with_animation(struct scene_node *node, int x, int y,
                                            struct animation *animation, uint32_t duration);

void scene_rect_set_size_with_animation(struct scene_rect *rect, int width, int height,
                                        struct animation *animation, uint32_t duration);

#endif /* _SCENE_ANIMATION_H_ */
