// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <kywc/binding.h>

#include "config.h"
#include "effect_p.h"
#include "output.h"
#include "util/dbus.h"
#include "util/macros.h"

static const char *color_filter_dbus_interface = "org.ukui.KWin";
static const char *color_filter_path = "/ColorFilter";

static struct color_filter_effect {
    struct effect *effect;
    struct config *config;
    struct effect_manager *manager;

    bool enabled;
    bool shortcut_enabled;
    enum kywc_output_color_filter type;

    struct wl_listener destroy;
    struct wl_listener new_output;
    struct wl_listener effect_enabled;
    struct wl_listener effect_disabled;
} *color_filter = NULL;

static const char *color_filters[] = { "None",       "GrayScale",    "Invert",
                                       "Protanopia", "Deuteranopia", "Tritanopia" };

static enum kywc_output_color_filter get_color_filter_by_name(const char *name)
{
    for (uint32_t index = 0; index < ARRAY_SIZE(color_filters); ++index) {
        if (strcmp(name, color_filters[index]) == 0) {
            return index;
        }
    }

    return KYWC_OUTPUT_COLOR_FILTER_NONE;
}

static bool update_output_color_filter(struct kywc_output *kywc_output, int index, void *data)
{
    struct color_filter_effect *color_filter = data;
    if (!color_filter->effect->enabled || !color_filter->enabled) {
        output_set_color_filter(kywc_output, KYWC_OUTPUT_COLOR_FILTER_NONE);
    } else {
        output_set_color_filter(kywc_output, color_filter->type);
    }

    return false;
}

static void effect_update_color_filter(struct color_filter_effect *color_filter,
                                       bool status_changed, bool type_changed)
{
    output_manager_for_each_output(update_output_color_filter, true, color_filter);

    if (status_changed) {
        dbus_emit_signal("/ColorFilter", "org.ukui.colorfilter", "statusChanged", "b",
                         color_filter->effect->enabled && color_filter->enabled);
        effect_set_option_boolean(color_filter->effect, "color_filter_enabled",
                                  color_filter->enabled);
    }

    if (type_changed) {
        dbus_emit_signal("/ColorFilter", "org.ukui.colorfilter", "colorFilterTypeChanged", "s",
                         color_filters[color_filter->type]);
        effect_set_option_int(color_filter->effect, "type", color_filter->type);
    }
}

static void effect_update_shortcut_enabled(struct color_filter_effect *color_filter)
{
    dbus_emit_signal("/ColorFilter", "org.ukui.colorfilter", "colorFilterShortCutEnabledChanged",
                     "b", color_filter->shortcut_enabled);
    effect_set_option_boolean(color_filter->effect, "shortcut_enabled",
                              color_filter->shortcut_enabled);
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    return false;
}

static int color_filter_set(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct color_filter_effect *color_filter = userdata;

    if (!(color_filter->effect->enabled && color_filter->enabled)) {
        return sd_bus_reply_method_return(m, NULL);
    }

    const char *str_type = NULL;
    CK(sd_bus_message_read(m, "s", &str_type));

    enum kywc_output_color_filter type = get_color_filter_by_name(str_type);
    if (type == KYWC_OUTPUT_COLOR_FILTER_NONE || color_filter->type == type) {
        return sd_bus_reply_method_return(m, NULL);
    }
    color_filter->type = type;

    effect_update_color_filter(color_filter, false, true);

    return sd_bus_reply_method_return(m, NULL);
}

static int color_filter_get_current(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct color_filter_effect *color_filter = userdata;
    return sd_bus_reply_method_return(m, "s", color_filters[color_filter->type]);
}

static int color_filter_set_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct color_filter_effect *color_filter = userdata;
    if (!color_filter->effect->enabled || color_filter->enabled) {
        return sd_bus_reply_method_return(m, NULL);
    }

    color_filter->enabled = true;
    effect_update_color_filter(color_filter, true, false);

    return sd_bus_reply_method_return(m, NULL);
}

static int color_filter_set_disabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct color_filter_effect *color_filter = userdata;
    if (!color_filter->effect->enabled || !color_filter->enabled) {
        return sd_bus_reply_method_return(m, NULL);
    }

    color_filter->enabled = false;
    effect_update_color_filter(color_filter, true, false);

    return sd_bus_reply_method_return(m, NULL);
}

static int color_filter_is_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct color_filter_effect *color_filter = userdata;

    bool enabled = color_filter->effect->enabled ? color_filter->enabled : false;
    return sd_bus_reply_method_return(m, "b", enabled);
}

static int color_filter_shortcut_set_enabled(sd_bus_message *m, void *userdata,
                                             sd_bus_error *ret_error)
{
    int enabled;
    CK(sd_bus_message_read(m, "b", &enabled));

    struct color_filter_effect *color_filter = userdata;
    if (enabled == color_filter->shortcut_enabled) {
        return sd_bus_reply_method_return(m, NULL);
    }

    color_filter->shortcut_enabled = enabled;
    effect_update_shortcut_enabled(color_filter);

    return sd_bus_reply_method_return(m, NULL);
}

static int color_filter_shortcut_is_enabled(sd_bus_message *m, void *userdata,
                                            sd_bus_error *ret_error)
{
    struct color_filter_effect *color_filter = userdata;
    return sd_bus_reply_method_return(m, "b", color_filter->shortcut_enabled);
}

