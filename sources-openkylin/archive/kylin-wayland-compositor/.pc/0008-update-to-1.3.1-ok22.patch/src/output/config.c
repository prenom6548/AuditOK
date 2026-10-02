// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "config.h"
#include "output_p.h"
#include "util/dbus.h"

static const char *service_path = "/com/kylin/Wlcom/Output";
static const char *service_interface = "com.kylin.Wlcom.Output";

static int list_outputs(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct output_manager *om = userdata;

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "(ss)"));

    struct output *output;
    wl_list_for_each(output, &om->outputs, link) {
        if (output->base.prop.backend != KYWC_OUTPUT_BACKEND_DRM) {
            continue;
        }
        json_object *config = json_object_object_get(om->config->json, output->base.name);
        const char *cfg = json_object_to_json_string(config);
        sd_bus_message_append(reply, "(ss)", output->base.name, cfg);
    }

    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

static int set_brightness(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL;
    uint32_t value = 0;
    CK(sd_bus_message_read(m, "su", &name, &value));

    struct kywc_output *kywc_output = kywc_output_by_name(name);
    if (kywc_output) {
        output_set_brightness(kywc_output, value);
    }

    return sd_bus_reply_method_return(m, NULL);
}

static int set_colortemp(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL;
    uint32_t value = 0;
    CK(sd_bus_message_read(m, "su", &name, &value));

    struct kywc_output *kywc_output = kywc_output_by_name(name);
    if (kywc_output) {
        output_set_color_temp(kywc_output, value);
    }
    return sd_bus_reply_method_return(m, NULL);
}

static int set_direct_scanout(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL;
    int value = 0;
    CK(sd_bus_message_read(m, "sb", &name, &value));

    struct kywc_output *kywc_output = kywc_output_by_name(name);
    if (kywc_output) {
        struct output *output = output_from_kywc_output(kywc_output);
        if (output->scene_output) {
            output->scene_output->direct_scanout = !!value;
        }
    }
    return sd_bus_reply_method_return(m, NULL);
}

static int set_color_filter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL;
    uint32_t type;
    CK(sd_bus_message_read(m, "su", &name, &type));

    struct kywc_output *kywc_output = kywc_output_by_name(name);
    if (kywc_output) {
        output_set_color_filter(kywc_output, type);
    }
    return sd_bus_reply_method_return(m, NULL);
}

static int set_color_feature(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *name = NULL;
    uint32_t color_feature;
    CK(sd_bus_message_read(m, "su", &name, &color_feature));

    struct kywc_output *kywc_output = kywc_output_by_name(name);
    if (kywc_output) {
        output_set_color_feature(kywc_output, color_feature);
    }
    return sd_bus_reply_method_return(m, NULL);
}

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ListAllOutputs", "", "a(ss)", list_outputs, 0),
    SD_BUS_METHOD("SetBrightness", "su", "", set_brightness, 0),
    SD_BUS_METHOD("SetColortemp", "su", "", set_colortemp, 0),
    SD_BUS_METHOD("SetDirectScanout", "sb", "", set_direct_scanout, 0),
    SD_BUS_METHOD("SetColorFilter", "su", "", set_color_filter, 0),
    SD_BUS_METHOD("SetColorFeature", "su", "", set_color_feature, 0),
    SD_BUS_VTABLE_END,
};

bool output_read_config(struct output *output, struct kywc_output_state *state)
{
    struct output_manager *om = output->manager;
    if (!om->config || !om->config->json) {
        return false;
    }

    /* get output in layout */
    json_object *config = json_object_object_get(om->config->json, output->base.name);
    if (!config) {
        return false;
    }

    /* finally, get all output config */
    json_object *data;

    if (!json_object_object_get_ex(config, "uuid", &data)) {
        return false;
    }

    const char *uuid = json_object_get_string(data);
    if (!uuid || strcmp(uuid, output->base.uuid)) {
        return false;
    }

    if (json_object_object_get_ex(config, "enabled", &data)) {
        state->power = state->enabled = json_object_get_boolean(data);
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
    if (json_object_object_get_ex(config, "transform", &data)) {
        state->transform = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "scale", &data)) {
        state->scale = json_object_get_double(data);
    }
    if (json_object_object_get_ex(config, "lx", &data)) {
        state->lx = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "ly", &data)) {
        state->ly = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "brightness", &data)) {
        state->brightness = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "color_temp", &data)) {
        state->color_temp = json_object_get_int(data);
    }

    return true;
}

