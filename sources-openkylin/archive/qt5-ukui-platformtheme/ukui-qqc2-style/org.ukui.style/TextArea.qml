/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
** Copyright (C) 2023, KylinSoft Co., Ltd.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 3 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this library.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Layouts 1.3
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.TextArea {
    id: control

    implicitWidth: Math.max(contentWidth + leftPadding + rightPadding,
                            implicitBackgroundWidth + leftInset + rightInset,
                            placeholder.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                             implicitBackgroundHeight + topInset + bottomInset,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    padding: textfield.leftRightPadding
    leftPadding: padding

    color: getStartColor(!control.enabled ? textfield.disableTextColor : textfield.normalTextColor)
    selectionColor: getStartColor(textfield.focusBorderColor)
    selectedTextColor: control.palette.highlightedText
    placeholderTextColor: getStartColor(!control.enabled ? textfield.placeHolderDisableTextColor : textfield.placeHolderNormalTextColor)
    font: app.font
    hoverEnabled: true
    selectByMouse: true
    wrapMode: TextArea.Wrap
    Layout.fillWidth: true
    Layout.fillHeight: true
    palette: app.palette
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var backColor: !control.enabled ? (isTransaprent ? textfield.disableTransparentBC : textfield.disableBC) :
                             (control.activeFocus) ? (isTransaprent ? textfield.inputNormalTransparentBC : textfield.inputNormalBC) :
                             click ? (isTransaprent ? textfield.inputNormalTransparentBC : textfield.inputNormalBC) :
                             control.hovered ? (isTransaprent ? textfield.hoveredTransparentBC : textfield.hoveredBC) :
                                               (isTransaprent ? textfield.normalTransparentBC : textfield.normalBC)
    property var backBorderColor: !control.enabled ? textfield.disableBorderColor : (control.activeFocus) ? textfield.focusBorderColor :
                                                                                                                           control.hovered ? textfield.hoverBorderColor : textfield.normalBorderColor

//    property bool hover: false
    property bool click: false
    //    property bool contextMenuEnabled: false
    property bool _contextMenuEnabled:{
    if(control.hasOwnProperty("contextMenuEnabled"))
        return control.contextMenuEnabled
    else
        return false
    }
    signal contextMenuAboutToShow()
    signal contextMenuClosed()

    function deleteSelectionOrNextCharacter()
    {
        if (!control.enabled || control.readOnly)
            return

        if (control.selectedText.length > 0) {
            control.remove(control.selectionStart, control.selectionEnd)
        } else if (control.cursorPosition < control.text.length) {
            control.remove(control.cursorPosition, control.cursorPosition + 1)
        }
    }

    Menu {
        id: fileMenu
        parent: control.Window ? control.Window.window.contentItem : null
        visible: false
        width: 273

        property int itemHeight: 39

        onClosed: control.contextMenuClosed()

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("undo")
            shortcutText: "Ctrl+Z"
            enabled: control.enabled && !control.readOnly && control.canUndo
            icon.name: "edit-undo-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.undo()
                fileMenu.close()
            }
        }

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("redo")
            shortcutText: "Ctrl+Y"
            enabled: control.enabled && !control.readOnly && control.canRedo
            icon.name: "edit-redo-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.redo()
                fileMenu.close()
            }
        }

        MenuSeparator {}

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("cut")
            shortcutText: "Ctrl+X"
            enabled: control.enabled && !control.readOnly && control.selectedText.length > 0
            icon.name: "edit-cut-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.cut()
                fileMenu.close()
            }
        }

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("copy")
            shortcutText: "Ctrl+C"
            enabled: control.enabled && control.selectedText.length > 0
            icon.name: "edit-copy-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.copy()
                fileMenu.close()
            }
        }

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("paste")
            shortcutText: "Ctrl+V"
            enabled: control.enabled && !control.readOnly && control.canPaste
            icon.name: "edit-paste-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.paste()
                fileMenu.close()
            }
        }

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("delete")
            shortcutText: "Del"
            enabled: control.enabled && !control.readOnly && (control.selectedText.length > 0 || control.cursorPosition < control.text.length)
            icon.name: "edit-delete-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.deleteSelectionOrNextCharacter()
                fileMenu.close()
            }
        }

        MenuSeparator {}

        MenuItem {
            implicitWidth: fileMenu.width
            implicitHeight: fileMenu.itemHeight
            text: qsTr("select all")
            shortcutText: "Ctrl+A"
            enabled: control.enabled && control.text.length > 0 && control.selectedText.length !== control.text.length
            icon.name: "edit-select-all-symbolic"
            icon.width: 16
            icon.height: 16
            onTriggered: {
                control.selectAll()
                fileMenu.close()
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Undo]
        enabled: control.activeFocus && control.enabled && !control.readOnly && control.canUndo
        onActivated: control.undo()
    }

    Shortcut {
        sequence: "Ctrl+Y"
        enabled: control.activeFocus && control.enabled && !control.readOnly && control.canRedo
        onActivated: control.redo()
    }

    Shortcut {
        sequences: [StandardKey.Cut]
        enabled: control.activeFocus && control.enabled && !control.readOnly && control.selectedText.length > 0
        onActivated: control.cut()
    }

    Shortcut {
        sequences: [StandardKey.Copy]
        enabled: control.activeFocus && control.enabled && control.selectedText.length > 0
        onActivated: control.copy()
    }

    Shortcut {
        sequences: [StandardKey.Paste]
        enabled: control.activeFocus && control.enabled && !control.readOnly && control.canPaste
        onActivated: control.paste()
    }

    Shortcut {
        sequence: "Del"
        enabled: control.activeFocus && control.enabled && !control.readOnly
                 && (control.selectedText.length > 0 || control.cursorPosition < control.text.length)
        onActivated: control.deleteSelectionOrNextCharacter()
    }

    Shortcut {
        sequences: [StandardKey.SelectAll]
        enabled: control.activeFocus && control.enabled && control.text.length > 0
                 && control.selectedText.length !== control.text.length
        onActivated: control.selectAll()
    }

    MouseArea {
        anchors.fill: parent
        enabled: control._contextMenuEnabled
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        cursorShape: control.enabled ? Qt.IBeamCursor : Qt.ArrowCursor

        onClicked: (mouse) => {
            if (control._contextMenuEnabled && mouse.button === Qt.RightButton) {
                control.forceActiveFocus(Qt.MouseFocusReason);
                const start = control.selectionStart;
                const end = control.selectionEnd;

                const window = control.Window ? control.Window.window : null;
                control.contextMenuAboutToShow()
                if (window) {
                    fileMenu.parent = window.contentItem;
                    const windowPos = control.mapToItem(window.contentItem, mouse.x, mouse.y);
                    fileMenu.x = windowPos.x;
                    fileMenu.y = windowPos.y;
                    fileMenu.open();
                } else {
                    fileMenu.popup(mouse.x, mouse.y);
                }

                control.select(start, end);
                control.forceActiveFocus(Qt.MouseFocusReason);

                mouse.accepted = false;
            }
        }
    }

    ParseInterface{
        id: parseInterface
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }

    background: Rectangle {
        id: backrec
        radius: textfield.radius
        implicitWidth: textfield.normalWidth
        implicitHeight: textfield.normalHeight
        color: gradientRec.visible ? "transparent" : getStartColor(backColor)

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(backColor) }

                GradientStop { position: 1.0; color: getEndColor(backColor)}
            }
            visible: !parseInterface.isSolidPattern(backColor)
        }
        Rectangle{
        anchors.fill: parent
        radius: textfield.radius
        color: "transparent"
        border.color: getStartColor(backBorderColor)
        border.width: (control.activeFocus) ? textfield.focusBorderWidth : textfield.borderWidth

        }
