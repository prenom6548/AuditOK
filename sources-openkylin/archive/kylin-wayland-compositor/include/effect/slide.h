// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_SLIDE_H_
#define _EFFECT_SLIDE_H_

#include <stdbool.h>

struct view;

#if HAVE_KDE_SLIDE
bool view_add_slide_effect(struct view *view, bool slide_out);
#else
static __attribute__((unused)) inline bool view_add_slide_effect(struct view *view, bool slide_out)
{
    return false;
}
#endif

#endif /* _EFFECT_SLIDE_H_ */
