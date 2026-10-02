// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <wayland-server-core.h>

#include <kywc/log.h>

#include "server.h"
#include "xwayland_p.h"

static void safe_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}

noreturn static void exec_xwayland(struct xwayland_server *server, int notify_fd)
{
    if (!set_cloexec(server->x_fd[0], false) || !set_cloexec(server->x_fd[1], false) ||
        !set_cloexec(server->wl_fd[1], false)) {
        kywc_log(KYWC_ERROR, "Failed to unset CLOEXEC on FD");
        _exit(EXIT_FAILURE);
    }
    if (server->options.enable_wm && !set_cloexec(server->wm_fd[1], false)) {
        kywc_log(KYWC_ERROR, "Failed to unset CLOEXEC on FD");
        _exit(EXIT_FAILURE);
    }

    char *argv[64] = { 0 };
    size_t i = 0;

    char listenfd0[16], listenfd1[16], displayfd[16];
    snprintf(listenfd0, sizeof(listenfd0), "%d", server->x_fd[0]);
    snprintf(listenfd1, sizeof(listenfd1), "%d", server->x_fd[1]);
    snprintf(displayfd, sizeof(displayfd), "%d", notify_fd);

    argv[i++] = "Xwayland";
    argv[i++] = server->display_name;
    argv[i++] = "-rootless";
    argv[i++] = "-core";

    argv[i++] = "-terminate";
#if HAVE_XWAYLAND_TERMINATE_DELAY
    char terminate_delay[16];
    if (server->options.terminate_delay > 0) {
        snprintf(terminate_delay, sizeof(terminate_delay), "%d", server->options.terminate_delay);
        argv[i++] = terminate_delay;
    }
#endif

#if HAVE_XWAYLAND_LISTENFD
    argv[i++] = "-listenfd";
    argv[i++] = listenfd0;
    argv[i++] = "-listenfd";
    argv[i++] = listenfd1;
#else
    argv[i++] = "-listen";
    argv[i++] = listenfd0;
    argv[i++] = "-listen";
    argv[i++] = listenfd1;
#endif
    argv[i++] = "-displayfd";
    argv[i++] = displayfd;

    char wmfd[16];
    if (server->options.enable_wm) {
        snprintf(wmfd, sizeof(wmfd), "%d", server->wm_fd[1]);
        argv[i++] = "-wm";
        argv[i++] = wmfd;
    }

#if HAVE_XWAYLAND_NO_TOUCH_POINTER_EMULATION
    if (server->options.no_touch_pointer_emulation) {
        argv[i++] = "-noTouchPointerEmulation";
    }
#else
    server->options.no_touch_pointer_emulation = false;
#endif

#if HAVE_XWAYLAND_FORCE_XRANDR_EMULATION
    if (server->options.force_xrandr_emulation) {
        argv[i++] = "-force-xrandr-emulation";
    }
#else
    server->options.force_xrandr_emulation = false;
#endif

    if (server->xauthority_file && *server->xauthority_file) {
        argv[i++] = "-auth";
        argv[i++] = server->xauthority_file;
    }

    argv[i++] = NULL;

    assert(i < sizeof(argv) / sizeof(argv[0]));

    char wayland_socket_str[16];
    snprintf(wayland_socket_str, sizeof(wayland_socket_str), "%d", server->wl_fd[1]);
    setenv("WAYLAND_SOCKET", wayland_socket_str, true);

    kywc_log(KYWC_INFO, "Starting Xwayland on :%d", server->display);

    // Closes stdout/stderr depending on log verbosity
    enum kywc_log_level verbosity = kywc_log_get_level();
    int devnull = open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0666);
    if (devnull < 0) {
        kywc_log(KYWC_ERROR, "XWayland: failed to open /dev/null");
        _exit(EXIT_FAILURE);
    }
    if (verbosity < KYWC_INFO) {
        dup2(devnull, STDOUT_FILENO);
    }
    if (verbosity < KYWC_ERROR) {
        dup2(devnull, STDERR_FILENO);
    }

    const char *xwayland_path = getenv("KYWC_XWAYLAND");
    if (xwayland_path) {
        kywc_log(KYWC_INFO, "Using Xwayland binary '%s' due to KYWC_XWAYLAND", xwayland_path);
    } else {
        xwayland_path = XWAYLAND_PATH;
    }

    // This returns if and only if the call fails
    execvp(xwayland_path, argv);

    kywc_log(KYWC_ERROR, "failed to exec %s", xwayland_path);
    close(devnull);
    _exit(EXIT_FAILURE);
}