//        Component.onCompleted: console.log("123321....", verticalScrollBar.visible)

        //没办法内置滚动条  滚动条没办法跟内容联动
//        ScrollBar.vertical: ScrollBar {

//        id: verticalScrollBar

//        parent: backrec

//        anchors.right: backrec.right

//        anchors.top: backrec.top

//        anchors.bottom: backrec.bottom

//        visible: control.contentHeight > control.height

//        orientation: Qt.Vertical

//        size: control.height * 1.0/ control.contentHeight*1.0 //control.flickableItem.visibleArea.heightRatio

//        //position: 0.5//control.flickableItem.visibleArea.yPosition

//        active: visible//control.flickableItem.moving || control.flickableItem.dragging

//        onPositionChanged: {
//            var newY = position * (control.contentHeight - control.height)
//            control.flickableItem.contentY = newY

//           // control.flickable.contentY = position * control.contentHeight
//        }


//    //    policy: (verticalScrollBarAlwaysVisible || control.flickableItem.contentHeight > control.height) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

//    //    orientation: Qt.Vertical

//    //    size: control.flickableItem.visibleArea.heightRatio

//    //    position: control.flickableItem.visibleArea.yPosition

//    //    active: control.flickableItem.moving || control.flickableItem.dragging

//        }

    }


    PlaceholderText {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: app.font
        color: getStartColor(!control.enabled ? textfield.placeHolderDisableTextColor : textfield.placeHolderNormalTextColor)
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        renderType: control.renderType
    }

    StylePrivate.UKUITextArea{
        id: textfield
    }

    StylePrivate.APPParameter{
        id: app
    }
    onPressed: {
        click = true;
    }
    onReleased: click = false

}
