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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

import QtQuick 2.15
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype SearchLineEdit
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \inherits GradientBorderItem
    \brief 提供带搜索图标和清除按钮的搜索输入框组件。

    SearchLineEdit 是一个功能完整的搜索输入框组件，提供搜索图标、占位文本、清除按钮等功能。
    支持透明模式、AI 按钮、右键菜单等高级特性，并具有流畅的动画效果。

    \section1 主要特性
    \list
        \li 内置搜索图标和占位文本
        \li 自动显示/隐藏清除按钮
        \li 支持透明模式和非透明模式
        \li 可选的 AI 按钮和分隔线
        \li 支持右键上下文菜单（撤销、重做、剪切、复制、粘贴等）
        \li 焦点状态下的边框高亮效果
        \li 流畅的动画过渡
        \li 支持从右到左布局（RTL）
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    SearchLineEdit {
        width: 300
        height: 32
        placeholderText: "搜索..."
        onTextChanged: {
            console.log("搜索内容:", text)
        }
    }
    \endqml

    启用 AI 按钮和右键菜单：
    \qml
    SearchLineEdit {
        width: 300
        height: 32
        showAIButton: true
        enableContextMenu: true
        enableTransparentMode: false
    }
    \endqml

    访问内部 TextInput：
    \qml
    SearchLineEdit {
        id: searchBox
        Component.onCompleted: {
            searchBox.textInput.selectAll()
            searchBox.textInput.forceActiveFocus()
        }
    }
    \endqml

    \sa GradientBorderItem, LineEditContextMenu, DtThemeButton
*/

