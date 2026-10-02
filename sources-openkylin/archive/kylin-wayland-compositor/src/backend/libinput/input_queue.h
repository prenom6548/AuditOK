// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _INPUT_QUEUE_H_
#define _INPUT_QUEUE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "util/spsc_chain.h"

/*
 * Lock-free SPSC input-event queue backed by a chain-based ring buffer.
 *
 * Producer: libinput thread  (input_queue_push)
 * Consumer: main thread      (input_queue_pop)
 *
 * Events are stored inline in pre-allocated ring buffers — no per-event
 * malloc.  The queue grows dynamically by linking additional fixed-capacity
 * buffers at the head when full.
 */

enum input_event_type {
    INPUT_EVENT_NONE = 0,
    INPUT_EVENT_DEVICE_ADDED,
    INPUT_EVENT_DEVICE_REMOVED,
    INPUT_EVENT_KEYBOARD_KEY,
    INPUT_EVENT_POINTER_MOTION,
    INPUT_EVENT_POINTER_MOTION_ABSOLUTE,
    INPUT_EVENT_POINTER_BUTTON,
    INPUT_EVENT_POINTER_AXIS,
    INPUT_EVENT_TOUCH_DOWN,
    INPUT_EVENT_TOUCH_UP,
    INPUT_EVENT_TOUCH_MOTION,
    INPUT_EVENT_TOUCH_CANCEL,
    INPUT_EVENT_TOUCH_FRAME,
    INPUT_EVENT_SWITCH_TOGGLE,
    INPUT_EVENT_GESTURE_SWIPE_BEGIN,
    INPUT_EVENT_GESTURE_SWIPE_UPDATE,
    INPUT_EVENT_GESTURE_SWIPE_END,
    INPUT_EVENT_GESTURE_PINCH_BEGIN,
    INPUT_EVENT_GESTURE_PINCH_UPDATE,
    INPUT_EVENT_GESTURE_PINCH_END,
    INPUT_EVENT_GESTURE_HOLD_BEGIN,
    INPUT_EVENT_GESTURE_HOLD_END,
    INPUT_EVENT_TABLET_TOOL_AXIS,
    INPUT_EVENT_TABLET_TOOL_PROXIMITY,
    INPUT_EVENT_TABLET_TOOL_TIP,
    INPUT_EVENT_TABLET_TOOL_BUTTON,
    INPUT_EVENT_TABLET_PAD_BUTTON,
    INPUT_EVENT_TABLET_PAD_RING,
    INPUT_EVENT_TABLET_PAD_STRIP,
    INPUT_EVENT_RAW_TAP,
    INPUT_EVENT_TABLET_TOOL_DESTROY,
    INPUT_EVENT_ZONE_BEGIN,
    INPUT_EVENT_ZONE_UPDATE,
    INPUT_EVENT_ZONE_END,
};

struct input_event {
    uint32_t type;
    uint32_t time_msec;
    void *dev;
    union {
        struct {
            uint32_t key;
            bool pressed;
        } key;
        struct {
            double dx, dy;
            double dx_unaccel, dy_unaccel;
        } motion;
        struct {
            double x, y;
        } motion_abs;
        struct {
            uint32_t button;
            bool pressed;
        } button;
        struct {
            uint32_t source, orientation;
            double delta;
            int32_t delta_discrete;
        } axis;
        struct {
            int32_t touch_id;
            double x, y;
            double major, minor;
            bool has_size;
        } touch;
        struct {
            uint32_t fingers;
            double dx, dy, scale, angle_delta;
            bool cancelled;
        } gesture;
        struct {
            uint32_t switch_type;
            int32_t switch_state;
        } sw;
        struct {
            void *tool;
            double x, y, dx, dy, pressure, distance;
            double tilt_x, tilt_y, rotation, slider, wheel_delta;
            int32_t wheel_discrete;
            uint32_t updated_axes;
        } tablet_axis;
        struct {
            void *tool;
            bool in_proximity;
            double x, y;
        } tablet_proximity;
        struct {
            void *tool;
            bool tip_down;
            double x, y;
        } tablet_tip;
        struct {
            void *tool;
            uint32_t button;
            bool pressed;
        } tablet_button;
        struct {
            uint32_t button;
            bool pressed;
            uint32_t mode, group, number;
            double position;
            uint32_t source;
        } tablet_pad;
        struct {
            void *dev;
            void *tool;
        } tablet_tool_destroy;
        struct {
            uint32_t fingers;
        } raw_tap;
        struct {
            int32_t zone;
            uint32_t fingers;
            double delta;
            bool cancelled;
        } zone_gesture;
    };
};

#define INPUT_QUEUE_CAPACITY 64

struct input_queue {
    struct spsc_chain *sc;
};

static inline bool input_queue_init(struct input_queue *q)
{
    q->sc = spsc_chain_create(sizeof(struct input_event), INPUT_QUEUE_CAPACITY);
    return q->sc != NULL;
}

static inline void input_queue_finish(struct input_queue *q)
{
    spsc_chain_destroy(q->sc);
    q->sc = NULL;
}

/* Push — called from libinput thread. Never blocks. */
static inline bool input_queue_push(struct input_queue *q, const struct input_event *e)
{
    return spsc_chain_push(q->sc, e);
}

/* Pop — called from main thread. Returns true if an event was available. */
static inline bool input_queue_pop(struct input_queue *q, struct input_event *e)
{
    return spsc_chain_pop(q->sc, e);
}

#endif /* _INPUT_QUEUE_H_ */
