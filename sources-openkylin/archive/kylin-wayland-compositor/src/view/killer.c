// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <wlr/types/wlr_xdg_foreign_registry.h>

#include <kywc/log.h>

#include "server.h"
#include "util/dialog.h"
#include "util/nls.h"
#include "util/queue.h"
#include "util/spawn.h"
#include "view_p.h"

static struct kill_dialog_manager {
    struct wl_list kill_dialogs;

    struct view_manager *view_manager;
    struct wl_listener server_destroy;
} *manager = NULL;

struct kill_dialog {
    struct wl_list link;

    struct wlr_xdg_foreign_exported parent_exported;
    struct wl_event_source *event_context;
    int dialog_socket;

    struct view *view;
    struct wl_listener view_destroy;
};

static struct kill_dialog *kill_dialog_from_view(struct view *view)
{
    struct kill_dialog *hang_window;
    wl_list_for_each(hang_window, &manager->kill_dialogs, link) {
        if (hang_window->view == view) {
            return hang_window;
        }
    }
    return NULL;
}

static void kill_dialog_destroy(struct kill_dialog *dialog)
{
    if (!dialog) {
        return;
    }

    if (dialog->event_context) {
        wl_event_source_remove(dialog->event_context);
    }

    /* Send close request to wldialog */
    if (dialog->dialog_socket >= 0) {
        dialog_close(dialog->dialog_socket);
        close(dialog->dialog_socket);
    }

    /* Remove the exported surface from the foreign registry */
    if (dialog->parent_exported.registry) {
        wlr_xdg_foreign_exported_finish(&dialog->parent_exported);
    }

    wl_list_remove(&dialog->link);
    wl_list_remove(&dialog->view_destroy.link);
    free(dialog);
}

#define KILL_POLL_RETRIES 16
#define KILL_POLL_INTERVAL_US 250000 /* 250ms */

static bool has_pid_aborted(pid_t pid)
{
    for (int i = 0; i < KILL_POLL_RETRIES; i++) {
        int status = 0;
        if (waitpid(pid, &status, WNOHANG | WNOWAIT) == pid) {
            if (WIFSIGNALED(status) || WIFEXITED(status)) {
                return true;
            }
        }
        usleep(KILL_POLL_INTERVAL_US);
    }
    return false;
}

static void kill_job_execute(void *job, void *gdata, int thread_index)
{
    pid_t pid = (pid_t)(intptr_t)job;

    if (kill(pid, SIGABRT) == 0 && has_pid_aborted(pid)) {
        kywc_log(KYWC_DEBUG, "Process %d terminated after SIGABRT", pid);
        return;
    }

    kywc_log(KYWC_WARN, "Process %d did not respond to SIGABRT, sending SIGKILL", pid);
    kill(pid, SIGKILL);
}

static void kill_dialog_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct kill_dialog *dialog = wl_container_of(listener, dialog, view_destroy);

    kill_dialog_destroy(dialog);
}

static void build_kill_dialog_request_json(json_object **out_obj, const char *app_id, pid_t pid,
                                           const char *parent_window)
{
    json_object *obj = json_object_new_object();
    json_object_object_add(obj, "title_bar", json_object_new_string("no_maximizable"));
    json_object_object_add(obj, "window_title",
                           json_object_new_string(tr("Program exception responding")));
    json_object_object_add(obj, "window_icon", json_object_new_string("dialog-warning"));

    if (parent_window) {
        json_object_object_add(obj, "parent_window", json_object_new_string(parent_window));
        json_object_object_add(obj, "modal", json_object_new_boolean(true));
    }

    char temp_str_title[512];
    if (app_id == NULL) {
        snprintf(temp_str_title, sizeof(temp_str_title),
                 tr("Program exception responding(process number: %d). Do you want to force the "
                    "app to close?"),
                 pid);
    } else {
        snprintf(temp_str_title, sizeof(temp_str_title),
                 tr("%s Program exception responding(process number: %d). Do you want to force the "
                    "app to close?"),
                 app_id, pid);
    }
    json_object *title_obj = json_object_new_object();
    json_object_object_add(title_obj, "text", json_object_new_string(temp_str_title));
    json_object_object_add(obj, "title", title_obj);

    char temp_str[512];
    snprintf(temp_str, sizeof(temp_str), tr("Note:Closing the app may loss all unsaved data."));
    json_object *subtitle_obj = json_object_new_object();
    json_object_object_add(subtitle_obj, "text", json_object_new_string(temp_str));
    json_object_object_add(obj, "sub_title", subtitle_obj);

    json_object_object_add(obj, "button_mode", json_object_new_int64(0));

    json_object_object_add(obj, "allow_text", json_object_new_string(tr("Force Close")));
    json_object_object_add(obj, "deny_text", json_object_new_string(tr("Wait")));

    json_object *dialog = json_object_new_object();
    json_object_object_add(dialog, "dialog", obj);
    *out_obj = dialog;
    kywc_log(KYWC_DEBUG, "Built hang window dialog JSON: %s", json_object_to_json_string(obj));
}

