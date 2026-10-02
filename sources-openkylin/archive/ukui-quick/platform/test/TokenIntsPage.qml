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

import QtQuick 2.0
import org.ukui.quick.platform 1.0
import QtQuick.Layouts 1.15
ListView {
    id: activeView
    width: parent.width
    height: parent.height
    interactive: true
    model: tokenModel
    delegate: SettingsValue {
        name: model.name
        value: model.dtToken
    }
    clip: true


    ListModel {
        id: tokenModel
        // dynamicRoles: true
        Component.onCompleted: {
            append({"dtToken": GlobalTheme.normalLine, "name": "normalLine"})
            append({"dtToken": GlobalTheme.focusLine, "name": "focusLine"})
            append({"dtToken": GlobalTheme.kRadiusMin, "name": "kRadiusMin"})
            append({"dtToken": GlobalTheme.kRadiusNormal, "name": "kRadiusNormal"})
            append({"dtToken": GlobalTheme.kRadiusMenu, "name": "kRadiusMenu"})
            append({"dtToken": GlobalTheme.kRadiusWindow, "name": "kRadiusWindow"})
            append({"dtToken": GlobalTheme.kMarginMin, "name": "kMarginMin"})
            append({"dtToken": GlobalTheme.kMarginNormal, "name": "kMarginNormal"})
            append({"dtToken": GlobalTheme.kMarginBig, "name": "kMarginBig"})
            append({"dtToken": GlobalTheme.kMarginWindow, "name": "kMarginWindow"})
            append({"dtToken": GlobalTheme.kMarginComponent, "name": "kMarginComponent"})
            append({"dtToken": GlobalTheme.kPaddingMinLeft, "name": "kPaddingMinLeft"})
            append({"dtToken": GlobalTheme.kPaddingMinRight, "name": "kPaddingMinRight"})
            append({"dtToken": GlobalTheme.kPaddingMinTop, "name": "kPaddingMinTop"})
            append({"dtToken": GlobalTheme.kPaddingMinBottom, "name": "kPaddingMinBottom"})
            append({"dtToken": GlobalTheme.kPaddingNormalLeft, "name": "kPaddingNormalLeft"})
            append({"dtToken": GlobalTheme.kPaddingNormalRight, "name": "kPaddingNormalRight"})
            append({"dtToken": GlobalTheme.kPaddingNormalTop, "name": "kPaddingNormalTop"})
            append({"dtToken": GlobalTheme.kPaddingNormalBottom, "name": "kPaddingNormalBottom"})
            append({"dtToken": GlobalTheme.kPadding8Left, "name": "kPadding8Left"})
            append({"dtToken": GlobalTheme.kPadding8Right, "name": "kPadding8Right"})
            append({"dtToken": GlobalTheme.kPadding8Top, "name": "kPadding8Top"})
            append({"dtToken": GlobalTheme.kPadding8Bottom, "name": "kPadding8Bottom"})
            append({"dtToken": GlobalTheme.kPaddingWindowLeft, "name": "kPaddingWindowLeft"})
            append({"dtToken": GlobalTheme.kPaddingWindowRight, "name": "kPaddingWindowRight"})
            append({"dtToken": GlobalTheme.kPaddingWindowTop, "name": "kPaddingWindowTop"})
            append({"dtToken": GlobalTheme.kPaddingWindowBottom, "name": "kPaddingWindowBottom"})
        }
    }
}