static void server_finish_process(struct xwayland_server *server)
{
    if (!server || server->display == -1) {
        return;
    }

    if (server->x_fd_read_event[0]) {
        wl_event_source_remove(server->x_fd_read_event[0]);
        wl_event_source_remove(server->x_fd_read_event[1]);

        server->x_fd_read_event[0] = server->x_fd_read_event[1] = NULL;
    }

    if (server->client) {
        wl_list_remove(&server->client_destroy.link);
        wl_client_destroy(server->client);
    }
    if (server->pipe_source) {
        wl_event_source_remove(server->pipe_source);
    }

    safe_close(server->wl_fd[0]);
    safe_close(server->wl_fd[1]);
    safe_close(server->wm_fd[0]);
    safe_close(server->wm_fd[1]);
    memset(server, 0, offsetof(struct xwayland_server, display));
    server->wl_fd[0] = server->wl_fd[1] = -1;
    server->wm_fd[0] = server->wm_fd[1] = -1;

    /* We do not kill the Xwayland process, it dies to broken pipe
     * after we close our side of the wm/wl fds. This is more reliable
     * than trying to kill something that might no longer be Xwayland.
     */
}

static void server_finish_display(struct xwayland_server *server)
{
    if (!server) {
        return;
    }

    wl_list_remove(&server->display_destroy.link);
    wl_list_init(&server->display_destroy.link);

    if (server->display == -1) {
        return;
    }

    safe_close(server->x_fd[0]);
    safe_close(server->x_fd[1]);
    server->x_fd[0] = server->x_fd[1] = -1;

    unlink_display_sockets(server->display);
    server->display = -1;
    server->display_name[0] = '\0';
}

static bool xwayland_server_start(struct xwayland_server *server);

static bool server_start_lazy(struct xwayland_server *server);

static void handle_client_destroy(struct wl_listener *listener, void *data)
{
    struct xwayland_server *server = wl_container_of(listener, server, client_destroy);

    if (server->pipe_source) {
        // Xwayland failed to start, let the readiness handler deal with it
        return;
    }

    // Don't call client destroy: it's being destroyed already
    server->client = NULL;
    wl_list_remove(&server->client_destroy.link);

    server_finish_process(server);

    if (time(NULL) - server->server_start > 5) {
        if (server->options.lazy) {
            kywc_log(KYWC_INFO, "Restarting Xwayland (lazy)");
            server_start_lazy(server);
        } else {
            kywc_log(KYWC_INFO, "Restarting Xwayland");
            xwayland_server_start(server);
        }
    }
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct xwayland_server *server = wl_container_of(listener, server, display_destroy);

    // Don't call client destroy: the display is being destroyed, it's too late
    if (server->client) {
        server->client = NULL;
        wl_list_remove(&server->client_destroy.link);
    }

    xwayland_server_destroy(server);
}

