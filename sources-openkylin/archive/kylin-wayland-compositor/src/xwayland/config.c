// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "config.h"
#include "util/dbus.h"
#include "util/macros.h"
#include "xwayland_p.h"

static const char *service_path = "/com/kylin/Wlcom/Xwayland";
static const char *service_interface = "com.kylin.Wlcom.Xwayland";

static int set_eavesdrops_key_mode(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct xwayland_manager *xwayland = userdata;
    uint32_t eavesdrops_key_mode;
    CK(sd_bus_message_read(m, "u", &eavesdrops_key_mode));

    if (eavesdrops_key_mode > ALL_KEYS) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid eavesdrops_key_mode.");
        return sd_bus_reply_method_error(m, &error);
    }

    xwayland->state.eavesdrops_key_mode = eavesdrops_key_mode;

    return sd_bus_reply_method_return(m, NULL);
}

static int get_eavesdrops_key_mode(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct xwayland_manager *xwayland = userdata;
    return sd_bus_reply_method_return(m, "u", xwayland->state.eavesdrops_key_mode);
}

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("SetEavesdropsKeyMode", "u", "", set_eavesdrops_key_mode,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetEavesdropsKeyMode", "", "u", get_eavesdrops_key_mode,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

bool xwayland_config_init(struct xwayland_manager *xwayland)
{
    xwayland->config = config_manager_add_config("Xwayland");
    if (!xwayland->config) {
        return false;
    }
    return dbus_register_object(NULL, service_path, service_interface, service_vtable, xwayland);
}

bool xwayland_read_config(struct xwayland_manager *xwayland)
{
    if (!xwayland->config || !xwayland->config->json) {
        return false;
    }

    json_object *data;
    if (json_object_object_get_ex(xwayland->config->json, "eavesdrops_key_mode", &data)) {
        xwayland->state.eavesdrops_key_mode = CLAMP(json_object_get_int(data), NONE_KEYS, ALL_KEYS);
    }

    return true;
}

void xwayland_write_config(struct xwayland_manager *xwayland)
{
    if (!xwayland->config) {
        return;
    }

    json_object_object_add(xwayland->config->json, "eavesdrops_key_mode",
                           json_object_new_int(xwayland->state.eavesdrops_key_mode));
}
