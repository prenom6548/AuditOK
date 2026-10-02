/*
 * Peony-Qt
 *
 * Copyright (C) 2024, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * @author: Renyg <renyangguang@kylinos.cn>
 * @date:   2024-08-30 16:58
 */

#ifndef CONVENIENTMACROS_H
#define CONVENIENTMACROS_H

#include <accessbility-helper.h>

//! logical control
#define CheckAndReturn(T, ...) if(T){ return __VA_ARGS__; }
#define CheckAndReturnAssert(T, ...) if(T){ Q_ASSERT(false); return __VA_ARGS__; }
#define CheckAndContinue(T) if(T){ continue; }
#define CheckAndContinueAssert(T) if(T){ Q_ASSERT(false); continue; }
#define CheckAndBreak(T) if(T){ break; }

//! Pointer Deletion
#define DSAFE_DELETE(a) {if(nullptr!=(a)){delete (a); (a)=nullptr;}}
#define DSAFE_DELETE_LATER(a) {if(nullptr!=(a)){(a)->deleteLater(); (a)=nullptr;}}
#define DSAFE_DELETE_ALL(container) {qDeleteAll(container);container.clear();}

/// @name Convenience macros for easier use of AccessibilityHelper
#define UI_TEST_INDEX_MACRO UI_TEST_INDEX
#define UI_TEST_INIT static uint8_t UI_TEST_INDEX_MACRO = 0; UI_TEST_INDEX_MACRO++;

#define UI_TEST_MAKE_ACCESSIBLE(widget) makeAccessible(widget)
#define UI_TEST_SET_ALL(widget, name) UI_TEST_MAKE_ACCESSIBLE(widget).setAll(name, UI_TEST_INDEX_MACRO)
#define UI_TEST_SET_ALL_NO_INIT(widget, name) UI_TEST_MAKE_ACCESSIBLE(widget).setAll(name, 0)
///@}

#endif // CONVENIENTMACROS_H
