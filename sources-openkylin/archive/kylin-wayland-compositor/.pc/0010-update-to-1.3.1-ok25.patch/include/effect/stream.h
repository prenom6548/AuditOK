// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_STREAM_H_
#define _EFFECT_STREAM_H_

#include <stdint.h>

struct output;
struct view;
struct stream;

enum stream_option {
    STREAM_NEED_NONE = 0,
    /* embed cursor into buffer */
    STREAM_NEED_CURSOR = 1 << 0,
};

struct stream_interface {
    void (*ready)(struct stream *stream, uint32_t node, void *data);
    void (*cancel)(struct stream *stream, void *data);
};

struct stream *stream_create_for_output(struct output *output, uint32_t options,
                                        const struct stream_interface *impl, void *data);

struct stream *stream_create_for_view(struct view *view, uint32_t options,
                                      const struct stream_interface *impl, void *data);

void stream_destroy(struct stream *stream);

#endif /* _EFFECT_STREAM_H_ */
