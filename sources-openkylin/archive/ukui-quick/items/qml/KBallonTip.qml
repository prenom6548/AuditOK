/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

/*!
    \qmltype KBallonTip
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供气泡提示组件，用于显示临时通知消息。

    KBallonTip 是一个轻量级的气泡提示组件，可以在屏幕上显示带有图标和文本的临时消息。
    支持多种预定义图标类型（错误、成功、警告、信息、加载中等），并提供自动隐藏功能。

    \section1 主要特性
    \list
        \li 支持多种预定义图标类型
        \li 支持自定义图标
        \li 自动显示/隐藏动画
        \li 可配置显示时长
        \li 支持模糊背景效果
        \li 支持阴影效果
        \li 可自定义屏幕位置和边距
    \endlist

    \section1 图标类型

    KBallonTip.IconType 枚举定义了以下图标类型：
    \list
        \li Error - 错误图标（dialog-error）
        \li Success - 成功图标（ukui-dialog-success）
        \li Warning - 警告图标（dialog-warning）
        \li Info - 信息图标（dialog-info）
        \li Loading - 加载动画图标
        \li None - 不显示图标
        \li Custom - 自定义图标（需设置 customIconSource）
    \endlist

    \section1 使用示例

    显示成功提示：
    \qml
    KBallonTip {
        id: tip
        iconType: KBallonTip.IconType.Success
        text: "操作成功！"
        interval: 2000
        position: window.Center
    }

    Button {
        text: "显示提示"
        onClicked: tip.popup()
    }
    \endqml

    显示自定义图标提示：
    \qml
    KBallonTip {
        id: customTip
        iconType: KBallonTip.IconType.Custom
        customIconSource: "my-custom-icon"
        text: "自定义消息"
    }
    \endqml

    \sa ContentWindow, DtThemeBackground, DtDropShadow
*/

import QtQuick 2.15
import QtQml 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
Item {
    id: root
    enum IconType { Error, Success, Warning, Info, Loading, None, Custom }
    /*!
        \qmlproperty enumeration KBallonTip::iconType

        气泡提示的图标类型。

        可选值：
        \list
            \li KBallonTip.IconType.Error - 错误图标
            \li KBallonTip.IconType.Success - 成功图标
            \li KBallonTip.IconType.Warning - 警告图标
            \li KBallonTip.IconType.Info - 信息图标
            \li KBallonTip.IconType.Loading - 加载动画
            \li KBallonTip.IconType.None - 无图标
            \li KBallonTip.IconType.Custom - 自定义图标
        \endlist

        默认值为 KBallonTip.IconType.None。
    */
    property int iconType: KBallonTip.IconType.None
    property alias screen: window.screen
    property alias position: window.position
    property int leftMargin: 0
    property int topMargin: 0
    property int rightMargin: 0
    property int bottomMargin: 0
    property alias text: tipText.text
    property string customIconSource: ""
    property alias interval: hideTimer.interval
    property alias useAvailableGeometry: window.useAvailableGeometry
    readonly property bool windowVisible: window.visible
    function popup() {
        if (!window.visible) {
            window.reLocated();
            window.show();
            showAnimation.start();
            hideTimer.start();
        }
    }
    /*!
        \qmlmethod void KBallonTip::hide()

        立即隐藏气泡提示。

        调用此方法会停止所有动画和定时器，并立即隐藏气泡提示窗口。
    */
    function hide() {
        showAnimation.stop();
        hideAnimation.stop();
        hideTimer.stop();
        window.hide();
    }

    Timer {
        id: hideTimer
        interval: 1750
        repeat: false
        onTriggered: {
            hideAnimation.start();
        }
    }
    Timer {
        id: loadingTimer
        interval: 100
        repeat: true
        property int frame: 0
        onTriggered: {
            if (iconType === KBallonTip.IconType.Loading) {
                if (frame < 7) {
                    frame++;
                } else {
                    frame = 0;
                }
                tipIcon.source = "ukui-loading-" + frame + ".symbolic";
            }
        }
    }
    NumberAnimation {
        id: showAnimation
        target: backGround
        property: "opacity"
        from: 0
        to: 1
        easing.type: Easing.Bezier
        easing.bezierCurve: [0.25, 0.1, 0.25, 1]
        duration: 350
    }
    NumberAnimation {
        id: hideAnimation
        target: backGround
        property: "opacity"
        from: 1
        to: 0
        easing.type: Easing.Bezier
        easing.bezierCurve: [0.25, 0.1, 0.25, 1]
        duration: 250
        onFinished: {
            window.hide();
        }
    }

    ContentWindow {
        id: window
        flags: Qt.Window | Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint
        windowType: WindowType.ToolTip
        enableBlurEffect: true
        margin.left: root.leftMargin- shadow.shadowSize
        margin.top: root.topMargin - shadow.shadowSize
        margin.right: root.rightMargin - shadow.shadowSize
        margin.bottom: root.bottomMargin - shadow.shadowSize
        Item {
            id: mainItem
            width: backGround.width + shadow.shadowSize * 2
            height: backGround.height + shadow.shadowSize * 2
            DtThemeBackground {
                id: backGround
                anchors.left: parent.left
                anchors.leftMargin: shadow.shadowSize
                anchors.top: parent.top
                anchors.topMargin: shadow.shadowSize
                radius: GlobalTheme.kRadiusMenu
                backgroundColor: GlobalTheme.kMenu
                borderColor: GlobalTheme.kLineWindowInactive
                useStyleTransparency: true
                width: childrenRect.width + 28
                height: childrenRect.height + 28
                RowLayout {
                    anchors.left: parent.left
                    anchors.leftMargin: 14
                    anchors.top: parent.top
                    anchors.topMargin: 14
                    spacing: 6
                    Icon {
                        id: tipIcon
                        width: 24
                        height: 24
                        visible: iconType !== KBallonTip.IconType.None
                    }
                    DtThemeText {
                        id: tipText
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                Component.onCompleted: {
                    if (iconType === KBallonTip.IconType.Loading) {
                        loadingTimer.start();
                        return;
                    } else {
                        loadingTimer.stop();
                    }
                    if (iconType === KBallonTip.IconType.Custom) {
                        tipIcon.source = Qt.binding(function() { return customIconSource; });
                    } else if (iconType === KBallonTip.IconType.Error) {
                        tipIcon.source = "dialog-error";
                    } else if (iconType === KBallonTip.IconType.Success) {
                        tipIcon.source = "ukui-dialog-success";
                        tipIcon.fallbackSource = "emblem-default";
                    } else if (iconType === KBallonTip.IconType.Warning) {
                        tipIcon.source = "dialog-warning";
                    } else if (iconType === KBallonTip.IconType.Info) {
                        tipIcon.source = "dialog-info";
                    }
                }
            }

            RectRegion {
                id: blurRegion
                x: mainItem.mapToItem(mainItem, backGround.x, backGround.y).x
                y: mainItem.mapToItem(mainItem, backGround.x, backGround.y).y
                width: backGround.width
                height: backGround.height
                radius: GlobalTheme.kRadiusMenu
            }
            Component.onCompleted: {
                window.blurRegion.appendRect(blurRegion);
                window.maskRegion.appendRect(blurRegion);
            }

            DtDropShadow {
                id: shadow
                anchors.fill: backGround
                shadowData: GlobalTheme.kShadowMenu
                shadowRadius: backGround.radius
            }
        }
    }
}
