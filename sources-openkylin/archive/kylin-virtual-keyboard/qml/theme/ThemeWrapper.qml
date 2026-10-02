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

import QtQuick 2.0

Item {
    id: themeWrapper

    // 主题文件路径，根据 currentTheme 和 currentThemeColor 自动计算
    property string themePath: getThemePath(themeWatcher.currentTheme, themeWatcher.currentThemeColor)
    // 统一的属性接口 - 从当前活动的主题源获取
    property color virtualKeyboardColor: getColor("virtualKeyboardColor")
    property color charKeyNormalColor: getColor("charKeyNormalColor")
    property color charKeyPressedColor: getColor("charKeyPressedColor")
    property color charKeyHoverColor: getColor("charKeyHoverColor")
    property color charKeyDropShadowColor: getColor("charKeyDropShadowColor")
    property color actionKeyNormalColor: getColor("actionKeyNormalColor")
    property color actionKeyPressedColor: getColor("actionKeyPressedColor")
    property color actionKeyHoverColor: getColor("actionKeyHoverColor")
    property color switchKeyNormalColor: getColor("switchKeyNormalColor")
    property color switchKeyPressedColor: getColor("switchKeyPressedColor")
    property color switchKeyOpenColor: getColor("switchKeyOpenColor")
    property color switchKeyOpenPressedColor: getColor("switchKeyOpenPressedColor")
    property color switchKeyHoverNormalColor: getColor("switchKeyHoverNormalColor")
    property color switchKeyHoverOpenColor: getColor("switchKeyHoverOpenColor")
    property color switchKeyNormalDropShadowColor: getColor("switchKeyNormalDropShadowColor")
    property color switchKeyOpenDropShadowColor: getColor("switchKeyOpenDropShadowColor")
    property color switchKeyOpenPressedDropShadowColor: getColor("switchKeyOpenPressedDropShadowColor")
    property color currentIMColor: getColor("currentIMColor")
    property color candidateListBackgroundColor: getColor("candidateListBackgroundColor")
    property color candidateDefaultColor: getColor("candidateDefaultColor")
    property color candidateHighlightColor: getColor("candidateHighlightColor")
    property color preeditBottomColor: getColor("preeditBottomColor")
    property color dragBarIndicatorColor: getColor("dragBarIndicatorColor")
    property color hideButtonPressedColor: getColor("hideButtonPressedColor")
    property color hideButtonHoverColor: getColor("hideButtonHoverColor")
    property color placementButtonPressedColor: getColor("placementButtonPressedColor")
    property color placementButtonHoverColor: getColor("placementButtonHoverColor")
    property color fontPrimaryColor: getColor("fontPrimaryColor")
    property color fontSecondaryColor: getColor("fontSecondaryColor")
    property int virtualKeyboardFloatPlacementRadius: getInt("virtualKeyboardFloatPlacementRadius")
    property int dragBarIndicatorRadius: getInt("dragBarIndicatorRadius")
    property int toolbarRadius: getInt("toolbarRadius")

    function getThemePath(theme, themeColor) {
        var colorTheme = themeColor.charAt(0).toUpperCase() + themeColor.slice(1) + "Theme.qml";
        return "qrc:/qml/theme/" + theme + "/" + colorTheme;
    }

    // 从主题加载器获取属性值的通用函数
    function getPropertyFromLoader(loader, propertyName) {
        if (loader.status === Loader.Ready && loader.item && loader.item.hasOwnProperty(propertyName)) {
            var value = loader.item[propertyName];
            if (value !== undefined)
                return value;
        }
        return undefined;
    }

    function getProperty(propertyName, defaultValue) {
        var value = getPropertyFromLoader(themeLoader, propertyName);
        if (value !== undefined)
            return value;

        return defaultValue;
    }

    function getColor(propertyName) {
        return getProperty(propertyName, Qt.rgba(0, 0, 0, 0));
    }

    function getInt(propertyName) {
        return getProperty(propertyName, 0);
    }

    // 根据当前主题获取图标路径
    // iconName: 图标文件名，例如 "backspace.svg"
    // 返回: 根据主题返回 "qrc:/img/light/backspace.svg" 或 "qrc:/img/dark/backspace.svg"
    function getIconPath(iconName) {
        return "qrc:/img/" + themeWatcher.currentThemeColor + "/" + iconName;
    }

    // 加载主题文件
    Loader {
        id: themeLoader

        asynchronous: false
        source: themePath
        onStatusChanged: {
            if (status === Loader.Error)
                console.error("Theme not available:", themePath);
            else if (status === Loader.Ready)
                console.info("Theme loaded:", themePath, "from theme:", themeWatcher.currentTheme, "color:", themeWatcher.currentThemeColor);
        }
    }
}
