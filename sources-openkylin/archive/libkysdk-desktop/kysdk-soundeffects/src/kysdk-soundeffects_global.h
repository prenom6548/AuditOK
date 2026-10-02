/*
 * libkysdk-soundeffects's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KYSDKSOUNDEFFECTS_GLOBAL_H
#define KYSDKSOUNDEFFECTS_GLOBAL_H

#include <QtCore/qglobal.h>

/**
 * @since 2.2.1.0
 */

#if defined(KYSDKSOUNDEFFECTS_LIBRARY)
#  define KYSDKSOUNDEFFECTS_EXPORT Q_DECL_EXPORT
#else
#  define KYSDKSOUNDEFFECTS_EXPORT Q_DECL_IMPORT
#endif

#endif // KYSDKSOUNDEFFECTS_GLOBAL_H
