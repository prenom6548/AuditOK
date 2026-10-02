// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _LIBKYWC_HEADER_P_H_
#define _LIBKYWC_HEADER_P_H_

#include "libkywc.h"

struct ky_context_provider {
    struct wl_list link;
    enum kywc_context_capability capability;

    bool (*bind)(struct ky_context_provider *provider, struct wl_registry *registry, uint32_t name,
                 const char *interface, uint32_t version);
    void (*destroy)(struct ky_context_provider *provider);
    void *data;
};

struct _kywc_context {
    struct wl_display *display;
    struct wl_registry *registry;
    bool own_display;

    uint32_t capabilities;
    const struct kywc_context_interface *impl;
    void *user_data;

    struct wl_list providers;
    struct wl_list pending_providers;

    struct ky_workspace_manager *workspace;
    struct ky_output_manager *output;
    struct ky_toplevel_manager *toplevel;
    struct ky_thumbnail_manager *thumbnail;
    struct ky_cursor_manager *cursor;
    struct ky_stream_manager *stream;
};

bool ky_context_add_provider(kywc_context *ctx, struct ky_context_provider *provider,
                             void *manager);

/**
 * workspace
 */
struct ky_workspace_manager {
    kywc_context *ctx;
    struct wl_list workspaces;

    void (*create_workspace)(struct ky_workspace_manager *manager, const char *name,
                             uint32_t position);
    void (*destroy)(struct ky_workspace_manager *manager);
    void *data;
};

struct ky_workspace {
    kywc_workspace base;

    struct ky_workspace_manager *manager;
    struct wl_list link;

    const struct kywc_workspace_interface *impl;
    void *user_data;

    void (*set_name)(struct ky_workspace *workspace, const char *name);
    void (*set_position)(struct ky_workspace *workspace, uint32_t position);
    void (*activate)(struct ky_workspace *workspace);
    void (*remove)(struct ky_workspace *workspace);
    void (*destroy)(struct ky_workspace *workspace);
    void *data;

    uint32_t pending_mask;
    bool newly_added;
};

struct ky_workspace_manager *ky_workspace_manager_create(kywc_context *ctx);

void ky_workspace_manager_destroy(struct ky_workspace_manager *manager);

void ky_workspace_manager_update_states(struct ky_workspace_manager *manager);

struct ky_workspace *ky_workspace_create(struct ky_workspace_manager *manager, const char *uuid);

void ky_workspace_destroy(struct ky_workspace *workspace);

void ky_workspace_update_name(struct ky_workspace *workspace, const char *name);

void ky_workspace_update_position(struct ky_workspace *workspace, uint32_t position);

void ky_workspace_update_activated(struct ky_workspace *workspace, bool activated);

/**
 * output
 */
struct ky_output_manager {
    kywc_context *ctx;
    struct wl_list outputs;

    struct ky_output *primary;

    void (*apply)(struct ky_output_manager *manager);
    void (*destroy)(struct ky_output_manager *manager);
    void *data;
};

struct ky_output {
    kywc_output base;

    struct ky_output_manager *manager;
    struct wl_list link;

    const struct kywc_output_interface *impl;
    void *user_data;

    void (*destroy)(struct ky_output *output);
    void *data;

    uint32_t pending_mask;
    bool newly_added;
};

struct ky_output_mode {
    struct kywc_output_mode base;
    struct ky_output *output;

    void (*destroy)(struct ky_output_mode *mode);
    void *data;
};

struct ky_output_manager *ky_output_manager_create(kywc_context *ctx);

void ky_output_manager_destroy(struct ky_output_manager *manager);

void ky_output_manager_update_states(struct ky_output_manager *manager);

void ky_output_manager_update_primary(struct ky_output_manager *manager, struct ky_output *primary);

struct ky_output *ky_output_create(struct ky_output_manager *manager, const char *uuid);

void ky_output_destroy(struct ky_output *output);

void ky_output_set_name(struct ky_output *output, const char *name);

void ky_output_set_make(struct ky_output *output, const char *make);

void ky_output_set_model(struct ky_output *output, const char *model);

void ky_output_set_serial_number(struct ky_output *output, const char *serial_number);

void ky_output_set_description(struct ky_output *output, const char *description);

void ky_output_set_physical_size(struct ky_output *output, int width, int height);

void ky_output_set_capabilities(struct ky_output *output, uint32_t capabilities);

