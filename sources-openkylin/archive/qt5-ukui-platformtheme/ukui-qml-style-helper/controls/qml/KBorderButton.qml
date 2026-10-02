/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KBorderButton.
 * 基于 KBorderlessButton，圆角描边对齐 kborderbutton.cpp。
 */

import QtQuick 2.12
import org.ukui.platformtheme.controls 1.0 as Private

Private.KBorderlessButton {
    id: control

    paletteStyle: Private.KyButton.StyleBorder

    property int borderWidth: 1
    property int radius: (typeof ukui_platformtheme_token !== "undefined"
                          && ukui_platformtheme_token
                          && ukui_platformtheme_token.kradiusNormal > 0)
                         ? ukui_platformtheme_token.kradiusNormal : 6

    readonly property real _inset: borderWidth

    leftPadding: buttonStyle.leftRightMargin + _inset + 1
    rightPadding: buttonStyle.leftRightMargin + _inset + 1
    topPadding: buttonStyle.upDownMargin + _inset + 1
    bottomPadding: buttonStyle.upDownMargin + _inset + 1

    implicitWidth: Math.max(implicitContentWidth + leftPadding + rightPadding,
                            buttonStyle.normalWidth)
    implicitHeight: Math.max(implicitContentHeight + topPadding + bottomPadding,
                            buttonStyle.normalHeight)

    background: Rectangle {
        anchors.fill: parent
        anchors.margins: control._inset
        radius: Math.max(0, control.radius - control._inset)
        color: "transparent"
        border.width: control.borderWidth
        border.color: buttonPalette.borderColor
        antialiasing: true
    }
}
