// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "cursor.h"
#include "keyboard.h"
#include "wlcctrl.h"

#define WLCCTRL_MAJOR 1
#define WLCCTRL_MINOR 0
#define WLCCTRL_PATCH 0

struct action {
    const struct command *cmd;
    const char *param;
};

struct action_list {
    struct action *items;
    size_t count;
    size_t capacity;
};

static bool action_list_append(struct action_list *list, const struct command *cmd,
                               const char *param)
{
    if (list->count == list->capacity) {
        size_t capacity = list->capacity ? list->capacity * 2 : 8;
        struct action *items = realloc(list->items, capacity * sizeof(*items));
        if (!items) {
            return false;
        }
        list->items = items;
        list->capacity = capacity;
    }

    list->items[list->count].cmd = cmd;
    list->items[list->count].param = param;
    list->count++;
    return true;
}

static void print_usage(FILE *f)
{
    fprintf(f, "Usage: wlcctrl [options...]\n");
    fprintf(f, "   --help\n");
    fprintf(f, "   -v, --version\n");
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, "list") == 0) {
            fprintf(f, "   -l, --list\n");
        } else {
            fprintf(f, "   --%s\n", commands[i].name);
        }
    }
    fprintf(f, "For more information, please see the basic manual page for the wlcctrl "
               "command-line tool.\n");
}

static bool is_bare_optional_command(const char *token)
{
    if (strncmp(token, "--", 2) != 0) {
        return false;
    }
    token += 2;

    for (size_t i = 0; i < command_count; i++) {
        if (commands[i].arg != ARG_OPTIONAL) {
            continue;
        }
        size_t len = strlen(commands[i].name);
        if (strncmp(token, commands[i].name, len) == 0 && token[len] == '\0') {
            return true;
        }
    }
    return false;
}

/* insert the default parameter after ARG_OPTIONAL commands given without one */
static char **preprocess_argv(int argc, char **argv, int *new_argc)
{
    size_t extra = 0;
    for (int i = 1; i < argc; i++) {
        if (is_bare_optional_command(argv[i]) && (i + 1 >= argc || argv[i + 1][0] == '-')) {
            extra++;
        }
    }

    *new_argc = argc + (int)extra;
    if (extra == 0) {
        return argv;
    }

    char **new_argv = malloc((argc + extra + 1) * sizeof(*new_argv));
    if (!new_argv) {
        return NULL;
    }

    static char default_param[] = "1";
    size_t j = 0;
    for (int i = 0; i < argc; i++) {
        new_argv[j++] = argv[i];
        if (i > 0 && is_bare_optional_command(argv[i]) &&
            (i + 1 >= argc || argv[i + 1][0] == '-')) {
            new_argv[j++] = default_param;
        }
    }
    new_argv[j] = NULL;
    return new_argv;
}

/* commands first, then the fixed --help/--version/--path entries */
static struct option *build_long_options(void)
{
    size_t n = command_count + 3;
    struct option *opts = calloc(n + 1, sizeof(*opts));
    if (!opts) {
        return NULL;
    }

    size_t i;
    for (i = 0; i < command_count; i++) {
        opts[i].name = commands[i].name;
        opts[i].has_arg = commands[i].arg == ARG_NONE ? no_argument : required_argument;
    }
    opts[i].name = "help";
    i++;
    opts[i].name = "version";
    opts[i].val = 'v';
    i++;
    opts[i].name = "path";
    opts[i].has_arg = required_argument;
    return opts;
}

static void context_handle_create(kywc_context *ctx, void *data)
{
    capture_context_create(ctx);
}

static void context_handle_destroy(kywc_context *ctx, void *data)
{
    capture_context_destroy(ctx);
}

/* kywc_context keeps the interface pointer, so these must outlive main */
static const struct kywc_context_interface context_impl_plain = { 0 };

static const struct kywc_context_interface context_impl_thumbnail = {
    .create = context_handle_create,
    .destroy = context_handle_destroy,
};

