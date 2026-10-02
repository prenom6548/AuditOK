// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_LIMIT_H_
#define _UTIL_LIMIT_H_

#include <stdbool.h>

void limit_set_nofile(void);

void limit_unset_nofile(void);

bool limit_set_cloexec(int fd, bool cloexec);

#endif /* _UTIL_LIMIT_H_ */
