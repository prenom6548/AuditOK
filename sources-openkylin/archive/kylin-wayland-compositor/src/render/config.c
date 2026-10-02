// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <wlr/config.h>
#include <wlr/render/vulkan.h>

#include "render/opengl.h"
#include "render/renderer.h"
#include "renderer_p.h"
#include "server.h"
#include "util/dbus.h"

static const char *service_path = "/com/kylin/Wlcom/Renderer";
static const char *service_interface = "com.kylin.Wlcom.Renderer";

static int is_hardware_rendering(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct server *server = userdata;
    struct wlr_renderer *renderer = server->renderer;
    return sd_bus_reply_method_return(m, "b", !renderer_is_software(renderer));
}

static int get_renderer_name(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct server *server = userdata;
    struct wlr_renderer *renderer = server->renderer;
    if (wlr_renderer_is_opengl(renderer)) {
        struct opengl_renderer *r = opengl_renderer_from_wlr_renderer(renderer);
        return sd_bus_reply_method_return(m, "s", r->egl->is_gles ? "gles2" : "gl");
#if WLR_HAS_VULKAN_RENDERER
    } else if (wlr_renderer_is_vk(renderer)) {
        return sd_bus_reply_method_return(m, "s", "vulkan");
#endif
    } else {
        return sd_bus_reply_method_return(m, "s", "pixman");
    }
}

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("IsHardwareRendering", "", "b", is_hardware_rendering,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetRendererName", "", "s", get_renderer_name, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

bool renderer_config_init(struct server *server)
{
    return dbus_register_object(NULL, service_path, service_interface, service_vtable, server);
}
