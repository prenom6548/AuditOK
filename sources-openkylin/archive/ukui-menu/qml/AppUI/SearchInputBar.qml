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
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.ukui.menu.core 1.0
import AppControls2 1.0 as AppControls2
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

UkuiItems.DtThemeBackground {
    property Item changeFocusTarget
    property alias text: textInput.text;

    useStyleTransparency: false
    backgroundColor: Platform.GlobalTheme.kGrayAlpha12
    alpha: textInput.activeFocus ? 0 : 1
    border.width: 1
    borderColor: textInput.activeFocus ? Platform.GlobalTheme.kBrandNormal : Platform.GlobalTheme.kGrayAlpha1
    Component.onCompleted: mainWindow.windowVisibleChanged.connect(clear)
    Component.onDestruction: mainWindow.windowVisibleChanged.disconnect(clear)
    function textInputFocus() {
        textInput.forceActiveFocus();
    }
    function clear() {
        textInput.clear();
    }
    Item {
        id: defaultSearch
        width: searchIcon.width + defaultText.contentWidth; height: parent.height
        anchors.verticalCenter: parent.verticalCenter
        x: 0

        Behavior on x {
            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
        }

        Item {
            id: searchIcon
            width: 32; height: 32
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: (parent.height - height) / 2

            UkuiItems.Icon {
                anchors.centerIn: parent
                width: parent.height / 2; height: width
                source: "search-symbolic"
                mode: UkuiItems.Icon.AutoHighlight
            }
        }


        UkuiItems.DtThemeText {
            id: defaultText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: searchIcon.right
            text: qsTr("Search App")
            visible: !textInput.activeFocus && textInput.text.length === 0
            textColor: Platform.GlobalTheme.kGrayAlpha8
            verticalAlignment: TextInput.AlignVCenter
        }
    }

    TextInput {
        id: textInput
        clip: true
        anchors.right: clearButton.left
        width: parent.width - searchIcon.width - clearButton.width -
               searchIcon.anchors.leftMargin - clearButton.anchors.rightMargin
        height: parent.height
        selectByMouse: true
        verticalAlignment: TextInput.AlignVCenter
        font.pointSize: defaultText.font.pointSize
        font.family: defaultText.font.family
        focus: true
        activeFocusOnTab: false
        color : Platform.GlobalTheme.kFontPrimary.pureColor;
        selectionColor : Platform.GlobalTheme.highlightActive.pureColor;

        function changeFocusToListView() {
            if (!mainWindow.isFullScreen) {
                normalUI.focus = true;
                changeFocusTarget.focus = true;
                appPage.content.resetFocus();
            }
        }
        Keys.onDownPressed: changeFocusToListView()
        Keys.onReturnPressed: changeFocusToListView()

        onActiveFocusChanged: {
            if (activeFocus) {
                defaultSearch.x = 0;
            } else {
                defaultSearch.x = text.length === 0 ? (parent.width - defaultSearch.width - searchIcon.width/2) / 2 : 0;
            }
        }
    }

    UkuiItems.DtThemeButton {
        id: clearButton
        anchors.right: parent.right
        anchors.rightMargin: (parent.height - height) / 2
        anchors.verticalCenter: parent.verticalCenter
        width: 32; height: width
        background.radius: width / 2
        visible: textInput.length !== 0
        activeFocusOnTab: false

        icon.source: "edit-clear-symbolic"
        icon.mode: UkuiItems.Icon.AutoHighlight

        onClicked: {
            textInput.clear();
            //失去焦点后，若输入框不为空，按下清除按钮后defaultSearch应该移动位置
            if (textInput.text.length === 0 && !textInput.activeFocus) {
                defaultSearch.x = (parent.width - defaultSearch.width - searchIcon.width/2) / 2;
            }
        }
    }

    MouseArea {
        anchors.left: textInput.left
        anchors.verticalCenter: parent.verticalCenter
        width: clearButton.visible ? textInput.width : textInput.width + clearButton.width
        height: parent.height
        cursorShape: Qt.IBeamCursor
        enabled: false
    }
}