struct ky_output_mode *ky_output_mode_create(struct ky_output *output);

void ky_output_mode_set_size(struct ky_output_mode *mode, int width, int height);

void ky_output_mode_set_refresh(struct ky_output_mode *mode, int refresh);

void ky_output_mode_set_preferred(struct ky_output_mode *mode);

void ky_output_mode_destroy(struct ky_output_mode *mode);

void ky_output_update_enabled(struct ky_output *output, bool enabled);

void ky_output_update_current_mode(struct ky_output *output, struct ky_output_mode *mode);

void ky_output_update_position(struct ky_output *output, int x, int y);

void ky_output_update_transform(struct ky_output *output, int transform);

void ky_output_update_scale(struct ky_output *output, float scale);

void ky_output_update_power(struct ky_output *output, bool power);

void ky_output_update_primary(struct ky_output *output, bool primary);

void ky_output_update_brightness(struct ky_output *output, uint32_t brightness);

void ky_output_update_color_temp(struct ky_output *output, uint32_t color_temp);

/**
 * toplevel
 */
struct ky_toplevel_manager {
    kywc_context *ctx;
    struct wl_list toplevels;

    void (*destroy)(struct ky_toplevel_manager *manager);
    void *data;
};

struct ky_toplevel {
    kywc_toplevel base;

    struct ky_toplevel_manager *manager;
    struct wl_list link;

    const struct kywc_toplevel_interface *impl;
    void *user_data;

    void (*set_maximized)(struct ky_toplevel *toplevel, const char *output);
    void (*unset_maximized)(struct ky_toplevel *toplevel);
    void (*set_minimized)(struct ky_toplevel *toplevel);
    void (*unset_minimized)(struct ky_toplevel *toplevel);
    void (*set_fullscreen)(struct ky_toplevel *toplevel, const char *output);
    void (*unset_fullscreen)(struct ky_toplevel *toplevel);
    void (*activate)(struct ky_toplevel *toplevel);
    void (*close)(struct ky_toplevel *toplevel);
    void (*enter_workspace)(struct ky_toplevel *toplevel, const char *workspace);
    void (*leave_workspace)(struct ky_toplevel *toplevel, const char *workspace);
    void (*move_to_workspace)(struct ky_toplevel *toplevel, const char *workspace);
    void (*move_to_output)(struct ky_toplevel *toplevel, const char *output);
    void (*set_position)(struct ky_toplevel *toplevel, int32_t x, int32_t y);
    void (*set_size)(struct ky_toplevel *toplevel, uint32_t width, uint32_t height);
    void (*destroy)(struct ky_toplevel *toplevel);
    void *data;

    uint32_t pending_mask;
    bool newly_added;
};

struct ky_toplevel_manager *ky_toplevel_manager_create(kywc_context *ctx);

void ky_toplevel_manager_destroy(struct ky_toplevel_manager *manager);

struct ky_toplevel *ky_toplevel_create(struct ky_toplevel_manager *manager, const char *uuid);

void ky_toplevel_destroy(struct ky_toplevel *toplevel);

void ky_toplevel_update_states(struct ky_toplevel *toplevel);

void ky_toplevel_set_capabilities(struct ky_toplevel *toplevel, uint32_t capabilities);

void ky_toplevel_update_title(struct ky_toplevel *toplevel, const char *title);

void ky_toplevel_update_app_id(struct ky_toplevel *toplevel, const char *app_id);

void ky_toplevel_update_primary_output(struct ky_toplevel *toplevel, const char *output_id);

void ky_toplevel_update_maximized(struct ky_toplevel *toplevel, bool maximized);

void ky_toplevel_update_minimized(struct ky_toplevel *toplevel, bool minimized);

void ky_toplevel_update_activated(struct ky_toplevel *toplevel, bool activated);

void ky_toplevel_update_fullscreen(struct ky_toplevel *toplevel, bool fullscreen);

void ky_toplevel_update_parent(struct ky_toplevel *toplevel, struct ky_toplevel *parent);

void ky_toplevel_update_icon(struct ky_toplevel *toplevel, const char *icon);

void ky_toplevel_update_geometry(struct ky_toplevel *toplevel, int32_t x, int32_t y, uint32_t width,
                                 uint32_t height);

void ky_toplevel_set_pid(struct ky_toplevel *toplevel, uint32_t pid);

void ky_toplevel_enter_workspace(struct ky_toplevel *toplevel, const char *workspace);

