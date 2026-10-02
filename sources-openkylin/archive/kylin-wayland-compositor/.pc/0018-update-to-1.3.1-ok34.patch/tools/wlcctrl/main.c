// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _DEFAULT_SOURCE
#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <kywc/boxes.h>
#include <kywc/identifier.h>
#include <linux/input-event-codes.h>

#include "buffer.h"
#include "cursor.h"
#include "keyboard.h"
#include "output.h"
#include "util/spawn.h"
#include "wlcctrl.h"

static struct {
    char *param;
    char *dx;
    char *dy;
    char *width;
    char *height;
    char *png_path;
    char *area;
    char *output;
    const char *primary_output;
    const char *action;
    const char *actions[48];
    char *params[48];
    bool list_toplevel;
    bool list_output;
    bool list_workspace;
    bool getpixel;
    int workspace_num;
    bool buffer_done;
} options;

#define WLCCTRL_MAJOR 1
#define WLCCTRL_MINOR 0
#define WLCCTRL_PATH 0

#define MAX_STRLEN 100 // 字符串最大长度

static void capture_action(kywc_context *context, const char *source_uuid, const char *output_uuid);

static void handle_destroy(kywc_context *ctx, void *data)
{
    struct kywc_buffer_helper *helper = data;
    kywc_buffer_helper_destroy(helper);
}

static void handle_create(kywc_context *ctx, void *data)
{
    struct kywc_buffer_helper *helper = kywc_buffer_helper_create(ctx);
    if (!helper) {
        return;
    }

    kywc_context_set_user_data(ctx, helper);
}

static void handle_new_workspace(kywc_context *context, kywc_workspace *workspace, void *data)
{
    options.workspace_num++;
}

static void handle_new_output(kywc_context *context, kywc_output *output, void *data) {}

static void handle_new_toplevel(kywc_context *context, kywc_toplevel *toplevel, void *data) {}

static struct kywc_context_interface context_impl = {
    .create = handle_create,
    .destroy = handle_destroy,
    .new_output = handle_new_output,
    .new_toplevel = handle_new_toplevel,
    .new_workspace = handle_new_workspace,
};

static void sleep_action(void)
{
    int sleep_ms = atoi(options.param);
    usleep(sleep_ms * 1000);
}

static bool print_toplevel(kywc_toplevel *toplevel, void *date)
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

static bool print_workspace(kywc_workspace *workspace, void *date)
{
    if (!workspace) {
        return false;
    }

    printf("workspace \"%s\"\n", workspace->uuid ? workspace->uuid : "unknown");
    printf("  name: %s\n", workspace->name ? workspace->name : "unknown");
    printf("  position: %d\n", workspace->position);
    printf("  activated: %s\n", workspace->activated ? "true" : "false");
    printf("\n");
    return false;
}

static bool active_toplevel(kywc_toplevel *toplevel, void *date)
{
    if (toplevel->activated) {
        printf("  uuid : %s\n", toplevel->uuid ? toplevel->uuid : "unknown");
    }

    return false;
}

static bool find_toplevel(kywc_toplevel *toplevel, void *data)
{
    if (!toplevel || !data) {
        return false;
    }

    const char *pattern = (const char *)data;
    regex_t regex;
    int ret;

    if ((ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE)) != 0) {
        char msgbuf[100];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Could not compile regex: %s\n", msgbuf);
        return false;
    }

    if ((toplevel->icon && (ret = regexec(&regex, toplevel->icon, 0, NULL, 0)) == 0) ||
        (toplevel->app_id && (ret = regexec(&regex, toplevel->app_id, 0, NULL, 0)) == 0) ||
        (toplevel->title && (ret = regexec(&regex, toplevel->title, 0, NULL, 0)) == 0)) {
        printf("uuid : %s\n", toplevel->uuid ? toplevel->uuid : "unknown");
    } else if (ret != REG_NOMATCH) {
        char msgbuf[100];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
    }

    regfree(&regex);
    return false;
}

