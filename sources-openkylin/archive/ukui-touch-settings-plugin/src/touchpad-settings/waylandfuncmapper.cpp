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
#include "waylandfuncmapper.h"

// 定义全局变量（仅一次）
namespace FuncMapper {
    QMap<uint, QString> id2name;
    QHash<QString, uint> name2id;

    // 实现init函数（复用你原有的所有add逻辑）
    void init() {
        if (!id2name.isEmpty()) return;
        auto add = [](uint id, const QString& name) {
            id2name[id] = name;
            name2id[name] = id;
        };

        add(0, "none");
        add(1, "user_define_function");
        add(2, "custom_configure_function");
        add(3, "new_workspace");
        add(4, "delete_workspace");
        add(5, "switch_to_left_workspace");
        add(6, "switch_to_right_workspace");
        add(7, "switch_to_workspace_1");
        add(8, "switch_to_workspace_2");
        add(9, "switch_to_workspace_3");
        add(10, "switch_to_workspace_4");
        add(11, "toggle_desktop_visibility");
        add(12, "show_desktop(minimize_all)");
        add(13, "restore_all_desktop_windows");
        add(14, "minimize_other_windows");
        add(15, "restore_other_windows");
        add(16, "minimize_or_restore_other_windows");
        add(17, "maximize_active_window");
        add(18, "minimize_active_window");
        add(19, "close_active_window");
        add(20, "show_active_window_menu");
        add(21, "capture_active_window");
        add(22, "tile_active_window_to_the_top");
        add(23, "tile_active_window_to_the_bottom");
        add(24, "tile_active_window_to_the_left");
        add(25, "tile_active_window_to_the_right");
        add(26, "tile_active_window_to_the_top_half");
        add(27, "tile_active_window_to_the_bottom_half");
        add(28, "tile_active_window_to_the_left_half");
        add(29, "tile_active_window_to_the_right_half");
        add(30, "move_active_window_to_left(display_wrap)");
        add(31, "move_active_window_to_right(display_wrap)");
        add(32, "move_and_maximize_active_window_left(display_wrap)");
        add(33, "move_and_maximize_active_window_right(display_wrap)");
        add(34, "traverse_all_maximized_window");
        add(35, "toggle_color_filter");
        add(36, "magnifier_zoom_in");
        add(37, "magnifier_zoom_out");
        add(38, "magnifier_zoom_stop");
        add(39, "show_menu");
        add(40, "notification_center");
        add(41, "left_click");
        add(42, "right_click");
        add(43, "middle_click");
        add(44, "app_switch");
        add(45, "reverse_app_switch");
        add(46, "multi_taskview");
        add(47, "global_search");
        add(48, "tablet_taskbar");
        add(49, "follow_gesture_to_switch_to_left_workspace");
        add(50, "follow_gesture_to_switch_to_right_workspace");
        add(51, "play");
        add(52, "pause");
        add(53, "next_track");
        add(54, "previous_track");
        add(55, "volume_up");
        add(56, "volume_down");
        add(57, "volume_mute");
    }

    // 实现funcName函数
    QString funcName(uint id, const QString& def) {
        init();
        return id2name.value(id, def);
    }

    // 实现funcId函数
    uint funcId(const QString& name, uint def) {
        init();
        return name2id.value(name, def);
    }

    QString getGifPathByFuncId(uint id) {
        return ":/gif/resources/"+ funcName(id) + ".gif";
    }
}
