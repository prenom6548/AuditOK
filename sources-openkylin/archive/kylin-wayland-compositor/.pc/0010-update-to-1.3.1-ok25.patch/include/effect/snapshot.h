// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_SNAPSHOT_H_
#define _EFFECT_SNAPSHOT_H_

#include <stdbool.h>
#include <stdint.h>

struct view;
struct server;
struct kywc_box;
struct wl_listener;
struct scene_node;
struct scene_tree;
struct scene_render_target;

enum snapshot_option {
    /* disable server decoration, include shadow */
    SNAPSHOT_DISABLE_DECOR = 1 << 0,
    /* disable shadow only if has it */
    SNAPSHOT_DISABLE_SHADOW = 1 << 1,
    /* disable round corners */
    SNAPSHOT_DISABLE_ROUND_CORNER = 1 << 2,
    /* the snapshot include the effects nodes */
    SNAPSHOT_ENABLE_SUBEFFECT_NODE = 1 << 5,
    /**
     * the content rendered to the 'set' dst_box doesn't contain shadows,
     * only includes surface and margin. the 'final' dst_box is the render_box.
     * shadows is rendered in render_box.
     */
    SNAPSHOT_RENDER_ENABLE_VIEW_BOX = 1 << 6,
    /* don't render blur behind snapshot when render snapshot */
    SNAPSHOT_RENDER_DISABLE_BLUR = 1 << 7,
};

enum snapshot_anchor {
    /* the content of the thumbnail in src_box fills dst box */
    SNAPSHOT_ANCHOR_NONE = 0,
    SNAPSHOT_ANCHOR_TOP = 1 << 1,
    SNAPSHOT_ANCHOR_LEFT = 1 << 2,
    SNAPSHOT_ANCHOR_RIGHT = 1 << 3,
    SNAPSHOT_ANCHOR_BOTTOM = 1 << 4,
    SNAPSHOT_ANCHOR_ALL = (1 << 5) - 1,
};

struct scene_buffer *view_try_get_single_buffer(struct view *view);

struct snapshot *snapshot_create_from_node(struct scene_node *node, float scale, uint32_t options);

struct snapshot *snapshot_create_from_view(struct view *view, float scale, uint32_t options);

void snapshot_destroy(struct snapshot *snapshot);

void snapshot_add_destroy_listener(struct snapshot *snapshot, struct wl_listener *listener);

void snapshot_mark_wants_update(struct snapshot *snapshot, bool wants);

void snapshot_detach(struct snapshot *snapshot);

/**
 * snapshot is rendered through a dedicated rendering interface (snapshot_render),
 * and the parameters of the rendering result are set through the snapshot interface.
 */
void snapshot_set_dst_box(struct snapshot *snapshot, const struct kywc_box *box);

void snapshot_set_anchors(struct snapshot *snapshot, uint32_t anchors);

void snapshot_set_opacity(struct snapshot *snapshot, float opacity);

void snapshot_set_mixed_snapshot(struct snapshot *snapshot, struct snapshot *mix_snapshot,
                                 float blend_factor);

/* logic coord in the scene */
void snapshot_get_render_box(struct snapshot *snapshot, struct kywc_box *box);

void snapshot_render(struct snapshot *snapshot, struct scene_render_target *target);

/**
 * snapshot is rendered through the node's rendering interface,
 * and the parameters of the rendering result are set through the node.
 */
struct scene_buffer *snapshot_create_scene_buffer(struct scene_tree *parent,
                                                  struct snapshot *snapshot);

#endif /* _EFFECT_SNAPSHOT_H_ */
