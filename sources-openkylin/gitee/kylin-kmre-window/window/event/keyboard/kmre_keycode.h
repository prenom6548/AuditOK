/* Copyright (C) 2007-2008 The Android Open Source Project
** Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
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

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int convert_key_scancode_to_linux_keycode(int32_t scancode);
extern bool linux_keycode_is_alphabetical(int32_t linux_key);
extern bool linux_keycode_is_modifier(int32_t linux_key);

#ifdef __cplusplus
}
#endif
