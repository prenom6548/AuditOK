// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_CAPTURE_H_
#define _EFFECT_CAPTURE_H_

#include "output.h"
#include "util/client.h"

struct capture;

enum capture_option {
    CAPTURE_NEED_NONE = 0,
    /* use buffer coord */
    CAPTURE_NEED_UNSCALED = 1 << 0,
    /**
     * embed cursor into buffer.
     * there is no guarantee the cursor will be embedded or not,
     * caused by hardware cursor is not support or
     * other capture request is different in cursor option.
     */
    CAPTURE_NEED_CURSOR = 1 << 1,
    /* prefer single plane buffer */
    CAPTURE_NEED_SINGLE_PLANE = 1 << 2,
    /**
     * used when CAPTURE_NEED_UNSCALED is set.
     * perfer unscaled with output scale.
     * available when capture type is output or area (in one output).
     */
    CAPTURE_PREFER_OUTPUT_SCALE = 1 << 3,
    /* buffer is not shared, similar to the buffer hook */
    CAPTURE_NEED_PRIVATE_BUFFER = 1 << 4,
};

struct capture_interface {
    void (*update)(struct capture *capture, struct wlr_buffer *buffer, bool buffer_changed,
                   void *data);
    void (*cancel)(struct capture *capture, void *data);

    /* buffer is not shared when this callback is set */
    struct wlr_buffer *(*buffer)(struct capture *capture, struct wlr_buffer *buffer, int width,
                                 int height, bool single_plane, void *data);
    /* called when capture_create to get the client info */
    void (*client)(struct capture *capture, struct client *client, void *data);
};

struct capture *capture_create_from_output(struct output *output, uint32_t options,
                                           const struct capture_interface *impl, void *data);

struct capture *capture_create_from_area(struct kywc_box *rect, uint32_t options,
                                         const struct capture_interface *impl, void *data);

struct capture *capture_create_from_fullscreen(uint32_t options,
                                               const struct capture_interface *impl, void *data);

void capture_mark_wants_update(struct capture *capture, bool wants, bool force);

void capture_get_buffer_size(struct capture *capture, int *width, int *height);

void capture_destroy(struct capture *capture);

void capture_read_buffer(struct wlr_buffer *buffer, uint32_t format, uint32_t stride,
                         struct kywc_box *box, void *data);

void capture_write_file(struct wlr_buffer *buffer, const char *path,
                        void (*done)(const char *path, void *data), void *user_data);

#endif /* _EFFECT_CAPTURE_H_ */