static bool kill_toplevel(kywc_toplevel *toplevel, void *date)
{
    if (!toplevel || !date) {
        return false;
    }

    char *app = date;
    if (toplevel->pid && toplevel->app_id && strcmp(app, toplevel->app_id) == 0) {
        if (kill(toplevel->pid, SIGTERM) == -1) {
            perror("kill");
            return true;
        }
        printf("Sent SIGTERM to process with PID %d\n", toplevel->pid);
        sleep(1);

        if (kill(toplevel->pid, 0) == -1 && errno == ESRCH) {
            printf("Process with PID %d has terminated.\n", toplevel->pid);
        } else {
            if (kill(toplevel->pid, SIGKILL) == -1) {
                perror("kill");
                return true;
            }
            printf("Sent SIGKILL to process with PID %d\n", toplevel->pid);
        }
        return true;
    }
    return false;
}

static void window_action(struct wlcom_ctrl_manager *manager)
{
    if (!options.action || !manager) {
        return;
    }

    if (strcmp(options.action, "getactivewindow") == 0) {
        int num = 0;
        kywc_context_for_each_toplevel(manager->kywc_ctx, active_toplevel, &num);
        return;
    }

    if (strcmp(options.action, "search") == 0) {
        kywc_context_for_each_toplevel(manager->kywc_ctx, find_toplevel, options.param);
        return;
    }

    if (strcmp(options.action, "windowkill") == 0) {
        kywc_context_for_each_toplevel(manager->kywc_ctx, kill_toplevel, options.param);
        return;
    }

    if (strcmp(options.action, "windowcapture") == 0) {
        capture_action(manager->kywc_ctx, options.param, NULL);
        return;
    }

    if (strcmp(options.action, "windowenter") == 0 || strcmp(options.action, "windowleave") == 0 ||
        strcmp(options.action, "movewindowtoworkspace") == 0 ||
        strcmp(options.action, "movewindowtooutput") == 0) {
        char w_uuid[MAX_STRLEN] = "";
        char uuid[MAX_STRLEN] = "";
        if (sscanf(options.param, "%99[^:]:%99s", w_uuid, uuid) != 2) {
            fprintf(stderr, "Error: The --%s option expects to add w_uuid:uuid parameters.\n",
                    options.param);
        }
        kywc_toplevel *toplevel = kywc_context_find_toplevel(manager->kywc_ctx, w_uuid);
        if (!toplevel) {
            fprintf(stderr,
                    "Please enter the correct window UUID for the specified application.\n");
            return;
        }

        if (strcmp(options.action, "windowenter") == 0) {
            kywc_toplevel_enter_workspace(toplevel, uuid);
        }
        if (strcmp(options.action, "windowleave") == 0) {
            kywc_toplevel_leave_workspace(toplevel, uuid);
        }
        if (strcmp(options.action, "movewindowtoworkspace") == 0) {
            kywc_toplevel_move_to_workspace(toplevel, uuid);
        }
        if (strcmp(options.action, "movewindowtooutput") == 0) {
            kywc_toplevel_move_to_output(toplevel, uuid);
        }
        return;
    }

    kywc_toplevel *toplevel = kywc_context_find_toplevel(manager->kywc_ctx, options.param);
    if (!toplevel) {
        fprintf(stderr, "Please enter the correct window UUID for the specified application.\n");
        return;
    }

    if (strcmp(options.action, "windowmaximize") == 0) {
        kywc_toplevel_set_maximized(toplevel,
                                    options.output ? options.output : options.primary_output);
    }

    if (strcmp(options.action, "windowfullscreen") == 0) {
        kywc_toplevel_set_fullscreen(toplevel,
                                     options.output ? options.output : options.primary_output);
    }

    if (strcmp(options.action, "getwindowname") == 0) {
        printf("uuid : %s\nname : %s\nicon: %s\ntitle: %s\npid: %d\n", toplevel->uuid,
               toplevel->app_id, toplevel->icon, toplevel->title, toplevel->pid);
        return;
    }

    if (strcmp(options.action, "getwindowgeometry") == 0) {
        printf("geometry: (%d, %d) %d x %d\n", toplevel->x, toplevel->y, toplevel->width,
               toplevel->height);
        return;
    }

    if (strcmp(options.action, "getwindowstatus") == 0) {
        printf("  activated: %s\n", toplevel->activated ? "yes" : "no");
        printf("  maximized: %s\n", toplevel->maximized ? "yes" : "no");
        printf("  minimized: %s\n", toplevel->minimized ? "yes" : "no");
        printf("  fullscreen: %s\n", toplevel->fullscreen ? "yes" : "no");
        return;
    }

    if (strcmp(options.action, "windowmove") == 0) {
        if (!options.dx || !options.dy) {
            fprintf(stderr, "The --windowmove option expects to add -x and -y parameters.\n");
            return;
        }
        unsigned long x = 0, y = 0;
        if (sscanf(options.dx, "%lu", &x) != 1) {
            return;
        }
        if (sscanf(options.dy, "%lu", &y) != 1) {
            return;
        }
        kywc_toplevel_set_position(toplevel, x, y);
        return;
    }

    if (strcmp(options.action, "windowsize") == 0) {
        if (!options.width || !options.height) {
            fprintf(stderr, "The --windowsize option expects to add -w and -h parameters.\n");
            return;
        }
        unsigned long width, height;
        if (sscanf(options.width, "%lu", &width) != 1) {
            return;
        }
        if (sscanf(options.height, "%lu", &height) != 1) {
            return;
        }

        kywc_toplevel_set_size(toplevel, width, height);
        return;
    }

    if (strcmp(options.action, "windowclose") == 0) {
        kywc_toplevel_close(toplevel);
    }
    if (strcmp(options.action, "windowminimize") == 0) {
        kywc_toplevel_set_minimized(toplevel);
    }
    if (strcmp(options.action, "windowactivate") == 0) {
        kywc_toplevel_activate(toplevel);
    }
    if (strcmp(options.action, "windowmap") == 0) {
        if (toplevel->minimized) {
            kywc_toplevel_unset_minimized(toplevel);
        } else if (toplevel->maximized) {
            kywc_toplevel_unset_maximized(toplevel);
        } else if (toplevel->fullscreen) {
            kywc_toplevel_unset_fullscreen(toplevel);
        }
        return;
    }
}

