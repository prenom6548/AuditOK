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
import org.ukui.quick.platform 1.0 as Platform

QtObject {
    id: ukuiTheme

    // 虚拟键盘背景色
    property color virtualKeyboardColor: getColorFromGradientByTransparency(Platform.GlobalTheme.windowActive)
    // 字符按键颜色
    property color charKeyNormalColor: getColorFromGradientByTransparency(Platform.GlobalTheme.kComponentAlphaNormal)
    property color charKeyPressedColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaClick)
    property color charKeyHoverColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaHover)
    property color charKeyDropShadowColor: Qt.rgba(0, 0, 0, 0.8)
    // 动作按键颜色
    property color actionKeyNormalColor: getColorFromGradientByTransparency(Platform.GlobalTheme.kContainSecondaryAlphaNormal)
    property color actionKeyPressedColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaClick)
    property color actionKeyHoverColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaHover)
    // 开关按键颜色
    property color switchKeyNormalColor: getColorFromGradientByTransparency(Platform.GlobalTheme.kContainSecondaryAlphaNormal)
    property color switchKeyPressedColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaClick)
    property color switchKeyOpenColor: getColorFromGradient(Platform.GlobalTheme.kBrandNormal)
    property color switchKeyOpenPressedColor: getColorFromGradient(Platform.GlobalTheme.kBrandClick)
    property color switchKeyHoverNormalColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaHover)
    property color switchKeyHoverOpenColor: getColorFromGradient(Platform.GlobalTheme.kBrandHover)
    // 开关按键阴影颜色
    property color switchKeyNormalDropShadowColor: Qt.rgba(0, 0, 0, 0.8)
    property color switchKeyOpenDropShadowColor: Qt.rgba(0, 0, 0, 0.8)
    property color switchKeyOpenPressedDropShadowColor: Qt.rgba(0, 0, 0, 0.8)
    // 输入法列表高亮颜色
    property color currentIMColor: getColorFromGradient(Platform.GlobalTheme.kBrandNormal)
    // 候选词相关颜色
    property color candidateListBackgroundColor: getColorFromGradientByTransparency(Platform.GlobalTheme.kComponentAlphaNormal)
    property color candidateDefaultColor: getColorFromGradient(Platform.GlobalTheme.kFontPrimary)
    property color candidateHighlightColor: getColorFromGradient(Platform.GlobalTheme.kBrandNormal)
    property color preeditBottomColor: getColorFromGradient(Platform.GlobalTheme.kDivider)
    // 拖拽条
    property color dragBarIndicatorColor: getColorFromGradient(Platform.GlobalTheme.kFontSecondary)
    // 关闭按钮
    property color hideButtonPressedColor: getColorFromGradient(Platform.GlobalTheme.kErrorClick)
    property color hideButtonHoverColor: getColorFromGradient(Platform.GlobalTheme.kErrorHover)
    // 模式切换
    property color placementButtonPressedColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaClick)
    property color placementButtonHoverColor: getColorFromGradient(Platform.GlobalTheme.kComponentAlphaHover)
    // 文本颜色
    property color fontPrimaryColor: getColorFromGradient(Platform.GlobalTheme.kFontPrimary)
    property color fontSecondaryColor: getColorFromGradient(Platform.GlobalTheme.kFontSecondary)
    // 圆角属性
    property int virtualKeyboardFloatPlacementRadius: Platform.GlobalTheme.kRadiusWindow
    property int dragBarIndicatorRadius: 2
    property int toolbarRadius: Platform.GlobalTheme.kRadiusMax

    function getColorFromGradient(gradientColor) {
        return ThemeUtils.getColorFromGradient(gradientColor);
    }

    function getColorFromGradientByTransparency(gradientColor) {
        return ThemeUtils.getColorFromGradientByTransparency(gradientColor, Platform.GlobalTheme.transparency);
    }
}
