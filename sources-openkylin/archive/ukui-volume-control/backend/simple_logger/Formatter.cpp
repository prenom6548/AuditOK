/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "Formatter.h"

namespace simple_logger
{
#ifndef HAS_STD_FORMAT
void FormatExpand(std::stringstream& ss, const char* fmt)
{
    // GCOVR_EXCL_BR_START - legacy `{}` scan branches (Debug throws; Release skips).
    while (*fmt) {
        // 多余的{}，即{}个数与实参个数不匹配
        if (*fmt == '{' && *(++fmt) == '}') {
#ifndef NDEBUG
            // 调试版本抛出异常，Release版本不抛异常
            throw std::runtime_error("Invalid format string: missing arguments");
#else
            // GCOVR_EXCL_BR_START - Release-only branch (whitebox builds are Debug).
            ++fmt;
            ++fmt;
            // GCOVR_EXCL_BR_STOP
#endif
        }

        ss << *fmt++;
    }
    // GCOVR_EXCL_BR_STOP
}
#endif
}