static void workspace_action(struct wlcom_ctrl_manager *manager)
{
    if (!options.action || !manager) {
        return;
    }

    if (strcmp(options.action, "workspaceadd") == 0) {
        unsigned int num;
        if (!options.param) {
            kywc_workspace_create(manager->kywc_ctx, NULL, options.workspace_num);
            options.workspace_num++;
            fprintf(stdout, "Successfully added a new workspace.\n");
        } else if (sscanf(options.param, "%u", &num) != 1) {
            fprintf(stderr, "The --workspaceadd option expects an integer.\n");
        } else if ((options.workspace_num + num) > MAX_WORKSPACES) {
            fprintf(stderr, "The total number of workspaces cannot exceed %d.\n", MAX_WORKSPACES);
        } else {
            for (unsigned int i = 0; i < num; i++) {
                kywc_workspace_create(manager->kywc_ctx, NULL, options.workspace_num + i);
            }
            fprintf(stdout, "Successfully added %d new workspaces.\n", num);
        }
        return;
    }

    if (strcmp(options.action, "workspacecapture") == 0) {
        char w_uuid[MAX_STRLEN] = "";
        char o_uuid[MAX_STRLEN] = "";
        if (sscanf(options.param, "%99[^:]:%99s", w_uuid, o_uuid) == 2) {
            capture_action(manager->kywc_ctx, w_uuid, o_uuid);
        } else {
            fprintf(
                stderr,
                "Error: The --workspacecapture option expects to add w_uuid:o_uuid parameters.\n");
        }
        return;
    }

    kywc_workspace *workspace = kywc_context_find_workspace(manager->kywc_ctx, options.param);
    if (!workspace) {
        fprintf(stderr, "Please enter the correct uuid for the specified workspace.\n");
        return;
    }

    if (strcmp(options.action, "workspaceremove") == 0) {
        kywc_workspace_remove(workspace);
    }

    if (strcmp(options.action, "workspaceactive") == 0) {
        kywc_workspace_activate(workspace);
    }

    return;
}

static void get_output_geometroy(struct wlcom_ctrl_manager *manager)
{
    if (!manager) {
        return;
    }

    kywc_output *output = kywc_context_find_output(manager->kywc_ctx, options.param);
    if (!output) {
        return;
    }

    printf("  physical size: %dx%d mm\n", output->physical_width, output->physical_height);
    printf("  scale: %f\n", output->scale);
    printf("  transform: %d\n", output->transform);
    if (output->enabled) {
        if (output->mode) {
            printf("  mode: %d x %d @ %d\n", output->mode->width, output->mode->height,
                   output->mode->refresh / 1000);
        } else {
            printf("  mode: unknown");
        }
        printf("  position: %d, %d\n", output->x, output->y);
    }
}

