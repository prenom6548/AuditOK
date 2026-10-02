// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <stdbool.h>
#include <json-c/json.h>

bool dialog_send_request(int socket_fd, json_object *req);

bool dialog_close(int socket_fd);

json_object *dialog_get_reply(int socket_fd);

#endif /* _DIALOG_H_ */
