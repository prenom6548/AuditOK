// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/types/wlr_buffer.h>

#include <kywc/identifier.h>

#include "effect_p.h"
#include "output.h"
#include "painter.h"
#include "util/dbus.h"
#include "util/file.h"
#include "util/string.h"
#include "view/view.h"

enum watermark_type {
    WATERMARK_TYPE_POSITION = 0,
    WATERMARK_TYPE_REPEAT,
    WATERMARK_TYPE_SCREEN,
    WATERMARK_TYPE_SCREEN_RATIO,
    WATERMARK_TYPE_LOCATION,
    WATERMARK_TYPE_LOCATION_PRIMARY,
    WATERMARK_TYPE_LAST,
};

struct watermark_info {
    char *file;
    double opacity;
    union {
        struct {
            uint32_t left : 1;
            uint32_t right : 1;
            uint32_t left_dist : 15;
            uint32_t right_dist : 15;
        };
        int x;
    };
    union {
        struct {
            uint32_t top : 1;
            uint32_t bottom : 1;
            uint32_t top_dist : 15;
            uint32_t bottom_dist : 15;
        };
        int y;
    };
    enum watermark_type type;
    bool topmost;
};

struct watermark_entry {
    struct wl_list link;
    struct watermark *watermark;

    struct scene_buffer *scene_buffer;

    struct output *output;
    struct wl_listener output_geometry;
    struct wl_listener output_disable;
};

struct watermark {
    struct wl_list link;
    const char *name;
    struct watermark_effect *effect;

    struct wl_list entries;    // per output
    struct wlr_buffer *buffer; // from image file
    struct wl_listener new_enabled_output;
    struct wl_listener primary_output;

    struct scene_tree *tree;
    struct watermark_info info;
};

struct watermark_effect {
    struct effect *effect;
    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    struct effect_manager *manager;
    struct dbus_object *dbus; // for dbus
    struct dbus_object *dbus_v2;

    struct watermark watermark; // default
    struct wl_list watermarks;
};

static struct wlr_buffer *watermark_get_or_create_buffer(struct watermark *watermark)
{
    if (watermark->buffer) {
        return watermark->buffer;
    }

    struct draw_info info = { .image = watermark->info.file };
    struct wlr_buffer *buffer = painter_draw_buffer(&info);
    if (!buffer) {
        return NULL;
    }

    watermark->buffer = buffer;
    return buffer;
}

static void watermark_entry_destroy(struct watermark_entry *entry)
{
    wl_list_remove(&entry->link);
    wl_list_remove(&entry->output_disable.link);
    wl_list_remove(&entry->output_geometry.link);
    scene_node_destroy(&entry->scene_buffer->node);
    free(entry);
}

static void watermark_entry_handle_output_disable(struct wl_listener *listener, void *data)
{
    struct watermark_entry *entry = wl_container_of(listener, entry, output_disable);
    watermark_entry_destroy(entry);
}

static void watermark_entry_update_buffer(struct watermark_entry *entry)
{
    struct watermark *watermark = entry->watermark;
    struct wlr_buffer *buffer = watermark_get_or_create_buffer(watermark);
    if (!buffer) {
        return;
    }

    if (entry->scene_buffer->buffer != buffer) {
        scene_buffer_set_buffer(entry->scene_buffer, buffer);
    }

    struct kywc_box geo = { 0 };
    if (entry->output) {
        geo = entry->output->geometry;
    }
    int x = geo.x, y = geo.y;
    int width = buffer->width;
    int height = buffer->height;

    switch (watermark->info.type) {
    case WATERMARK_TYPE_POSITION:
        x = watermark->info.x;
        y = watermark->info.y;
        break;
    case WATERMARK_TYPE_REPEAT:
        scene_buffer_set_repeat_mode(entry->scene_buffer, RENDER_TEXTURE_REPEAT_NORAML);
        /* fallthrough to screen */
    case WATERMARK_TYPE_SCREEN:
        width = geo.width;
        height = geo.height;
        break;
    case WATERMARK_TYPE_SCREEN_RATIO:;
        float output_ratio = (float)geo.width / geo.height;
        float buffer_ratio = (float)width / height;
        if (output_ratio < buffer_ratio) {
            width = geo.width;
            height = geo.width / buffer_ratio;
            y += (geo.height - height) / 2;
        } else {
            height = geo.height;
            width = geo.height * buffer_ratio;
            x += (geo.width - width) / 2;
        }
        break;
    case WATERMARK_TYPE_LOCATION:
    case WATERMARK_TYPE_LOCATION_PRIMARY:
        if (watermark->info.left && watermark->info.right) {
            x += watermark->info.left_dist;
            width = geo.width - watermark->info.left_dist - watermark->info.right_dist;
        } else if (watermark->info.left) {
            x += watermark->info.left_dist;
        } else if (watermark->info.right) {
            x += geo.width - width - watermark->info.right_dist;
        } else {
            x += (geo.width - width) / 2;
        }
        if (watermark->info.top && watermark->info.bottom) {
            y += watermark->info.top_dist;
            height = geo.height - watermark->info.top_dist - watermark->info.bottom_dist;
        } else if (watermark->info.top) {
            y += watermark->info.top_dist;
        } else if (watermark->info.bottom) {
            y += geo.height - height - watermark->info.bottom_dist;
        } else {
            y += (geo.height - height) / 2;
        }
        if (width < 0 || width > geo.width) {
            width = geo.width;
        }
        if (height < 0 || height > geo.height) {
            height = geo.height;
        }
        if (x < geo.x || x + width > geo.x + geo.width) {
            x = geo.x;
        }
        if (y < geo.y || y + height > geo.y + geo.height) {
            y = geo.y;
        }
        break;
    default:
        break;
    }

    scene_node_set_position(&entry->scene_buffer->node, x, y);
    scene_buffer_set_dest_size(entry->scene_buffer, width, height);
}

