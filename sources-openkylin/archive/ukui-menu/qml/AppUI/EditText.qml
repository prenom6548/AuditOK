/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

Item {
    id: contain
    property bool textCenterIn: false
    property bool editStatus: false
    property string modelTitle: ""   // 应用组初始名称（从model获取）
    property string title: editLoader.item.title // 应用组可编辑名称
    property int textSizeOffset: mainWindow.isFullScreen ? 13 : 1
    property int textBorderWidth: mainWindow.isFullScreen ? 2 : 1

    signal textEditingFinished()

    Loader {
        id: editLoader
        anchors.fill: parent
        sourceComponent: contain.editStatus ? editText : unEditText
    }

    Component {
        id: unEditText
        UkuiItems.DtThemeText {
            id: textShow
            property string title: textShow.text
            text: contain.modelTitle

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: contain.textCenterIn ? Text.AlignHCenter : Text.AlignLeft
            elide: Text.ElideRight
            textColor: Platform.GlobalTheme.textActive
            font.bold: true
            pointSizeOffset: textSizeOffset

            MouseArea {
                id: textArea
                anchors.fill: parent
                onClicked: contain.editStatus = true;
            }
        }
    }

    Component {
        id: editText
        UkuiItems.DtThemeBackground {
            property string title: textEdit.text

            radius: Platform.GlobalTheme.kRadiusNormal
            useStyleTransparency: false
            backgroundColor: textEdit.activeFocus ? Platform.GlobalTheme.kGrayAlpha1 : Platform.GlobalTheme.kGrayAlpha0
            border.width: textBorderWidth
            borderColor: textEdit.activeFocus ? Platform.GlobalTheme.kBrandNormal : Platform.GlobalTheme.kGrayAlpha0
            TextInput {
                id: textEdit
                clip: true
                text: contain.modelTitle

                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: buttonMouseArea.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: contain.textCenterIn ? Text.AlignHCenter : Text.AlignLeft

                selectByMouse: true
                maximumLength: 14
                font.pointSize: Platform.GlobalTheme.fontSize + textSizeOffset
                font.family: Platform.GlobalTheme.fontFamily;
                font.bold: true
                color : Platform.GlobalTheme.kFontPrimary.pureColor;
                selectionColor : Platform.GlobalTheme.highlightActive.pureColor;

                onEditingFinished: {
                    textEditingFinished();
                }

                Component.onCompleted: {
                    forceActiveFocus();
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.IBeamCursor
                    enabled: false
                }
            }

            MouseArea {
                id: buttonMouseArea
                hoverEnabled: true
                width: mainWindow.isFullScreen ? 24 : 16; height: width
                anchors.right: parent.right
                anchors.rightMargin: mainWindow.isFullScreen ? 24 : 10
                anchors.verticalCenter: parent.verticalCenter
                visible: textEdit.activeFocus

                UkuiItems.Icon {
                    anchors.centerIn: parent
                    width: parent.width; height: width
                    source: "image://theme/edit-clear-symbolic"
                    mode: UkuiItems.Icon.AutoHighlight
                }

                onClicked: {
                    textEdit.text = "";
                }
            }
        }
    }
}
