// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef WLCCTRL_COMMANDS_H
#define WLCCTRL_COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include "wlcctrl.h"

/* runtime context passed to every command handler */
struct context {
    struct wlcom_ctrl_manager *manager; /* NULL without wayland requirements */
    kywc_context *kywc;                 /* NULL without kywc capabilities */
    const char *output;                 /* -o <o_uuid> */
    const char *path;                   /* --path <file> */
    const char *x, *y;                  /* -x/-y values */
    const char *w, *h;                  /* -w/-h values */
    const char *primary_output;         /* lazily resolved primary output uuid */
};

enum arg_type {
    ARG_NONE,
    ARG_REQUIRED,
    ARG_OPTIONAL,
};

struct command {
    const char *name;
    enum arg_type arg;
    uint32_t caps;  /* required KYWC_CONTEXT_CAPABILITY_* bits */
    uint32_t needs; /* required NEED_* wayland resources */
    int (*run)(struct context *ctx, const char *param);
};

extern const struct command commands[];
extern const size_t command_count;

const struct command *command_find(const char *name);

/* lazily resolve the uuid of the primary output */
const char *context_primary_output(struct context *ctx);

/* strict numeric parsers shared by handlers */
bool parse_u32(const char *s, uint32_t *out);

bool parse_i32(const char *s, int32_t *out);

bool parse_ints(const char *s, long *vals, size_t count);

bool split_pair(const char *param, char **first, char **second);

/* commands.c */
int cmd_list_toplevels(struct context *ctx, const char *param);

int cmd_list_outputs(struct context *ctx, const char *param);

int cmd_getdisplaygeometry(struct context *ctx, const char *param);

int cmd_exec(struct context *ctx, const char *param);

int cmd_sleep(struct context *ctx, const char *param);

/* window.c */
kywc_toplevel *window_resolve(struct context *ctx, const char *query);

int cmd_getactivewindow(struct context *ctx, const char *param);

int cmd_search(struct context *ctx, const char *param);

int cmd_getwindowname(struct context *ctx, const char *param);

int cmd_getwindowgeometry(struct context *ctx, const char *param);

int cmd_getwindowstatus(struct context *ctx, const char *param);

int cmd_windowkill(struct context *ctx, const char *param);

int cmd_windowclose(struct context *ctx, const char *param);

int cmd_windowminimize(struct context *ctx, const char *param);

int cmd_windowmaximize(struct context *ctx, const char *param);

int cmd_windowfullscreen(struct context *ctx, const char *param);

int cmd_windowactivate(struct context *ctx, const char *param);

int cmd_windowmap(struct context *ctx, const char *param);

int cmd_windowmove(struct context *ctx, const char *param);

int cmd_windowsize(struct context *ctx, const char *param);

int cmd_windowenter(struct context *ctx, const char *param);

int cmd_windowleave(struct context *ctx, const char *param);

int cmd_movewindowtoworkspace(struct context *ctx, const char *param);

int cmd_movewindowtooutput(struct context *ctx, const char *param);

/* workspace.c */
int cmd_list_workspaces(struct context *ctx, const char *param);

int cmd_workspaceadd(struct context *ctx, const char *param);

int cmd_workspaceremove(struct context *ctx, const char *param);

int cmd_workspaceactive(struct context *ctx, const char *param);

/* capture.c */
int cmd_windowcapture(struct context *ctx, const char *param);

int cmd_workspacecapture(struct context *ctx, const char *param);

int cmd_fullscreencapture(struct context *ctx, const char *param);

int cmd_setarea(struct context *ctx, const char *param);

int cmd_getpixelcolor(struct context *ctx, const char *param);

void capture_context_create(kywc_context *ctx);

void capture_context_destroy(kywc_context *ctx);

/* input.c */
int cmd_keystring(struct context *ctx, const char *param);

int cmd_key(struct context *ctx, const char *param);

int cmd_keydown(struct context *ctx, const char *param);

int cmd_keyup(struct context *ctx, const char *param);

int cmd_mousemove(struct context *ctx, const char *param);

int cmd_mousebutton(struct context *ctx, const char *param);

int cmd_mousepress(struct context *ctx, const char *param);

int cmd_mouserelease(struct context *ctx, const char *param);

int cmd_scroll(struct context *ctx, const char *param);

int cmd_getmouselocation(struct context *ctx, const char *param);

#endif
