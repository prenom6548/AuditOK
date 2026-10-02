// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _THEME_H_
#define _THEME_H_

#include <wayland-server-core.h>

#define THEME_MAX_SHADOW_LAYERS (2)

struct wlr_fbox;
struct wlr_buffer;
struct server;
struct icon;

enum theme_type {
    THEME_TYPE_UNDEFINED = -1,
    THEME_TYPE_DEFAULT,
    THEME_TYPE_LIGHT = THEME_TYPE_DEFAULT,
    THEME_TYPE_DARK,
};

enum justification {
    JUSTIFY_LEFT = 0,
    JUSTIFY_CENTER,
    JUSTIFY_RIGHT,
};

enum theme_button_type {
    THEME_BUTTON_TYPE_MINIMIZE = 0,
    THEME_BUTTON_TYPE_MAXIMIZE,
    THEME_BUTTON_TYPE_RESTORE,
    THEME_BUTTON_TYPE_CLOSE,
    THEME_BUTTON_TYPE_MINIMIZE_HOVER,
    THEME_BUTTON_TYPE_MAXIMIZE_HOVER,
    THEME_BUTTON_TYPE_RESTORE_HOVER,
    THEME_BUTTON_TYPE_CLOSE_HOVER,
    THEME_BUTTON_TYPE_MINIMIZE_CLICKED,
    THEME_BUTTON_TYPE_MAXIMIZE_CLICKED,
    THEME_BUTTON_TYPE_RESTORE_CLICKED,
    THEME_BUTTON_TYPE_CLOSE_CLICKED,
};

enum theme_update_mask {
    THEME_UPDATE_MASK_NONE = 0,
    /* theme type changed */
    THEME_UPDATE_MASK_TYPE = 1 << 0,
    /* font_name, font_size or text color changed */
    THEME_UPDATE_MASK_FONT = 1 << 1,
    /* border_color changed */
    THEME_UPDATE_MASK_BORDER_COLOR = 1 << 2,
    /* background_color changed */
    THEME_UPDATE_MASK_BACKGROUND_COLOR = 1 << 3,
    /* accent_color changed */
    THEME_UPDATE_MASK_ACCENT_COLOR = 1 << 4,
    /* corner_radius changed */
    THEME_UPDATE_MASK_CORNER_RADIUS = 1 << 5,
    /* opacity changed */
    THEME_UPDATE_MASK_OPACITY = 1 << 6,
    /* modal_mask_color changed */
    THEME_UPDATE_MASK_MODAL_MASK_COLOR = 1 << 7,
    /* shadow_color changed */
    THEME_UPDATE_MASK_SHADOW_COLOR = 1 << 8,
    /* icon_size button_width title_height... changed */
    THEME_UPDATE_MASK_DECORATION_SIZE = 1 << 9,
    /* state (normal, hover, click) color changed */
    THEME_UPDATE_MASK_STATE_COLOR = 1 << 10,
};

struct theme_update_event {
    enum theme_type theme_type;
    uint32_t update_mask;
};

struct theme_gradient {
    float background[4];
    float start[4], stop[4];
    int angle;
};

struct theme_shadow_layer {
    float color[4];
    int off_x, off_y;
    int blur, spread; // radius
};

struct theme_shadow {
    struct theme_shadow_layer layers[THEME_MAX_SHADOW_LAYERS];
    int num_layers;
};

struct theme {
    const char *name;
    enum theme_type type;
    bool builtin;

    /* border color */
    float active_border_color[4];
    float inactive_border_color[4];

    /* background color */
    float active_bg_color[4];
    float inactive_bg_color[4];

    /* text color */
    float active_text_color[4];
    float inactive_text_color[4];

    /* shadow color */
    struct theme_shadow active_shadow_color;
    struct theme_shadow inactive_shadow_color;
    struct theme_shadow modal_active_shadow_color;
    struct theme_shadow modal_inactive_shadow_color;
    struct theme_shadow menu_shadow_color;

    /* modal mask color */
    float modal_mask_color[4];

    /* state color */
    struct theme_gradient normal_state_color;
    struct theme_gradient hover_state_color;
    struct theme_gradient click_state_color;

    /* font */
    const char *font_name;
    int font_size;

    float accent_color[4];
    int window_radius;
    int menu_radius;
    int opacity; // 0 - 100

    enum justification text_justify;
    bool layout_is_right_to_left;
    bool text_is_right_align;
    bool ssd_need_maximize_button;

    /* icon size */
    int button_width;
    int icon_size;

    int border_width;
    int title_height;
    int subtitle_height;
    int normal_radius;

    /* button svg string */
    const char *button_svg;
    struct wl_list scaled_buffers;
};

struct theme_manager *theme_manager_create(struct server *server);

void theme_manager_add_update_listener(struct wl_listener *listener, bool pre_update);

void theme_manager_add_icon_update_listener(struct wl_listener *listener);

void theme_manager_update_theme(uint32_t mask);

struct theme *theme_manager_get_theme(void);

bool theme_manager_set_widget_theme(const char *name, enum theme_type type);

bool theme_manager_set_font(const char *name, int size);

bool theme_manager_set_accent_color(int32_t color);

bool theme_manager_set_icon_theme(const char *icon_theme_name);

bool theme_manager_set_corner_radius(int32_t window_radius, int32_t menu_radius);

bool theme_manager_set_opacity(int32_t opacity);

struct wlr_buffer *theme_button_buffer_load(struct theme *theme, float scale,
                                            enum theme_button_type type, struct wlr_fbox *src);

const char *theme_icon_get_name(struct icon *icon);

struct wlr_buffer *theme_icon_get_buffer(struct icon *icon, int size, float scale);

struct icon *theme_icon_from_app_id(const char *app_id);

bool theme_icon_is_fallback(struct icon *icon);

#endif /* _THEME_H_ */
