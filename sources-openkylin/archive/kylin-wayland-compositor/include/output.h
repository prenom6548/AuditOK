// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <kywc/output.h>

#include "server.h"

enum configure_type {
    CONFIGURE_TYPE_NONE = 0,
    CONFIGURE_TYPE_INIT,
    CONFIGURE_TYPE_UPDATE,
};

struct configure_event {
    enum configure_type type;
};

struct output_configure_event {
    struct output *output;
    uint32_t changes;
    bool geometry_changed;
};

struct output_pending_config {
    struct output *output;
    struct kywc_output_state state;
    struct wl_list link;
};

struct output {
    struct kywc_output base;
    struct wlr_output *wlr_output;
    struct scene_output *scene_output;
    struct wlr_output_layout_output *layout_output;
    struct wl_event_source *frame_timer;

    struct wl_list link;
    struct output_manager *manager;

    struct wl_list configure_link;
    struct kywc_output_state pending_state;

    /* effective geometry in layout coord */
    struct kywc_box geometry;
    struct kywc_box usable_area;

    struct {
        struct wl_signal configure;
        /* emit when output is disabled: off or destroy */
        struct wl_signal disable;
        /* emit when output geometry changed */
        struct wl_signal geometry;
        /* emit when output usable area changed */
        struct wl_signal usable_area;
        /* emit when output need update usable area */
        struct wl_signal update_usable_area;
        struct wl_signal update_late_usable_area;
    } events;

    struct wl_listener present;
    struct wl_listener frame;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_state;

    uint32_t color_temp;
    uint32_t brightness;
    enum kywc_output_color_feature color_feature;
    enum kywc_output_color_filter color_filter;
    enum kywc_output_vrr_policy vrr_policy;

    uint32_t quirks;

    bool state_usable;
    bool hardware_color;
    bool frame_timer_pending;
    bool scene_commit, has_pending;
};

struct output *output_from_resource(struct wl_resource *resource);

struct output *output_from_kywc_output(struct kywc_output *kywc_output);

struct output *output_from_wlr_output(struct wlr_output *wlr_output);

void output_add_update_usable_area_listener(struct kywc_output *kywc_output,
                                            struct wl_listener *listener, bool late);

void output_update_usable_area(struct kywc_output *kywc_output);

bool output_state_attempt_vrr(struct output *output, struct wlr_output_state *state,
                              bool fullscreen);

bool output_state_attempt_tearing(struct output *output, struct wlr_output_state *state,
                                  bool is_tearing);

bool output_use_hardware_color(struct output *output);

void output_schedule_frame(struct wlr_output *wlr_output);

void output_schedule_frame_timer(struct wlr_output *wlr_output);

typedef bool (*output_iterator_func_t)(struct kywc_output *output, int index, void *data);

uint32_t output_manager_for_each_output(output_iterator_func_t iterator, bool enabled, void *data);

struct output_manager *output_manager_create(struct server *server);

void output_manager_add_configured_listener(struct wl_listener *listener);

void output_manager_add_layout_damage_listener(struct wl_listener *listener);

void output_manager_add_max_scale_listener(struct wl_listener *listener);

void output_manager_add_new_enabled_listener(struct wl_listener *listener);

void output_manager_power_outputs(bool power);

float output_manager_get_max_scale(void);

struct kywc_output *output_manager_get_fallback(void);

bool output_set_color_feature(struct kywc_output *kywc_output,
                              enum kywc_output_color_feature color_feature);

bool output_set_color_filter(struct kywc_output *kywc_output,
                             enum kywc_output_color_filter color_filter);

bool output_set_color_temp(struct kywc_output *kywc_output, uint32_t color_temp);

bool output_set_brightness(struct kywc_output *kywc_output, uint32_t brightness);

bool output_set_power(struct kywc_output *kywc_output, bool power);

enum layout_edge {
    LAYOUT_EDGE_TOP,
    LAYOUT_EDGE_BOTTOM,
    LAYOUT_EDGE_LEFT,
    LAYOUT_EDGE_RIGHT,
};

bool output_at_layout_edge(struct output *output, enum layout_edge edge);

struct output *output_find_specified_output(struct output *output, enum layout_edge edge);

bool output_state_is_mirror_mode(void);

void output_layout_get_workarea(struct wlr_box *box);

void output_layout_get_size(int *width, int *height);

#if HAVE_UKUI_BACKGROUND
struct wlr_buffer *output_get_background_buffer(struct wlr_output *wlr_output,
                                                uint32_t background_type, bool *updated);
#else
static __attribute__((unused)) inline struct wlr_buffer *
output_get_background_buffer(struct wlr_output *wlr_output, uint32_t background_type, bool *updated)
{
    return NULL;
}
#endif

#endif /* _OUTPUT_H_ */
