/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_MONOTONIC_CLOCK_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_MONOTONIC_CLOCK_P_H

#include "lottie-internal-visibility_p.h"

#include <QtGlobal>

namespace UkuiQuick {

// Lottie 只关心经过了多长时间，不关心日历时间。把这个很小的依赖单独
// 表达出来，生产代码和确定性测试就可以使用不同的单调时钟实现。
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieMonotonicClock
{
public:
    virtual ~LottieMonotonicClock() = default;
    virtual qint64 nowNsecs() const = 0;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_MONOTONIC_CLOCK_P_H