static void watermark_entry_handle_output_geometry(struct wl_listener *listener, void *data)
{
    struct watermark_entry *entry = wl_container_of(listener, entry, output_geometry);
    /*  update buffer when output geometry changed */
    watermark_entry_update_buffer(entry);
}

static bool watermark_entry_create(struct watermark *watermark, struct output *output)
{
    struct watermark_entry *entry = calloc(1, sizeof(*entry));
    if (!entry) {
        return false;
    }

    entry->watermark = watermark;
    wl_list_insert(&watermark->entries, &entry->link);

    entry->output = output;
    entry->output_disable.notify = watermark_entry_handle_output_disable;
    entry->output_geometry.notify = watermark_entry_handle_output_geometry;
    if (output) {
        wl_signal_add(&output->events.disable, &entry->output_disable);
        wl_signal_add(&output->events.geometry, &entry->output_geometry);
    } else {
        wl_list_init(&entry->output_disable.link);
        wl_list_init(&entry->output_geometry.link);
    }

    entry->scene_buffer = scene_buffer_create(watermark->tree, NULL);
    scene_node_lower_to_bottom(&entry->scene_buffer->node);
    scene_buffer_set_opacity(entry->scene_buffer, watermark->info.opacity);
    scene_node_set_input_bypassed(&entry->scene_buffer->node, true);

    watermark_entry_update_buffer(entry);

    return true;
}

static void watermark_destroy_entries(struct watermark *watermark)
{
    wl_list_remove(&watermark->new_enabled_output.link);
    wl_list_init(&watermark->new_enabled_output.link);
    wl_list_remove(&watermark->primary_output.link);
    wl_list_init(&watermark->primary_output.link);

    struct watermark_entry *entry, *tmp;
    wl_list_for_each_safe(entry, tmp, &watermark->entries, link) {
        watermark_entry_destroy(entry);
    }

    wlr_buffer_drop(watermark->buffer);
    watermark->buffer = NULL;

    free(watermark->info.file);
    watermark->info.file = NULL;
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct watermark *watermark = wl_container_of(listener, watermark, new_enabled_output);
    struct kywc_output *output = data;
    watermark_entry_create(watermark, output_from_kywc_output(output));
}

static void handle_primary_output(struct wl_listener *listener, void *data)
{
    struct watermark *watermark = wl_container_of(listener, watermark, primary_output);
    struct kywc_output *output = data;

    struct watermark_entry *entry, *tmp;
    wl_list_for_each_safe(entry, tmp, &watermark->entries, link) {
        watermark_entry_destroy(entry);
    }

    watermark_entry_create(watermark, output_from_kywc_output(output));
}

