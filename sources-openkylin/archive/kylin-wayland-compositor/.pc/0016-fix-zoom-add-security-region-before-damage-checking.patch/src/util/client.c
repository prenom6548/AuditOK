// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <kywc/log.h>

#include "util/client.h"
#include "util/string.h"
#include "util/sysfs.h"

char *client_get_exe(pid_t pid)
{
    char link_file[64];
    snprintf(link_file, sizeof(link_file), "/proc/%d/exe", (int)pid);

    char link_target[PATH_MAX];
    // PATH_MAX includes the terminating NULL byte
    ssize_t read_len = readlink(link_file, link_target, sizeof(link_target) - 1);
    if (read_len < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to readlink() %s", link_file);
        return NULL;
    }

    /* should not assume that the returned contents are null-terminated */
    link_target[read_len] = '\0';

    return strdup(link_target);
}

char *client_get_comm(pid_t pid)
{
    char file[64];
    snprintf(file, sizeof(file), "/proc/%d/comm", (int)pid);

    char comm[128];
    size_t read_len = sysfs_read_data(file, comm, sizeof(comm) - 1);
    if (read_len == 0) {
        return NULL;
    }

    comm[read_len] = '\0';
    string_strip_space(comm);

    return strdup(comm);
}

void client_set_comm(struct client *client)
{
    char file[64];
    snprintf(file, sizeof(file), "/proc/%d/comm", (int)client->pid);
    size_t read_len = sysfs_read_data(file, client->comm, sizeof(client->comm) - 1);
    client->comm[read_len] = '\0';
    string_strip_space(client->comm); // strip '\n'
}

pid_t client_get_parent(pid_t pid)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *file = fopen(path, "r");
    if (!file) {
        kywc_log_errno(KYWC_ERROR, "Failed to open %s", path);
        return -1;
    }

    char line[256];
    pid_t ppid = -1;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line + 5, "%d", &ppid);
            break;
        }
    }

    fclose(file);
    return ppid;
}

pid_t client_get_ancestor(pid_t pid)
{
    if (pid <= 1) {
        return pid < 1 ? -1 : pid;
    }

    pid_t current = pid;
    pid_t parent;

    while (1) {
        parent = client_get_parent(current);
        if (parent == -1) {
            return -1;
        }
        if (parent <= 1) {
            return current;
        }
        current = parent;
    }
}

char *client_get_environ(pid_t pid, const char *env)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/environ", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        kywc_log_errno(KYWC_ERROR, "Failed to open %s", path);
        return NULL;
    }

    size_t len, varlen = strlen(env);
    char *line = NULL, *value = NULL;
    ssize_t nread;

    while ((nread = getdelim(&line, &len, '\0', fp)) != -1) {
        if (strncmp(line, env, varlen) == 0 && line[varlen] == '=') {
            value = strdup(line + varlen + 1);
            break;
        }
    }

    free(line);
    fclose(fp);
    return value;
}
