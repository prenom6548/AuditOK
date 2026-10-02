// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_CLIENT_H_
#define _UTIL_CLIENT_H_

#include <sys/types.h>

enum client_type {
    CLIENT_TYPE_UNKNOWN = 0,
    CLIENT_TYPE_WAYLAND,
    CLIENT_TYPE_X11,
    CLIENT_TYPE_DBUS,
};

struct client {
    enum client_type type;
    pid_t pid;
    uid_t uid;
    gid_t gid;
};

/* free is needed */
char *client_get_exe(pid_t pid);

/* free is needed */
char *client_get_comm(pid_t pid);

pid_t client_get_parent(pid_t pid);

pid_t client_get_ancestor(pid_t pid);

#endif /* _UTIL_CLIENT_H_ */