static int handle_kill_dialog(int fd, uint32_t mask, void *data)
{
    struct kill_dialog *dialog = data;

    if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
        kywc_log(KYWC_ERROR, "Error event on hang window dialog socket");
        kill_dialog_destroy(dialog);
        return 0;
    }

    struct view *view = dialog->view;

    json_object *obj = dialog_get_reply(fd);
    if (!obj) {
        kywc_log(KYWC_ERROR, "Failed to get dialog reply");
        kill_dialog_destroy(dialog);
        return 0;
    }

    json_object *action_obj;
    if (json_object_object_get_ex(obj, "user_action", &action_obj)) {
        int64_t action = json_object_get_int64(action_obj);
        kywc_log(KYWC_DEBUG, "Hang window dialog action: %ld", action);

        if (action == 0) {
            kywc_log(KYWC_INFO, "User chose to force close view: %s", view->base.app_id);
            queue_add_job(manager->view_manager->server->queue, (void *)(intptr_t)view->base.pid,
                          NULL, kill_job_execute, NULL);
        } else {
            kywc_log(KYWC_DEBUG, "User chose to wait for view: %s", view->base.app_id);
        }
    }

    json_object_put(obj);
    kill_dialog_destroy(dialog);

    return 0;
}

static void kill_dialog_create(struct view *view)
{
    struct kill_dialog *dialog = kill_dialog_from_view(view);
    if (dialog) {
        return;
    }

    dialog = calloc(1, sizeof(*dialog));
    if (!dialog) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog");
        return;
    }
    dialog->dialog_socket = -1;
    wl_list_init(&dialog->link);
    wl_list_init(&dialog->view_destroy.link);

    /* Export the hung window's surface as a foreign toplevel handle so
     * wldialog can import it and set itself as a modal child of it. */
    const char *parent_window = NULL;
    char parent_handle[sizeof("wayland:") + WLR_XDG_FOREIGN_HANDLE_SIZE];
    if (manager->view_manager->foreign_registry &&
        wlr_xdg_foreign_exported_init(&dialog->parent_exported,
                                      manager->view_manager->foreign_registry)) {
        dialog->parent_exported.surface = view->surface;
        snprintf(parent_handle, sizeof(parent_handle), "wayland:%s",
                 dialog->parent_exported.handle);
        parent_window = parent_handle;
    } else {
        kywc_log(KYWC_WARN, "Failed to export parent window handle, dialog will not be modal");
    }

    /* Create a dialog to ask user what to do */
    json_object *json_req = NULL;
    build_kill_dialog_request_json(&json_req, view->base.app_id, view->base.pid, parent_window);
    if (!json_req) {
        kywc_log(KYWC_ERROR, "Failed to build kill dialog JSON");
        kill_dialog_destroy(dialog);
        return;
    }

    dialog->dialog_socket = spawn_dialog("wldialog");
    if (dialog->dialog_socket < 0) {
        kywc_log(KYWC_ERROR, "Failed to create kill dialog");
        json_object_put(json_req);
        kill_dialog_destroy(dialog);
        return;
    }

    int flags = fcntl(dialog->dialog_socket, F_GETFL);
    if (flags != -1) {
        fcntl(dialog->dialog_socket, F_SETFL, flags | O_NONBLOCK);
    }

    if (!dialog_send_request(dialog->dialog_socket, json_req)) {
        kywc_log(KYWC_ERROR, "Failed to send dialog request");
        json_object_put(json_req);
        kill_dialog_destroy(dialog);
        return;
    }

    json_object_put(json_req);

    dialog->view = view;

    dialog->view_destroy.notify = kill_dialog_handle_view_destroy;
    wl_signal_add(&view->base.events.destroy, &dialog->view_destroy);

    wl_list_insert(&manager->kill_dialogs, &dialog->link);

    dialog->event_context =
        wl_event_loop_add_fd(manager->view_manager->server->event_loop, dialog->dialog_socket,
                             WL_EVENT_READABLE, handle_kill_dialog, dialog);
    if (!dialog->event_context) {
        kywc_log(KYWC_ERROR, "Failed to add dialog socket to event loop");
        kill_dialog_destroy(dialog);
        return;
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct kill_dialog *dialog, *tmp;
    wl_list_for_each_safe(dialog, tmp, &manager->kill_dialogs, link) {
        kill_dialog_destroy(dialog);
    }

    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

void kill_dialog_manager_create(struct view_manager *view_manager)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        kywc_log(KYWC_ERROR, "Failed to create hang window dialog manager");
        return;
    }

    manager->view_manager = view_manager;
    wl_list_init(&manager->kill_dialogs);

    view_manager->impl.show_hang_window = kill_dialog_create;

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(view_manager->server, &manager->server_destroy);
}
