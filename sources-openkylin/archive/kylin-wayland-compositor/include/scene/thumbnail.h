// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_THUMBNAIL_H_
#define _SCENE_THUMBNAIL_H_

#include "scene.h"
#include "util/client.h"
#include "view/view.h"
#include "view/workspace.h"

struct server;
struct kywc_output_state;
struct thumbnail;

enum thumbnail_option {
    /* disable server decoration, include shadow */
    THUMBNAIL_DISABLE_DECOR = 1 << 0,
    /* disable shadow only if has it */
    THUMBNAIL_DISABLE_SHADOW = 1 << 1,
    /* disable round corners */
    THUMBNAIL_DISABLE_ROUND_CORNER = 1 << 2,
    /* prefer single plane buffer */
    THUMBNAIL_ENABLE_SINGLE_PLANE = 1 << 3,
    /* enable security check, buffer is not shared */
    THUMBNAIL_ENABLE_SECURITY = 1 << 4,
    /* enable style effect on node */
    THUMBNAIL_ENABLE_STYLE_EFFECT = 1 << 5,
    /* without the child node who has effect */
    THUMBNAIL_WITHOUT_SUBEFFECT_NODE = 1 << 6,
};

struct thumbnail_interface {
    void (*update)(struct thumbnail *thumbnail, struct wlr_buffer *buffer, bool buffer_changed,
                   void *data);
    void (*cancel)(struct thumbnail *thumbnail, void *data);

    /* buffer is not shared when this callback is set */
    struct wlr_buffer *(*buffer)(struct thumbnail *thumbnail, struct wlr_buffer *buffer, int width,
                                 int height, bool single_plane, void *data);
    /* called when thumbnail_create to get the client info */
    void (*client)(struct thumbnail *thumbnail, struct client *client, void *data);
};

bool thumbnail_manager_create(struct server *server);

struct thumbnail *thumbnail_create_from_node(struct scene_node *node, uint32_t options, float scale,
                                             const struct thumbnail_interface *impl, void *data);

struct thumbnail *thumbnail_create_from_view(struct view *view, uint32_t options, float scale,
                                             const struct thumbnail_interface *impl, void *data);

struct thumbnail *thumbnail_create_from_workspace(struct workspace *workspace,
                                                  struct kywc_output *output, uint32_t options,
                                                  float scale,
                                                  const struct thumbnail_interface *impl,
                                                  void *data);

struct thumbnail *thumbnail_create_from_output(struct scene_output *output,
                                               struct kywc_output_state *output_state, float scale,
                                               const struct thumbnail_interface *impl, void *data);

void thumbnail_mark_wants_update(struct thumbnail *thumbnail, bool wants, bool force);

/* can be used before thumbnail update, but may change after update */
void thumbnail_get_buffer_size(struct thumbnail *thumbnail, int *width, int *height);

/* logic coord, didn't scale. */
void thumbnail_get_source_extends(struct thumbnail *thumbnail, struct kywc_box *bbox);

/* logic coord, didn't scale. */
void thumbnail_get_source_radius(struct thumbnail *thumbnail, int radius[static 4]);

/* logic coord, didn't scale */
bool thumbnail_get_source_padding(struct thumbnail *thumbnail, struct view_padding *padding);

/* normalized box of effective content in buffer */
void thumbnail_get_effective_box(struct thumbnail *thumbnail, struct kywc_fbox *box);

void thumbnail_update(struct thumbnail *thumbnail);

void thumbnail_destroy(struct thumbnail *thumbnail);

#endif /* _SCENE_THUMBNAIL_H_ */
