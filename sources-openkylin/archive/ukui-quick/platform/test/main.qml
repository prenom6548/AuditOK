/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

import QtQuick 2.15
import QtQuick.Controls 1.4

TabView {
    Tab {
        width: parent.width
        height: parent.height
        title: "Palette"
        PaletteColorPage {}
    }
    Tab {
        width: parent.width
        title: "Settings"
        SettingsValuePage {}
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "AppLauncher"
        AppLauncherPage {}
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "ToolTip"
        ToolTipTest {}
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "Design Token"
        TokenColorsPage {}
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "Design Token"
        TokenIntsPage {}
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "DtTheme Item"
        DtItemPage {}
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "Output"
        OutputManagerPage {
        }
    }
    Tab {
        width: parent.width
        height: parent.height
        title: "Workspace"
        WorkspacePage {}
    }
}
