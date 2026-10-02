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
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Pane {
    id: control
    property var dColor:{
        if(control.hasOwnProperty("dtColor"))
            return control.dtColor
        else
            return ""
    }
    onDColorChanged: {
        if(dColor !== "")
            rec.color = parseInterface.getDTColor(dColor)
    }

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    palette: appP.palette
    StylePrivate.APPParameter{
        id: appP
        onParametryChanged: {
            if(control.hasOwnProperty("dtColor"))
                rec.color = parseInterface.getDTColor(dtColor)
        }
    }
    ParseInterface{
        id: parseInterface
    }
    padding: 12

    background: Rectangle {
        id: rec
        color: {
            if(dColor !== ""){
                return parseInterface.getDTColor(dColor)
            }
            else
            return appP.palette.window
        }
    }
}
