// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <errno.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"

static bool toplevel_matches(const kywc_toplevel *toplevel, const char *pattern)
{
    return (toplevel->title && strcasestr(toplevel->title, pattern)) ||
           (toplevel->app_id && strcasestr(toplevel->app_id, pattern)) ||
           (toplevel->icon && strcasestr(toplevel->icon, pattern));
}

struct match_data {
    const char *pattern;
    kywc_toplevel *match;
    int count;
};

static bool count_matched_toplevels(kywc_toplevel *toplevel, void *data)
{
    struct match_data *md = data;

    if (toplevel_matches(toplevel, md->pattern)) {
        md->match = toplevel;
        md->count++;
    }
    return false;
}

static bool print_matched_toplevel(kywc_toplevel *toplevel, void *data)
{
    struct match_data *md = data;

    if (toplevel_matches(toplevel, md->pattern)) {
        fprintf(stderr, "  %s  %s\n", toplevel->uuid ? toplevel->uuid : "unknown",
                toplevel->title ? toplevel->title : "unknown");
    }
    return false;
}

kywc_toplevel *window_resolve(struct context *ctx, const char *query)
{
    kywc_toplevel *toplevel = kywc_context_find_toplevel(ctx->kywc, query);
    if (toplevel) {
        return toplevel;
    }

    struct match_data md = { .pattern = query };
    kywc_context_for_each_toplevel(ctx->kywc, count_matched_toplevels, &md);
    if (md.count == 1) {
        return md.match;
    }

    if (md.count > 1) {
        fprintf(stderr, "Multiple windows match '%s'; use one of these UUIDs:\n", query);
        kywc_context_for_each_toplevel(ctx->kywc, print_matched_toplevel, &md);
    } else {
        fprintf(stderr, "No window matches '%s'.\n", query);
    }
    return NULL;
}

static bool print_activated_toplevel(kywc_toplevel *toplevel, void *data)
{
    if (toplevel->activated) {
        printf("  uuid : %s\n", toplevel->uuid ? toplevel->uuid : "unknown");
    }
    return false;
}

int cmd_getactivewindow(struct context *ctx, const char *param)
{
    kywc_context_for_each_toplevel(ctx->kywc, print_activated_toplevel, NULL);
    return 0;
}

static bool match_toplevel(kywc_toplevel *toplevel, void *data)
{
    const regex_t *regex = data;

    if ((toplevel->icon && regexec(regex, toplevel->icon, 0, NULL, 0) == 0) ||
        (toplevel->app_id && regexec(regex, toplevel->app_id, 0, NULL, 0) == 0) ||
        (toplevel->title && regexec(regex, toplevel->title, 0, NULL, 0) == 0)) {
        printf("uuid : %s\n", toplevel->uuid ? toplevel->uuid : "unknown");
    }
    return false;
}

int cmd_search(struct context *ctx, const char *param)
{
    regex_t regex;
    int ret = regcomp(&regex, param, REG_EXTENDED | REG_ICASE);
    if (ret != 0) {
        char msgbuf[128];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Could not compile regex: %s\n", msgbuf);
        return -1;
    }

    kywc_context_for_each_toplevel(ctx->kywc, match_toplevel, &regex);
    regfree(&regex);
    return 0;
}

int cmd_getwindowname(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    printf("uuid : %s\nname : %s\nicon: %s\ntitle: %s\npid: %d\n",
           toplevel->uuid ? toplevel->uuid : "unknown",
           toplevel->app_id ? toplevel->app_id : "unknown",
           toplevel->icon ? toplevel->icon : "unknown",
           toplevel->title ? toplevel->title : "unknown", (int)toplevel->pid);
    return 0;
}

int cmd_getwindowgeometry(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    printf("geometry: (%d, %d) %u x %u\n", toplevel->x, toplevel->y, toplevel->width,
           toplevel->height);
    return 0;
}

int cmd_getwindowstatus(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    printf("  activated: %s\n", toplevel->activated ? "yes" : "no");
    printf("  maximized: %s\n", toplevel->maximized ? "yes" : "no");
    printf("  minimized: %s\n", toplevel->minimized ? "yes" : "no");
    printf("  fullscreen: %s\n", toplevel->fullscreen ? "yes" : "no");
    return 0;
}

static bool kill_toplevel(kywc_toplevel *toplevel, void *data)
{
    const char *app = data;

    if (!toplevel->pid || !toplevel->app_id || strcmp(app, toplevel->app_id) != 0) {
        return false;
    }

    if (kill(toplevel->pid, SIGTERM) == -1) {
        perror("kill");
        return true;
    }
    printf("Sent SIGTERM to process with PID %d\n", (int)toplevel->pid);
    sleep(1);

    if (kill(toplevel->pid, 0) == -1 && errno == ESRCH) {
        printf("Process with PID %d has terminated.\n", (int)toplevel->pid);
    } else {
        if (kill(toplevel->pid, SIGKILL) == -1) {
            perror("kill");
            return true;
        }
        printf("Sent SIGKILL to process with PID %d\n", (int)toplevel->pid);
    }
    return true;
}

