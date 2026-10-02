// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "output_p.h"
#include "util/macros.h"

#define HASH_SIZE 33

static bool output_read_usd_config(struct output *output, json_object *config_json,
                                   struct kywc_output_state *state)
{
    if (!config_json) {
        return false;
    }

    json_object *config = json_object_object_get(config_json, output->base.prop.edid_hash);
    if (!config) {
        // try to get config form output name
        config = json_object_object_get(config_json, output->base.name);
    }

    if (!config) {
        kywc_log(KYWC_WARN, "%s config not found", output->base.name);
        return false;
    }

    json_object *data;
    if (json_object_object_get_ex(config, "enable", &data)) {
        state->enabled = state->power = json_object_get_boolean(data);
    }
    if (!state->enabled) {
        return true;
    }

    if (json_object_object_get_ex(config, "primary", &data)) {
        if (json_object_get_boolean(data)) {
            output_manager_set_pending_primary(output);
        }
    }
    if (json_object_object_get_ex(config, "width", &data)) {
        state->width = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "height", &data)) {
        state->height = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "refresh", &data)) {
        state->refresh = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "rotation", &data)) {
        state->transform = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "scale", &data)) {
        state->scale = json_object_get_double(data);
    }
    if (json_object_object_get_ex(config, "x", &data)) {
        state->lx = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "y", &data)) {
        state->ly = json_object_get_int(data);
    }

    return true;
}

static json_object *get_usd_layout_config_object(struct output_manager *output_manager)
{
    char layout_name[128];
    const char *path = "/etc/ukui/usd/config/screen/wlcom";
    snprintf(layout_name, sizeof(layout_name), "%s/%s", path, output_manager->outputs_layout);
    kywc_log(KYWC_DEBUG, "Sync outputs config from USD: %s", layout_name);
    json_object *config_json = json_object_from_file(layout_name);
    return config_json;
}

static int compare_modes(struct kywc_output_mode *mode1, struct kywc_output_mode *mode2)
{
    return (mode1->width != mode2->width) ? (mode1->width - mode2->width)
                                          : (mode1->height - mode2->height);
}

static bool is_mode_supported_for_other_outputs(struct kywc_output *current_output,
                                                struct kywc_output_mode *current_mode)
{
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback() || kywc_output == current_output) {
            continue;
        }

        struct kywc_output_mode *mode;
        wl_list_for_each(mode, &kywc_output->prop.modes, link) {
            if (mode->width == current_mode->width && mode->height == current_mode->height) {
                return true;
            }
        }
    }

    return false;
}

static float get_min_preferred_scale(struct kywc_output *current_output, int32_t width,
                                     int32_t height)
{
    float min_scale = output_preferred_scale(current_output, width, height);

    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback() || kywc_output == current_output) {
            continue;
        }

        float preferred_scale = output_preferred_scale(kywc_output, width, height);
        if (preferred_scale < min_scale) {
            min_scale = preferred_scale;
        }
    }

    return min_scale;
}

static int32_t output_get_mode_max_refresh(struct output *output, int width, int height)
{
    struct kywc_output *kywc_output = &output->base;
    struct kywc_output_mode *mode;
    int32_t max_refresh = 0;
    wl_list_for_each(mode, &kywc_output->prop.modes, link) {
        if (mode->width == width && mode->height == height && mode->refresh > max_refresh) {
            max_refresh = mode->refresh;
        }
    }

    return max_refresh;
}

static void output_fill_mirror_mode_state(struct output *output, int32_t width, int32_t height,
                                          float scale)
{
    struct kywc_output_state *pending = &output->pending_state;
    *pending = output->base.state;

    pending->enabled = pending->power = true;
    pending->width = width;
    pending->height = height;
    pending->refresh = output_get_mode_max_refresh(output, width, height);
    pending->scale = scale;

    pending->lx = pending->ly = 0;

    pending->transform = WL_OUTPUT_TRANSFORM_NORMAL;
    pending->brightness = pending->brightness == 0 ? 100 : CLAMP(pending->brightness, 10, 100);
    pending->color_temp = pending->color_temp == 0 ? 6500 : CLAMP(pending->color_temp, 1000, 25000);
}

