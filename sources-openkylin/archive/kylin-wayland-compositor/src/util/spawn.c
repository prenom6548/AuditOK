// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#include <kywc/log.h>

#include "util/limit.h"
#include "util/spawn.h"

bool spawn_shell(char *command)
{
    wordexp_t p = { 0 };
    if (wordexp(command, &p, WRDE_NOCMD) || !p.we_wordv[0]) {
        kywc_log(KYWC_ERROR, "Command illegal: %s", command);
        wordfree(&p);
        return false;
    }

    pid_t child = fork();
    switch (child) {
    case -1:
        kywc_log_errno(KYWC_ERROR, "Unable to fork()");
        wordfree(&p);
        return false;
    case 0:
        setsid();
        /* do not give our signal mask to the new process */
        sigset_t set;
        sigfillset(&set);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        limit_unset_nofile();

        pid_t grandchild = fork();
        if (grandchild == 0) {
            char *argv[] = { "/bin/sh", "-c", command, NULL };
            execvp(argv[0], argv);

            kywc_log_errno(KYWC_ERROR, "execvp failed");
            _exit(1);
        } else if (grandchild < 0) {
            kywc_log(KYWC_ERROR, "Unable to fork()");
        }
        _exit(0);
    default:
        break;
    }

    waitpid(child, NULL, 0);
    wordfree(&p);
    return true;
}

bool spawn_invoke(const char *command)
{
    wordexp_t p = { 0 };
    if (wordexp(command, &p, WRDE_NOCMD) || !p.we_wordv[0]) {
        kywc_log(KYWC_ERROR, "Command illegal: %s", command);
        wordfree(&p);
        return false;
    }

    pid_t child = fork();
    switch (child) {
    case -1:
        kywc_log(KYWC_ERROR, "Unable to fork()");
        wordfree(&p);
        return false;
    case 0:
        setsid();
        /* do not give our signal mask to the new process */
        sigset_t set;
        sigfillset(&set);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        limit_unset_nofile();

        pid_t grandchild = fork();
        if (grandchild == 0) {
            /* close stdout/stderr */
            int devnull = open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0660);
            if (devnull < 0) {
                kywc_log_errno(KYWC_ERROR, "failed to open /dev/null");
                _exit(1);
            }
            if (kywc_log_get_level() < KYWC_DEBUG) {
                dup2(devnull, STDOUT_FILENO);
                dup2(devnull, STDERR_FILENO);
            }

            char **argv = p.we_wordv;
            execvp(argv[0], argv);

            kywc_log_errno(KYWC_ERROR, "execvp failed");
            close(devnull);
            _exit(1);
        } else if (grandchild < 0) {
            kywc_log(KYWC_ERROR, "Unable to fork()");
        }
        _exit(0);
    default:
        break;
    }

    waitpid(child, NULL, 0);
    wordfree(&p);
    return true;
}

int spawn_dialog(const char *command)
{
    wordexp_t p = { 0 };
    if (wordexp(command, &p, WRDE_NOCMD) != 0 || p.we_wordc == 0) {
        kywc_log(KYWC_ERROR, "Command illegal or empty: %s", command);
        wordfree(&p);
        return -1;
    }

    /* new command-line arguments list */
    size_t new_argc = p.we_wordc + 1;
    char **new_argv = NULL;

    int sockpair[2] = { -1, -1 };
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair) != 0) {
        kywc_log_errno(KYWC_ERROR, "socketpair failed");
        goto error;
    }

    if (!limit_set_cloexec(sockpair[0], true) || !limit_set_cloexec(sockpair[1], true)) {
        kywc_log(KYWC_ERROR, "Failed to set CLOEXEC on socketpair");
        goto error;
    }

    new_argv = calloc(new_argc + 1, sizeof(char *));
    if (!new_argv) {
        goto error;
    }

    for (size_t i = 0; i < p.we_wordc; i++) {
        new_argv[i] = p.we_wordv[i];
    }
    char fd_arg[16];
    snprintf(fd_arg, sizeof(fd_arg), "%d", sockpair[1]);
    new_argv[new_argc - 1] = strdup(fd_arg);
    if (!new_argv[new_argc - 1]) {
        goto error;
    }

    pid_t child = fork();
    switch (child) {
    case -1:
        kywc_log_errno(KYWC_ERROR, "fork failed");
        goto error;
    case 0:
        if (setsid() == -1) {
            kywc_log_errno(KYWC_ERROR, "setsid failed");
        }

        sigset_t set;
        sigfillset(&set);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        /* Close the parent's end */
        close(sockpair[0]);

        pid_t grandchild = fork();
        if (grandchild == 0) {
            /* Make sure the socket is NOT CLOEXEC in child so it survives exec */
            if (!limit_set_cloexec(sockpair[1], false)) {
                _exit(1);
            }

            /* Redirect stderr to /dev/null unless in debug mode */
            int devnull = open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0660);
            if (devnull < 0) {
                kywc_log_errno(KYWC_ERROR, "failed to open /dev/null");
                _exit(1);
            }
            if (kywc_log_get_level() < KYWC_DEBUG) {
                dup2(devnull, STDERR_FILENO);
            }
            close(devnull);

            /* Execute the command */
            execvp(new_argv[0], new_argv);
            kywc_log_errno(KYWC_ERROR, "execvp(%s) failed", p.we_wordv[0]);
            _exit(1);
        } else if (grandchild < 0) {
            kywc_log(KYWC_ERROR, "Unable to fork()");
            _exit(1);
        }
        _exit(0);
    default:
        break;
    }

    waitpid(child, NULL, 0);
    close(sockpair[1]);
    wordfree(&p);
    free(new_argv[new_argc - 1]);
    free(new_argv);
    return sockpair[0];

