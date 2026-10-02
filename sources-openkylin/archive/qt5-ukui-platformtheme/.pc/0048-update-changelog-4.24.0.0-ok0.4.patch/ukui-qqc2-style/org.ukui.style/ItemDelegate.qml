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

T.ItemDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             itemdelegate.implicitHeight + topInset + bottomInset)

    padding: itemdelegate.padding
    spacing: app.space

    icon.width: app.iconWidth
    icon.height: app.iconWidth
    hoverEnabled: true
    font: app.font
    palette: app.palette

//    icon.color: control.palette.text
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var isAlternate:{
        if(control.hasOwnProperty("alternate")){
            if(control.alternate && index%2 === 1)
                return true
        }
        else
            return false
    }
    property var borderHColor: !control.enabled ? itemdelegate.disableBorderHColor : /*control.pressed ? itemdelegate.clickBorderHColor :
                               control.hovered ? itemdelegate.hoverBorderHColor : */itemdelegate.normalBorderHColor
    property var backHColor: !control.enabled ? itemdelegate.disableHBC : /*control.pressed ? itemdelegate.clickedHBC :
                             control.hovered ? itemdelegate.hoveredHBC :*/ itemdelegate.normalHBC

    property var borderColor: !control.enabled ? (isTransaprent ? itemdelegate.disableTransparentBorderColor : itemdelegate.disableBorderColor) :
                               (control.pressed) ? (isTransaprent ? itemdelegate.clickedTransparentBorderColor : itemdelegate.clickBorderColor) :
                               control.hovered ? (isTransaprent ? itemdelegate.hoveredTransparentBorderColor : itemdelegate.hoverBorderColor) :
                                                 (isTransaprent ? itemdelegate.normalTransparentBorderColor  : itemdelegate.normalBorderColor)
    property var backColor: !control.enabled ? (isTransaprent ? (isAlternate ? itemdelegate.disableAlternateTransparentBC : itemdelegate.disableTransparentBC ): (isAlternate ? itemdelegate.disableAlternateBC : itemdelegate.disableBC)) :
                             (control.pressed) ? (isTransaprent ? (isAlternate ? itemdelegate.clickedAlternateTransparentBC : itemdelegate.clickedTransparentBC ): (isAlternate ? itemdelegate.clickedAlternatedBC : itemdelegate.clickedBC)) :
                             control.hovered ? (isTransaprent ? (isAlternate ? itemdelegate.hoveredAlternateTransparentBC : itemdelegate.hoveredTransparentBC ): (isAlternate ? itemdelegate.hoveredAlternatedBC : itemdelegate.hoveredBC)) :
                                               (isTransaprent ? (isAlternate ? itemdelegate.normalAlternateTransparentBC  : itemdelegate.normalTransparentBC  ): (isAlternate ? itemdelegate.normalAlternateBC : itemdelegate.normalBC))
    property var borderCheckedColor: !control.enabled ? (isTransaprent ? itemdelegate.disableCheckedTransparentBorderColor : itemdelegate.disableBorderCheckedColor) :
                                      //control.pressed ? (isTransaprent ? itemdelegate.clickedCheckedTransparentBorderColor : itemdelegate.clickBorderCheckedColor) :
                                      //control.hovered ? (isTransaprent ? itemdelegate.hoveredCheckedTransparentBorderColor : itemdelegate.hoverBorderCheckedColor) :
                                                        (isTransaprent ?  itemdelegate.normalCheckedTransparentBorderColor : itemdelegate.normalBorderCheckedColor)
    property var backCheckedColor: !control.enabled ? (isTransaprent ? itemdelegate.disableCheckedTransparentBC : itemdelegate.disableCheckedBC) :
                                    //control.pressed ? (isTransaprent ? itemdelegate.clickedCheckedTransparentBC : itemdelegate.clickedCheckedBC) :
                                    //control.hovered ? (isTransaprent ? itemdelegate.hoveredCheckedTransparentBC : itemdelegate.hoveredCheckedBC) :
                                    (isTransaprent ?  itemdelegate.normalCheckedTransparentBC : itemdelegate.normalCheckedBC)

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

//    contentItem: IconLabel {
//        spacing: control.spacing
//        mirrored: control.mirrored
//        display: control.display
//        alignment: control.display === IconLabel.IconOnly || control.display === IconLabel.TextUnderIcon ? Qt.AlignCenter : Qt.AlignLeft

//        icon: control.icon
//        text: control.text
//        font: app.font
//        color: getStartColor(control.enabled ? itemdelegate.normalTextColor : itemdelegate.disableTextColor)
//    }

    contentItem: IconLabelContent {
        id: iconLabel
        controlRoot: control
        //        anchors.verticalCenter: control.verticalCenter

        leftSpace: control.mirrored ? control.spacing : control.leftPadding
        textColor: !control.enabled ? ((control.highlighted && control.checked) ? itemdelegate.disableHTextColor : itemdelegate.disableTextColor) : (control.highlighted && control.checked) ? itemdelegate.normalHTextColor : itemdelegate.normalTextColor
        //x: 0// control.text ? (control.mirrored ? control.leftPadding : control.width - width - control.rightPadding) : control.leftPadding + (control.availableWidth - width) / 2
        //anchors.centerIn: control
        implicitWidth: iconLabel.adjustWidth(control.leftPadding + control.rightPadding) + indicator.implicitWidth + control.spacing
    }

    background: Rectangle {
        anchors.fill: control
        radius: itemdelegate.radius
        border.width: itemdelegate.borderWidth
        border.color: getStartColor((control.highlighted && control.checked) ? borderHColor : control.checked ? borderCheckedColor : borderColor)
//        implicitWidth: 100
//        implicitHeight: 40
//        visible: control.down || (control.highlighted && control.checked) || control.visualFocus
        color: gradientRec.visible ? "transparent" : getStartColor((control.highlighted && control.checked) ? backHColor : control.checked ? backCheckedColor : backColor)

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            border.width: parent.border.width
            border.color: parent.border.color
            gradient:  Gradient {
                id:gradient
                GradientStop { position: 0.0; color: getStartColor((control.highlighted && control.checked) ? backHColor : control.checked ? backCheckedColor : backColor) }

                GradientStop { position: 1.0; color: getEndColor((control.highlighted && control.checked) ? backHColor : control.checked ? backCheckedColor : backColor)}
            }
            visible: !parseInterface.isSolidPattern((control.highlighted && control.checked) ? backHColor : control.checked ? backCheckedColor : backColor)
        }
    }

    StylePrivate.UKUIItemDelegate{
        id: itemdelegate
    }

    StylePrivate.APPParameter{
        id: app
    }
}
