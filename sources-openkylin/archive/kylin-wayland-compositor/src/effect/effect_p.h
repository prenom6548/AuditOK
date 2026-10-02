// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_P_H_
#define _EFFECT_P_H_

#include "effect/effect.h"
#include "server.h"

struct effect_manager {
    struct wl_list effects;
    struct config *config;

    struct server *server;
    struct wl_listener server_ready;
    struct wl_listener server_destroy;

    double time_scale;
};

bool effect_manager_config_init(struct effect_manager *effect_manager);

bool effect_init_config(struct effect *effect);

bool effect_option_is_enabled_option(const struct effect_option *option);

bool effect_get_option_boolean(struct effect *effect, const char *key, bool value);

void effect_set_option_boolean(struct effect *effect, const char *key, bool value);

int effect_get_option_int(struct effect *effect, const char *key, int value);

void effect_set_option_int(struct effect *effect, const char *key, int value);

double effect_get_option_double(struct effect *effect, const char *key, double value);

void effect_set_option_double(struct effect *effect, const char *key, double value);

const char *effect_get_option_string(struct effect *effect, const char *key, const char *value);

void effect_set_option_string(struct effect *effect, const char *key, const char *value);

void effect_color_filter_toggle(void);

void effect_zoom_in(void);

void effect_zoom_out(void);

void effect_zoom_exit(void);

bool showfps_effect_create(struct effect_manager *manager);

bool capture_manager_create(struct server *server);

bool ky_capture_manager_create(struct server *server);

bool ky_stream_manager_create(struct server *server);

bool wlr_screencopy_manager_create(struct server *server);

bool blurrer_create(struct server *server);

bool move_effect_create(struct effect_manager *effect_manager);

bool blur_effect_create(struct effect_manager *effect_manager);

#if HAVE_UKUI_SCREENSHOT
bool screenshot_effect_create(struct effect_manager *effect_manager);
#else
static __attribute__((unused)) inline bool
screenshot_effect_create(struct effect_manager *effect_manager)
{
    return false;
}
#endif

#if HAVE_UKUI_WATERMARK
bool watermark_effect_create(struct effect_manager *manager);
#else
static __attribute__((unused)) inline bool
watermark_effect_create(struct effect_manager *effect_manager)
{
    return false;
}
#endif

bool scale_effect_create(struct effect_manager *manager);

bool mouse_click_effect_create(struct effect_manager *manager);

bool mouse_trail_effect_create(struct effect_manager *manager);

bool touch_click_effect_create(struct effect_manager *manager);

bool touch_long_effect_create(struct effect_manager *manager);

bool touch_trail_effect_create(struct effect_manager *manager);

bool fade_effect_create(struct effect_manager *manager);

#if HAVE_KDE_SLIDE
bool slide_effect_create(struct effect_manager *manager);
#else
static __attribute__((unused)) inline bool
slide_effect_create(struct effect_manager *effect_manager)
{
    return false;
}
#endif

bool translation_effect_create(struct effect_manager *manager);

bool output_transform_effect_create(struct effect_manager *manager);

bool shake_cursor_effect_create(struct effect_manager *manager);

bool shake_view_effect_create(struct effect_manager *effect_manager);

bool locate_pointer_effect_create(struct effect_manager *manager);

bool magic_lamp_effect_create(struct effect_manager *manager);

bool zoom_effect_create(struct effect_manager *effect_manager);

bool node_transform_effect_create(struct effect_manager *manager);

bool showkey_effect_create(struct effect_manager *manager);

bool wallpaper_effect_create(struct effect_manager *manager);

bool color_filter_effect_create(struct effect_manager *manager);

bool color_picker_effect_create(struct effect_manager *manager);

#if HAVE_UKUI_EFFECT
bool ukui_effect_create(struct server *server);
#else
static __attribute__((unused)) inline bool ukui_effect_create(struct server *server)
{
    return false;
}
#endif

bool snapshot_manager_create(struct server *serser);

bool stream_manager_create(struct server *server);

#endif /* _EFFECT_P_H_ */
