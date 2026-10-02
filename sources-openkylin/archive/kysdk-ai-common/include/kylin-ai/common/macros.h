/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __AISDK_MACROS_H__
#define __AISDK_MACROS_H__

#ifndef AISDK_EXTERN
#define AISDK_EXTERN extern
#endif

#define AISDK_AVAILABLE_IN_ALL _AISDK_EXTERN

#define AISDK_DEPRECATED __attribute__((__deprecated__))

#define AISDK_DEPRECATED_FOR(f) __attribute__((__deprecated__("Use " #f " instead")))

#ifdef __cplusplus
#define AISDK_BEGIN_EXTERN_C extern "C" {
#define AISDK_END_EXTERN_C   }  // extern "C"
#else
#define AISDK_BEGIN_EXTERN_C
#define AISDK_END_EXTERN_C
#endif  // __cplusplus

#endif
