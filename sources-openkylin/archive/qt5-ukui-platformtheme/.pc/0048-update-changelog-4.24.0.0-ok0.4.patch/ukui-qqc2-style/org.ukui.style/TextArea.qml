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
