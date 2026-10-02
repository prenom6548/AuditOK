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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.HorizontalHeaderView {
    id: control

    implicitWidth: syncView ? syncView.width : 0
    implicitHeight: Math.max(contentHeight, headerview.normalHeight)

    delegate: Item {
        // Qt6: add cellPadding (and font etc) as public API in headerview
        readonly property real cellPadding: 8

        implicitWidth: text.implicitWidth + (cellPadding * 2)
        implicitHeight: Math.max(control.height, text.implicitHeight + (cellPadding * 2))

        Text {
            id: text
            text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole]
                                                                   : model[control.textRole])
                                   : modelData
            width: parent.width
            height: parent.height
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: getStartColor(headerview.normalTextColor)
        }
        Rectangle{
            anchors.right: parent.right
            width: headerview.lineWidth
            anchors.verticalCenter: parent.verticalCenter
            height: 14
            color: headerview.lineColor
            visible: index !== control.model.count - 1
        }
    }
    StylePrivate.APPParameter{
        id: appP
    }

    StylePrivate.UKUIHeaderView{
        id: headerview
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
}
