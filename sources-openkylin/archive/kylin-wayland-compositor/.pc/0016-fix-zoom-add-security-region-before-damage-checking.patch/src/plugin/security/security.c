// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <wayland-server-core.h>

#include <kywc/binding.h>
#include <kywc/log.h>
#include <kywc/plugin.h>
#include <kywc/view.h>

#include "gui_exe.h"
#include "security-protocol.h"
#include "security/security.h"
#include "util/client.h"
#include "util/limit.h"

struct security_view {
    struct wl_list link;
    const struct kywc_view *view;
    struct wl_listener view_destroy;
    struct security *security;
};

static struct security_plugin {
    struct wl_display *display;
    struct wl_listener display_destroy;
    const struct kywc_plugin_interface *interface;

    struct key_binding *binding;
    struct wl_list views;

    struct wl_client *gui;
    struct wl_listener gui_destroy;

    struct wl_global *global;
    unsigned char *image_data;
} *security = NULL;

static struct wl_client *security_spawn_gui(struct wl_display *display)
{
    int fd = memfd_create("gui_app", MFD_CLOEXEC);
    if (fd < 0) {
        return NULL;
    }

    int fds[2] = { -1, -1 };

    size_t size = sizeof(gui_exe);
    ssize_t written = write(fd, gui_exe, size);
    if (written != (ssize_t)size) {
        goto error;
    }

    /* create a socket pair with cloexec */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
        kywc_log_errno(KYWC_ERROR, "socketpair failed");
        goto error;
    }
    if (!limit_set_cloexec(fds[0], true) || !limit_set_cloexec(fds[1], true)) {
        kywc_log(KYWC_ERROR, "Failed to set O_CLOEXEC on socket");
        goto error;
    }

    struct wl_client *client = wl_client_create(display, fds[0]);
    if (!client) {
        kywc_log_errno(KYWC_ERROR, "wl_client_create failed");
        goto error;
    }
    fds[0] = -1; /* not ours anymore */

    pid_t child = fork();
    switch (child) {
    case -1:
        kywc_log(KYWC_ERROR, "Unable to fork()");
        goto error;
    case 0:
        setsid();
        /* do not give our signal mask to the new process */
        sigset_t set;
        sigfillset(&set);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        limit_unset_nofile();

        pid_t grandchild = fork();
        if (grandchild == 0) {
            if (!limit_set_cloexec(fds[1], false)) {
                kywc_log(KYWC_ERROR, "Failed to unset CLOEXEC on FD");
                _exit(1);
            }

            char wayland_socket_str[16];
            snprintf(wayland_socket_str, sizeof(wayland_socket_str), "%d", fds[1]);
            setenv("WAYLAND_SOCKET", wayland_socket_str, true);

            char path[64];
            snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

            execl(path, "gui_app", NULL);
            _exit(1);
        } else if (grandchild < 0) {
            kywc_log(KYWC_ERROR, "Unable to fork()");
        }
        _exit(0);
    default:
        break;
    }

    waitpid(child, NULL, 0);
    close(fds[1]);
    close(fd);
    return client;

error:
    if (fds[0] >= 0) {
        close(fds[0]);
    }
    if (fds[1] >= 0) {
        close(fds[1]);
    }
    close(fd);
    return NULL;
}

static void shortcut_action(struct key_binding *binding, void *data, struct seat *seat,
                            enum key_binding_stage stage)
{
    if (security->gui) {
        return;
    }

    security->gui = security_spawn_gui(security->display);
    if (!security->gui) {
        kywc_log(KYWC_ERROR, "security client spawm gui");
        return;
    }

    wl_client_add_destroy_listener(security->gui, &security->gui_destroy);
}

static bool security_view_handle_access(struct security *security, struct client *client,
                                        enum security_action action, void *data)
{
    char *launcher = client_get_environ(client->pid, "LAUNCHED_FROM");
    bool allow = launcher != NULL;
    free(launcher);

