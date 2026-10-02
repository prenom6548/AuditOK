// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SECURITY_P_H_
#define _SECURITY_P_H_

#include "security.h"

bool global_filter_manager_create(struct server *server);

bool ky_security_manager_create(struct server *server);

bool security_context_manager_create(struct wl_display *display);

#endif /* _SECURITY_P_H_ */
