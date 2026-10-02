// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _KYWC_OUTPUT_H_
#define _KYWC_OUTPUT_H_

#include <stdbool.h>
#include <stdint.h>

#include <wayland-server-protocol.h>

#include "boxes.h"

enum kywc_output_vrr_policy {
    KYWC_OUTPUT_VRR_POLICY_NEVER = 0,
    KYWC_OUTPUT_VRR_POLICY_ALWAYS,
    KYWC_OUTPUT_VRR_POLICY_AUTO,
};

enum kywc_output_capability {
    KYWC_OUTPUT_CAPABILITY_POWER = 1 << 0,
    KYWC_OUTPUT_CAPABILITY_BRIGHTNESS = 1 << 1,
    KYWC_OUTPUT_CAPABILITY_COLOR_TEMP = 1 << 2,
    KYWC_OUTPUT_CAPABILITY_COLOR_FILTER = 1 << 3,
    KYWC_OUTPUT_CAPABILITY_VRR = 1 << 4,
    KYWC_OUTPUT_CAPABILITY_RGB_RANGE = 1 << 5,
    KYWC_OUTPUT_CAPABILITY_OVERSCAN = 1 << 6,
    KYWC_OUTPUT_CAPABILITY_SCALING_MODE = 1 << 7,
};

enum kywc_output_color_filter {
    KYWC_OUTPUT_COLOR_FILTER_NONE = 0,
    KYWC_OUTPUT_COLOR_FILTER_GRAYSCALE,
    KYWC_OUTPUT_COLOR_FILTER_INVERT,
    KYWC_OUTPUT_COLOR_FILTER_PROTANOPIA,
    KYWC_OUTPUT_COLOR_FILTER_DEUTERANOPIA,
    KYWC_OUTPUT_COLOR_FILTER_TRITANOPIA,
};

enum kywc_output_scaling_mode {
    KYWC_OUTPUT_SCALING_MODE_NONE,
    KYWC_OUTPUT_SCALING_MODE_FULL,
    KYWC_OUTPUT_SCALING_MODE_CENTER,
    KYWC_OUTPUT_SCALING_MODE_FULL_ASPECT,
};

enum kywc_output_rgb_range {
    KYWC_OUTPUT_RGB_RANGE_AUTO,
    KYWC_OUTPUT_RGB_RANGE_FULL,
    KYWC_OUTPUT_RGB_RANGE_LIMITED,
};

enum kywc_output_color_feature {
    KYWC_OUTPUT_COLOR_FEATURE_AUTO = 0,
    KYWC_OUTPUT_COLOR_FEATURE_DISABLE,
    KYWC_OUTPUT_COLOR_FEATURE_SOFTGAMMA,
    KYWC_OUTPUT_COLOR_FEATURE_SOFTCTM,
    KYWC_OUTPUT_COLOR_FEATURE_SOFTCOLOR,
};

enum kywc_output_backend {
    KYWC_OUTPUT_BACKEND_NONE = 0,
    KYWC_OUTPUT_BACKEND_DRM,
    KYWC_OUTPUT_BACKEND_HEADLESS,
    KYWC_OUTPUT_BACKEND_FBDEV,
};

enum kywc_output_state_field {
    KYWC_OUTPUT_STATE_ENABLE = 1 << 0,
    KYWC_OUTPUT_STATE_POWER = 1 << 1,
    KYWC_OUTPUT_STATE_MODE = 1 << 2,
    KYWC_OUTPUT_STATE_TRANSFORM = 1 << 3,
    KYWC_OUTPUT_STATE_SCALE = 1 << 4,
    KYWC_OUTPUT_STATE_VRR_POLICY = 1 << 5,
    KYWC_OUTPUT_STATE_POSITION = 1 << 6,
    KYWC_OUTPUT_STATE_BRIGHTNESS = 1 << 7,
    KYWC_OUTPUT_STATE_COLOR_TEMP = 1 << 8,
    KYWC_OUTPUT_STATE_OVERSCAN = 1 << 9,
    KYWC_OUTPUT_STATE_COLOR_FEATURE = 1 << 10,
    KYWC_OUTPUT_STATE_COLOR_FILTER = 1 << 11,
    KYWC_OUTPUT_STATE_SCALING_MODE = 1 << 12,
    KYWC_OUTPUT_STATE_RGB_RANGE = 1 << 13,
    KYWC_OUTPUT_STATE_WAIT_TIMELINE = 1 << 14,
    KYWC_OUTPUT_STATE_SIGNAL_TIMELINE = 1 << 15,
};

struct kywc_output_state {
    bool enabled, power;
    int32_t width, height, refresh; // refresh in mHz
    enum wl_output_transform transform;
    enum kywc_output_vrr_policy vrr_policy;
    float scale;

    /* layout coord */
    int32_t lx, ly;
    uint32_t brightness;
    uint32_t color_temp;
    uint32_t overscan; // range[0,100]

    enum kywc_output_color_feature color_feature;
    enum kywc_output_color_filter color_filter;
    enum kywc_output_scaling_mode scaling_mode;
    enum kywc_output_rgb_range rgb_range;
};

struct kywc_output_mode {
    int32_t width, height;
    int32_t refresh; // mHz
    bool preferred;
    struct wl_list link;
};

struct kywc_output_prop {
    enum kywc_output_backend backend;
    uint32_t capabilities;
    int32_t phys_width, phys_height;          // mm
    const char *make, *model, *serial, *desc; // may be NULL
    const char *edid;                         // base64
    const char *edid_hash;                    // md5
    struct wl_list modes;
};

/* connector with a monitor */
struct kywc_output {
    const char *name;
    const char *uuid;
    bool destroying;

    struct kywc_output_prop prop;
    struct kywc_output_state state;

    struct {
        struct wl_signal on;
        struct wl_signal off;
        struct wl_signal scale;
        struct wl_signal transform;
        struct wl_signal mode;
        struct wl_signal position;
        struct wl_signal power;
        struct wl_signal brightness;
        struct wl_signal color_temp;
        struct wl_signal color_filter;

        struct wl_signal destroy;
    } events;
};

void kywc_output_add_new_listener(struct wl_listener *listener);

void kywc_output_add_primary_listener(struct wl_listener *listener);

struct kywc_output *kywc_output_get_primary(void);

void kywc_output_effective_geometry(struct kywc_output *kywc_output, struct kywc_box *box);

bool kywc_output_contains_point(struct kywc_output *kywc_output, int x, int y);

struct kywc_output *kywc_output_at_point(double lx, double ly);

struct kywc_output *kywc_output_by_name(const char *name);

struct kywc_output *kywc_output_by_uuid(const char *uuid);

#endif /* _KYWC_OUTPUT_H_ */
