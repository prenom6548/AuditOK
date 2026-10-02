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
import QtQuick.Window 2.15
import org.ukui.platformtheme.controls 1.0
import org.ukui.qqc2style.private 1.0


ToolButton{
    id: control
    property bool minBtn: control.hasOwnProperty("type") && control.type == "minimize"
    property bool maxBtn: control.hasOwnProperty("type") && control.type == "maximize"
    property bool closeBtn: control.hasOwnProperty("type") && control.type == "close"
    property bool menuBtn: control.hasOwnProperty("type") && control.type == "menu"

    property int i:0
    leftPadding: 7
    rightPadding: 7
    width: 30
    height: 30

    Layout.preferredHeight: 30
    Layout.preferredWidth : 30
    Layout.maximumHeight: 30
    Layout.maximumWidth: 30
    Layout.minimumHeight: 30
    Layout.minimumWidth: 30
    property string _toolTipText: {
        if(control.hasOwnProperty("toolTipText") && control.toolTipText != "")
            return control.toolTipText
        if(minBtn)
            return qsTr("min")
        if(menuBtn)
            return qsTr("Options")
        if(maxBtn){
            if(i%2 === 0)
                return qsTr("maximize")
            else
                return qsTr("restory")
        }
        if(closeBtn)
            return qsTr("closed")
    }
    property var buttonType: {
        if(minBtn || maxBtn || menuBtn)
            return "windowTitleBarWindowButton"
        else if(closeBtn)
            return "windowTitleBarCloseButton"

        return ""
    }
    icon.name: {
        if(minBtn)
            return "window-minimize-symbolic"
        if(menuBtn)
            return "open-menu-symbolic"
        else if (maxBtn){
            if(i%2 === 0)
                return "window-maximize-symbolic"
            else
                return "window-restore-symbolic"
        }
        else if(closeBtn)
            return "window-close-symbolic"

        return ""
    }
    hoverEnabled: true


    ToolTip{
        visible: text != "" && control.hovered && !maxBtn
        text: _toolTipText
    }

}