    // struct security_view *sec_view = data;
    // sec_view->view may be NULL
    kywc_log_ratelimited(KYWC_ERROR, "accessed by %s(%d): %d", client->comm, client->pid, allow);

    return allow;
}

static void security_view_destroy(struct security_view *sec_view)
{
    wl_list_remove(&sec_view->view_destroy.link);
    wl_list_remove(&sec_view->link);
    if (sec_view->security) {
        security->interface->security_set_user_data(sec_view->security, NULL);
    }
    if (sec_view->view) {
        security->interface->security_unref(sec_view->security);
    }
    free(sec_view);
}

static void security_view_handle_destroy(struct security *security, void *data)
{
    struct security_view *sec_view = data;
    sec_view->security = NULL;
    security_view_destroy(sec_view);
}

static const struct security_interface security_view_impl = {
    .access = security_view_handle_access,
    .destroy = security_view_handle_destroy,
};

static void security_view_handle_view_destroy(struct wl_listener *listener, void *data)
{
    struct security_view *sec_view = wl_container_of(listener, sec_view, view_destroy);
    wl_list_remove(&sec_view->view_destroy.link);
    wl_list_init(&sec_view->view_destroy.link);
    sec_view->view = NULL;
    security->interface->security_unref(sec_view->security);
}

static struct security_view *security_view_create(struct kywc_view *view)
{
    struct security_view *sec_view = calloc(1, sizeof(*sec_view));
    if (!sec_view) {
        return NULL;
    }

    sec_view->security = security->interface->security_view_create(
        view, SECURITY_DECISION_SOFT_DENY, &security_view_impl, sec_view);
    if (!sec_view->security) {
        free(sec_view);
        return NULL;
    }

    sec_view->view = view;
    wl_list_insert(&security->views, &sec_view->link);
    sec_view->view_destroy.notify = security_view_handle_view_destroy;
    wl_signal_add(&view->events.destroy, &sec_view->view_destroy);

    return sec_view;
}

static void security_handle_new_view(const struct kywc_view *view)
{
    char *launcher = client_get_environ(view->pid, "LAUNCHED_FROM");
    if (launcher) {
        kywc_log(KYWC_ERROR, "security view %s launched by %s", view->exec, launcher);
        security_view_create((struct kywc_view *)view);
    }
    free(launcher);
}

static const struct kywc_plugin_hooks security_hooks = {
    .new_view = security_handle_new_view,
};

static void security_handle_configuration_destroy(struct wl_client *client,
                                                  struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void security_handle_set_protector_mode(struct wl_client *client,
                                               struct wl_resource *resource, uint32_t mode)
{
    security->interface->security_set_protector_mode(mode);
}

static void security_handle_set_protector_color(struct wl_client *client,
                                                struct wl_resource *resource, uint32_t r,
                                                uint32_t g, uint32_t b, uint32_t a)
{
    const float max = (float)UINT32_MAX;
    security->interface->security_set_protector_color(r / max, g / max, b / max, a / max);
}

static void security_handle_set_protector_image(struct wl_client *client,
                                                struct wl_resource *resource, int32_t fd,
                                                uint32_t width, uint32_t height)
{
    struct stat s;
    if (fstat(fd, &s) < 0) {
        goto out;
    }

    size_t size = width * height * 4;
    if ((size_t)s.st_size != size) {
        goto out;
    }

    unsigned char *mapped = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        goto out;
    }

    unsigned char *image_data = malloc(size);
    if (!image_data) {
        munmap(mapped, size);
        goto out;
    }

    memcpy(image_data, mapped, size);
    munmap(mapped, size);

    if (security->interface->security_set_protector_image(width, height, image_data)) {
        free(security->image_data);
        security->image_data = image_data;
    }

out:
    close(fd);
}

