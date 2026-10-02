// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_TIME_H_
#define _UTIL_TIME_H_

#include <stdint.h>
#include <time.h>

uint32_t current_time_msec(void);

int64_t timespec_diff_usec(const struct timespec *a, const struct timespec *b);

static inline uint32_t usec_to_msec(uint64_t usec)
{
    return (uint32_t)(usec / 1000);
}

#endif /* _UTIL_TIME_H_ */
