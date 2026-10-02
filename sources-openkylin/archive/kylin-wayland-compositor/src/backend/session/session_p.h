// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SESSION_P_H_
#define _SESSION_P_H_

#include <stdbool.h>

struct session;

struct session_interface {
    void (*destroy)(struct session *session);

    int (*open)(struct session *session, const char *path);
    void (*close)(struct session *session, int fd);

    bool (*change_vt)(struct session *session, int vt);
    const char *(*seat_name)(struct session *session);
    int (*get_fd)(struct session *session);
    int (*dispatch)(struct session *session, int timeout);
};

bool logind_create(struct session *session);

void session_set_active(struct session *session);

void session_set_inactive(struct session *session);

#endif /* _SESSION_P_H_ */