static bool output_manager_add_mirror_mode_state(struct output_manager *output_manager)
{
    struct kywc_output *first_output = NULL;
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        first_output = kywc_output;
        break;
    }

    if (!first_output) {
        return false;
    }

    struct kywc_output_mode *mode, *max_mode = NULL;
    wl_list_for_each(mode, &first_output->prop.modes, link) {
        if (!is_mode_supported_for_other_outputs(first_output, mode)) {
            continue;
        }

        if (!max_mode || compare_modes(mode, max_mode) > 0) {
            max_mode = mode;
        }
    }

    if (!max_mode) {
        return false;
    }

    float scale = get_min_preferred_scale(first_output, max_mode->width, max_mode->height);
    kywc_log(KYWC_DEBUG, "Add mirror mode: %dx%d scale: %f", max_mode->width, max_mode->height,
             scale);

    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        output_fill_mirror_mode_state(output, max_mode->width, max_mode->height, scale);
    }

    return true;
}

struct output_hash {
    const char *name, *hash;
};

static int compare_output_hash(const void *p1, const void *p2)
{
    const char *v1 = ((struct output_hash *)p1)->hash;
    const char *v2 = ((struct output_hash *)p2)->hash;
    return strcmp(v1, v2);
}

static void output_manager_generate_usd_layout(struct output_manager *output_manager,
                                               char *layout_hash)
{
    struct output_hash *o_hashs = NULL;
    int actual_cnt = 0;
    size_t hash_chunk_size = HASH_SIZE - 1;

    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        actual_cnt++;
    }

    if (!actual_cnt) {
        return;
    }

    o_hashs = malloc(actual_cnt * sizeof(struct output_hash));
    if (!o_hashs) {
        return;
    }

    int index = 0;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        o_hashs[index].name = kywc_output->name;
        o_hashs[index].hash = kywc_output->prop.edid_hash;
        index++;
    }

    if (actual_cnt == 1) {
        const char *hash = kywc_identifier_md5_generate((void *)o_hashs[0].hash, hash_chunk_size);
        if (hash) {
            memcpy(layout_hash, hash, hash_chunk_size);
            layout_hash[hash_chunk_size] = '\0';
            free((void *)hash);
        }
        free(o_hashs);
        return;
    }

    qsort(o_hashs, actual_cnt, sizeof(struct output_hash), compare_output_hash);

    uint8_t *hashs = malloc(actual_cnt * hash_chunk_size);
    if (!hashs) {
        free(o_hashs);
        return;
    }

    for (int i = 0; i < actual_cnt; ++i) {
        memcpy(hashs + i * hash_chunk_size, o_hashs[i].hash, hash_chunk_size);
    }
    free(o_hashs);

    const char *hash = kywc_identifier_md5_generate(hashs, actual_cnt * hash_chunk_size);
    if (hash) {
        memcpy(layout_hash, hash, hash_chunk_size);
        layout_hash[hash_chunk_size] = '\0';
        free((void *)hash);
    }

    free(hashs);
}

static void output_manager_get_usd_configs(bool output_removed)
{
    if (output_manager->server->terminate || !output_manager->server->start ||
        output_manager->initial_configured) {
        return;
    }

    if (!output_manager_has_actual_outputs()) {
        struct output *fallback_output = output_from_kywc_output(output_manager_get_fallback());
        output_manager_add_pending_state(fallback_output, &fallback_output->pending_state);
        return;
    }

    bool fill_mirror_state = false;
    /* update current outputs layout */
    output_manager_generate_usd_layout(output_manager, output_manager->outputs_layout);
    json_object *usd_config = get_usd_layout_config_object(output_manager);
    if (!usd_config) {
        fill_mirror_state = output_manager_add_mirror_mode_state(output_manager);
        kywc_log(KYWC_WARN, "USD configuration missing try to fill mirror mode");
    } else {
        kywc_log(KYWC_INFO, "Configure outputs from USD layout %s", output_manager->outputs_layout);
    }

    /* get all outputs configuration */
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }

        if (!fill_mirror_state) {
            output_fill_preferred_state(output);
        }

        kywc_log(KYWC_INFO, "output %s edid_hash: %s", kywc_output->name,
                 kywc_output->prop.edid_hash);
        output_read_usd_config(output, usd_config, &output->pending_state);
        output_manager_add_pending_state(output, &output->pending_state);
    }

    if (usd_config) {
        json_object_put(usd_config);
    }
}

bool ukui_output_config_init(struct output_manager *output_manager)
{
    if (output_manager->has_layout_manager) {
        return false;
    }

    output_manager->impl.get_configures = output_manager_get_usd_configs;
    return true;
}
