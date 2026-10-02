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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.ComboBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    padding: combobox.leftRightPadding
    leftPadding: padding + (!control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
    rightPadding: padding + (control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
    hoverEnabled: true

    palette: app.palette
    font: app.font
    property bool firstShow: false;

    ParseInterface{
        id: parseInterface
    }
    property var focusEnable: app.focusEnable

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var scrollbarWidth : 0

    property var backColor: !isTransaprent ? (!control.enabled ? combobox.disableBC : ((control.pressed || pop.visible) ? combobox.clickedBC :
                                            (control.hovered ? combobox.hoveredBC : combobox.normalBC))) :
                                            (!control.enabled ? combobox.disableTransparentBC :
                                            ((control.pressed || pop.visible)? combobox.clickedTransparentBC : (control.hovered ? combobox.hoveredTransparentBC : combobox.normalTransparentBC)))
    property var borderColor: !isTransaprent ? (!control.enabled ? combobox.disableBorderColor :
                                              ((focusEnable && control.focus && controlFocusReason()) ? combobox.focusBorderColor:
                                              (control.pressed ? combobox.clickBorderColor :
                                              (control.hovered ? combobox.hoveredBorderColor : combobox.normalBorderColor)))) :
                                              (!control.enabled ? combobox.disableTransparentBorderColor : ((focusEnable && control.focus && controlFocusReason()) ? combobox.focusBorderColor :
                                              (control.pressed ? combobox.clickedTransparentBorderColor : (control.hovered ? combobox.hoveredTransparentBorderColor : combobox.normalTransparentBorderColor))))


    delegate: MenuItem {
        id: menuitem
        width: control.width - 2 * control.leftPadding - scrollbarWidth
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
        icon.name: (control.iconNameRole && model[control.iconNameRole] !== undefined) ? model[control.iconNameRole] : null
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        autoExclusive: true
        checked: control.currentIndex === index
        property bool transparent: control.isTransaprent
        Component.onCompleted: {

            //            console.log("delegate menuitem......", control.width, control.leftPadding, control.rightPadding, scrollbarWidth)
            if(menuitem.hasOwnProperty("itemHasChecked")){
                itemHasChecked = true
            }
            if(control.currentIndex === index){

                if(menuitem.hasOwnProperty("isKeyBorderSelect"))
                    menuitem.isKeyBorderSelect = true
            }
        }

    }

    contentItem: T.TextField {
        id: textfield
        leftPadding: 0//!control.mirrored ? 12 : control.editable && activeFocus ? 3 : 1
        rightPadding:0// control.mirrored ? 12 : control.editable && activeFocus ? 3 : 1
        anchors.verticalCenter: control.verticalCenter

        text: control.editable ? control.editText : control.displayText

        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator
        selectByMouse: control.selectTextByMouse

        font: control.font
        color: getStartColor(!control.enabled ? combobox.disableTextColor : combobox.normalTextColor)
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        verticalAlignment: Text.AlignVCenter
        anchors.left: control.mirrored ? indicatorImage.right : control.left
        anchors.leftMargin: StylePrivate.UKUITextFiled.leftRightPadding
        anchors.right: control.mirrored ? control.right : indicatorImage.left
        anchors.rightMargin: StylePrivate.UKUITextFiled.leftRightPadding
        onPressed: {
            control.focusEnable = false;
        }

    }

    indicator: Image {
        id: indicatorImage
        x: control.mirrored ? control.padding : control.width - width - control.padding
        y: control.topPadding + (control.availableHeight - height) / 2
        z: 10
        sourceSize.width:  app.iconWidth
        sourceSize.height: app.iconWidth
        source: iconSource()
        cache: false

        MouseArea{
            Accessible.name: qsTr("Open drop-down")
            anchors.fill: parent
            onClicked:
            {
                pop.visible = !pop.visible;
                control.focus = true
                control.focusEnable = false;

            }
        }
    }

    background: Rectangle {
        id: bRec
        implicitWidth: combobox.normalWidth
        implicitHeight: combobox.normalHeight
        radius: combobox.radius

        color: gradientRec.visible ? "transparent" : getStartColor(backColor)

        border.color: getStartColor(borderColor)
        border.width: (focusEnable && control.focus && controlFocusReason()) ? combobox.focusBorderWidth : combobox.borderWidth;

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient:  Gradient {
                id:gradient
                GradientStop { position: 0.0; color: getStartColor(backColor) }

                GradientStop { position: 1.0; color: getEndColor(backColor)}
            }
            visible: !parseInterface.isSolidPattern(backColor)
            border.color: parent.border.color
            border.width: parent.border.width

        }
//        Rectangle{
//        anchors.fill: parent
//        radius: parent.radius
//        color: "transparent"
//        border.color: getStartColor(borderColor)
//        border.width: (focusEnable && control.focus && controlFocusReason()) ? combobox.focusBorderWidth : combobox.borderWidth;
//        }
    }

    onFocusChanged: {
        if(controlFocusReason())
            control.focusEnable = true;
        else
            control.focusEnable = false;

    }
    onPressedChanged: control.focusEnable = false;


    popup: T.Popup {
        id: pop
        y: control.height
        width: control.width
        topMargin: 6
        bottomMargin: 6
        closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

        onVisibleChanged: {
            if(visible && parent)
            {
                var p = mapToGlobal(x, y)
                if(popupwindow.x !== p.x || popupwindow.y !== p.y){
                    popupwindow.hide()
                    popupwindow.x = p.x
                    popupwindow.y = p.y
                }
                popupwindow.show();
                listview.model = control.delegateModel
            }
            else if(!visible ){
                if(popupwindow.visible)
                    popupwindow.hide()
                listview.model = ""
            }

            indicatorImage.source = ""
            indicatorImage.source = iconSource()
            if(popupwindow.visible != visible){
                popupwindow.visible = visible
            }
        }
        onYChanged: {
            if(visible && parent)
            {
                var p = mapToGlobal(x, y)
                if(popupwindow.x !== p.x || popupwindow.y !== p.y){
                    popupwindow.x = p.x
                    popupwindow.y = p.y
                }
            }
        }
        onXChanged: {
            if(visible && parent)
            {
                var p = mapToGlobal(x, y)
                if(popupwindow.x !== p.x || popupwindow.y !== p.y){
                    popupwindow.x = p.x
                    popupwindow.y = p.y
                }
            }
        }

        background: Item{

            Window {
                id: popupwindow
//                flags: Qt.Window | Qt.Popup
                visible: pop.visible
                color: "transparent"
                width: listview.implicitWidth +scrollbarWidth + 2 * combobox.upDownMargin
                height: listview.contentHeight + 2 * combobox.upDownMargin

                onHeightChanged: {
                    outScreen();

                    var size = popHandle.getScreenSize();
                    var p = popHandle.getScreenPoint();

                    if(popupwindow.y + height >= size.height + p.y){
                        var h = size.height + p.y - popupwindow.y
                        if(h > 0){
                            height = h
                            listview.implicitHeight = Math.max(0, h - 2 * combobox.upDownMargin)
                        }
                    }

                }
                onVisibleChanged: {
                    if(visible){
                        firstShow = true
                        listview.focus = true
                    }
                    else
                        firstShow = false
                    if(pop.visible != visible){
                        pop.visible = visible
                    }
                }
                StylePrivate.PopupHandle{
                    id: popHandle
//                    blur: true
                    titlebarVisible: false
                    anchors.fill: parent
                    visible: parent.visible
                    radius: ukuiPopup.radius
                }
                Rectangle{
                    id: popRec
                    width: parent.width
                    height: parent.height
                    visible: true
                    // radius: ukuiPopup.radius
                    color: {
                        var baccolor = getStartColor(ukuiPopup.backColor)
                        var alphaValue = popHandle.blur ? (isTransaprent ? app.menuTransparency : 1.0) : 1.0;
                        return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, alphaValue)
                    }
                    //border.color: StylePrivate.ParseColorInterface.startColor(ukuiPopup.backBorderColor)
                    //opacity: app.menuTransparency
                    // layer.enabled: true

                    ScrollBar {
                        id: scrollbar
                        active: visible
                        orientation: Qt.Vertical
                        visible: false
                        //                        visible: popHandle.outScreen
                        anchors.top: parent.top
                        anchors.topMargin: pop.topPadding
                        anchors.right: parent.right
                        height: listview.height
                        position: listview.visibleArea.yPosition
                        size: listview.visibleArea.heightRatio
                        property bool transparent: control.isTransaprent

                        onPositionChanged: {
                            if (listview.visibleArea.yPosition != position || vscrollbar.position !== position) {
                                vscrollbar.position = position;
                            }
                        }
                        onVisibleChanged:{
                            if(visible)
                                scrollbarWidth = scrollbar.width
                            else
                                scrollbarWidth = 0
//                            listview.implicitWidth = listview.contentHeight  + scrollbarWidth

                        }
                    }
                    // layer.effect: DropShadow {
                    //     transparentBorder: true
                    //     radius: ukuiPopup.radius
                    //     samples: 16
                    //     horizontalOffset: 0
                    //     verticalOffset: 0
                    //     width: popRec.width + 8
                    //     height: popRec.height + 8
                    //     color: StylePrivate.ParseColorInterface.startColor(ukuiPopup.shadowColor)
                    // }
                }


                ListView {
                    id: listview
                    Accessible.name: qsTr("Items")
                    //                    clip: popHandle.outScreen
                    implicitHeight: contentHeight
                    implicitWidth: control.width  - 2 * control.leftPadding - scrollbarWidth
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds
//                    model: control.delegateModel
                    currentIndex: control.highlightedIndex
                    highlightMoveDuration: 0
                    highlightRangeMode: ListView.NoHighlightRange
                    snapMode: ListView.NoSnap
                     //                    interactive: popHandle.outScreen
                    anchors.top: popRec.top
                    anchors.topMargin: combobox.upDownMargin
                    anchors.left: popRec.left
                    anchors.leftMargin: combobox.upDownMargin
                    visible: parent.visible
                    interactive: scrollbar.visible

                    clip: outScreen()
                    onVisibleChanged: {
                        if(visible){
                            listview.implicitHeight = listview.contentHeight
                        }
                    }
                    onModelChanged: {
                        listview.implicitHeight = listview.contentHeight
                        popupwindow.height = listview.contentHeight + 2 * combobox.upDownMargin
                    }

                    onContentHeightChanged: {
                        listview.implicitHeight = listview.contentHeight
                        popupwindow.height = listview.contentHeight + 2 * combobox.upDownMargin
                    }

                    ScrollBar.vertical: ScrollBar {
                        id: vscrollbar
                        active: visible
                        visible: false
                        width: 0
                    }

                    onCurrentIndexChanged: {
                        indexSelect();
                    }

                }
                Shortcut{
                    sequence: "Escape"
                    onActivated: pop.visible = false
                }
            }
        }
    }

    StylePrivate.UKUIPopup{
        id: ukuiPopup
    }

    StylePrivate.UKUIComboBox{
        id: combobox
    }

    StylePrivate.APPParameter{
        id: app
        onParametryChanged:{
            iconUpdate();
        }
        onIconThemeChanged: {
            iconUpdate()
        }
    }
    onEnabledChanged: {
        iconUpdate()
    }
    function iconUpdate(){
        indicatorImage.source = ""
        indicatorImage.source = iconSource()
        control.update();
    }
    function outScreen(){
        var size = popHandle.getScreenSize();
        var p = popHandle.getScreenPoint();
        if(popupwindow.y + popupwindow.height >= size.height + p.y){
            var h = size.height + p.y - popupwindow.y
            if(h > 0){
                listview.clip = true
                scrollbar.visible = true
                vscrollbar.visible = true
                return true
            }
        }
        listview.clip = false
        scrollbar.visible = false
        vscrollbar.visible = false

        return false

    }

    function iconSource(){
        var icon = "ukui-down-symbolic";
        if(!pop.visible){
            if(!app.themeHasIcon(icon))
                icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/ukui-down-symbolic.svg"
        }
        else{
            icon = "ukui-up-symbolic"
            if(!app.themeHasIcon(icon))
                icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/ukui-up-symbolic.svg"
        }

        var model = "normal"
        if(!control.enabled){
            if(app.isDark)
                model = "highDisbale"
            else
                model = "disenable"
        }
        else if(app.isDark){
            model = "highlight"
        }

        return "image://imageProvider/" + icon + "/" + model;

    }

    function indexSelect(){
        var children = listview.contentItem.children
        for (var i = 0; i < control.count; i++) {
            if(i === listview.currentIndex){
                var item = listview.itemAtIndex(i);

                if(item === null || item === undefined){
                    item = children[i]
                }
                if(item !== null && item !== undefined && item.hasOwnProperty("isKeyBorderSelect") && !item.isKeyBorderSelect)
                    item.isKeyBorderSelect = true
            }
            else if(listview.itemAtIndex(i) !== undefined && listview.itemAtIndex(i) !== null){
                if(listview.itemAtIndex(i).hasOwnProperty("isKeyBorderSelect") && listview.itemAtIndex(i).isKeyBorderSelect)
                    listview.itemAtIndex(i).isKeyBorderSelect = false
            }
        }
        if(firstShow && listview.currentIndex >= 0){
            var item = listview.currentItem
            if(item.checkable && item.checked){
                listview.positionViewAtIndex(listview.currentIndex, ListView.Contain)
                firstShow = false
            }
        }

    }
    function controlFocusReason(){
        return app.focusEnable && (control.focusReason === Qt.TabFocusReason || control.focusReason === Qt.BacktabFocusReason)
    }

}