static bool watermark_create_entries(struct watermark *watermark, struct watermark_info *info)
{
    char *file = string_expand_path(info->file);
    if (!file || !file_exists(file)) {
        free(file);
        return false;
    }

    if (info->opacity < 0 || info->opacity > 1 || info->type < 0 ||
        info->type >= WATERMARK_TYPE_LAST) {
        return false;
    }

    watermark->info = *info;
    watermark->info.file = file;

    enum layer layer = watermark->info.topmost ? LAYER_SCREEN_LOCK : LAYER_WATERMARK;
    struct view_layer *view_layer = view_manager_get_layer(layer, false);
    watermark->tree = view_layer->tree;

    /* no need to use output when WATERMARK_TYPE_POSITION */
    if (info->type == WATERMARK_TYPE_POSITION) {
        watermark_entry_create(watermark, NULL);
        return true;
    }

    if (info->type == WATERMARK_TYPE_LOCATION_PRIMARY) {
        struct kywc_output *kywc_output = kywc_output_get_primary();
        assert(kywc_output);
        watermark_entry_create(watermark, output_from_kywc_output(kywc_output));
        kywc_output_add_primary_listener(&watermark->primary_output);
        return true;
    }

    struct scene *scene = watermark->effect->manager->server->scene;
    struct scene_output *output;
    wl_list_for_each(output, &scene->outputs, link) {
        watermark_entry_create(watermark, output_from_wlr_output(output->output));
    }

    output_manager_add_new_enabled_listener(&watermark->new_enabled_output);
    return true;
}

static void watermark_init(struct watermark *watermark, struct watermark_effect *effect)
{
    watermark->effect = effect;
    wl_list_init(&watermark->entries);
    wl_list_init(&watermark->new_enabled_output.link);
    wl_list_init(&watermark->primary_output.link);
    watermark->new_enabled_output.notify = handle_new_enabled_output;
    watermark->primary_output.notify = handle_primary_output;
}

static struct watermark *watermark_create(struct watermark_effect *effect)
{
    struct watermark *watermark = calloc(1, sizeof(*watermark));
    if (!watermark) {
        return NULL;
    }

    watermark_init(watermark, effect);
    wl_list_insert(&effect->watermarks, &watermark->link);
    watermark->name = kywc_identifier_uuid_generate();

    return watermark;
}

static void watermark_destroy(struct watermark *watermark)
{
    watermark_destroy_entries(watermark);
    wl_list_remove(&watermark->link);
    free((void *)watermark->name);
    free(watermark);
}

static struct watermark *watermark_by_name(struct watermark_effect *effect, const char *name)
{
    struct watermark *watermark;
    wl_list_for_each(watermark, &effect->watermarks, link) {
        if (strcmp(watermark->name, name) == 0) {
            return watermark;
        }
    }
    return NULL;
}

static int watermark_update(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *file = NULL;
    double opacity = 0;
    CK(sd_bus_message_read(m, "sd", &file, &opacity));

    struct watermark_effect *effect = userdata;
    watermark_destroy_entries(&effect->watermark);

    if (file && *file) {
        struct watermark_info info = {
            .file = file,
            .opacity = opacity,
            .type = WATERMARK_TYPE_SCREEN,
            .topmost = true,
        };
        if (!watermark_create_entries(&effect->watermark, &info)) {
            const sd_bus_error error =
                SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid args.");
            return sd_bus_reply_method_error(m, &error);
        }
    }

    return sd_bus_reply_method_return(m, NULL);
}

static int watermark_update_ex(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *file = NULL;
    double opacity = 0;
    int x, y, type, topmost;
    CK(sd_bus_message_read(m, "sdiiib", &file, &opacity, &x, &y, &type, &topmost));

    struct watermark_effect *effect = userdata;
    /* destroy current watermark entries, create new entries */
    watermark_destroy_entries(&effect->watermark);

    if (file && *file) {
        struct watermark_info info = {
            .file = file,
            .opacity = opacity,
            .x = x,
            .y = y,
            .type = type,
            .topmost = topmost,
        };
        if (!watermark_create_entries(&effect->watermark, &info)) {
            const sd_bus_error error =
                SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid args.");
            return sd_bus_reply_method_error(m, &error);
        }
    }

    return sd_bus_reply_method_return(m, NULL);
}

static const sd_bus_vtable watermark_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("updateWatermark", "sd", "", watermark_update, 0),
    SD_BUS_METHOD("updateWatermarkEx", "sdiiib", "", watermark_update_ex, 0),
    SD_BUS_VTABLE_END,
};

