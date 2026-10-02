/* Copyright (C) 2011 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int android_initDisplayControl(void);
int android_startDisplayControl(void);
void android_stopDisplayControl(void);
int android_display_control_set_path(const char* path);
void android_display_control_server_path(char* buff, size_t buffsize);

#ifdef __cplusplus
}
#endif