GradientBorderItem {
    id: root
    /*!
        \qmlproperty string SearchLineEdit::text

        搜索框的文本内容。

        该属性是内部 TextInput 的 text 属性的别名，可用于获取或设置搜索框的文本。
    */
    property alias text: textInput.text
    /*!
        \qmlproperty TextInput SearchLineEdit::textInput

        内部的 TextInput 组件。

        通过该属性可以访问底层的 TextInput 对象，以便使用其所有属性和方法，
        例如 selectAll()、forceActiveFocus() 等。
    */
    property alias textInput: textInput
    /*!
        \qmlproperty var SearchLineEdit::backgroundColor

        组件背景色。

        使用GlobalTheme中的颜色对象，支持纯色和渐变色。
    */
    property alias backgroundColor: back.backgroundColor
    /*!
        \qmlproperty bool SearchLineEdit::showAIButton

        是否显示 AI 按钮和分隔线。

        当设置为 true 时，搜索框右侧会显示一个 AI 按钮和分隔线。
        点击 AI 按钮可以切换特殊边框高亮状态。
        默认值为 false。
    */
    property bool showAIButton: false
    /*!
        \qmlproperty bool SearchLineEdit::enableTransparentMode

        控件是否使用透明模式。

        \list
            \li true - 使用半透明背景色（GlobalTheme.kComponentAlpha 系列）
            \li false - 使用不透明背景色（GlobalTheme.kComponent 系列）
        \endlist

        默认值为 true。
    */
    property bool enableTransparentMode: true
    /*!
        \qmlproperty string SearchLineEdit::placeholderText

        占位文本，当搜索框为空时显示。

        默认值为 "search"（会被翻译）。
    */
    property string placeholderText: qsTr("search")
    /*!
        \qmlproperty bool SearchLineEdit::enableContextMenu

        是否启用右键上下文菜单。

        当设置为 true 时，右键点击搜索框会显示包含撤销、重做、剪切、复制、
        粘贴、删除和全选等操作的上下文菜单。
        默认值为 false。
    */
    property bool enableContextMenu :false

    borderWidth: enableSpecificBorder ? 2 : textInput.activeFocus ? 2 : 1
    borderRadius: GlobalTheme.kRadiusNormal
    borderColor: getBorderColor()
    function getBorderColor() {
        return textInput.activeFocus ? GlobalTheme.kBrandNormal
                                     : enabled ? GlobalTheme.kLineComponentNormal
                                               : GlobalTheme.kLineComponentDisable
    }
    function getBackgroundColor() {
        if(enableTransparentMode) {
            return textInput.activeFocus ? GlobalTheme.kComponentInputAlpha
                                         : mouseArea.containsPress ? GlobalTheme.kComponentAlphaClick
                                                                   : mouseArea.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                                             : enabled ? GlobalTheme.kComponentAlphaNormal
                                                                                                       : GlobalTheme.kComponentAlphaDisable
        } else {
            return textInput.activeFocus ? GlobalTheme.kComponentInput
                                         : mouseArea.containsPress ? GlobalTheme.kComponentClick
                                                                   : mouseArea.containsMouse ? GlobalTheme.kComponentHover
                                                                                             : enabled ? GlobalTheme.kComponentNormal
                                                                                                       : GlobalTheme.kComponentDisable
        }
    }

    DtThemeBackground {
        id: back
        clip: false
        anchors.centerIn: parent
        width: root.width - 2 * root.borderWidth
        height: root.height - 2 * root.borderWidth
        radius: root.borderRadius * (1 - 1 / 3 * root.borderWidth / root.borderRadius)
        useStyleTransparency: false
        backgroundColor: getBackgroundColor()
        border.width: 0

        Item {
            id: defaultSearch
            width: searchIcon.width + defaultText.width
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            x: getDefaultSearchPosition()

            Item {
                id: searchIcon
                width: height
                height: root.height
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left

                Icon {
                    anchors.centerIn: parent
                    width: height
                    height: 16/32 * root.height
                    source: "search-symbolic"
                    mode: Icon.AutoHighlight
                }
            }

            DtThemeText {
                id: defaultText
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: searchIcon.right
                text: placeholderText
                visible: textInput.text.length === 0
                textColor: root.enabled ? GlobalTheme.kFontPlaceholderText : GlobalTheme.kFontPlaceholderTextDisable
                verticalAlignment: TextInput.AlignVCenter
            }

            Behavior on x {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.Bezier
                    easing.bezierCurve: [0.25, 0.1, 0.05, 1, 1, 1]
                }
            }

            function getDefaultSearchPosition() {
                let posX = text.length !== 0 ? 0
                                             : textInput.activeFocus ? 0
                                                                     : showAIButton ? (back.width - defaultSearch.width - searchIcon.width / 2 - aiButton.width) / 2
                                                                                    : (back.width - defaultSearch.width - searchIcon.width / 2) / 2
                if (LayoutMirroring.enabled) {
                    posX = back.width - defaultSearch.width - posX;
                }
                return posX;
            }
        }

        TextInput {
            id: textInput
            z: 1
            clip: true
            width: showAIButton ? parent.width - searchIcon.width - clearButton.width - divider.width - aiButton.width
                                : parent.width - searchIcon.width - clearButton.width
            height: parent.height
            selectByMouse: true
            verticalAlignment: TextInput.AlignVCenter
            font.pointSize: defaultText.font.pointSize
            font.family: defaultText.font.family
            focus: true
            activeFocusOnTab: false
            activeFocusOnPress: true
            color: GlobalTheme.kFontPrimary.pureColor
            selectionColor: GlobalTheme.highlightActive.pureColor
            x: getTextInputPosition()

            Behavior on x {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.Bezier
                    easing.bezierCurve: [0.25, 0.1, 0.05, 1, 1, 1]
                }
            }

            function getTextInputPosition() {
                let posX = text.length !== 0 ? searchIcon.width
                                             : textInput.activeFocus ? searchIcon.width
                                                                     : defaultSearch.x + searchIcon.width
                if (LayoutMirroring.enabled) {
                    posX = back.width - textInput.width - posX;
                }
                return posX;
            }
        }

        DtThemeButton {
            id: clearButton
            z: 1
            anchors.right: showAIButton ? divider.right : parent.right
            anchors.rightMargin: showAIButton ? 4 : 0
            anchors.verticalCenter: parent.verticalCenter
            width: 24 / 32 * root.height
            height: 24 / 32 * root.height
            background.radius: width / 2
            visible: textInput.length !== 0
            activeFocusOnTab: false
            background.backgroundColor: GlobalTheme.kGrayAlpha0
            icon.source: "edit-clear-symbolic"
            icon.mode: Icon.AutoHighlight
            icon.width: root.height / 2
            icon.height: root.height / 2
            onClicked: {
                textInput.clear();
            }
        }

        DtThemeBackground {
            id: divider
            anchors.right: aiButton.left
            anchors.verticalCenter: parent.verticalCenter
            visible: showAIButton
            width: 1
            height: root.height / 2
            backgroundColor: GlobalTheme.kLineNormal
        }

        DtThemeButton {
            id: aiButton
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            width: 30 / 32 * root.height
            height: 30 / 32 * root.height
            background.radius: width / 2
            visible: showAIButton
            activeFocusOnTab: false
            background.backgroundColor: GlobalTheme.kGrayAlpha0
            icon.source: "ukui-nlp-symbolic"
            icon.width: root.height / 2
            icon.height: root.height / 2
            icon.mode: !root.enabled ? Icon.Disabled : enableSpecificBorder ? Icon.ForceHighlight : Icon.AutoHighlight
            icon.dtThemeHighlightColor: enableSpecificBorder ? GlobalTheme.kBrandNormal : GlobalTheme.highlightedTextActive
            onClicked: {
                enableSpecificBorder = !enableSpecificBorder;
            }
        }

        MouseArea {
            id: mouseArea
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: showAIButton ? root.width - aiButton.width : root.width
            height: parent.height
            cursorShape: Qt.IBeamCursor
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            propagateComposedEvents: true
            onClicked: (mouse)=> {
                if (mouse.button === Qt.LeftButton) {
                    textInput.forceActiveFocus(Qt.MouseFocusReason)
                } else if (mouse.button === Qt.RightButton) {
                    if(enableContextMenu) {
                        textInput.forceActiveFocus(Qt.MouseFocusReason);
                        menuLoader.item.show(-1,-1,root);
                    }
                }
            }
        }
        Loader {
            id: menuLoader
            active: enableContextMenu
            sourceComponent: menuComponent
        }
        Component {
            id: menuComponent
            LineEditContextMenu {
                 id: menu
                 actions: [
                     Action {
                         iconName: "edit-undo"
                         text: qsTr("&Undo") + menu.getShortCutString(StandardKey.Undo)
                         enabled: textInput.canUndo
                         onTriggered: function() {
                             textInput.undo();
                         }
                     },
                     Action {
                         iconName: "edit-redo"
                         text: qsTr("&Redo") + menu.getShortCutString(StandardKey.Redo)
                         enabled: textInput.canRedo
                         onTriggered: function() {
                             textInput.redo();
                         }
                     },
                     Action {
                         isSeparator: true
                     },
                     Action {
                         iconName: "edit-cut"
                         text: qsTr("Cu&t") + menu.getShortCutString(StandardKey.Cut)
                         enabled: !textInput.readOnly && textInput.echoMode === TextInput.Normal && textInput.selectedText !== ""
                         onTriggered: function() {
                             textInput.cut();
                         }
                     },
                     Action {
                         iconName: "edit-copy"
                         text: qsTr("&Copy") + menu.getShortCutString(StandardKey.Copy)
                         enabled: textInput.echoMode === TextInput.Normal && textInput.selectedText !== ""
                         onTriggered: function() {
                             textInput.copy();
                         }
                     },
                     Action {
                         iconName: "edit-paste"
                         text: qsTr("&Paste") + menu.getShortCutString(StandardKey.Paste)
                         enabled: !textInput.readOnly && textInput.canPaste
                         onTriggered: function() {
                             textInput.paste();
                         }
                     },
                     Action {
                         iconName: "edit-delete"
                         text: qsTr("Delete")
                         enabled: !textInput.readOnly && textInput.text !== "" && textInput.selectedText !== ""
                         onTriggered: function() {
                             textInput.remove(textInput.selectionStart, textInput.selectionEnd);
                         }
                     },
                     Action {
                         isSeparator: true
                     },
                     Action {
                         iconName: "edit-select-all"
                         text: qsTr("Select All") + menu.getShortCutString(StandardKey.SelectAll)
                         enabled: textInput.text !== "" && textInput.selectedText !== textInput.text
                         onTriggered: function() {
                             textInput.selectAll();
                         }
                     }
                 ]
            }
        }
    }

}
