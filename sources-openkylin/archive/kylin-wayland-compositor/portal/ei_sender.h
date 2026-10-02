// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EI_SENDER_H_
#define _EI_SENDER_H_

#include <libei.h>

#include "portal.h"

struct xkb_keyboard {
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
};

struct ei_device_region {
    struct wl_list link;
    struct ei_device *device;
    struct kywc_box geo;
};

struct ei_sender {
    int eis_fd;
    struct ei *ei;
    struct ei_seat *seat;
    struct ei_device *pointer;
    struct ei_device *abs_pointer;
    struct ei_device *keyboard;
    /* current touch is not required */
    uint32_t sequence_num;

    struct xkb_keyboard keyboard_info;

    struct event_context *event_loop;
};

struct ei_sender *ei_sender_create(struct portal_session *session, char *name);

void ei_sender_close(struct ei_sender *sender);

void ei_sender_pointer_motion(struct ei_sender *sender, double x, double y);

void ei_sender_pointer_motion_absolute(struct ei_sender *sender, double x, double y);

void ei_sender_button(struct ei_sender *sender, uint32_t button, bool press);

void ei_sender_axis(struct ei_sender *sender, double x, double y, bool finish);

void ei_sender_axis_discrete(struct ei_sender *sender, uint32_t axis, int steps);

void ei_sender_keycode(struct ei_sender *sender, uint32_t keycode, bool press);

void ei_sender_keysym(struct ei_sender *sender, uint32_t keysym, bool press);

#endif /* _EI_SENDER_H_ */