static bool get_primary_output(kywc_output *output, void *data)
{
    if (!output) {
        return false;
    }

    if (output->primary) {
        options.primary_output = output->uuid;
        return true;
    }
    return false;
}

static void output_action(struct wlcom_ctrl_manager *manager)
{
    if (!manager) {
        return;
    }

    if (options.action && strcmp(options.action, "getdisplaygeometry") == 0) {
        get_output_geometroy(manager);
        return;
    }

    kywc_context_for_each_output(manager->kywc_ctx, get_primary_output, NULL);
    if (options.action &&
        (strcmp(options.action, "fullscreencapture") == 0 ||
         strcmp(options.action, "setarea") == 0 || strcmp(options.action, "getpixelcolor") == 0)) {
        capture_action(manager->kywc_ctx, NULL,
                       options.output ? options.output : options.primary_output);
        return;
    }
}

static void print_list_info(struct wlcom_ctrl_manager *manager)
{
    if (!manager) {
        return;
    }

    if (options.list_toplevel) {
        kywc_context_for_each_toplevel(manager->kywc_ctx, print_toplevel, NULL);
        options.list_toplevel = false;
    }

    if (options.list_workspace) {
        kywc_context_for_each_workspace(manager->kywc_ctx, print_workspace, NULL);
        options.list_workspace = false;
    }

    if (options.list_output) {
        kywc_context_for_each_output(manager->kywc_ctx, print_output, NULL);
        options.list_output = false;
    }
}

static void cursor_handle_position(kywc_cursor *cursor, uint32_t x, uint32_t y)
{
    printf("current cursor position : %d, %d\n", x, y);
}

static void cursor_handle_hotspot(kywc_cursor *cursor, uint32_t x, uint32_t y) {}

static void cursor_handle_entry(kywc_cursor *cursor) {}

static void cursor_handle_leave(kywc_cursor *cursor) {}

static void cursor_handle_destroy(kywc_cursor *cursor) {}

static struct kywc_cursor_interface cursor_impl = {
    .position = cursor_handle_position,
    .hotspot = cursor_handle_hotspot,
    .enter = cursor_handle_entry,
    .leave = cursor_handle_leave,
    .destroy = cursor_handle_destroy,
};

static void point_action(struct wlcom_ctrl_manager *manager)
{
    if (!options.action || !manager || !manager->cursor) {
        return;
    }

    if (strcmp(options.action, "mousemove") == 0) {
        uint32_t x, y;
        if (sscanf(options.param, "%u,%u", &x, &y) == 2) {
            uint32_t pos_x, pos_y;
            output_transform_coord(manager->cursor->output, x - 1, y - 1, &pos_x, &pos_y);
            cursor_set_motion_absolute(manager->cursor, pos_x, pos_y);
            output_transform_coord(manager->cursor->output, x, y, &pos_x, &pos_y);
            cursor_set_motion_absolute(manager->cursor, pos_x, pos_y);
        } else {
            fprintf(stderr,
                    "The --mousemove option expects two integers separated with a comma.\n");
            return;
        }
    } else if (strcmp(options.action, "mousebutton") == 0 ||
               strcmp(options.action, "mousepress") == 0 ||
               strcmp(options.action, "mouserelease") == 0) {
        uint32_t button;
        if (sscanf(options.param, "%u", &button) != 1) {
            fprintf(stderr, "The --%s option expects an integer.\n", options.action);
            return;
        }

        int buttons[] = {
            0, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_EXTRA, BTN_SIDE, BTN_FORWARD, BTN_BACK,
        };
        int button_state = buttons[button];

        if (strcmp(options.action, "mousebutton") == 0) {
            cursor_button_click(manager->cursor, button_state);
        }

        if (strcmp(options.action, "mousepress") == 0) {
            cursor_button_press(manager->cursor, button_state, true);
        }

        if (strcmp(options.action, "mouserelease") == 0) {
            cursor_button_press(manager->cursor, button_state, false);
        }

    } else if (strcmp(options.action, "scroll") == 0) {
        if (!options.dx && !options.dy) {
            fprintf(stderr, "The --scorll option expects to add -x or -y parameters.\n");
            return;
        }
        int x = 0, y = 0;
        if (options.dx) {
            if (sscanf(options.dx, "%d", &x) != 1) {
                return;
            }
        }
        if (options.dy) {
            if (sscanf(options.dy, "%d", &y) != 1) {
                return;
            }
        }
        cursor_axis_set(manager->cursor, x, y);
    } else if (strcmp(options.action, "getmouselocation") == 0) {
        kywc_cursor_create(manager->kywc_ctx, manager->seat->wl_seat, &cursor_impl, NULL);
        wl_display_roundtrip(manager->wl_display);
    }
}

