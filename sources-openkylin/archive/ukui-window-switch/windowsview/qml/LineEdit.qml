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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
*
*/

import QtQuick 2.15
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

UkuiItem.DtThemeBackground {
    id: rootItem
    property bool editable: false
    property bool enableLayoutMirroring: false
    property string defaultText: ""
    property string itemDisplayText: defaultText
    property int contentTextWidth: textDisplay.contentWidth
    property bool isDefaultText : true

    signal textEditingFinished()

    implicitWidth: 100
    implicitHeight: 32
    color: "transparent"
    borderColor: editable ? UkuiPlatform.GlobalTheme.kBrandNormal : UkuiPlatform.GlobalTheme.kGrayAlpha0
    border.width : UkuiPlatform.GlobalTheme.focusLine
    radius: UkuiPlatform.GlobalTheme.kRadiusNormal
    backgroundColor: UkuiPlatform.GlobalTheme.kComponentInputAlpha

    Accessible.role: Accessible.Button
    Accessible.name: "edit-text-" + itemDisplayText
    Accessible.description: "push-button, click to modify the text"

    MouseArea {
        id: rootMouseArea
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
        cursorShape: editable ? Qt.IBeamCursor : Qt.ArrowCursor
        onClicked: {
            if (!editable) {
                // 点击进入编辑状态
                editable = true;
            }
        }
    }

    onEditableChanged: {
        if (editable) {
            // textInput进入可输入状态，需要将焦点强制转移给textInput
            textInput.forceActiveFocus();
            textInput.selectAll();
        }
    }

    TextInput {
        id: textInput
        property bool isTextEdited: false
        anchors.fill: parent
        anchors.centerIn : parent
        anchors.leftMargin : 4
        anchors.rightMargin : 4
        horizontalAlignment: TextInput.AlignLeft
        verticalAlignment: TextInput.AlignVCenter
        LayoutMirroring.enabled: enableLayoutMirroring

        clip: true
        maximumLength : 32
        focus: true
        selectByMouse: true
        color: UkuiPlatform.GlobalTheme.kFontPrimary.pureColor
        selectionColor: UkuiPlatform.GlobalTheme.kBrandNormal.pureColor
        visible: editable

        text: itemDisplayText

        onEditingFinished: {
            editable = false;
            if (isTextEdited) {
                updateItemDisplayText()
                // 文本编辑结束，文本是否修改的状态值置为false
                isTextEdited = false
                // 更新Text显示文本
                text = itemDisplayText
                // 发送文本编辑完成信号
                textEditingFinished()
            }
        }

        onTextEdited: {
            if (isTextEdited) {
                return
            } else {
                // 文本首次被修改，文本是否修改的状态值置为true
                isTextEdited = true
            }
        }

        Keys.priority: Keys.AfterItem
        Keys.onPressed: {
            event.accepted = true
        }
    }

    UkuiItem.DtThemeText {
        id: textDisplay
        anchors.fill: parent
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignLeft
        LayoutMirroring.enabled: enableLayoutMirroring
        verticalAlignment: Text.AlignVCenter

        text: itemDisplayText
        elide: enableLayoutMirroring ? Text.ElideLeft : Text.ElideRight
        visible: !editable

        UkuiItem.Tooltip {
            anchors.fill: parent
            mainText: textDisplay.text
            posFollowCursor: true
            active: !editable && textDisplay.truncated
        }
    }

    function updateItemDisplayText() {
        itemDisplayText = textInput.length > 0 ? textInput.displayText : rootItem.defaultText
        isDefaultText = textInput.length > 0 ? false : true
    }
}