static const struct kywc_security_configuration_v1_interface security_configuration_impl = {
    .destroy = security_handle_configuration_destroy,
    .set_protector_mode = security_handle_set_protector_mode,
    .set_protector_color = security_handle_set_protector_color,
    .set_protector_image = security_handle_set_protector_image,
};

static void security_configuration_bind(struct wl_client *client, void *data, uint32_t version,
                                        uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &kywc_security_configuration_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &security_configuration_impl, security, NULL);
}

static bool security_filter_global(const struct wl_client *client, void *data)
{
    /* only expose this global to security gui */
    return security->gui == client;
}

static void security_plugin_teardown(void *teardown_data)
{
    kywc_log(KYWC_ERROR, "security plugin teardown");
    wl_list_remove(&security->display_destroy.link);
    wl_list_remove(&security->gui_destroy.link);

    if (security->gui) {
        wl_client_destroy(security->gui);
    }

    if (security->global) {
        security->interface->security_remove_global_filter(security->global);
        wl_global_destroy(security->global);
    }

    kywc_key_binding_destroy(security->binding);

    struct security_view *view, *tmp;
    wl_list_for_each_safe(view, tmp, &security->views, link) {
        security_view_destroy(view);
    }

    free(security->image_data);
    free(security);
    security = NULL;
}

static void security_handle_gui_destroy(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_ERROR, "security gui destroy");
    wl_list_remove(&security->gui_destroy.link);
    wl_list_init(&security->gui_destroy.link);
    security->gui = NULL;
}

static void security_handle_display_destroy(struct wl_listener *listener, void *data)
{
    kywc_log(KYWC_ERROR, "security display destroy");
    wl_list_remove(&security->display_destroy.link);
    wl_list_init(&security->display_destroy.link);

    security->interface->security_remove_global_filter(security->global);
    wl_global_destroy(security->global);
    security->global = NULL;
}

static bool security_plugin_setup(void *plugin, void **teardown_data)
{
    security = calloc(1, sizeof(*security));
    if (!security) {
        return false;
    }

    security->interface = kywc_plugin_get_interface(plugin);
    if (!security->interface) {
        goto error;
    }

    security->binding =
        kywc_key_binding_create("Alt+Shift+d:no", "Show security gui", FUNC_ID_CUSTOM, 0);
    if (!security->binding) {
        goto error;
    }

    if (!kywc_key_binding_register(security->binding, KEY_BINDING_TYPE_CUSTOM_DEF, shortcut_action,
                                   NULL)) {
        goto error;
    }

    security->display = security->interface->display;

    security->global =
        wl_global_create(security->display, &kywc_security_configuration_v1_interface, 1, security,
                         security_configuration_bind);
    if (!security->global) {
        kywc_log(KYWC_ERROR, "security configuration create failed");
        goto error;
    }

    security->display_destroy.notify = security_handle_display_destroy;
    wl_display_add_destroy_listener(security->display, &security->display_destroy);

    security->gui_destroy.notify = security_handle_gui_destroy;
    wl_list_init(&security->gui_destroy.link);

    wl_list_init(&security->views);
    security->interface->set_hooks(plugin, &security_hooks);
    security->interface->security_add_global_filter(security->global, security_filter_global,
                                                    security);

    kywc_log(KYWC_ERROR, "security plugin setup");

    return true;

error:
    if (security->binding) {
        kywc_key_binding_destroy(security->binding);
    }
    free(security);
    security = NULL;
    return false;
}

static struct kywc_plugin_info security_plugin_info = {
    .name = "security",
    .vendor = PLUGINVENDORSTRING,
    .class = "security",
    .description = "security demo",
    .version = PLUGIN_VERSION_NUMERIC(1, 0, 0),
    .abi_version = ABI_VERSION(1, 0),
    .flags = KYWC_PLUGIN_FIXED,
};

struct kywc_plugin_data security_plugin_data = {
    .info = &security_plugin_info,
    .options = NULL,
    .setup = security_plugin_setup,
    .option = NULL,
    .teardown = security_plugin_teardown,
};