static int xserver_handle_ready(int fd, uint32_t mask, void *data)
{
    struct xwayland_server *server = data;

    if (mask & WL_EVENT_READABLE) {
        /* Xwayland writes to the pipe twice, so if we close it too early
         * it's possible the second write will fail and Xwayland shuts down.
         * Make sure we read until end of line marker to avoid this.
         */
        char buf[64];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0 && errno != EINTR) {
            /* Clear mask to signal start failure after reaping child */
            kywc_log(KYWC_ERROR, "read from Xwayland display_fd failed");
            mask = 0;
        } else if (n <= 0 || buf[n - 1] != '\n') {
            /* Returning 1 here means recheck and call us again if required. */
            return 1;
        }
    }

    while (waitpid(server->pid, NULL, 0) < 0) {
        if (errno == EINTR) {
            continue;
        }

        /* If some application has installed a SIGCHLD handler, they
         * may race and waitpid() on our child, which will cause this
         * waitpid() to fail. We have a signal from the
         * notify pipe that things are ready, so this waitpid() is only
         * to prevent zombies, which will have already been reaped by
         * the application's SIGCHLD handler.
         */
        if (errno == ECHILD) {
            break;
        }
        kywc_log(KYWC_ERROR, "waitpid for Xwayland fork failed");
        goto error;
    }
    /* Xwayland will only write on the fd once it has finished its
     * initial setup. Getting an event here without READABLE means
     * the server end failed.
     */
    if (!(mask & WL_EVENT_READABLE)) {
        assert(mask & WL_EVENT_HANGUP);
        kywc_log(KYWC_ERROR, "Xwayland startup failed, not setting up xwm");
        goto error;
    }
    kywc_log(KYWC_DEBUG, "Xserver is ready");

    close(fd);
    wl_event_source_remove(server->pipe_source);
    server->pipe_source = NULL;
    server->ready = true;

    struct xwayland_server_ready_event event = {
        .server = server,
        .wm_fd = server->wm_fd[0],
    };
    wl_signal_emit_mutable(&server->events.ready, &event);

    /* We removed the source, so don't need recheck */
    return 0;

error:
    /* clean up */
    close(fd);
    server_finish_process(server);
    server_finish_display(server);
    return 0;
}

static bool server_start_display(struct xwayland_server *server, struct wl_display *wl_display)
{
    server->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(wl_display, &server->display_destroy);

    server->display = open_display_sockets(server->x_fd);
    if (server->display < 0) {
        server_finish_display(server);
        return false;
    }

    snprintf(server->display_name, sizeof(server->display_name), ":%d", server->display);
    return true;
}

static bool xwayland_server_start(struct xwayland_server *server)
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, server->wl_fd) != 0) {
        kywc_log(KYWC_ERROR, "socketpair failed");
        server_finish_process(server);
        return false;
    }
    if (!set_cloexec(server->wl_fd[0], true) || !set_cloexec(server->wl_fd[1], true)) {
        kywc_log(KYWC_ERROR, "Failed to set O_CLOEXEC on socket");
        server_finish_process(server);
        return false;
    }
    if (server->options.enable_wm) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, server->wm_fd) != 0) {
            kywc_log(KYWC_ERROR, "socketpair failed");
            server_finish_process(server);
            return false;
        }
        if (!set_cloexec(server->wm_fd[0], true) || !set_cloexec(server->wm_fd[1], true)) {
            kywc_log(KYWC_ERROR, "Failed to set O_CLOEXEC on socket");
            server_finish_process(server);
            return false;
        }
    }

    server->server_start = time(NULL);

    server->client = wl_client_create(server->wl_display, server->wl_fd[0]);
    if (!server->client) {
        kywc_log(KYWC_ERROR, "wl_client_create failed");
        server_finish_process(server);
        return false;
    }

    server->wl_fd[0] = -1; /* not ours anymore */

    server->client_destroy.notify = handle_client_destroy;
    wl_client_add_destroy_listener(server->client, &server->client_destroy);

    int notify_fd[2];
    if (pipe(notify_fd) == -1) {
        kywc_log(KYWC_ERROR, "pipe failed");
        server_finish_process(server);
        return false;
    }
    if (!set_cloexec(notify_fd[0], true)) {
        kywc_log(KYWC_ERROR, "Failed to set CLOEXEC on FD");
        server_finish_process(server);
        return false;
    }

    struct wl_event_loop *loop = wl_display_get_event_loop(server->wl_display);
    server->pipe_source =
        wl_event_loop_add_fd(loop, notify_fd[0], WL_EVENT_READABLE, xserver_handle_ready, server);

    wl_signal_emit_mutable(&server->events.start, NULL);

    server->pid = fork();
    if (server->pid < 0) {
        kywc_log(KYWC_ERROR, "fork failed");
        close(notify_fd[0]);
        close(notify_fd[1]);
        server_finish_process(server);
        return false;
    } else if (server->pid == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            kywc_log(KYWC_ERROR, "second fork failed");
            _exit(EXIT_FAILURE);
        } else if (pid == 0) {
            exec_xwayland(server, notify_fd[1]);
        }

        _exit(EXIT_SUCCESS);
    }

    /* close child fds */
    /* remain managing x sockets for lazy start */
    close(notify_fd[1]);
    close(server->wl_fd[1]);
    safe_close(server->wm_fd[1]);
    server->wl_fd[1] = server->wm_fd[1] = -1;

    return true;
}

