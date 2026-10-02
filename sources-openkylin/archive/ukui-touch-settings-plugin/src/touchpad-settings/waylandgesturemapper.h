/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef WAYLANDGESTUREMAPPER_H
#define WAYLANDGESTUREMAPPER_H

#include <QMap>
#include <QHash>
#include <QString>

enum class GestureType {
    Tap,
    Swipe_Up,
    Swipe_Down,
    Swipe_Left,
    Swipe_Right
};

// ===================== 触控板（touchpad）手势 ID 宏定义 =====================
// 位段规则：0x[设备][手指][类型][方向][边缘]
// 设备：1=touchpad 触控板；手指：3=3指/4=4指；类型：1=tap轻触/2=swipe滑动；方向：00=无/01=上/02=下/03=左/04=右；边缘：0=无
#define GESTURE_TOUCHPAD_3FINGER_TAP              0x131000  // 0x1_3_1_00_0 | 触控板-3指-轻触-无方向-无边缘
#define GESTURE_TOUCHPAD_3FINGER_SWIPE_UP         0x133010  // 0x1_3_2_01_0 | 触控板-3指-滑动-向上-无边缘
#define GESTURE_TOUCHPAD_3FINGER_SWIPE_DOWN       0x133020  // 0x1_3_2_02_0 | 触控板-3指-滑动-向下-无边缘
#define GESTURE_TOUCHPAD_3FINGER_SWIPE_LEFT       0x133040  // 0x1_3_2_03_0 | 触控板-3指-滑动-向左-无边缘
#define GESTURE_TOUCHPAD_3FINGER_SWIPE_RIGHT      0x133080  // 0x1_3_2_04_0 | 触控板-3指-滑动-向右-无边缘
#define GESTURE_TOUCHPAD_4FINGER_TAP              0x141000  // 0x1_4_1_00_0 | 触控板-4指-轻触-无方向-无边缘
#define GESTURE_TOUCHPAD_4FINGER_SWIPE_UP         0x143010  // 0x1_4_2_01_0 | 触控板-4指-滑动-向上-无边缘
#define GESTURE_TOUCHPAD_4FINGER_SWIPE_DOWN       0x143020  // 0x1_4_2_02_0 | 触控板-4指-滑动-向下-无边缘
#define GESTURE_TOUCHPAD_4FINGER_SWIPE_LEFT       0x143040  // 0x1_4_2_03_0 | 触控板-4指-滑动-向左-无边缘
#define GESTURE_TOUCHPAD_4FINGER_SWIPE_RIGHT      0x143080  // 0x1_4_2_04_0 | 触控板-4指-滑动-向右-无边缘
// ===================== 触摸屏（touchscreen）手势 ID 宏定义 =====================
// 位段规则：0x[设备][手指][类型][方向][边缘]
// 设备：2=touchscreen 触摸屏；手指：1=1指/4=4指；类型：2=swipe滑动/3=hold长按；方向：01=上/02=下/03=左/04=右；边缘：2=bottom底部/0=无
#define GESTURE_TOUCHSCREEN_1FINGER_SWIPE_UP_BOTTOM  0x212012  // 0x2_1_2_01_2 | 触摸屏-1指-滑动-向上-底部边缘
#define GESTURE_TOUCHSCREEN_1FINGER_HOLD             0x213000  // 0x2_1_3_00_0 | 触摸屏-1指-长按-无方向-无边缘
#define GESTURE_TOUCHSCREEN_4FINGER_SWIPE_UP         0x242010  // 0x2_4_2_01_0 | 触摸屏-4指-滑动-向上-无边缘
#define GESTURE_TOUCHSCREEN_4FINGER_SWIPE_DOWN       0x242020  // 0x2_4_2_02_0 | 触摸屏-4指-滑动-向下-无边缘
#define GESTURE_TOUCHSCREEN_4FINGER_SWIPE_LEFT       0x242030  // 0x2_4_2_03_0 | 触摸屏-4指-滑动-向左-无边缘
#define GESTURE_TOUCHSCREEN_4FINGER_SWIPE_RIGHT      0x242040  // 0x2_4_2_04_0 | 触摸屏-4指-滑动-向右-无边缘

namespace GestureMapper {
    extern QMap<uint, QString> id2name;
    extern QHash<QString, uint> name2id;

    void init();
    QString gestureName(uint id, const QString& def = {});
    uint gestureId(const QString& name, uint def = 0);
    QString getGifPathByGestureId(uint id);
}

#endif // WAYLANDGESTUREMAPPER_H