int cmd_windowkill(struct context *ctx, const char *param)
{
    kywc_context_for_each_toplevel(ctx->kywc, kill_toplevel, (void *)param);
    return 0;
}

static int toplevel_simple_action(struct context *ctx, const char *param,
                                  void (*action)(kywc_toplevel *))
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    action(toplevel);
    return 0;
}

static void map_toplevel(kywc_toplevel *toplevel)
{
    if (toplevel->minimized) {
        kywc_toplevel_unset_minimized(toplevel);
    } else if (toplevel->maximized) {
        kywc_toplevel_unset_maximized(toplevel);
    } else if (toplevel->fullscreen) {
        kywc_toplevel_unset_fullscreen(toplevel);
    }
}

int cmd_windowclose(struct context *ctx, const char *param)
{
    return toplevel_simple_action(ctx, param, kywc_toplevel_close);
}

int cmd_windowminimize(struct context *ctx, const char *param)
{
    return toplevel_simple_action(ctx, param, kywc_toplevel_set_minimized);
}

int cmd_windowactivate(struct context *ctx, const char *param)
{
    return toplevel_simple_action(ctx, param, kywc_toplevel_activate);
}

int cmd_windowmap(struct context *ctx, const char *param)
{
    return toplevel_simple_action(ctx, param, map_toplevel);
}

int cmd_windowmaximize(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    kywc_toplevel_set_maximized(toplevel, ctx->output ? ctx->output : context_primary_output(ctx));
    return 0;
}

int cmd_windowfullscreen(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    kywc_toplevel_set_fullscreen(toplevel, ctx->output ? ctx->output : context_primary_output(ctx));
    return 0;
}

int cmd_windowmove(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    if (!ctx->x || !ctx->y) {
        fprintf(stderr, "The --windowmove option expects -x and -y parameters.\n");
        return -1;
    }

    uint32_t x, y;
    if (!parse_u32(ctx->x, &x) || !parse_u32(ctx->y, &y)) {
        fprintf(stderr, "The --windowmove option expects integer -x and -y parameters.\n");
        return -1;
    }

    kywc_toplevel_set_position(toplevel, x, y);
    return 0;
}

int cmd_windowsize(struct context *ctx, const char *param)
{
    kywc_toplevel *toplevel = window_resolve(ctx, param);
    if (!toplevel) {
        return -1;
    }

    if (!ctx->w || !ctx->h) {
        fprintf(stderr, "The --windowsize option expects -w and -h parameters.\n");
        return -1;
    }

    uint32_t width, height;
    if (!parse_u32(ctx->w, &width) || !parse_u32(ctx->h, &height)) {
        fprintf(stderr, "The --windowsize option expects integer -w and -h parameters.\n");
        return -1;
    }

    kywc_toplevel_set_size(toplevel, width, height);
    return 0;
}

typedef void (*toplevel_workspace_action_fn)(kywc_toplevel *, const char *);

static int toplevel_workspace_action(struct context *ctx, const char *param,
                                     toplevel_workspace_action_fn action, const char *option)
{
    char *w_uuid, *uuid;
    if (!split_pair(param, &w_uuid, &uuid)) {
        fprintf(stderr, "The --%s option expects w_uuid:uuid parameters.\n", option);
        return -1;
    }

    kywc_toplevel *toplevel = window_resolve(ctx, w_uuid);
    free(w_uuid);
    if (!toplevel) {
        free(uuid);
        return -1;
    }

    action(toplevel, uuid);
    free(uuid);
    return 0;
}

int cmd_windowenter(struct context *ctx, const char *param)
{
    return toplevel_workspace_action(ctx, param, kywc_toplevel_enter_workspace, "windowenter");
}

int cmd_windowleave(struct context *ctx, const char *param)
{
    return toplevel_workspace_action(ctx, param, kywc_toplevel_leave_workspace, "windowleave");
}

int cmd_movewindowtoworkspace(struct context *ctx, const char *param)
{
    return toplevel_workspace_action(ctx, param, kywc_toplevel_move_to_workspace,
                                     "movewindowtoworkspace");
}

int cmd_movewindowtooutput(struct context *ctx, const char *param)
{
    return toplevel_workspace_action(ctx, param, kywc_toplevel_move_to_output,
                                     "movewindowtooutput");
}
