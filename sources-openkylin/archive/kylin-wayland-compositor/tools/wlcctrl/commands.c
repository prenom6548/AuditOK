// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "util/macros.h"
#include "util/spawn.h"

#include "commands.h"

#define CAP_OUTPUT KYWC_CONTEXT_CAPABILITY_OUTPUT
#define CAP_TOPLEVEL KYWC_CONTEXT_CAPABILITY_TOPLEVEL
#define CAP_WORKSPACE KYWC_CONTEXT_CAPABILITY_WORKSPACE
#define CAP_THUMBNAIL KYWC_CONTEXT_CAPABILITY_THUMBNAIL
#define CAP_CURSOR KYWC_CONTEXT_CAPABILITY_CURSOR

const struct command commands[] = {
    /* listing */
    { "list", ARG_NONE, CAP_TOPLEVEL, 0, cmd_list_toplevels },
    { "outputs", ARG_NONE, CAP_OUTPUT, 0, cmd_list_outputs },
    { "workspace", ARG_NONE, CAP_WORKSPACE, 0, cmd_list_workspaces },

    /* toplevel query */
    { "getactivewindow", ARG_NONE, CAP_TOPLEVEL, 0, cmd_getactivewindow },
    { "search", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_search },
    { "getwindowname", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_getwindowname },
    { "getwindowgeometry", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_getwindowgeometry },
    { "getwindowstatus", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_getwindowstatus },

    /* toplevel actions */
    { "windowkill", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowkill },
    { "windowclose", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowclose },
    { "windowminimize", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowminimize },
    { "windowmaximize", ARG_REQUIRED, CAP_TOPLEVEL | CAP_OUTPUT, 0, cmd_windowmaximize },
    { "windowfullscreen", ARG_REQUIRED, CAP_TOPLEVEL | CAP_OUTPUT, 0, cmd_windowfullscreen },
    { "windowactivate", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowactivate },
    { "windowmap", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowmap },
    { "windowmove", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowmove },
    { "windowsize", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowsize },
    { "windowenter", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowenter },
    { "windowleave", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_windowleave },
    { "movewindowtoworkspace", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_movewindowtoworkspace },
    { "movewindowtooutput", ARG_REQUIRED, CAP_TOPLEVEL, 0, cmd_movewindowtooutput },
    { "windowcapture", ARG_REQUIRED, CAP_TOPLEVEL | CAP_THUMBNAIL, 0, cmd_windowcapture },

    /* workspace */
    { "workspaceadd", ARG_OPTIONAL, CAP_WORKSPACE, 0, cmd_workspaceadd },
    { "workspaceremove", ARG_REQUIRED, CAP_WORKSPACE, 0, cmd_workspaceremove },
    { "workspaceactive", ARG_REQUIRED, CAP_WORKSPACE, 0, cmd_workspaceactive },
    { "workspacecapture", ARG_REQUIRED, CAP_WORKSPACE | CAP_THUMBNAIL, 0, cmd_workspacecapture },

    /* output */
    { "getdisplaygeometry", ARG_REQUIRED, CAP_OUTPUT, 0, cmd_getdisplaygeometry },
    { "fullscreencapture", ARG_NONE, CAP_OUTPUT | CAP_THUMBNAIL, 0, cmd_fullscreencapture },
    { "setarea", ARG_REQUIRED, CAP_OUTPUT | CAP_THUMBNAIL, 0, cmd_setarea },
    { "getpixelcolor", ARG_REQUIRED, CAP_OUTPUT | CAP_THUMBNAIL, 0, cmd_getpixelcolor },

    /* pointer */
    { "mousemove", ARG_REQUIRED, CAP_OUTPUT, NEED_SEAT | NEED_POINTER | NEED_OUTPUTS,
      cmd_mousemove },
    { "mousebutton", ARG_REQUIRED, CAP_OUTPUT, NEED_SEAT | NEED_POINTER | NEED_OUTPUTS,
      cmd_mousebutton },
    { "mousepress", ARG_REQUIRED, CAP_OUTPUT, NEED_SEAT | NEED_POINTER | NEED_OUTPUTS,
      cmd_mousepress },
    { "mouserelease", ARG_REQUIRED, CAP_OUTPUT, NEED_SEAT | NEED_POINTER | NEED_OUTPUTS,
      cmd_mouserelease },
    { "scroll", ARG_NONE, CAP_OUTPUT, NEED_SEAT | NEED_POINTER | NEED_OUTPUTS, cmd_scroll },
    { "getmouselocation", ARG_NONE, CAP_CURSOR, NEED_SEAT, cmd_getmouselocation },

    /* keyboard */
    { "keystring", ARG_REQUIRED, 0, NEED_SEAT | NEED_KEYBOARD, cmd_keystring },
    { "key", ARG_REQUIRED, 0, NEED_SEAT | NEED_KEYBOARD, cmd_key },
    { "keydown", ARG_REQUIRED, 0, NEED_SEAT | NEED_KEYBOARD, cmd_keydown },
    { "keyup", ARG_REQUIRED, 0, NEED_SEAT | NEED_KEYBOARD, cmd_keyup },

    /* misc */
    { "exec", ARG_REQUIRED, 0, 0, cmd_exec },
    { "sleep", ARG_REQUIRED, 0, 0, cmd_sleep },
};