static bool init_runtime(struct context *ctx, uint32_t caps, uint32_t needs)
{
    ctx->manager = wayland_init(needs);
    if (!ctx->manager) {
        fprintf(stderr, "Failed to connect to the wayland display.\n");
        return false;
    }

    if (caps != 0) {
        const struct kywc_context_interface *impl = &context_impl_plain;
        if (caps & KYWC_CONTEXT_CAPABILITY_THUMBNAIL) {
            impl = &context_impl_thumbnail;
        }

        ctx->kywc = kywc_context_create_by_display(ctx->manager->wl_display, caps, impl, NULL);
        if (!ctx->kywc) {
            fprintf(stderr, "Failed to create the kywc context.\n");
            return false;
        }
        ctx->manager->kywc_ctx = ctx->kywc;
        kywc_context_process(ctx->kywc);
    }

    if (needs & NEED_KEYBOARD) {
        ctx->manager->keyboard = wlcctrl_keyboard_init(ctx->manager);
        struct xkb_rule_names rule_names = { 0 };
        if (!ctx->manager->keyboard || keyboard_init(ctx->manager->keyboard, &rule_names) != 0) {
            fprintf(stderr, "Failed to initialise keyboard.\n");
            return false;
        }
    }

    if (needs & NEED_POINTER) {
        ctx->manager->cursor = wlcctrl_cursor_init(ctx->manager);
        if (!ctx->manager->cursor) {
            fprintf(stderr, "Failed to initialise pointer.\n");
            return false;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        print_usage(stdout);
        return EXIT_SUCCESS;
    }

    int parsed_argc;
    char **parsed_argv = preprocess_argv(argc, argv, &parsed_argc);
    if (!parsed_argv) {
        return EXIT_FAILURE;
    }

    struct option *long_options = build_long_options();
    if (!long_options) {
        return EXIT_FAILURE;
    }

    struct context ctx = { 0 };
    struct action_list actions = { 0 };
    int rc = EXIT_SUCCESS;

    int option_index = -1, opt;
    while ((opt = getopt_long(parsed_argc, parsed_argv, "lo:x:y:w:h:v", long_options,
                              &option_index)) != -1) {
        switch (opt) {
        case 'l':
            if (!action_list_append(&actions, command_find("list"), NULL)) {
                goto oom;
            }
            break;
        case 'o':
            ctx.output = optarg;
            break;
        case 'x':
            ctx.x = optarg;
            break;
        case 'y':
            ctx.y = optarg;
            break;
        case 'w':
            ctx.w = optarg;
            break;
        case 'h':
            ctx.h = optarg;
            break;
        case 'v':
            printf("wlcctrl v%d.%d.%d\n", WLCCTRL_MAJOR, WLCCTRL_MINOR, WLCCTRL_PATCH);
            goto out;
        case 0:
            if ((size_t)option_index < command_count) {
                if (!action_list_append(&actions, &commands[option_index], optarg)) {
                    goto oom;
                }
            } else if (strcmp(long_options[option_index].name, "help") == 0) {
                print_usage(stdout);
                goto out;
            } else { /* --path */
                ctx.path = optarg;
            }
            break;
        default:
            print_usage(stderr);
            rc = EXIT_FAILURE;
            goto out;
        }
    }

    uint32_t caps = 0, needs = 0;
    for (size_t i = 0; i < actions.count; i++) {
        caps |= actions.items[i].cmd->caps;
        needs |= actions.items[i].cmd->needs;
    }

    if ((caps != 0 || needs != 0) && !init_runtime(&ctx, caps, needs)) {
        rc = EXIT_FAILURE;
        goto out;
    }

    for (size_t i = 0; i < actions.count; i++) {
        if (actions.items[i].cmd->run(&ctx, actions.items[i].param) != 0) {
            rc = EXIT_FAILURE;
            break;
        }
    }

out:
    if (ctx.manager) {
        if (ctx.manager->keyboard) {
            keyboard_destroy(ctx.manager->keyboard);
        }
        if (ctx.manager->cursor) {
            cursor_destroy(ctx.manager->cursor);
        }
    }
    if (ctx.kywc) {
        kywc_context_destroy(ctx.kywc);
    }
    if (ctx.manager) {
        wayland_deinit(ctx.manager);
    }
    free(actions.items);
    free(long_options);
    if (parsed_argv != argv) {
        free(parsed_argv);
    }
    return rc;

oom:
    fprintf(stderr, "Out of memory.\n");
    rc = EXIT_FAILURE;
    goto out;
}