static void keyboard_actions(struct wlcom_ctrl_manager *manager)
{
    if (!manager->keyboard) {
        return;
    }

    if (options.action && strcmp(options.action, "key") == 0) {
        keyboard_set(manager->keyboard, options.param);
        return;
    }

    if (options.action && strcmp(options.action, "keydown") == 0) {
        keyboard_press(manager->keyboard, options.param, true);
        return;
    }

    if (options.action && strcmp(options.action, "keyup") == 0) {
        keyboard_press(manager->keyboard, options.param, false);
        return;
    }
}

static void exec_action(void)
{
    if (options.param) {
        spawn_shell(options.param);
    }
}

static const char usage[] =
    "Usage: wlcctrl [options...]\n"
    "   --help\n"
    "   -v, --version\n"
    "   --exec\n"
    "   --keystring\n"
    "   --key\n"
    "   --keyup\n"
    "   --keydown\n"
    "   --sleep\n"
    "   --mousebutton\n"
    "   --mousemove\n"
    "   --mousepress\n"
    "   --mouserelease\n"
    "   --scroll\n"
    "   --getmouselocation\n"
    "   -l, --list\n"
    "   --windowmove\n"
    "   --windowsize\n"
    "   --windowminimize\n"
    "   --windowmaximize\n"
    "   --windowfullscreen\n"
    "   --windowenter\n"
    "   --windowleave\n"
    "   --movewindowtoworkspace\n"
    "   --movewindowtooutput\n"
    "   --windowmap\n"
    "   --windowclose\n"
    "   --windowkill\n"
    "   --windowactivate\n"
    "   --windowcapture\n"
    "   --getactivewindow\n"
    "   --getwindowname\n"
    "   --getwindowgeometry\n"
    "   --getwindowstatus\n"
    "   --search\n"
    "   --outputs\n"
    "   --getdisplaygeometry\n"
    "   --fullscreencapture\n"
    "   --setarea\n"
    "   --getpixelcolor\n"
    "   --workspace\n"
    "   --workspacecapture\n"
    "   --workspaceadd\n"
    "   --workspaceremove\n"
    "   --workspaceactive\n"
    "For more information, please see the basic manual page for the wlcctrl command-line tool.\n";

static const struct option long_options[] = {
    { "help", no_argument, 0, 0 },
    { "version", no_argument, 0, 'v' },
    { "list", no_argument, 0, 'l' },

    { "workspace", no_argument, 0, 0 },
    { "workspacecapture", required_argument, 0, 0 }, // uuid
    { "workspaceadd", required_argument, 0, 0 },
    { "workspaceremove", required_argument, 0, 0 },
    { "workspaceactive", required_argument, 0, 0 },

    { "exec", required_argument, 0, 0 },

    { "mousemove", required_argument, 0, 0 },
    { "mousebutton", required_argument, 0, 0 },
    { "scroll", no_argument, 0, 0 },
    { "getmouselocation", no_argument, 0, 0 },

    { "mousepress", required_argument, 0, 0 },
    { "mouserelease", required_argument, 0, 0 },

    { "getactivewindow", no_argument, 0, 0 },
    { "getwindowname", required_argument, 0, 0 },     // uuid
    { "getwindowgeometry", required_argument, 0, 0 }, // uuid
    { "getwindowstatus", required_argument, 0, 0 },   // uuid

    { "windowkill", required_argument, 0, 0 },            // nameid
    { "search", required_argument, 0, 0 },                // param app_id
    { "windowclose", required_argument, 0, 0 },           // uuid
    { "windowsize", required_argument, 0, 0 },            // uuid
    { "windowmove", required_argument, 0, 0 },            // uuid
    { "windowminimize", required_argument, 0, 0 },        // uuid
    { "windowmaximize", required_argument, 0, 0 },        // uuid
    { "windowfullscreen", required_argument, 0, 0 },      // uuid
    { "windowactivate", required_argument, 0, 0 },        // uuid
    { "windowmap", required_argument, 0, 0 },             // uuid
    { "windowcapture", required_argument, 0, 0 },         // uuid
    { "windowenter", required_argument, 0, 0 },           // uuid
    { "windowleave", required_argument, 0, 0 },           // uuid
    { "movewindowtoworkspace", required_argument, 0, 0 }, // uuid
    { "movewindowtooutput", required_argument, 0, 0 },    // uuid

    { "outputs", no_argument, 0, 0 },
    { "getdisplaygeometry", required_argument, 0, 0 }, // uuid
    { "fullscreencapture", no_argument, 0, 0 },        // uuid

    { "setarea", required_argument, 0, 0 }, // x,y,width,height
    { "path", required_argument, 0, 0 },
    { "getpixelcolor", required_argument, 0, 0 }, // x,y

    { "key", required_argument, 0, 0 },
    { "keyup", required_argument, 0, 0 },
    { "keydown", required_argument, 0, 0 },
    { "keystring", required_argument, 0, 0 },
    { "sleep", required_argument, 0, 0 },

    { 0, 0, 0, 0 },
};