static int xwayland_socket_connected(int fd, uint32_t mask, void *data)
{
    struct xwayland_server *server = data;

    wl_event_source_remove(server->x_fd_read_event[0]);
    wl_event_source_remove(server->x_fd_read_event[1]);
    server->x_fd_read_event[0] = server->x_fd_read_event[1] = NULL;

    xwayland_server_start(server);

    return 0;
}

static bool server_start_lazy(struct xwayland_server *server)
{
    struct wl_event_loop *loop = wl_display_get_event_loop(server->wl_display);

    if (!(server->x_fd_read_event[0] = wl_event_loop_add_fd(
              loop, server->x_fd[0], WL_EVENT_READABLE, xwayland_socket_connected, server))) {
        return false;
    }

    if (!(server->x_fd_read_event[1] = wl_event_loop_add_fd(
              loop, server->x_fd[1], WL_EVENT_READABLE, xwayland_socket_connected, server))) {
        wl_event_source_remove(server->x_fd_read_event[0]);
        server->x_fd_read_event[0] = NULL;
        return false;
    }

    return true;
}

static void handle_idle(void *data)
{
    struct xwayland_server *server = data;
    server->idle_source = NULL;
    xwayland_server_start(server);
}

void xwayland_server_destroy(struct xwayland_server *server)
{
    if (!server) {
        return;
    }

    if (server->idle_source != NULL) {
        wl_event_source_remove(server->idle_source);
    }
    server_finish_process(server);
    server_finish_display(server);
    wl_signal_emit_mutable(&server->events.destroy, NULL);
    free(server);
}

struct xwayland_server *xwayland_server_create(struct server *server,
                                               struct xwayland_server_options *options)
{
    if (!getenv("KYWC_XWAYLAND") && access(XWAYLAND_PATH, X_OK) != 0) {
        kywc_log(KYWC_ERROR, "Cannot find Xwayland binary \"%s\"", XWAYLAND_PATH);
        return NULL;
    }

    struct xwayland_server *xwayland_server = calloc(1, sizeof(*xwayland_server));
    if (!xwayland_server) {
        return NULL;
    }

    xwayland_server->wl_display = server->display;
    xwayland_server->options = *options;

#if !HAVE_XWAYLAND_TERMINATE_DELAY
    xwayland_server->options.terminate_delay = 0;
#endif

    xwayland_server->x_fd[0] = xwayland_server->x_fd[1] = -1;
    xwayland_server->wl_fd[0] = xwayland_server->wl_fd[1] = -1;
    xwayland_server->wm_fd[0] = xwayland_server->wm_fd[1] = -1;

    wl_signal_init(&xwayland_server->events.start);
    wl_signal_init(&xwayland_server->events.ready);
    wl_signal_init(&xwayland_server->events.destroy);

    if (!server_start_display(xwayland_server, xwayland_server->wl_display)) {
        goto error_alloc;
    }

    if (server->session) {
        xwayland_server->xauthority_file = generate_xauthority_file(xwayland_server->display);
        if (xwayland_server->xauthority_file && *xwayland_server->xauthority_file) {
            setenv("XAUTHORITY", xwayland_server->xauthority_file, true);
        }
    } else {
        unsetenv("XAUTHORITY");
    }

    if (xwayland_server->options.lazy) {
        if (!server_start_lazy(xwayland_server)) {
            goto error_display;
        }
    } else {
        struct wl_event_loop *loop = wl_display_get_event_loop(xwayland_server->wl_display);
        xwayland_server->idle_source = wl_event_loop_add_idle(loop, handle_idle, xwayland_server);
        if (xwayland_server->idle_source == NULL) {
            goto error_display;
        }
    }

    return xwayland_server;

error_display:
    server_finish_display(xwayland_server);
error_alloc:
    free(xwayland_server);
    return NULL;
}
