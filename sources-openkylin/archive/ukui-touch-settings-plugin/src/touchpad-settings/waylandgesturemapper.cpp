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
#include "waylandgesturemapper.h"

// 定义全局变量（仅一次）
namespace GestureMapper {
    QMap<uint, QString> id2name;
    QHash<QString, uint> name2id;

    // 实现init函数（你可根据实际业务补充gesture的add逻辑）
    void init() {
        if (!id2name.isEmpty()) return;
        auto add = [](uint id, const QString& name) {
            id2name[id] = name;
            name2id[name] = id;
        };

        add(GESTURE_TOUCHPAD_3FINGER_TAP, "touchpad_(3)fingers_tap");
        add(GESTURE_TOUCHPAD_3FINGER_SWIPE_UP, "touchpad_(3)fingers_swipe_up");
        add(GESTURE_TOUCHPAD_3FINGER_SWIPE_DOWN, "touchpad_(3)fingers_swipe_down");
        add(GESTURE_TOUCHPAD_3FINGER_SWIPE_LEFT, "touchpad_(3)fingers_swipe_left");
        add(GESTURE_TOUCHPAD_3FINGER_SWIPE_RIGHT, "touchpad_(3)fingers_swipe_right");
        add(GESTURE_TOUCHPAD_4FINGER_TAP, "touchpad_(4)fingers_tap");
        add(GESTURE_TOUCHPAD_4FINGER_SWIPE_UP, "touchpad_(4)fingers_swipe_up");
        add(GESTURE_TOUCHPAD_4FINGER_SWIPE_DOWN, "touchpad_(4)fingers_swipe_down");
        add(GESTURE_TOUCHPAD_4FINGER_SWIPE_LEFT, "touchpad_(4)fingers_swipe_left");
        add(GESTURE_TOUCHPAD_4FINGER_SWIPE_RIGHT, "touchpad_(4)fingers_swipe_right");
        add(GESTURE_TOUCHSCREEN_1FINGER_SWIPE_UP_BOTTOM, "touchscreen_(1)fingers_swipe_up(bottom)");
        add(GESTURE_TOUCHSCREEN_1FINGER_HOLD, "touchscreen_(1)fingers_hold");
        add(GESTURE_TOUCHSCREEN_4FINGER_SWIPE_UP, "touchscreen_(4)fingers_swipe_up");
        add(GESTURE_TOUCHSCREEN_4FINGER_SWIPE_DOWN, "touchscreen_(4)fingers_swipe_down");
        add(GESTURE_TOUCHSCREEN_4FINGER_SWIPE_LEFT, "touchscreen_(4)fingers_swipe_left");
        add(GESTURE_TOUCHSCREEN_4FINGER_SWIPE_RIGHT, "touchscreen_(4)fingers_swipe_right)");
    }

    QString gestureName(uint id, const QString& def) {
        init();
        return id2name.value(id, def);
    }

    uint gestureId(const QString& name, uint def) {
        init();
        return name2id.value(name, def);
    }

    QString getGifPathByGestureId(uint id) {
        return ":/gif/resources/" + gestureName(id) + ".gif";
    }
}
