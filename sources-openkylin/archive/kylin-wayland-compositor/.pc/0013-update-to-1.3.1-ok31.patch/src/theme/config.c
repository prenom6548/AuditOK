// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "theme_p.h"
#include "util/dbus.h"

static const char *service_path = "/com/kylin/Wlcom/Theme";
static const char *service_interface = "com.kylin.Wlcom.Theme";

static int print_theme_config(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct theme_manager *manager = userdata;
    const char *config = json_object_to_json_string(manager->config->json);
    return sd_bus_reply_method_return(msg, "s", config);
}

static int set_widget_theme(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    char *theme = NULL;
    uint32_t type = THEME_TYPE_DEFAULT;
    CK(sd_bus_message_read(msg, "su", &theme, &type));
    bool ret = theme_manager_set_widget_theme(theme, type);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static int set_icon_theme(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    char *icon_theme_name = NULL;
    CK(sd_bus_message_read(msg, "s", &icon_theme_name));
    bool ret = theme_manager_set_icon_theme(icon_theme_name);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static int set_background(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    char *picture = NULL;
    uint32_t options = 0, color = 0;
    CK(sd_bus_message_read(msg, "suu", &picture, &options, &color));
    bool ret = theme_manager_set_background(picture, options, color);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static int set_font(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    char *font_name = NULL;
    int32_t size;
    CK(sd_bus_message_read(msg, "si", &font_name, &size));
    bool ret = theme_manager_set_font(font_name, size);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static int set_accent_color(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    int32_t accent_color;
    CK(sd_bus_message_read(msg, "i", &accent_color));
    bool ret = theme_manager_set_accent_color(accent_color);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static int set_corner_radius(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    int32_t window_radius, menu_radius;
    CK(sd_bus_message_read(msg, "ii", &window_radius, &menu_radius));
    bool ret = theme_manager_set_corner_radius(window_radius, menu_radius);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static int set_opacity(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    int32_t opacity;
    CK(sd_bus_message_read(msg, "i", &opacity));
    bool ret = theme_manager_set_opacity(opacity);
    return sd_bus_reply_method_return(msg, "b", ret);
}

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("PrintThemeConfig", "", "s", print_theme_config, 0),
    SD_BUS_METHOD("SetWidgetTheme", "su", "b", set_widget_theme, 0),
    SD_BUS_METHOD("SetIconTheme", "s", "b", set_icon_theme, 0),
    SD_BUS_METHOD("SetBackground", "suu", "b", set_background, 0),
    SD_BUS_METHOD("SetFont", "si", "b", set_font, 0),
    SD_BUS_METHOD("SetAccentColor", "i", "b", set_accent_color, 0),
    SD_BUS_METHOD("SetCornerRadius", "ii", "b", set_corner_radius, 0),
    SD_BUS_METHOD("SetOpacity", "i", "b", set_opacity, 0),
    SD_BUS_VTABLE_END,
};

bool theme_manager_config_init(struct theme_manager *manager)
{
    manager->config = config_manager_add_config("theme");
    if (!manager->config) {
        return false;
    }
    return dbus_register_object(NULL, service_path, service_interface, service_vtable, manager);
}

void theme_manager_read_config(struct theme_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return;
    }

    json_object *data;
    /* some global configs */
    if (json_object_object_get_ex(manager->config->json, "font_name", &data)) {
        free(manager->global.font_name);
        manager->global.font_name = strdup(json_object_get_string(data));
    }

    if (json_object_object_get_ex(manager->config->json, "font_size", &data)) {
        manager->global.font_size = json_object_get_int(data);
    }

    if (json_object_object_get_ex(manager->config->json, "accent_color", &data)) {
        manager->global.accent_color = json_object_get_int(data);
    }

    if (json_object_object_get_ex(manager->config->json, "window_radius", &data)) {
        manager->global.window_radius = json_object_get_int(data);
    }

    if (json_object_object_get_ex(manager->config->json, "menu_radius", &data)) {
        manager->global.menu_radius = json_object_get_int(data);
    }

    if (json_object_object_get_ex(manager->config->json, "opacity", &data)) {
        manager->global.opacity = json_object_get_int(data);
    }

    if (json_object_object_get_ex(manager->config->json, "type", &data)) {
        manager->global.type = json_object_get_int(data);
        return;
    }

    /* get system default config */
    if (manager->config->sys_json &&
        json_object_object_get_ex(manager->config->sys_json, "type", &data)) {
        manager->global.type = json_object_get_int(data);
    }
}

void theme_manager_write_config(struct theme_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return;
    }

    if (manager->global.type > THEME_TYPE_DEFAULT) {
        json_object_object_add(manager->config->json, "type",
                               json_object_new_int(manager->global.type));
    } else {
        json_object_object_del(manager->config->json, "type");
    }

    if (manager->global.font_name && strcmp(manager->global.font_name, "sans")) {
        json_object_object_add(manager->config->json, "font_name",
                               json_object_new_string(manager->global.font_name));
    } else {
        json_object_object_del(manager->config->json, "font_name");
    }

    if (manager->global.font_size != 11) {
        json_object_object_add(manager->config->json, "font_size",
                               json_object_new_int(manager->global.font_size));
    } else {
        json_object_object_del(manager->config->json, "font_size");
    }

    if (manager->global.accent_color >= 0) {
        json_object_object_add(manager->config->json, "accent_color",
                               json_object_new_int(manager->global.accent_color));
    } else {
        json_object_object_del(manager->config->json, "accent_color");
    }

    if (manager->global.window_radius != 12) {
        json_object_object_add(manager->config->json, "window_radius",
                               json_object_new_int(manager->global.window_radius));
    } else {
        json_object_object_del(manager->config->json, "window_radius");
    }

    if (manager->global.menu_radius != 8) {
        json_object_object_add(manager->config->json, "menu_radius",
                               json_object_new_int(manager->global.menu_radius));
    } else {
        json_object_object_del(manager->config->json, "menu_radius");
    }

    if (manager->global.opacity != 100) {
        json_object_object_add(manager->config->json, "opacity",
                               json_object_new_int(manager->global.opacity));
    } else {
        json_object_object_del(manager->config->json, "opacity");
    }
}

const char *theme_manager_read_icon_config(struct theme_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return NULL;
    }

    json_object *data;
    if (json_object_object_get_ex(manager->config->json, "icon_theme_name", &data)) {
        return json_object_get_string(data);
    }

    /* get system default config */
    if (manager->config->sys_json &&
        json_object_object_get_ex(manager->config->sys_json, "icon_theme_name", &data)) {
        return json_object_get_string(data);
    }

    return NULL;
}

void theme_manager_write_icon_config(struct theme_manager *manager, const char *name)
{
    if (!manager->config || !manager->config->json) {
        return;
    }

    if (strcmp(name, FALLBACK_ICON_THEME_NAME)) {
        json_object_object_add(manager->config->json, "icon_theme_name",
                               json_object_new_string(name));
    } else {
        json_object_object_del(manager->config->json, "icon_theme_name");
    }
}

const char *theme_manager_read_background_config(struct theme_manager *manager, uint32_t *options,
                                                 int32_t *color)
{
    if (!manager->config || !manager->config->json) {
        return NULL;
    }

    const char *picture = NULL;
    json_object *data;

    if (json_object_object_get_ex(manager->config->json, "background_picture", &data)) {
        picture = json_object_get_string(data);
    }
    if (json_object_object_get_ex(manager->config->json, "background_options", &data)) {
        *options = json_object_get_int(data);
    }
    if (json_object_object_get_ex(manager->config->json, "background_color", &data)) {
        *color = json_object_get_int(data);
    }

    return picture;
}

void theme_manager_write_background_config(struct theme_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return;
    }

    if (manager->background.picture) {
        json_object_object_add(manager->config->json, "background_picture",
                               json_object_new_string(manager->background.picture));
    } else {
        json_object_object_del(manager->config->json, "background_picture");
    }

    if (manager->background.options != 0) {
        json_object_object_add(manager->config->json, "background_options",
                               json_object_new_int(manager->background.options));
    } else {
        json_object_object_del(manager->config->json, "background_options");
    }

    if (manager->background.color >= 0) {
        json_object_object_add(manager->config->json, "background_color",
                               json_object_new_int(manager->background.color));
    } else {
        json_object_object_del(manager->config->json, "background_color");
    }
}