const size_t command_count = ARRAY_SIZE(commands);

const struct command *command_find(const char *name)
{
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

static bool find_primary_output(kywc_output *output, void *data)
{
    struct context *ctx = data;

    if (output->primary) {
        ctx->primary_output = output->uuid;
        return true;
    }
    return false;
}

const char *context_primary_output(struct context *ctx)
{
    if (!ctx->primary_output && ctx->kywc) {
        kywc_context_for_each_output(ctx->kywc, find_primary_output, ctx);
    }
    return ctx->primary_output;
}

bool parse_u32(const char *s, uint32_t *out)
{
    if (*s == '-') {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long v = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v > UINT32_MAX) {
        return false;
    }
    *out = (uint32_t)v;
    return true;
}

bool parse_i32(const char *s, int32_t *out)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < INT32_MIN || v > INT32_MAX) {
        return false;
    }
    *out = (int32_t)v;
    return true;
}

/* parse a comma-separated integer list like "10,40,500,100" */
bool parse_ints(const char *s, long *vals, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        char *end;
        errno = 0;
        long v = strtol(s, &end, 10);
        if (errno != 0 || end == s) {
            return false;
        }
        vals[i] = v;

        s = end;
        if (i + 1 == count) {
            return *s == '\0';
        }
        if (*s != ',') {
            return false;
        }
        s++;
    }
    return true;
}

/* split "first:second" into two newly allocated strings */
bool split_pair(const char *param, char **first, char **second)
{
    const char *sep = strchr(param, ':');
    if (!sep || sep == param || sep[1] == '\0') {
        return false;
    }

    *first = strndup(param, sep - param);
    *second = strdup(sep + 1);
    if (!*first || !*second) {
        free(*first);
        free(*second);
        return false;
    }
    return true;
}

static bool print_toplevel(kywc_toplevel *toplevel, void *data)
{
    if (!toplevel) {
        return false;
    }

    printf("toplevel \"%s\"\n", toplevel->uuid ? toplevel->uuid : "unknown");
    printf("  title: %s\n", toplevel->title ? toplevel->title : "unknown");
    printf("  icon: %s\n", toplevel->icon ? toplevel->icon : "unknown");
    printf("  activated: %s\n", toplevel->activated ? "yes" : "no");
    printf("  maximized: %s\n", toplevel->maximized ? "yes" : "no");
    printf("  minimized: %s\n", toplevel->minimized ? "yes" : "no");
    printf("  fullscreen: %s\n", toplevel->fullscreen ? "yes" : "no");
    printf("\n");
    return false;
}

static bool print_output(kywc_output *output, void *data)
{
    if (!output) {
        return false;
    }

    printf("output \"%s\"\n", output->uuid ? output->uuid : "unknown");
    printf("primary : %s\n", output->primary ? "yes" : "no");
    printf("enabled: %s\n", output->enabled ? "true" : "false");
    printf("  name: %s\n", output->name ? output->name : "unknown");
    printf("\n");
    return false;
}

int cmd_list_toplevels(struct context *ctx, const char *param)
{
    kywc_context_for_each_toplevel(ctx->kywc, print_toplevel, NULL);
    return 0;
}

int cmd_list_outputs(struct context *ctx, const char *param)
{
    kywc_context_for_each_output(ctx->kywc, print_output, NULL);
    return 0;
}

int cmd_getdisplaygeometry(struct context *ctx, const char *param)
{
    kywc_output *output = kywc_context_find_output(ctx->kywc, param);
    if (!output) {
        fprintf(stderr, "Please enter the correct uuid for the specified output.\n");
        return -1;
    }

    printf("  physical size: %dx%d mm\n", output->physical_width, output->physical_height);
    printf("  scale: %f\n", output->scale);
    printf("  transform: %d\n", output->transform);
    if (output->enabled) {
        if (output->mode) {
            printf("  mode: %d x %d @ %d\n", output->mode->width, output->mode->height,
                   output->mode->refresh / 1000);
        } else {
            printf("  mode: unknown\n");
        }
        printf("  position: %d, %d\n", output->x, output->y);
    }
    return 0;
}

int cmd_exec(struct context *ctx, const char *param)
{
    char *command = strdup(param);
    if (!command) {
        perror("strdup");
        return -1;
    }

    bool ok = spawn_shell(command);
    free(command);
    if (!ok) {
        fprintf(stderr, "Failed to execute: %s\n", param);
        return -1;
    }
    return 0;
}

int cmd_sleep(struct context *ctx, const char *param)
{
    uint32_t ms;
    if (!parse_u32(param, &ms)) {
        fprintf(stderr, "The --sleep option expects a non-negative integer.\n");
        return -1;
    }

    struct timespec duration = {
        .tv_sec = ms / 1000,
        .tv_nsec = (long)(ms % 1000) * 1000000,
    };
    nanosleep(&duration, NULL);
    return 0;
}
