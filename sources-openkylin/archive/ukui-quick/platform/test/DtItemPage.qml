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
 *  Authors: amingamingaming <wangyiming01@kylinos.cn>
 */

import QtQuick 2.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import QtQuick.Controls 2.12
DtThemeBackground {
    id: iconPage
    width: parent.width
    height: parent.height
    useStyleTransparency: false
    backgroundColor: GlobalTheme.kContainGeneralAlphaNormal
    Grid {
        columns: 4
        spacing: 20
        Icon {
            id: iconTest
            width: 100
            height: 100
            clip: true
            source: "battery-level-100-charging-symbolic"
            mode: Icon.AutoHighlight
            dtThemeHighlightColor: GlobalTheme.kWarningClick
        }

        Icon {
            id: iconTest2
            width: 100
            height: 100
            clip: true
            source: "system-log-out-symbolic"
            mode: Icon.Highlight
            dtThemeHighlightColor: GlobalTheme.kWarningClick
        }

        TouchSlider {
            width: 200
            height: 16
            orientation: Qt.Horizontal
        }

        DtThemeBackground {
            width:50
            height: 50
            backgroundColor: GlobalTheme.kContainGeneralAlphaNormal
            HoverButton {
                width: 48
                height: 48
                icon.width: 44
                icon.height: 44
                icon.source: "list-add-symbolic"
            }
        }

        TouchSlider {
            width: 16
            height: 200
            orientation: Qt.Vertical
        }

        SearchLineEdit {
            width:300
            height:32
            placeholderText:"placeholdertext"
        }

        SearchLineEdit {
            width:300
            height:60
            borderRadius: 15
            showAIButton: true
            enableSpecificBorder: false
            placeholderText:"搜索"
            enabled: false
            textInput.enabled: true
        }

        SearchLineEdit {
            width:500
            height:130
            showAIButton: true
            enableSpecificBorder: false
            enableTransparentMode : true
            borderRadius: 10
            borderWidth: 5
            enabled: true
            enableContextMenu:true
        }

        BorderlessButton {
            icon.source: "application-system-symbolic"
            text.text: "输入文本"
            onClicked: console.log("Button clicked")
        }

        BorderlessButton {
            text.text: "输入文本"
            onClicked: console.log("Button clicked")
        }
    }
}