static int color_filter_get_available(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "s"));

    for (uint32_t i = 1; i < ARRAY_SIZE(color_filters); ++i) {
        CK(sd_bus_message_append(reply, "s", color_filters[i]));
    }

    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);

    return 0;
}

static const sd_bus_vtable color_filter_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("availableColorFilter", "", "as", color_filter_get_available, 0),
    SD_BUS_METHOD("setColorFilter", "s", "", color_filter_set, 0),
    SD_BUS_METHOD("getCurrentColorFilter", "", "s", color_filter_get_current, 0),
    SD_BUS_METHOD("openColorFilter", "", "", color_filter_set_enabled, 0),
    SD_BUS_METHOD("closeColorFilter", "", "", color_filter_set_disabled, 0),
    SD_BUS_METHOD("isColorFilterEnabled", "", "b", color_filter_is_enabled, 0),
    SD_BUS_METHOD("isColorFilterShortCutEnabled", "", "b", color_filter_shortcut_is_enabled, 0),
    SD_BUS_METHOD("setColorFilterShortCutEnabled", "b", "", color_filter_shortcut_set_enabled, 0),
    SD_BUS_VTABLE_END,
};

static void load_color_filter_config(struct color_filter_effect *color_filter)
{
    color_filter->type = KYWC_OUTPUT_COLOR_FILTER_PROTANOPIA;

    enum kywc_output_color_filter type =
        effect_get_option_int(color_filter->effect, "type", KYWC_OUTPUT_COLOR_FILTER_PROTANOPIA);
    if (type != KYWC_OUTPUT_COLOR_FILTER_NONE) {
        color_filter->type = type;
    }

    color_filter->enabled =
        effect_get_option_boolean(color_filter->effect, "color_filter_enabled", false);
    color_filter->shortcut_enabled =
        effect_get_option_boolean(color_filter->effect, "shortcut_enabled", false);
}

void effect_color_filter_toggle(void)
{
    if (!color_filter || !color_filter->effect->enabled || !color_filter->shortcut_enabled) {
        return;
    }

    color_filter->enabled = !color_filter->enabled;
    effect_update_color_filter(color_filter, true, false);
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct color_filter_effect *effect = wl_container_of(listener, effect, destroy);

    wl_list_remove(&effect->destroy.link);
    wl_list_remove(&effect->effect_enabled.link);
    wl_list_remove(&effect->effect_disabled.link);
    wl_list_remove(&effect->new_output.link);

    free(effect);
    color_filter = NULL;
}

static void handle_effect_enabled(struct wl_listener *listener, void *data)
{
    struct color_filter_effect *color_filter =
        wl_container_of(listener, color_filter, effect_enabled);

    output_manager_add_new_enabled_listener(&color_filter->new_output);
    effect_update_color_filter(color_filter, true, false);
}

static void handle_effect_disabled(struct wl_listener *listener, void *data)
{
    struct color_filter_effect *color_filter =
        wl_container_of(listener, color_filter, effect_disabled);

    wl_list_remove(&color_filter->new_output.link);
    wl_list_init(&color_filter->new_output.link);

    effect_update_color_filter(color_filter, true, false);
}

static void color_filter_handle_new_output(struct wl_listener *listener, void *data)
{
    struct color_filter_effect *color_filter = wl_container_of(listener, color_filter, new_output);

    struct kywc_output *output = data;
    update_output_color_filter(output, 0, color_filter);
}

static const struct effect_interface effect_impl = {
    .configure = handle_effect_configure,
};

bool color_filter_effect_create(struct effect_manager *manager)
{
    color_filter = calloc(1, sizeof(*color_filter));
    if (!color_filter) {
        return false;
    }

    color_filter->effect = effect_create("color_filter", 200, true, &effect_impl, color_filter);
    if (!color_filter->effect) {
        free(color_filter);
        return false;
    }

    color_filter->manager = manager;

    load_color_filter_config(color_filter);

    dbus_register_object(NULL, color_filter_path, color_filter_dbus_interface, color_filter_vtable,
                         color_filter);

    struct key_binding *binding =
        kywc_key_binding_create("Win+Ctrl+c", "colorfilter_toggle", FUNC_ID_TOGGLE_COLOR_FILTER, 0);
    if (binding && !kywc_key_binding_register(binding, KEY_BINDING_TYPE_COLOR_FILTER,
                                              function_manager_handle_key_binding, NULL)) {
        kywc_key_binding_destroy(binding);
    }

    wl_list_init(&color_filter->new_output.link);
    color_filter->new_output.notify = color_filter_handle_new_output;

    color_filter->destroy.notify = handle_effect_destroy;
    wl_signal_add(&color_filter->effect->events.destroy, &color_filter->destroy);
    color_filter->effect_enabled.notify = handle_effect_enabled;
    wl_signal_add(&color_filter->effect->events.enable, &color_filter->effect_enabled);
    color_filter->effect_disabled.notify = handle_effect_disabled;
    wl_signal_add(&color_filter->effect->events.disable, &color_filter->effect_disabled);

    if (color_filter->effect->enabled) {
        handle_effect_enabled(&color_filter->effect_enabled, NULL);
    }

    return true;
}