static bool thumbnail_handle_buffer(kywc_thumbnail *thumbnail,
                                    const struct kywc_thumbnail_buffer *buffer, void *data)
{
    options.buffer_done = true;
    kywc_context *ctx = kywc_thumbnail_get_context(thumbnail);
    struct kywc_buffer_helper *helper = kywc_context_get_user_data(ctx);
    if (!helper) {
        return false;
    }

    struct kywc_buffer *kywc_buffer =
        kywc_buffer_helper_import_thumbnail(helper, thumbnail, buffer);
    if (!kywc_buffer) {
        return false;
    }

    if (options.getpixel) {
        uint32_t x, y;
        if (sscanf(options.area, "%u,%u", &x, &y) == 2) {
            uint32_t pixel = kywc_buffer_get_pixel(kywc_buffer, x, y);
            printf("pixel position %d,%d\tR:%d G:%d B:%d A:%0.1f\n", x, y, (pixel >> 16) & 0xff,
                   (pixel >> 8) & 0xff, pixel & 0xff, ((pixel >> 24) & 0xff) / 255.0);
        } else {
            fprintf(stderr, "The --getpixel option expects two integers separated with a comma.\n");
        }
        return false;
    }

    struct kywc_box area = { 0, 0, buffer->width, buffer->height };
    if (options.area) {
        if (sscanf(options.area, "%u,%u,%u,%u", &area.x, &area.y, &area.width, &area.height) != 4) {
            printf("The --setarea option expects four integers separated with a comma.\n");
            return false;
        }
    }

    const char *filename = options.png_path, *path = NULL;
    if (!filename) {
        path = kywc_identifier_time_generate(NULL, ".png");
        filename = path;
    }

    if (!kywc_buffer_write_to_file(kywc_buffer, area.x, area.y, area.width, area.height,
                                   filename)) {
        printf("Create file %s faild!\n", filename);
    }
    printf("Create file %s success!\n", filename);

    free((void *)path);
    return false;
}

static void thumbnail_handle_destroy(kywc_thumbnail *thumbnail, void *data)
{
    options.buffer_done = true;
    struct kywc_buffer *kywc_buffer = kywc_thumbnail_get_user_data(thumbnail);
    kywc_buffer_destroy(kywc_buffer);
}

static struct kywc_thumbnail_interface thumbnail_impl = {
    .buffer = thumbnail_handle_buffer,
    .destroy = thumbnail_handle_destroy,
};

static void capture_action(kywc_context *context, const char *source_uuid, const char *output_uuid)
{
    bool thumbnail_flag = false;
    options.buffer_done = false;
    if (source_uuid && !output_uuid) {
        if (kywc_thumbnail_create_from_toplevel(context, source_uuid, false, &thumbnail_impl,
                                                NULL)) {
            thumbnail_flag = true;
        }
    } else if (source_uuid && output_uuid) {
        if (kywc_thumbnail_create_from_workspace(context, source_uuid, output_uuid, &thumbnail_impl,
                                                 NULL)) {
            thumbnail_flag = true;
        }
    } else if (!source_uuid && output_uuid) {
        if (kywc_thumbnail_create_from_output(context, output_uuid, &thumbnail_impl, NULL)) {
            thumbnail_flag = true;
        }
    }

    if (!thumbnail_flag) {
        return;
    }

    int ret = 0;
    struct wl_display *wl_display = kywc_context_get_display(context);
    while (!options.buffer_done && ret >= 0) {
        ret = wl_display_dispatch(wl_display);
    }
}

