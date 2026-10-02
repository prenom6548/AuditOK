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

import QtQuick 2.12
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype BorderlessButton
    \inqmlmodule org.ukui.quick.items
    \inherits MouseArea
    \brief 提供DT主题风格的无边框按钮组件.

    BorderlessButton是一个使用DT主题系统的无边框按钮组件，
    支持鼠标交互。按钮包含图标和文字。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0

    BorderlessButton {
        icon.source: "application-system-symbolic"
        text.text: "BorderlessButton"
        onClicked: console.log("BorderlessButton clicked")
    }
    \endqml
*/
MouseArea {
    /*!
        \qmlproperty Icon BorderlessButton::icon
        \readonly
        按钮的图标对象。
    */
    readonly property alias icon: iconBase

    /*!
        \qmlproperty DtThemeText BorderlessButton::text
        \readonly
        按钮的文字对象。
    */
    readonly property alias text: textBase

    /*!
        \qmlproperty bool BorderlessButton::iconOnRight
        图标是否显示在文字右侧。默认值为 false。
    */
    property bool iconOnRight: false

    //图标和文字的间距
    readonly property int iconMargin : textBase.implicitHeight * 0.4
    //控件默认宽度
    readonly property int defaultWidth : 56
    //控件默认高度
    readonly property int defaultHeight : 20
    hoverEnabled: true
    height: textBase.implicitHeight === 0 ? defaultHeight : textBase.implicitHeight
    width: textBase.implicitWidth === 0 ? defaultWidth : isNullIcon(iconBase) ? textBase.implicitWidth : textBase.implicitWidth + iconMargin + iconBase.width
    activeFocusOnTab: true
    cursorShape: enabled ? Qt.PointingHandCursor: Qt.ArrowCursor

    Icon {
        id: iconBase
        anchors.left: iconOnRight ? undefined : parent.left
        anchors.right: iconOnRight ? parent.right : undefined
        anchors.verticalCenter : parent.verticalCenter
        width: height
        height: isNullIcon(iconBase) ? 0: parent.height * 0.8
        visible: !isNullIcon(iconBase)
        mode: getIconMode()
        dtThemeHighlightColor: getHighlightColor()
    }

    DtThemeText {
        id: textBase
        anchors.left: isNullIcon(iconBase) || iconOnRight ? parent.left : iconBase.right
        anchors.right: !isNullIcon(iconBase) && iconOnRight ? iconBase.left : undefined
        anchors.leftMargin: !isNullIcon(iconBase) && !iconOnRight ? iconMargin : 0
        anchors.rightMargin: !isNullIcon(iconBase) && iconOnRight ? iconMargin : 0
        anchors.verticalCenter : parent.verticalCenter
        pointSizeOffset: 1
        enabled: parent.enabled
        textColor: getTextColor()
    }

    function getTextColor() {
        if(isNullIcon(iconBase)) {
            return containsPress ? GlobalTheme.kBrandClick :
                                   containsMouse ? GlobalTheme.kBrandHover
                                                 : enabled ? GlobalTheme.kBrandNormal
                                                           : GlobalTheme.kFontPrimaryDisable

        } else {
            return containsPress ? GlobalTheme.kBrandClick :
                                   containsMouse ? GlobalTheme.kBrandHover
                                                 : enabled ? GlobalTheme.kFontPrimary
                                                           : GlobalTheme.kFontPrimaryDisable
        }
    }

    function isNullIcon(icon) {
        if(icon.source === undefined || icon.source === null || icon.source === "" ||
                icon.source.toString() === "QVariant(QIcon, QIcon(null))") {
            return icon.fallbackSource === undefined || icon.fallbackSource === ""
        }
        return false;
    }

    function getIconMode() {
        if(iconBase.visible) {
            return !parent.enabled ? Icon.Disabled
                                   : containsMouse ? Icon.Highlight
                                                   : Icon.AutoHighlight
        }
        return Icon.Normal
    }

    function getHighlightColor() {
        if(iconBase.visible) {
            return containsPress ? GlobalTheme.kBrandClick
                                 : containsMouse ? GlobalTheme.kBrandHover
                                                 : GlobalTheme.kFontPrimary
        }
        return GlobalTheme.kGrayAlpha0
    }
}
