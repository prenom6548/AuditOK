/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0

ApplicationWindow {
    id: control

    /// token 常为 QBrush，先收成 color 再取 rgb
    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token
    readonly property color _containColor: _hasToken
                                           ? ukui_platformtheme_token.kContainGeneralNormal
                                           : palette.base
    readonly property real _bgAlpha: {
        var a = (control.active && blurEnabled) ? app.menuTransparency : 1.0
        if (a === undefined || a === null)
            return 1.0
        return a
    }
    readonly property color _bgFillColor: Qt.rgba(_containColor.r, _containColor.g, _containColor.b, _bgAlpha)

    // ApplicationWindow 对带 alpha 的 color 表现异常：窗口本身透明，由 background 铺色
    color: "transparent"
    background: Rectangle {
        anchors.fill: parent
        color: control._bgFillColor
    }

    property var _titleIcon: {
        if(control.hasOwnProperty("titleIcon"))
            return control.titleIcon
        return ""
    }
    property bool isNormal : true
    property alias minButton: minBtn
    property alias maxButton: maxBtn
    property alias closeButton: closeBtn
    property alias menuButton: menuBtn
    property bool blurEnabled: false


    signal closeBtnClicked
    signal maxOrRestoryBtnClicked(bool isNormal)
    signal minBtnClicked

    width:  600
    height: 480
    minimumWidth: 200
    header: Item{
        height: 40
        width: parent.width

        RowLayout{
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            Image{
                id: titleImage
                visible: _titleIcon != "" && control.title != ""
                source: "image://imageProvider/" + _titleIcon + "/" + (control.active ? "normal" : "disenable");
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: false
                sourceSize.width:  24
                sourceSize.height: 24
            }
            Label{
                visible: control.title != ""
                text: control.title
                property var windowLabel: true
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Private.KHeadBarMenuBtn{
                id: menuBtn
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: false
            }
            Private.KWindowButton{
                id: minBtn
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: false
                property string type: "minimize"
                onClicked: {
                    control.showMinimized()
                    minBtnClicked()
                }
            }
            Private.KWindowButton{
                id: maxBtn
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: false
                property string type: "maximize"
                onClicked:{
                    if(isNormal){
                        control.showMaximized()
                    }
                    else{
                        control.showNormal()
                    }
                    isNormal = !isNormal
                    maxOrRestoryBtnClicked(isNormal)
                }
                onHoveredChanged: {
                    maxBtnFlyoutTimer.stop()
                    if (hovered)
                        maxBtnFlyoutTimer.start()
                }

            }
            Private.KWindowButton{
                id: closeBtn
                Layout.alignment: Qt.AlignVCenter |Qt.AlignRight
                Layout.fillWidth: false
                property string type: "close"
                onClicked: {
                    closeBtnClicked()
                    control.close()
                }
            }
        }
    }

    Timer {
        id: maxBtnFlyoutTimer
        interval: Qt.styleHints.mousePressAndHoldInterval
        repeat: false
        onTriggered: {
            if (!maxBtn.hovered)
                return
            kdialog.showTileFlyout(maxBtn.x, maxBtn.y, Math.round(maxBtn.width), Math.round(maxBtn.height))
        }
    }

    Private.KyQuickDialog {
        id: kdialog
    }
    /// KAboutDialog 等子组件通过 root.kyQuickDialog 调用 openSupportLink / privacyClicked
    property alias kyQuickDialog: kdialog

    function iconSource(){
        if(_titleIcon != "")
            return "image://imageProvider/" + _titleIcon + "/" + (control.active ? "normal" : "disenable");
    }
    APPParameter{
        id: app
        onParametryChanged:{
            titleImage.source = ""
            titleImage.source = iconSource()
        }
        onIconThemeChanged: {
            titleImage.source = ""
            titleImage.source = iconSource()
        }
    }
    Private.UKUIWindow{
        id:ukuiWindow
        blurEnabled: control.blurEnabled
        shadowEnabled: true
        borderEnabled: true
        roundCornerEnabled: true
    }
    Component.onCompleted: ukuiWindow.setUseStyleWindowManager(true)
}
