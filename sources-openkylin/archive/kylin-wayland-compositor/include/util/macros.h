// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_MACROS_H_
#define _UTIL_MACROS_H_

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifndef ANGLE
#define ANGLE(angle) (angle * M_PI / 180.0)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(value, min, max) (((value) < (min)) ? (min) : ((value) > (max)) ? (max) : (value))
#endif

#ifndef COLOR_VALID
#define COLOR_VALID(color) (color[3] != 0.0f)
#define COLOR_INVALID(color) (color[3] == 0.0f)
#endif

#ifndef STRING_VALID
#define STRING_VALID(string) (string && *string)
#define STRING_INVALID(string) (!string || !*string)
#endif

#ifndef FLOAT_EQUAL
#define FLOAT_EQUAL(a, b) (fabs((a) - (b)) < 1e-6f)
#define DOUBLE_EQUAL(a, b) (fabs((a) - (b)) < 1e-9f)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#endif /* _UTIL_MACROS_H */