error:
    if (sockpair[0] >= 0) {
        close(sockpair[0]);
    }
    if (sockpair[1] >= 0) {
        close(sockpair[1]);
    }
    if (new_argv) {
        free(new_argv[new_argc - 1]);
    }
    free(new_argv);

    wordfree(&p);
    return -1;
}

#ifndef UTIL_USED_IN_CLIENT

pid_t spawn_session(const char *session)
{
    wordexp_t p = { 0 };
    if (wordexp(session, &p, WRDE_NOCMD) || !p.we_wordv[0]) {
        kywc_log(KYWC_ERROR, "Session illegal: %s", session);
        wordfree(&p);
        return -1;
    }

    pid_t child = fork();
    switch (child) {
    case -1:
        kywc_log(KYWC_ERROR, "Unable to fork()");
        wordfree(&p);
        return -1;
    case 0:
        /* child */
        setsid();
        /* do not give our signal mask to the new process */
        sigset_t set;
        sigfillset(&set);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        limit_unset_nofile();

        /* close stdout/stderr */
        int devnull = open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0660);
        if (devnull < 0) {
            kywc_log_errno(KYWC_ERROR, "failed to open /dev/null");
            _exit(1);
        }
        if (kywc_log_get_level() < KYWC_DEBUG) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
        }

        char **argv = p.we_wordv;
        execvp(argv[0], argv);

        kywc_log_errno(KYWC_ERROR, "execvp failed");
        close(devnull);
        _exit(1);
    default:
        wordfree(&p);
        return child;
    }
}

void spawn_wait(pid_t pid)
{
    waitpid(pid, NULL, WNOHANG);
}

struct wl_client *spawn_client(struct wl_display *display, const char *command)
{
    wordexp_t p = { 0 };
    if (wordexp(command, &p, WRDE_NOCMD) || !p.we_wordv[0]) {
        kywc_log(KYWC_ERROR, "Command illegal: %s", command);
        wordfree(&p);
        return NULL;
    }

    int fds[2] = { -1, -1 };
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
            kywc_log(KYWC_INFO, "Run %s in %s", command, wayland_socket_str);

            /* close stdout/stderr */
            int devnull = open("/dev/null", O_WRONLY | O_CREAT | O_CLOEXEC, 0660);
            if (devnull < 0) {
                kywc_log_errno(KYWC_ERROR, "failed to open /dev/null");
                _exit(1);
            }
            if (kywc_log_get_level() < KYWC_DEBUG) {
                dup2(devnull, STDOUT_FILENO);
                dup2(devnull, STDERR_FILENO);
            }

            char **argv = p.we_wordv;
            execvp(argv[0], argv);

            kywc_log_errno(KYWC_ERROR, "execvp failed");
            close(devnull);
            _exit(1);
        } else if (grandchild < 0) {
            kywc_log(KYWC_ERROR, "Unable to fork()");
        }
        _exit(0);
    default:
        break;
    }

    waitpid(child, NULL, 0);
    wordfree(&p);
    close(fds[1]);
    return client;

error:
    if (fds[0] >= 0) {
        close(fds[0]);
    }
    if (fds[1] >= 0) {
        close(fds[1]);
    }
    wordfree(&p);
    return NULL;
}

#endif