void ky_toplevel_leave_workspace(struct ky_toplevel *toplevel, const char *workspace);

/**
 * thumbnail
 */
struct ky_thumbnail;
struct ky_cursor;

struct ky_thumbnail_manager {
    kywc_context *ctx;
    struct wl_list thumbnails;

    void (*capture_output)(struct ky_thumbnail_manager *manager, struct ky_thumbnail *thumbnail,
                           const char *uuid);
    void (*capture_workspace)(struct ky_thumbnail_manager *manager, struct ky_thumbnail *thumbnail,
                              const char *uuid, const char *output);
    void (*capture_toplevel)(struct ky_thumbnail_manager *manager, struct ky_thumbnail *thumbnail,
                             const char *uuid, bool without_decoration);
    void (*destroy)(struct ky_thumbnail_manager *manager);
    void *data;
};

struct ky_thumbnail {
    kywc_thumbnail base;

    struct ky_thumbnail_manager *manager;
    struct wl_list link;

    const struct kywc_thumbnail_interface *impl;
    void *user_data;

    void (*release)(struct ky_thumbnail *thumbnail, bool wants_buffer);
    void (*destroy)(struct ky_thumbnail *thumbnail);
    void *data;

    struct kywc_thumbnail_buffer buffer;
};

struct ky_thumbnail_manager *ky_thumbnail_manager_create(kywc_context *ctx);

void ky_thumbnail_manager_destroy(struct ky_thumbnail_manager *manager);

void ky_thumbnail_release(struct ky_thumbnail *thumbnail, bool wants_buffer);

void ky_thumbnail_destroy(struct ky_thumbnail *thumbnail);

bool ky_thumbnail_update_buffer(struct ky_thumbnail *thumbnail,
                                const struct kywc_thumbnail_buffer *buffer, bool *want_buffer);

struct ky_thumbnail *thumbnail_from_kywc_thumbnail(kywc_thumbnail *kywc_thumbnail);

struct ky_cursor_manager {
    kywc_context *ctx;
    struct wl_list cursors;
    void (*create_cursor)(struct ky_cursor_manager *manager, struct ky_cursor *cursor,
                          struct wl_seat *seat, struct ky_thumbnail *thumbnail);
    void (*destroy)(struct ky_cursor_manager *manager);
    void *data;
};

struct ky_cursor_manager *ky_cursor_manager_create(kywc_context *ctx);

void ky_cursor_manager_destroy(struct ky_cursor_manager *manager);

struct ky_cursor {
    kywc_cursor base;

    struct ky_cursor_manager *manager;
    struct wl_list link;

    const struct kywc_cursor_interface *impl;
    void *user_data;

    void (*destroy)(struct ky_cursor *cursor);
    void *data;
};

void ky_cursor_update_position(struct ky_cursor *cursor, uint32_t x, uint32_t y);

void ky_cursor_update_hotspot(struct ky_cursor *cursor, uint32_t x, uint32_t y);

void ky_cursor_enter(struct ky_cursor *cursor);

void ky_cursor_leave(struct ky_cursor *cursor);

void ky_cursor_destroy(struct ky_cursor *cursor);

/**
 * stream
 */
struct ky_stream;

struct ky_stream_manager {
    kywc_context *ctx;
    struct wl_list streams;

    void (*stream_output)(struct ky_stream_manager *manager, struct ky_stream *stream,
                          const char *uuid, enum kywc_stream_cursor_mode cursor_mode);
    void (*stream_toplevel)(struct ky_stream_manager *manager, struct ky_stream *stream,
                            const char *uuid, enum kywc_stream_cursor_mode cursor_mode);
    void (*destroy)(struct ky_stream_manager *manager);
    void *data;
};

struct ky_stream {
    kywc_stream base;

    struct ky_stream_manager *manager;
    struct wl_list link;

    const struct kywc_stream_interface *impl;
    void *user_data;

    void (*destroy)(struct ky_stream *stream);
    void *data;
};

struct ky_stream_manager *ky_stream_manager_create(kywc_context *ctx);

void ky_stream_manager_destroy(struct ky_stream_manager *manager);

void ky_stream_ready(struct ky_stream *stream, uint32_t node);

void ky_stream_destroy(struct ky_stream *stream);

struct ky_stream *stream_from_kywc_stream(kywc_stream *kywc_stream);

#endif /* _LIBKYWC_HEADER_P_H_ */
