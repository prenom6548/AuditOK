/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This file is part of Kylin-Screenshot.
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
*/
// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __X11_INC_H__
#define __X11_INC_H__

#include <QThread>
#include <QDateTime>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/extensions/Xfixes.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/shape.h>
#include <X11/Xlibint.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#ifdef Bool
#undef Bool
#endif

#ifdef COUNT
#undef COUNT
#endif

#ifdef Unsorted
#undef Unsorted
#endif

#ifdef Status
#undef Status
#endif

#ifdef CursorShape
#undef CursorShape
#endif

#endif // __X11_INC_H__
