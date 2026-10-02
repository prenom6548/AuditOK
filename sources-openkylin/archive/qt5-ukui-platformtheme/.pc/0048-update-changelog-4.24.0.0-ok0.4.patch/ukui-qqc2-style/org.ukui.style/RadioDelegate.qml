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

T.RadioDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + implicitIndicatorWidth + spacing + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    hoverEnabled: true
    font: app.font
    palette: app.palette

    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property alias indicatorColor: indicatorRec.color
    property alias childrenColor: childrenRec.color
    property bool isHoveredExpanded: false //记录小圆是否处于 “hover 放大到 8px” 的状态

    property var tansparentNormalColor: radiobutton.normalTransparentIndicatorColor
    property var normalColor: radiobutton.normalIndicatorColor
    property var borderColor: control.checked ? (!control.enabled ? radiobutton.checked_disableIndicatorBorderColor :
                                                                    control.pressed ? radiobutton.checked_clickIndicatorBorderColor : control.hovered ?
                                                                    radiobutton.checked_hoverIndicatorBorderColor : radiobutton.checked_normalIndicatorBorderColor) :
                                                                   (!control.enabled ? radiobutton.disableIndicatorBorderColor : control.pressed ?
                                                                    radiobutton.clickIndicatorBorderColor : control.hovered ?
                                                                    radiobutton.hoverIndicatorBorderColor : radiobutton.normalIndicatorBorderColor)

    property var backColor: control.checked ? (!control.enabled ? radiobutton.checked_disableIndicatorColor :
                                                                  control.pressed ? radiobutton.checked_clickIndicatorColor : control.hovered ?
                                                                  radiobutton.checked_hoverIndicatorColor : radiobutton.checked_normalIndicatorColor) :
                                               isTransaprent ? (!control.enabled ? radiobutton.disableTransparentIndicatorColor : control.pressed ?
                                                                 radiobutton.clickTransparentIndicatorColor : control.hovered ?
                                                                 radiobutton.hoverTransparentIndicatorColor : tansparentNormalColor) :
                                                               (!control.enabled ? radiobutton.disableIndicatorColor : control.pressed ?
                                                                 radiobutton.clickIndicatorColor : control.hovered ?
                                                                 radiobutton.hoverIndicatorColor : normalColor)

    property var childrenDtColor: !control.enabled ? radiobutton.checked_disableChildrenColor :
                                                   control.pressed ? radiobutton.checked_clickChildrenColor:
                                                                     control.hovered ? radiobutton.checked_hoverChildrenColor:
                                                                                       radiobutton.checked_normalChildrenColor

    ParseInterface{
        id: parseInterface
    }
    StylePrivate.UKUIRadioButton{
        id: radiobutton
    }

    StylePrivate.APPParameter{
        id: app
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }

    padding: 6
    spacing: radiobutton.space

    contentItem: IconLabelContent {
        id: iconLabel
        controlRoot: control
        anchors.left: control.left
        anchors.right: control.right
        anchors.leftMargin: control.mirrored ? control.indicator.width + control.spacing : control.leftPadding
        anchors.rightMargin: !control.mirrored ? control.indicator.width + control.spacing : control.rightPadding
        //leftSpace: control.mirrored ? control.indicator.width + control.spacing : control.leftPadding
        //x: control.text ? (control.mirrored ? control.leftPadding : control.width - width - control.rightPadding) : control.leftPadding + (control.availableWidth - width) / 2
        //anchors.centerIn: control
        implicitWidth: iconLabel.adjustWidth(control.leftPadding + control.rightPadding)

    }

    // keep in sync with RadioButton.qml (shared RadioIndicator.qml was removed for performance reasons)
    indicator: Rectangle {
        id: indicatorRec
        implicitWidth: radiobutton.indicatorWidth
        implicitHeight: implicitWidth

        x: control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        radius: width / 2
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
            radius: width / 2
            border.width: radiobutton.borderWidth
            border.color: getStartColor((app.focusEnable && control.visualFocus) ? (control.checked ? borderColor : radiobutton.checked_normalIndicatorColor) : borderColor)
            color: "transparent"

        }

        Rectangle {
            id:childrenRec
            // width: control.hovered ? 8 : radiobutton.childrenWidth
            width: 8
            height: width
            // anchors.centerIn: parent
            anchors.horizontalCenter: parent.horizontalCenter  // 水平居中
            anchors.verticalCenter: parent.verticalCenter      // 垂直居中
            radius: width / 2
            color: gradientChildrenRec.visible ? "transparent" : getStartColor(childrenDtColor)
            visible: control.checked

            Rectangle{
                id: gradientChildrenRec
                anchors.fill: parent
                radius: parent.radius
                border.width: parent.border.width
                border.color: parent.border.color
                gradient:  Gradient {
                    GradientStop { position: 0.0; color: getStartColor(childrenDtColor) }

                    GradientStop { position: 1.0; color: getEndColor(childrenDtColor)}
                }
                visible: !parseInterface.isSolidPattern(childrenDtColor)
            }
        }
        NumberAnimation {
            id: scaleAnim
            target: childrenRec
            property: "scale"
            from: 0.0
            to: radiobutton.childrenWidth / 8  // 关键：计算基准大小对应的缩放比例
            duration: 200
            easing.type: Easing.InOutQuint
        }
        NumberAnimation {
            id: uncheckScaleAnim
            target: childrenRec
            property: "scale"
            from: radiobutton.childrenWidth / 8  // 从基准大小开始
            to: 0.0
            duration: 150
            easing.type: Easing.InOutQuint
            onRunningChanged: {
                if (!running && !control.checked) {
                    childrenRec.visible = false;
                    control.isHoveredExpanded = false;  // 重置状态
                }
            }
        }
        ColorAnimation {
            id: transparentColorAnim
            target: control
            property: "tansparentNormalColor"
            from: radiobutton.checked_normalIndicatorColor
            to: radiobutton.normalTransparentIndicatorColor
            duration: 150
            easing.type: Easing.InOutQuint
            running: false
        }
        ColorAnimation {
            id: colorAnim
            target: control
            property: "normalColor"
            from: radiobutton.checked_normalIndicatorColor
            to: radiobutton.normalIndicatorColor
            duration: 150
            easing.type: Easing.InOutQuint
            running: false
        }
        NumberAnimation {
            id: hoverScaleAnim
            target: childrenRec
            property: "scale"
            duration: 100
            easing.type: Easing.InOutQuint
        }
    }
    onHoveredChanged: {
        if (!control.enabled || !control.checked) {
            return;
        }

        const baseScale = radiobutton.childrenWidth / 8;
        const targetScale = 1.0;

        hoverScaleAnim.stop();

        if (hovered) {
            // 鼠标移入：从基准大小放大到8px
            if (childrenRec.scale === baseScale) {  // 仅当当前是基准大小时才放大
                hoverScaleAnim.from = baseScale;
                hoverScaleAnim.to = targetScale;
                hoverScaleAnim.start();
                control.isHoveredExpanded = true;  // 标记为“已放大到8px”
            }
        } else {
            if (control.isHoveredExpanded) {
                hoverScaleAnim.from = targetScale;
                hoverScaleAnim.to = baseScale;
                hoverScaleAnim.start();
                control.isHoveredExpanded = false;
            }
        }
    }

    function handleChildrenScale() {
        if (!control.enabled || !control.checked) {
            return;
        }

        const baseScale = radiobutton.childrenWidth / 8;
        const expandedScale = 1.0;

        hoverScaleAnim.stop();

        // 新增：按下状态处理
        if (control.pressed) {
            // 按下时立即缩小到基准大小
            if (childrenRec.scale !== baseScale) {
                hoverScaleAnim.from = childrenRec.scale;
                hoverScaleAnim.to = baseScale;
                hoverScaleAnim.start();
                control.isHoveredExpanded = false;
            }
        } else if (control.hovered) {
            // 松开后恢复为8px
            if (childrenRec.scale === baseScale) {
                hoverScaleAnim.from = baseScale;
                hoverScaleAnim.to = expandedScale;
                hoverScaleAnim.start();
                control.isHoveredExpanded = true;
            }
        } else {
            // 非hover状态处理
            if (control.isHoveredExpanded) {
                hoverScaleAnim.from = expandedScale;
                hoverScaleAnim.to = baseScale;
                hoverScaleAnim.start();
                control.isHoveredExpanded = false;
            }
        }
    }
    // 新增：处理按下和松开事件
    onPressedChanged: {
        // 按下时强制更新小圆圈大小
        if (control.pressed && control.hovered && control.checked && control.enabled) {
            handleChildrenScale();
        }
        // 松开时恢复
        else if (!control.pressed && control.hovered && control.checked && control.enabled) {
            handleChildrenScale();
        }
    }

    onCheckedChanged: {
        if (checked) {
            childrenRec.scale = 0.0;
            childrenRec.visible = true;
            scaleAnim.start();
            control.isHoveredExpanded = false;
        } else {
            uncheckScaleAnim.start();
            if(isTransaprent)
                transparentColorAnim.start()
            else
                colorAnim.start();
            control.isHoveredExpanded = false;
        }
    }

    background: Item {
        anchors.fill: parent
        // visible: control.down || control.highlighted
        // color: control.down ? control.palette.midlight : control.palette.light
    }
}
