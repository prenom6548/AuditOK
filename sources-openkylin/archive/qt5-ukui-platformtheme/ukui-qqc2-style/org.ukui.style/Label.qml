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

T.Label {
    id:control


    property var dtColor:{
        if(control.hasOwnProperty("labelDTColor"))
            return control.labelDTColor
        else
            return ""
    }

    property var _windowLabel: {
        if(control.hasOwnProperty("windowLabel"))
            return control.windowLabel
        else
            return false
    }

    ParseInterface{
        id: parseInterface
    }

    onDtColorChanged: {
        if(dtColor !== "")
            control.color = parseInterface.getDTColor(dtColor)
    }

    color: {
        if(dtColor !== ""){
            return parseInterface.getDTColor(dtColor)
        }
        else
        return parseInterface.pStartColor(enabled ? (_windowLabel && !Qt.application.active) ? parseInterface.getDTColor("kfont-secondary-disable") :
                                                                             label.normalColor : label.disableColor)
    }//StylePrivate.UKUILable.normalColor
    linkColor: parseInterface.pStartColor(label.linkColor)

    font: app.font//appP.font
    palette: app.palette

    Connections{
        target: Qt.application
        function onStateChanged(){
            if(control.hasOwnProperty("windowLabel")){
                control.color = parseInterface.pStartColor(enabled ? (_windowLabel && !Qt.application.active) ? parseInterface.getDTColor("kfont-secondary-disable") :
                                                                                                                label.normalColor : label.disableColor)
            }
        }
    }

    StylePrivate.UKUILabel{
        id: label
        onParametryChanged:{
            control.update();
        }
    }

    StylePrivate.APPParameter{
        id: app
        onParametryChanged: {
            if(control.hasOwnProperty("labelDTColor"))
                control.color = parseInterface.getDTColor(dtColor)
            else if(control.hasOwnProperty("windowLabel")){
                control.color = parseInterface.pStartColor(enabled ? (_windowLabel && !Qt.application.active) ? parseInterface.getDTColor("kfont-secondary-disable") :
                                                                                                                label.normalColor : label.disableColor)
            }
        }
    }
}