void output_write_config(struct output *output)
{
    struct output_manager *om = output->manager;
    if (!om->config || !om->config->json || output->base.prop.backend != KYWC_OUTPUT_BACKEND_DRM) {
        return;
    }

    struct kywc_output_state *state = &output->base.state;
    /* get output in layout, create if no */
    json_object *config = json_object_object_get(om->config->json, output->base.name);
    if (!config) {
        config = json_object_new_object();
        json_object_object_add(om->config->json, output->base.name, config);
    }

    json_object_object_add(config, "uuid", json_object_new_string(output->base.uuid));
    json_object_object_add(config, "enabled", json_object_new_boolean(state->enabled));
    json_object_object_add(config, "width", json_object_new_int(state->width));
    json_object_object_add(config, "height", json_object_new_int(state->height));
    json_object_object_add(config, "refresh", json_object_new_int(state->refresh));
    json_object_object_add(config, "scale", json_object_new_double(state->scale));
    json_object_object_add(config, "transform", json_object_new_int(state->transform));
    json_object_object_add(config, "lx", json_object_new_int(state->lx));
    json_object_object_add(config, "ly", json_object_new_int(state->ly));
    json_object_object_add(config, "brightness", json_object_new_int(state->brightness));
    json_object_object_add(config, "color_temp", json_object_new_int(state->color_temp));
}

struct output_uuid {
    const char *name, *uuid;
};

static void output_get_layout(struct output *output, const char *active_layout,
                              char layout[static UUID_SIZE * 2])
{
    memcpy(layout, active_layout, UUID_SIZE - 1);
    layout[UUID_SIZE - 1] = ':';
    memcpy(layout + UUID_SIZE, output->base.uuid, UUID_SIZE - 1);
    layout[UUID_SIZE * 2 - 1] = '\0';
}

static const char *output_manager_get_active_layout(struct output_manager *output_manager)
{
    struct output_manager *om = output_manager;
    if (!om->layout_config || !om->layout_config->json) {
        return NULL;
    }

    /* get outputs_layout from layouts */
    json_object *outputs_layout =
        json_object_object_get(om->layout_config->json, om->outputs_layout);
    if (!outputs_layout) {
        return NULL;
    }

    json_object *data;
    /* get active_layout from outputs_layout */
    if (json_object_object_get_ex(outputs_layout, "active_layout", &data)) {
        return json_object_get_string(data);
    }

    return NULL;
}

static void output_manager_set_active_layout(struct output_manager *manager,
                                             const char *active_layout)
{
    if (!manager->layout_config || !manager->layout_config->json || !active_layout) {
        return;
    }

    /* get outputs_layout in layouts, create if no */
    json_object *outputs_layout =
        json_object_object_get(manager->layout_config->json, manager->outputs_layout);
    if (!outputs_layout) {
        outputs_layout = json_object_new_object();
        json_object_object_add(manager->layout_config->json, manager->outputs_layout,
                               outputs_layout);
    }

    json_object_object_add(outputs_layout, "active_layout", json_object_new_string(active_layout));
}