static int watermark2_create(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *file = NULL;
    double opacity = 0;
    int x, y, type, topmost;
    CK(sd_bus_message_read(m, "sdiiib", &file, &opacity, &x, &y, &type, &topmost));

    struct watermark_effect *effect = userdata;
    struct watermark *watermark = watermark_create(effect);
    if (!watermark) {
        return sd_bus_reply_method_return(m, "s", NULL);
    }

    if (file && *file) {
        struct watermark_info info = {
            .file = file,
            .opacity = opacity,
            .x = x,
            .y = y,
            .type = type,
            .topmost = topmost,
        };
        if (!watermark_create_entries(watermark, &info)) {
            const sd_bus_error error =
                SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid args.");
            return sd_bus_reply_method_error(m, &error);
        }
    }

    return sd_bus_reply_method_return(m, "s", watermark->name);
}

static int watermark2_update(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL, *file = NULL;
    double opacity = 0;
    int x, y, type, topmost;
    CK(sd_bus_message_read(m, "ssdiiib", &name, &file, &opacity, &x, &y, &type, &topmost));

    struct watermark_effect *effect = userdata;
    struct watermark *watermark = watermark_by_name(effect, name);
    if (!watermark) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    /* destroy current watermark entries, create new entries */
    watermark_destroy_entries(watermark);

    if (file && *file) {
        struct watermark_info info = {
            .file = file,
            .opacity = opacity,
            .x = x,
            .y = y,
            .type = type,
            .topmost = topmost,
        };
        if (!watermark_create_entries(watermark, &info)) {
            const sd_bus_error error =
                SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid args.");
            return sd_bus_reply_method_error(m, &error);
        }
    }

    return sd_bus_reply_method_return(m, "b", true);
}

static int watermark2_destroy(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL;
    CK(sd_bus_message_read(m, "s", &name));

    struct watermark_effect *effect = userdata;
    struct watermark *watermark = watermark_by_name(effect, name);
    if (!watermark) {
        return sd_bus_reply_method_return(m, NULL);
    }

    watermark_destroy(watermark);

    return sd_bus_reply_method_return(m, NULL);
}

static const sd_bus_vtable watermark2_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("createWatermark", "sdiiib", "s", watermark2_create, 0),
    SD_BUS_METHOD("updateWatermark", "ssdiiib", "b", watermark2_update, 0),
    SD_BUS_METHOD("destroyWatermark", "s", "", watermark2_destroy, 0),
    SD_BUS_VTABLE_END,
};

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    struct watermark_effect *effect = wl_container_of(listener, effect, enable);
    assert(wl_list_empty(&effect->watermarks));
    effect->dbus = dbus_register_object("org.ukui.KWin", "/Watermark", "org.ukui.kwin.Watermark",
                                        watermark_vtable, effect);
    effect->dbus_v2 = dbus_register_object(NULL, "/com/kylin/Wlcom/Watermark",
                                           "com.kylin.Wlcom.Watermark", watermark2_vtable, effect);
}

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    struct watermark_effect *effect = wl_container_of(listener, effect, disable);
    watermark_destroy_entries(&effect->watermark);
    struct watermark *watermark, *tmp;
    wl_list_for_each_safe(watermark, tmp, &effect->watermarks, link) {
        watermark_destroy(watermark);
    }
    dbus_unregister_object(effect->dbus);
    dbus_unregister_object(effect->dbus_v2);
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct watermark_effect *effect = wl_container_of(listener, effect, destroy);
    assert(wl_list_empty(&effect->watermarks));
    wl_list_remove(&effect->destroy.link);
    wl_list_remove(&effect->enable.link);
    wl_list_remove(&effect->disable.link);
    free(effect);
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    return false;
}

static bool handle_allow_enabled(struct effect *effect, bool enabled)
{
    return false;
}

static const struct effect_interface watermark_effect_impl = {
    .configure = handle_effect_configure,
    .allow_enabled = handle_allow_enabled,
};

bool watermark_effect_create(struct effect_manager *manager)
{
    struct watermark_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return false;
    }

    /* enabled by default */
    effect->effect = effect_create("watermark", 0, true, &watermark_effect_impl, NULL);
    if (!effect->effect) {
        free(effect);
        return false;
    }

    effect->manager = manager;
    wl_list_init(&effect->watermarks);
    effect->effect->category = EFFECT_CATEGORY_UTILS;
    watermark_init(&effect->watermark, effect);

    effect->enable.notify = handle_effect_enable;
    wl_signal_add(&effect->effect->events.enable, &effect->enable);
    effect->disable.notify = handle_effect_disable;
    wl_signal_add(&effect->effect->events.disable, &effect->disable);
    effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&effect->effect->events.destroy, &effect->destroy);

    if (effect->effect->enabled) {
        handle_effect_enable(&effect->enable, NULL);
    }

    return true;
}
