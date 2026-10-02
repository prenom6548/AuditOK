/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MPVLIBLOAD_H
#define MPVLIBLOAD_H

#include <dlfcn.h>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <QString>
#include <QVariant>

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

// Forward declarations for mpv types
struct mpv_handle;
struct mpv_node;
struct mpv_event;
struct mpv_render_context; // 替换为新的渲染上下文结构体

class MpvLibLoad {
public:
    static MpvLibLoad* getInstance();
    ~MpvLibLoad();

    bool load();
    bool isLoaded() const { return loaded; }

    // client.h functions
    unsigned long (*mpv_client_api_version)(void);
    mpv_handle* (*mpv_create)(void);
    int (*mpv_initialize)(mpv_handle *ctx);
    void (*mpv_destroy)(mpv_handle *ctx);
    int (*mpv_set_option)(mpv_handle *ctx, const char *name, int format, void *data);
    int (*mpv_set_option_string)(mpv_handle *ctx, const char *name, const char *data);
    int (*mpv_command)(mpv_handle *ctx, const char **args);
    int (*mpv_command_node)(mpv_handle *ctx, mpv_node *args, mpv_node *result);
    int (*mpv_set_property)(mpv_handle *ctx, const char *name, int format, void *data);
    int (*mpv_get_property)(mpv_handle *ctx, const char *name, int format, void *data);
    char* (*mpv_get_property_string)(mpv_handle *ctx, const char *name);
    const char* (*mpv_error_string)(int error);
    void (*mpv_free)(void *data);
    void (*mpv_free_node_contents)(mpv_node *node);
    int (*mpv_observe_property)(mpv_handle *mpv, unsigned long long reply_userdata,
                               const char *name, int format);
    mpv_event* (*mpv_wait_event)(mpv_handle *ctx, double timeout);
    void (*mpv_wakeup)(mpv_handle *ctx);
    int (*mpv_command_async)(mpv_handle *ctx, uint64_t reply_userdata,
                          const char **args);
    int (*mpv_set_property_async)(mpv_handle *ctx, uint64_t reply_userdata,
                               const char *name, mpv_format format, void *data);
    void (*mpv_set_wakeup_callback)(mpv_handle *ctx, void (*cb)(void *d), void *d);
    void (*mpv_terminate_destroy)(mpv_handle *ctx);
    mpv_handle* (*mpv_request_log_messages)(mpv_handle *ctx, const char *min_level);

    // 替换为新的 render_gl.h functions
    int (*mpv_render_context_create)(mpv_render_context **res, mpv_handle *mpv, mpv_render_param *params);
    int (*mpv_render_context_set_parameter)(mpv_render_context *ctx, mpv_render_param param);
    int (*mpv_render_context_get_info)(mpv_render_context *ctx, mpv_render_param param);
    void (*mpv_render_context_set_update_callback)(mpv_render_context *ctx, void (*callback)(void *cb_ctx), void *cb_ctx);
    uint64_t (*mpv_render_context_update)(mpv_render_context *ctx);
    int (*mpv_render_context_render)(mpv_render_context *ctx, mpv_render_param *params);
    void (*mpv_render_context_report_swap)(mpv_render_context *ctx);
    void (*mpv_render_context_free)(mpv_render_context *ctx);

private:
    MpvLibLoad();

    void *libmpv_handle;
    bool loaded;
    static MpvLibLoad *m_instance;

    template<typename T>
    bool loadSymbol(T &func, const char *symbol_name);
};

#endif // MPVLIBLOAD_H
