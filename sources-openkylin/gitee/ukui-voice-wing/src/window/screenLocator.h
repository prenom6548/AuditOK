/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCREENLOCATOR_H
#define SCREENLOCATOR_H

#include <QRect>

class QScreen;

class ScreenLocator {
public:
    static ScreenLocator &instance();

    // 返回当前活动窗口所在屏幕；Wayland 不可用或无活动窗口时回退到主屏
    QScreen *screenForActiveWindow();

private:
    ScreenLocator() = default;
    ~ScreenLocator();

    ScreenLocator(const ScreenLocator &) = delete;
    ScreenLocator &operator=(const ScreenLocator &) = delete;

    bool ensureInitialized();
    QScreen *screenForGeometry(const QRect &geometry) const;

    bool m_ready = false;
    bool m_initAttempted = false;
};

#endif // SCREENLOCATOR_H