int main(int argc, char *argv[])
{
#ifdef _GNU_SOURCE
    optind = 0;
#endif

    if (argc <= 1) {
        printf("%s", usage);
        return 0;
    }

    options.workspace_num = 0;
    options.list_output = false;
    options.list_toplevel = false;
    options.list_workspace = false;
    options.getpixel = false;

    struct wlcom_ctrl_manager *manager = wayland_init();
    if (!manager) {
        return -1;
    }

    uint32_t caps = KYWC_CONTEXT_CAPABILITY_WORKSPACE | KYWC_CONTEXT_CAPABILITY_OUTPUT |
                    KYWC_CONTEXT_CAPABILITY_TOPLEVEL | KYWC_CONTEXT_CAPABILITY_THUMBNAIL |
                    KYWC_CONTEXT_CAPABILITY_CURSOR;
    kywc_context *ctx =
        kywc_context_create_by_display(manager->wl_display, caps, &context_impl, NULL);
    if (!ctx) {
        return -1;
    }
    manager->kywc_ctx = ctx;
    kywc_context_process(ctx);

    int option_index = -1, opt_count = 0, opt;
    const char *name;
    while ((opt = getopt_long(argc, argv, "lo:x:y:S:w:h:v", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'l':
            options.list_toplevel = true;
            break;
        case 'o':
            options.output = optarg;
            break;
        case 'x':
            options.dx = optarg;
            break;
        case 'y':
            options.dy = optarg;
            break;
        case 'w':
            options.width = optarg;
            break;
        case 'h':
            options.height = optarg;
            break;
        case 'v':
            printf("wlcctrl v%d.%d.%d\n", WLCCTRL_MAJOR, WLCCTRL_MINOR, WLCCTRL_PATH);
            return EXIT_SUCCESS;
        case '?':
            if (optopt == 0) {
                if (argv[optind - 1] && strcmp(argv[optind - 1], "--workspaceadd") == 0) {
                    optarg = "1";
                    opt = 0;
                    name = "workspaceadd";
                    goto default_handle;
                }
            }
            printf("%s", usage);
            goto end;
        case 0:
            name = long_options[option_index].name;

        default_handle:
            if (strncmp(argv[optind - 1], "--", 2) == 0) {
                const char *provided_arg = argv[optind - 1] + 2;
                if (strcmp(provided_arg, name) != 0) {
                    printf("Error: '--%s' is not a valid option. Did you mean '--%s'?\n",
                           provided_arg, name);
                    goto end;
                }
            }

            if (strcmp(name, "help") == 0) {
                printf("%s", usage);
                goto end;
            } else if (strcmp(name, "outputs") == 0) {
                options.list_output = true;
            } else if (strcmp(name, "workspace") == 0) {
                options.list_workspace = true;
            } else if (strcmp(name, "workspacecapture") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "workspaceadd") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "workspaceremove") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "workspaceactive") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "getactivewindow") == 0) {
                options.actions[opt_count] = name;
            } else if (strcmp(name, "getwindowname") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "getwindowgeometry") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "getwindowstatus") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowminimize") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowmaximize") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowmove") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowclose") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowactivate") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowmap") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowcapture") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowsize") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowfullscreen") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowenter") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowleave") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "movewindowtoworkspace") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "movewindowtooutput") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "search") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "windowkill") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "keystring") == 0) { // keyboard actions
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "key") == 0) { // keyboard actions
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "keyup") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "keydown") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "mousebutton") == 0) { // mouse action
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "scroll") == 0) {
                options.actions[opt_count] = name;
            } else if (strcmp(name, "mousemove") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "getmouselocation") == 0) {
                options.actions[opt_count] = name;
            } else if (strcmp(name, "getdisplaygeometry") == 0) { // output actions
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "exec") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "mousepress") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "mouserelease") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else if (strcmp(name, "path") == 0) {
                options.png_path = optarg;
            } else if (strcmp(name, "fullscreencapture") == 0) {
                options.actions[opt_count] = name;
            } else if (strcmp(name, "setarea") == 0) {
                options.actions[opt_count] = name;
                options.area = optarg;
            } else if (strcmp(name, "getpixelcolor") == 0) {
                options.actions[opt_count] = name;
                options.getpixel = true;
                options.area = optarg;
            } else if (strcmp(name, "sleep") == 0) {
                options.actions[opt_count] = name;
                options.params[opt_count] = optarg;
            } else {
                printf("%s", usage);
                goto end;
            }
            break;
        default:
            printf("Unexpected option: %c\n", opt);
            printf("%s", usage);
            goto end;
        }
        opt_count++;
    }

    manager->cursor = wlcctrl_cursor_init();
    manager->keyboard = wlcctrl_keyboard_init();
    struct xkb_rule_names rule_names = { 0 };
    if (keyboard_init(manager->keyboard, &rule_names) != 0) {
        printf("Failed to initialise keyboard.\n");
        goto end;
    }

    for (int i = 0; i < opt_count; i++) {
        print_list_info(manager);
        options.action = options.actions[i];
        options.param = options.params[i];
        if (options.action) {
            if (options.action && strcmp(options.action, "sleep") == 0) {
                sleep_action();
            }

            if (strcmp(options.action, "search") == 0 ||
                strcmp(options.action, "getactivewindow") == 0 ||
                strcmp(options.action, "windowclose") == 0 ||
                strcmp(options.action, "windowkill") == 0 ||
                strcmp(options.action, "getwindowname") == 0 ||
                strcmp(options.action, "windowmap") == 0 ||
                strcmp(options.action, "windowcapture") == 0 ||
                strcmp(options.action, "getwindowgeometry") == 0 ||
                strcmp(options.action, "getwindowstatus") == 0 ||
                strcmp(options.action, "windowsize") == 0 ||
                strcmp(options.action, "windowmove") == 0 ||
                strcmp(options.action, "windowminimize") == 0 ||
                strcmp(options.action, "windowmaximize") == 0 ||
                strcmp(options.action, "windowfullscreen") == 0 ||
                strcmp(options.action, "windowenter") == 0 ||
                strcmp(options.action, "windowleave") == 0 ||
                strcmp(options.action, "movewindowtoworkspace") == 0 ||
                strcmp(options.action, "movewindowtooutput") == 0 ||
                strcmp(options.action, "windowactivate") == 0) {
                window_action(manager);
            }

            if (strcmp(options.action, "mousemove") == 0 || strcmp(options.action, "scroll") == 0 ||
                strcmp(options.action, "mousebutton") == 0 ||
                strcmp(options.action, "mousepress") == 0 ||
                strcmp(options.action, "mouserelease") == 0 ||
                strcmp(options.action, "getmouselocation") == 0) {
                point_action(manager);
            }

            if (strcmp(options.action, "exec") == 0) {
                exec_action();
            }

            if (strcmp(options.action, "getdisplaygeometry") == 0 ||
                strcmp(options.action, "fullscreencapture") == 0 ||
                strcmp(options.action, "setarea") == 0 ||
                strcmp(options.action, "getpixelcolor") == 0) {
                output_action(manager);
            }

            if (strcmp(options.action, "workspacecapture") == 0 ||
                strcmp(options.action, "workspaceremove") == 0 ||
                strcmp(options.action, "workspaceactive") == 0 ||
                strcmp(options.action, "workspaceadd") == 0) {
                workspace_action(manager);
            }

            if (strcmp(options.action, "keystring") == 0) {
                if (key_string_action(manager->keyboard, options.param) != 0) {
                    printf("Failed to initialise keyboard.\n");
                    goto end;
                }
            }

            if (strcmp(options.action, "key") == 0 || strcmp(options.action, "keyup") == 0 ||
                strcmp(options.action, "keydown") == 0) {
                keyboard_actions(manager);
            }
        }
    }
end:
    if (manager->keyboard) {
        keyboard_destroy(manager->keyboard);
    }
    if (manager->cursor) {
        cursor_destroy(manager->cursor);
    }
    kywc_context_destroy(ctx);
    wayland_deinit(manager);
    return 0;
}
