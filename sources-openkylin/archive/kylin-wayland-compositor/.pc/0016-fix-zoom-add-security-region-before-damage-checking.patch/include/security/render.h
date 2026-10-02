// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SECURITY_RENDER_H_
#define _SECURITY_RENDER_H_

#include <kywc/boxes.h>

#include "render/pass.h"
#include "scene/scene.h"
#include "security/security.h"

/* return true if this node has security */
bool security_check_node(struct scene_node *node);

/* return true if this output has security */
bool security_check_output(struct scene_output *output);

bool security_check_buffer(struct wlr_buffer *wlr_buffer);

void security_target_reset(struct scene_render_target *target);

void security_target_add_node(struct scene_render_target *target, struct scene_node *node, int lx,
                              int ly);

void security_target_add_output(struct scene_render_target *target, struct scene_output *output);

void security_target_add_buffer(struct scene_render_target *target, struct wlr_buffer *wlr_buffer,
                                const struct kywc_fbox *src_box, enum wl_output_transform transform,
                                const struct kywc_box *dst_box);

/* with security region in target buffer */
void security_target_add_buffer_with_region(struct scene_render_target *target,
                                            struct wlr_buffer *wlr_buffer,
                                            const pixman_region32_t *region);

bool security_get_region(struct wlr_buffer *wlr_buffer, struct client *client,
                         pixman_region32_t *region);

bool security_access(struct client *client, enum security_action action, ...);

struct security_render_options {
    struct kywc_box dst_box; // logical box in scene layout coord
    struct render_round_corner radius;
    const pixman_region32_t *clip; // clip region in (0, 0) (box.width, box.height)

    struct wlr_texture *texture;
    const pixman_region32_t *secure_region; // secure regoin in texture
    const struct kywc_fbox *src_box;        // source box in texture
    enum wl_output_transform src_transform;
};

void security_render(struct scene_render_target *target,
                     const struct security_render_options *opts);

void security_render_node(struct scene_node *node, int lx, int ly,
                          struct scene_render_target *target);

#endif /* _SECURITY_RENDER_H_ */