static bool output_read_layout_config(struct output *output, struct kywc_output_state *state,
                                      const char *active_layout)
{
    struct output_manager *om = output->manager;
    if (!om->layout_config || !om->layout_config->json || !active_layout) {
        return false;
    }

    char layout[UUID_SIZE * 2];
    output_get_layout(output, active_layout, layout);
    json_object *config = json_object_object_get(om->layout_config->json, layout);
    if (!config) {
        return false;
    }

    json_object *data;
    if (json_object_object_get_ex(config, "enabled", &data)) {
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
    if (json_object_object_get_ex(config, "transform", &data)) {
        state->transform = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "scale", &data)) {
        state->scale = json_object_get_double(data);
    }
    if (json_object_object_get_ex(config, "lx", &data)) {
        state->lx = json_object_get_int(data);
    }
    if (json_object_object_get_ex(config, "ly", &data)) {
        state->ly = json_object_get_int(data);
    }
    return true;
}

static void output_write_layout_config(struct output *output, const char *active_layout)
{
    struct output_manager *om = output->manager;
    if (!om->layout_config || !om->layout_config->json || !active_layout) {
        return;
    }

    char layout[UUID_SIZE * 2];
    output_get_layout(output, active_layout, layout);

    json_object *config = json_object_object_get(om->layout_config->json, layout);
    if (!config) {
        config = json_object_new_object();
        json_object_object_add(om->layout_config->json, layout, config);
    }

    struct kywc_output *kywc_output = &output->base;
    struct kywc_output_state *state = &kywc_output->state;
    bool primary = kywc_output_get_primary() == kywc_output;

    json_object_object_add(config, "enabled", json_object_new_boolean(state->enabled));
    if (!state->enabled) {
        return;
    }

    json_object_object_add(config, "primary", json_object_new_boolean(primary));
    json_object_object_add(config, "width", json_object_new_int(state->width));
    json_object_object_add(config, "height", json_object_new_int(state->height));
    json_object_object_add(config, "refresh", json_object_new_int(state->refresh));
    json_object_object_add(config, "scale", json_object_new_double(state->scale));
    json_object_object_add(config, "transform", json_object_new_int(state->transform));
    json_object_object_add(config, "lx", json_object_new_int(state->lx));
    json_object_object_add(config, "ly", json_object_new_int(state->ly));
}

static int compare_output_uuid(const void *p1, const void *p2)
{
    const char *v1 = ((struct output_uuid *)p1)->name;
    const char *v2 = ((struct output_uuid *)p2)->name;
    return strcmp(v1, v2);
}

static void output_manager_generate_layout(struct output_manager *output_manager, char *layout_uuid,
                                           bool is_active_layout)
{
    struct output_uuid *o_uuids = NULL;
    int actual_cnt = 0;
    size_t uuid_chunk_size = UUID_SIZE - 1;

    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        if (is_active_layout && !kywc_output->state.enabled) {
            continue;
        }
        actual_cnt++;
    }

    if (!actual_cnt) {
        return;
    }

    o_uuids = malloc(actual_cnt * sizeof(struct output_uuid));
    if (!o_uuids) {
        return;
    }

    int index = 0;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        if (is_active_layout && !kywc_output->state.enabled) {
            continue;
        }
        o_uuids[index].name = kywc_output->name;
        o_uuids[index].uuid = kywc_output->uuid;
        index++;
    }

    if (actual_cnt == 1) {
        memcpy(layout_uuid, o_uuids[0].uuid, uuid_chunk_size);
        layout_uuid[uuid_chunk_size] = '\0';
        free(o_uuids);
        return;
    }

    qsort(o_uuids, actual_cnt, sizeof(struct output_uuid), compare_output_uuid);

    uint8_t *uuids = malloc(actual_cnt * uuid_chunk_size);
    if (!uuids) {
        free(o_uuids);
        return;
    }

    for (int i = 0; i < actual_cnt; ++i) {
        memcpy(uuids + i * uuid_chunk_size, o_uuids[i].uuid, uuid_chunk_size);
    }
    free(o_uuids);

    const char *uuid = kywc_identifier_md5_generate_uuid(uuids, actual_cnt * uuid_chunk_size);
    if (uuid) {
        memcpy(layout_uuid, uuid, uuid_chunk_size);
        layout_uuid[uuid_chunk_size] = '\0';
        free((void *)uuid);
    }

    free(uuids);
}

