/*
* Copyright 2025 KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*/

import "../../js/themeutils.js" as ThemeUtils
import QtQuick 2.0

QtObject {
    id: lightTheme

    // 虚拟键盘背景色
    property color virtualKeyboardColor: "#EBEDEF"
    // 字符按键颜色
    property color charKeyNormalColor: "#FFFFFF"
    property color charKeyPressedColor: "#DDE0E4"
    property color charKeyHoverColor: "#EDF1F5"
    property color charKeyDropShadowColor: "#95A0AD"
    // 动作按键颜色
    property color actionKeyNormalColor: "#CED3D9"
    property color actionKeyPressedColor: "#A5ACB5"
    property color actionKeyHoverColor: "#C0C6CE"
    // 开关按键颜色
    property color switchKeyNormalColor: "#CED3D9"
    property color switchKeyPressedColor: "#A5ACB5"
    property color switchKeyHoverNormalColor: "#C0C6CE"
    property color switchKeyOpenColor: "#3690F8"
    property color switchKeyOpenPressedColor: "#2C73C6"
    property color switchKeyHoverOpenColor: "#328AF0"
    // 开关按键阴影颜色
    property color switchKeyNormalDropShadowColor: "#95A0AD"
    property color switchKeyOpenDropShadowColor: "#1174E5"
    property color switchKeyOpenPressedDropShadowColor: "#165FB3"
    // 输入法列表高亮颜色
    property color currentIMColor: "#328AF0"
    // 候选词相关颜色
    property color candidateListBackgroundColor: "#DDE0E4"
    property color candidateDefaultColor: "#262626"
    property color candidateHighlightColor: "#3790FA"
    property color preeditBottomColor: "#E4E6E9"
    // 拖拽条
    property color dragBarIndicatorColor: "#1D1D1D"
    // 关闭按钮
    property color hideButtonPressedColor: "#C43232"
    property color hideButtonHoverColor: "#E93C3C"
    // 模式切换
    property color placementButtonPressedColor: "#C8C8C8"
    property color placementButtonHoverColor: "#DCDCDC"
    // 文本颜色
    property color fontPrimaryColor: Qt.rgba(0, 0, 0, 0.85)
    property color fontSecondaryColor: Qt.rgba(0, 0, 0, 0.35)
    // 圆角属性
    property int virtualKeyboardFloatPlacementRadius: 16
    property int dragBarIndicatorRadius: 2
    property int toolbarRadius: 8
}
