// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _OUTPUT_P_H_
#define _OUTPUT_P_H_

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "output.h"
#include "server.h"

struct output_manager_interface {
    void (*get_configures)(bool output_removed);
};

struct output_manager {
    struct server *server;

    struct wl_list outputs;
    struct wl_list configure_outputs; /* output configure_link  */

    struct kywc_output *fallback_output;
    struct kywc_output *primary_output;
    struct kywc_output *pending_primary;

    float max_scale;

    struct {
        struct wl_signal new_output;
        struct wl_signal new_enabled_output;
        struct wl_signal primary_output;
        struct wl_signal configured;
        struct wl_signal layout_damage;
        struct wl_signal max_scale;
    } events;

    struct config *config;
    struct config *layout_config;
    struct output_manager_interface impl;

    struct wl_listener new_output;
    struct wl_listener configured;
    struct wl_listener backend_destroy;
    struct wl_listener server_start;
    struct wl_listener server_destroy;
    struct wl_listener server_suspend;
    struct wl_listener server_hibernate;
    struct wl_listener server_resume;

    char outputs_layout[UUID_SIZE];
    bool has_layout_manager;
    bool initial_configured;
};

extern struct output_manager *output_manager;

/* output.c */
void output_log_state(enum kywc_log_level level, struct output *output, const char *desc,
                      const struct kywc_output_state *state);

uint32_t output_get_state_changes(struct output *output, const struct kywc_output_state *state);

void output_update_state(struct output *output);

bool output_manager_has_actual_outputs(void);

/* config.c */
bool output_manager_config_init(struct output_manager *output_manager);

bool output_read_config(struct output *output, struct kywc_output_state *state);

void output_write_config(struct output *output);

/* configure.c */
void output_fill_preferred_state(struct output *output);

float output_preferred_scale(struct kywc_output *kywc_output, int width, int height);

void output_manager_set_pending_primary(struct output *output);

void output_manager_add_pending_state(struct output *output, struct kywc_output_state *state);

bool output_manager_configure_outputs(struct output *destroying_output);

/* modes.c */
void output_add_custom_modes(struct output *output);

void output_add_mode(struct output *output, struct wlr_output_mode *mode, bool check);

/* layout.c */
void output_manager_sort_outputs(void);

void output_set_layout(struct output *output);

/* output protocols */
bool xdg_output_manager_v1_create(struct server *server);

bool ky_output_manager_create(struct server *server);

#if HAVE_KDE_OUTPUT
bool kde_output_management_create(struct server *server);
#else
static __attribute__((unused)) inline bool kde_output_management_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_WLR_OUTPUT
bool wlr_output_management_create(struct server *server);
#else
static __attribute__((unused)) inline bool wlr_output_management_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_UKUI_OUTPUT
bool ukui_output_management_create(struct server *server);
#else
static __attribute__((unused)) inline bool ukui_output_management_create(struct server *server)
{
    return false;
}
#endif

#if HAVE_UKUI_OUTPUT_CONFIG
bool ukui_output_config_init(struct output_manager *output_manager);
#else
static __attribute__((unused)) inline bool
ukui_output_config_init(struct output_manager *output_manager)
{
    return false;
}
#endif

#if HAVE_UKUI_BACKGROUND
bool ukui_background_manager_create(struct server *server);
#else
static __attribute__((unused)) inline bool ukui_background_manager_create(struct server *server)
{
    return false;
}
#endif

#endif /* _OUTPUT_P_H_ */
