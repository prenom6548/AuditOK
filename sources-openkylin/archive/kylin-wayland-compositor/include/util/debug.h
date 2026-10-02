// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_DEBUG_H_
#define _UTIL_DEBUG_H_

void debug_backtrace(void);

#ifdef TRACY_ENABLE
#include <tracy/TracyC.h>

#define PROFILE_FRAME() TracyCFrameMark
#define PROFILE_FRAME_NAME(name) TracyCFrameMarkNamed(name)
#define PROFILE_ZONE(ctx, name) TracyCZoneN(ctx, name, true)
#define PROFILE_ZONE_END(ctx) TracyCZoneEnd(ctx)

#else

#define PROFILE_FRAME()
#define PROFILE_FRAME_NAME(name)
#define PROFILE_ZONE(ctx, name)
#define PROFILE_ZONE_END(ctx)

#endif

#endif /* _UTIL_DEBUG_H_ */