static void output_manager_get_layout_configs(bool output_removed)
{
    if (output_manager->server->terminate || !output_manager->server->start) {
        return;
    }

    if (!output_manager_has_actual_outputs()) {
        struct output *fallback_output = output_from_kywc_output(output_manager_get_fallback());
        output_manager_add_pending_state(fallback_output, &fallback_output->pending_state);
        return;
    }

    /* update current outputs layout */
    output_manager_generate_layout(output_manager, output_manager->outputs_layout, false);

    const char *active_layout = output_manager_get_active_layout(output_manager);
    kywc_log(KYWC_INFO, "Configure outputs layout %s with active layout %s",
             output_manager->outputs_layout, active_layout ? active_layout : "none");

    /* get all outputs configuration */
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }

        output_fill_preferred_state(output);
        output_read_layout_config(output, &output->pending_state, active_layout);
        output_manager_add_pending_state(output, &output->pending_state);
    }
}

static void fallback_default_configure(bool output_removed)
{
    if (output_manager->server->terminate || !output_manager->server->start) {
        return;
    }

    if (!output_manager_has_actual_outputs()) {
        struct output *fallback_output = output_from_kywc_output(output_manager_get_fallback());
        output_manager_add_pending_state(fallback_output, &fallback_output->pending_state);
        return;
    }

    if (output_removed) {
        return;
    }

    /* get all outputs configuration */
    struct output *output;
    wl_list_for_each(output, &output_manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }

        output_fill_preferred_state(output);
        output_manager_add_pending_state(output, &output->pending_state);
    }
}

static void output_manager_save_layouts(struct output_manager *manager)
{
    char active_layout[UUID_SIZE];
    output_manager_generate_layout(manager, active_layout, true);
    output_manager_set_active_layout(manager, active_layout);

    struct output *output;
    wl_list_for_each(output, &manager->outputs, link) {
        struct kywc_output *kywc_output = &output->base;
        if (kywc_output == output_manager_get_fallback()) {
            continue;
        }
        output_write_layout_config(output, active_layout);
    }

    if (kywc_log_get_level() >= KYWC_INFO) {
        kywc_log(KYWC_INFO, "Save outputs layout %s with active layout %s", manager->outputs_layout,
                 active_layout);

        wl_list_for_each(output, &manager->outputs, link) {
            struct kywc_output *kywc_output = &output->base;
            if (kywc_output == output_manager_get_fallback()) {
                continue;
            }
            output_log_state(KYWC_INFO, output, "layout", &kywc_output->state);
        }
    }
}

static void output_manager_handle_configured(struct wl_listener *listener, void *data)
{
    struct output_manager *output_manager = wl_container_of(listener, output_manager, configured);
    struct configure_event *event = data;
    if (!output_manager->has_layout_manager || !output_manager_has_actual_outputs() ||
        event->type != CONFIGURE_TYPE_UPDATE) {
        return;
    }

    output_manager_save_layouts(output_manager);
}

bool output_manager_config_init(struct output_manager *output_manager)
{
    if (output_manager->has_layout_manager) {
        output_manager->layout_config = config_manager_add_config("layouts");
        if (!output_manager->layout_config) {
            output_manager->has_layout_manager = false;
        } else {
            /* listener output configured signal */
            output_manager->configured.notify = output_manager_handle_configured;
            output_manager_add_configured_listener(&output_manager->configured);
            output_manager->impl.get_configures = output_manager_get_layout_configs;
        }
    } else {
        output_manager->impl.get_configures = fallback_default_configure;
    }

    output_manager->config = config_manager_add_config("outputs");
    if (!output_manager->config) {
        return false;
    }

    return dbus_register_object(NULL, service_path, service_interface, service_vtable,
                                output_manager);
}
