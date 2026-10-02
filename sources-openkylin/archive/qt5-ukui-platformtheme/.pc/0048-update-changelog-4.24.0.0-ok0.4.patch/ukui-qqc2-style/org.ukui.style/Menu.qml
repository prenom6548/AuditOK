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

import QtQuick 2.7
import QtQuick.Window 2.15
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.5
import QtQuick.Templates 2.5 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate


T.Menu {
    id: control
    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem ? contentItem.implicitWidth + leftPadding + rightPadding : 0)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentItem ? contentItem.implicitHeight : 0) + topPadding + bottomPadding

    margins: 0
    palette: app.palette


    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var _opacity:{
        if(control.hasOwnProperty("backOpacity"))
            return control.backOpacity
        return app.menuTransparency
    }

    property var scrollbarWidth: 0
    //    leftPadding: menu.leftRightPadding
    //    rightPadding: menu.leftRightPadding
    //    topPadding: menu.topBottomPadding
    //    bottomPadding: menu.topBottomPadding
    property var maxW: 0
    onVisibleChanged: {
            adjustGeometry()
    }

    onYChanged: {
        Qt.callLater(adjustGeometry)
    }

    onXChanged: {
        Qt.callLater(adjustGeometry)
    }
    onWidthChanged: {
        if(visible && control.width > 0 && control.width !== popupwindow.width)
            popupwindow.width = control.width
    }
    onHeightChanged: {
        if(visible && control.height > 0 && control.height !== popupwindow.height){
            popupwindow.height = control.height
        }
    }

    Connections{
        target: popupwindow
        function onWidthChanged(){
            if(parent && visible){
                if( popupwindow.x !== parent.mapToGlobal(x, y).x - (control.mirrored ? popupwindow.width : 0))
                    popupwindow.x = parent.mapToGlobal(x, y).x - (control.mirrored ? popupwindow.width : 0)
            }

        }
    }
    Connections{
        target: parent
        function onXChanged(){
            adjustGeometry()
        }
        function onYChanged(){
            adjustGeometry()
        }
    }

    // 宿主窗口移动时 mapToGlobal 会变，但 Menu 本地 x/y 可能不变
    Connections {
        target: Window.window
        function onXChanged() {
            if (control.visible || popupwindow.visible)
                Qt.callLater(adjustGeometry)
        }
        function onYChanged() {
            if (control.visible || popupwindow.visible)
                Qt.callLater(adjustGeometry)
        }
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }

    function adjustGeometry(){
        var _x = popupwindow.x;
        var _y = popupwindow.y
        var w = popupwindow.width;
        var h = popupwindow.height;

        if(control.width > 0 && control.width !== w)
            w = control.width
        else if(control1.implicitWidth > 50 && control1.implicitWidth !== w)
            w = Math.ceil(control1.implicitWidth)
        else if(control1.implicitWidth > 0 && w < 50 && control1.implicitWidth !== w)
            w = Math.ceil(control1.implicitWidth)

        if(control.height > 0 && control.height !== h){
            h = control.height
        }
        else if(control1.implicitHeight > 50 && control1.implicitHeight !== h){
            h = Math.ceil(control1.implicitHeight)
        }
        else if(control1.implicitHeight > 0 && h < 50 && control1.implicitHeight !== h){
            h = Math.ceil(control1.implicitHeight)
        }

        // 与内层 Window 同步：仅用 control.visible 会在可见性短暂不一致时漏掉位置更新
        if ((control.visible || popupwindow.visible) && parent) {
            var globalPos = parent.mapToGlobal(control.x, control.y)

            if( _x !== globalPos.x - (control.mirrored ? w : 0))
                _x = globalPos.x - (control.mirrored ? w : 0)
            if( _y !== globalPos.y)
                _y = globalPos.y
        }

        var nx = Math.ceil(_x)
        var ny = Math.ceil(_y)
        if (nx !== popupwindow.x || ny !== popupwindow.y || w !== popupwindow.width || h !== popupwindow.height) {
            popupwindow.setGeometry(nx, ny, w, h)
            // 窗口已显示后部分平台/实现对仅 setGeometry 的位移不生效，显式写入属性确保同步
            if (popupwindow.width !== w)
                popupwindow.width = w
            if (popupwindow.height !== h)
                popupwindow.height = h
            if (popupwindow.x !== nx)
                popupwindow.x = nx
            if (popupwindow.y !== ny)
                popupwindow.y = ny
            popupwindow.requestUpdate()
        }
        if( popupwindow.visible !== control.visible){
            popupwindow.visible = control.visible
        }

        if(control.visible)
            outScreen()
    }

    background: Item{
        width: popupwindow.width
        height: popupwindow.height
        StylePrivate.UKUIMenu{
            id: menu
        }


        ParseInterface{
            id: parseInterface
        }

        StylePrivate.APPParameter{
            id: app
        }

        Window {
            id: popupwindow

            width: Math.ceil(control1.implicitWidth)
            height: Math.ceil(control1.implicitHeight)
            //        blurControl: control
//            visible: control.visible
            color: "transparent"
            onWidthChanged: {
                var size = popHandle.getScreenSize();
                var p = popHandle.getScreenPoint();
                if(popupwindow.x + width >= size.width + p.x){
                    var _x = size.width + p.x - popupwindow.width
                    if(_x > 0){
                        popupwindow.x = Math.ceil(_x)
                    }
                }
                if(control.width > 0 && control.width !== popupwindow.width){
                    width = Math.ceil(control.width)
                    control1.implicitWidth = control.width
                }
            }
            onHeightChanged: {
                outScreen();

                var size = popHandle.getScreenSize();
                var p = popHandle.getScreenPoint();
                if(popupwindow.y + height >= size.height + p.y){
                    var h = size.height + p.y - popupwindow.y
                    if(h > 0 && height != h && control.height <= 0){
                        height = Math.ceil(h)
                    }
                }
                if(control.height > 0 && control.height !== popupwindow.height){
                    height = Math.ceil(control.height)
                    control1.implicitHeight = Math.ceil(control.height)
                }

            }
            StylePrivate.PopupHandle{
                id: popHandle
//                blur: true
                titlebarVisible: false
                anchors.fill: parent
                visible: parent.visible
                radius: menu.radius
            }
            onVisibleChanged: {
                if(visible){
                    if(control.width > 0 && control.width !== popupwindow.width)
                        width = Math.ceil(control.width)
                    if(control.height > 0 && control.height !== popupwindow.height)
                        height = Math.ceil(control.height)

                    if(width <= 0)
                        width = 1
                    if(height <= 0)
                        height = 1
                    listview.currentIndex = -1
                    listview.focus = true
                }
                else {
                    listview.contentY = 0
                }
                control1.visible = visible
                control.visible = visible
            }
            Component.onCompleted: {
                for(var i = 0; i < control.contentModel.count; i++)
                {
                    control1.addItem(control.itemAt(i));
                }
            }


            Connections{
                target: Qt.application
                function onStateChanged(){
                    if(!Qt.application.active)
                        control.visible = false
                }
            }

            T.Menu {
                id: control1

                visible: parent.visible

                palette: app.palette
                font: app.font

                margins: 0

                leftPadding: menu.leftRightPadding
                rightPadding: menu.leftRightPadding
                topPadding: menu.topBottomPadding
                bottomPadding: menu.topBottomPadding
                contentData: control.contentData

                onImplicitWidthChanged: {
                    if(control.width > 0 && control.width !== popupwindow.width){
                        popupwindow.width = Math.ceil(control.width)
                    }
                    else
                        popupwindow.width = Math.ceil(implicitWidth)
                }
                onImplicitHeightChanged: {
                    if(control.height > 0 && control.height !== listview.contentHeight + topPadding + bottomPadding)
                        popupwindow.height = Math.ceil(control.height)
                    else
                        popupwindow.height = Math.ceil(listview.contentHeight + topPadding + bottomPadding);
                    control1.implicitHeight = Math.ceil(popupwindow.height);

                    if(background.height != implicitHeight){
                        background.height = implicitHeight
                    }
                }
                Component.onCompleted: {
                    implicitHeight = listview.contentHeight + topPadding + bottomPadding;
                }
                onVisibleChanged: {

                    menuItemChange()

                    popupwindow.visible = control1.visible
                    if(visible)
                        listview.focus = true
                }
                Connections {
                    target: control
                    function onCountChanged(count) {
                        if(control1 != null){
                            for(var j = 0; j < control.contentModel.count; j++)
                            {
                                control1.addItem(control.itemAt(j));
                            }
                            listview.model = control1.contentModel
                            menuItemChange()
                        }
                    }

                }

                delegate: MenuItem {
                    id: menuitem
                    property bool transparent: control.isTransaprent
                    Component.onCompleted: {
                        if(control.hasOwnProperty("type") && menuitem.hasOwnProperty("type")){
                            menuitem.type = control.type
                        }
                    }

                }
                onCurrentIndexChanged: {
                    listview.currentIndex = currentIndex || 0
                }
                contentItem: ListView {
                    id: listview
                    Accessible.name: qsTr("Menu items")
                    implicitHeight: contentHeight
                    model: control1.contentModel
                    visible: parent.visible
                    height: control1.implicitHeight - control1.topPadding - control1.bottomPadding
                    interactive: scrollbar.visible

                    onContentYChanged: {
                        if(contentY < 0)
                            contentY = 0
                    }


                    onImplicitHeightChanged: {
                        control1.implicitHeight = listview.contentHeight + control1.topPadding + control1.bottomPadding
                        if(control.height > 0 && control.height < listview.contentHeight && control1.implicitHeight !== control.height){
                            control1.implicitHeight = control.height
                        }
                    }
                    onVisibleChanged: {
                        if(visible){
                            listview.focus = true

                            control1.implicitHeight = listview.contentHeight + control1.topPadding + control1.bottomPadding
                            if(control.height > 0 && control.height < listview.contentHeight && control1.implicitHeight !== control.height){
                                control1.implicitHeight = control.height
                            }
                        }
                    }
                    onContentHeightChanged: {
                        implicitHeight = listview.contentHeight;
                        control1.implicitHeight = listview.contentHeight + topPadding + bottomPadding;

                        if(control.height > 0 && control.height < listview.contentHeight){
                            listview.clip = true
                            scrollbar.visible = true
                            vscrollbar.visible = true
                            control1.implicitHeight = control.height
                            implicitHeight = control.height -  topPadding - bottomPadding
                             Qt.callLater(menuItemChange)
                        }
                        else{
                            listview.clip = false
                            scrollbar.visible = false
                            vscrollbar.visible = false

                        }
                    }
                    onCurrentIndexChanged: {
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
                    }
                    clip: outScreen()
                    //Component.onCompleted: currentIndex = control1.currentIndex || 0


                    //todo   why ApplicationWindow.window is null
                    //currentIndex: control1.currentIndex || 0
                    keyNavigationEnabled: true
                    keyNavigationWraps: true
                    ScrollBar.vertical: ScrollBar {//会与menuitem重叠  所以宽度设为0  单独使背景里的scrollbar，在position改变时  listview滚动到下面的时候有点奇怪
                        id: vscrollbar
                        active: visible
                        visible: false
                        width: 0
                    }

                    property bool hasCheckables: false
                    property bool hasIcons: false
                }

                Connections {
                    target: control1.contentItem.contentItem

                    function onChildrenChanged() {
                        menuItemChange()
                    }
                }

                background: Rectangle {
                    id: backRec
                    radius: _opacity >= 1 ? menu.radius : 0
                    implicitHeight: control1.implicitHeight
                    implicitWidth: control1.implicitWidth

                    color: {
                        if(gradientRec.visible)
                            return "transparent"
                        var baccolor = getStartColor(isTransaprent ? menu.normalTransparentBC : menu.normalBC)
                        var alphaValue = popHandle.blur ? _opacity : 1.0
                        return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, alphaValue)
                    }
//                    border.color: getStartColor(isTransaprent ? menu.normalTransparentBorderColor : menu.normalBorderColor)
//                    border.width: menu.border
                    //        layer.enabled: true
                    //                    Component.onCompleted: console.log("menu back opacity...", opacity, menu.bcColorAlpha)
                    ScrollBar {
                        id: scrollbar
                        active: visible
                        orientation: Qt.Vertical
                        visible: false
                        anchors.top: parent.top
                        anchors.topMargin: control1.topPadding
                        anchors.right: parent.right
                        height: listview.height
                        position: listview.visibleArea.yPosition
                        size: listview.visibleArea.heightRatio
                        property bool transparent: control.isTransaprent

                        onPositionChanged: {
                            if (active && listview.visibleArea.yPosition != position) {
                                vscrollbar.position = position;
                            }
                        }
                        onVisibleChanged:{
                              scrollbarWidth = scrollbar.width
                            if(control.width > 0 && control.width !== maxW  + scrollbarWidth + control1.leftPadding + control1.rightPadding){
                                control1.implicitWidth = control.width
                            }
                            else
                                control1.implicitWidth = maxW  + scrollbarWidth + control1.leftPadding + control1.rightPadding
                        }
                    }
                    Rectangle{
                        id: gradientRec
                        anchors.fill: parent
                        radius: parent.radius
                        // border.width: parent.border.width
                        // border.color: parent.border.color
                        gradient:  Gradient {
                            id:gradient
                            GradientStop { position: 0.0; color: {
                                    var baccolor = getStartColor(isTransaprent ? menu.normalTransparentBC : menu.normalBC)
                                    return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, _opacity)
                                }
                            }

                            GradientStop { position: 1.0; color: {
                                    var baccolor = getEndColor(isTransaprent ? menu.normalTransparentBC : menu.normalBC)
                                    return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, _opacity)
                                }
                            }
                        }
                        visible: !parseInterface.isSolidPattern(isTransaprent ? menu.normalTransparentBC : menu.normalBC)
                    }


                }
            }
        }

    }
    delegate: MenuItem {

    }


    Shortcut{
        sequence: "Escape"
        onActivated: control.visible = false
    }
    function menuItemChange()
    {
        var checkable = false;
        var hasChecked = false;
        var checkedHasImage = false
        var needIcon = false;
        var children = control1.contentItem.contentItem.children
        for (var i in children) {
            var child = control1.contentItem.contentItem.children[i];

            if(child.checkable)
                hasChecked = true
            if (child.checkable && child.checked) {
                control1.contentItem.hasCheckables = true;
                checkable = true;
                if(child.icon.source.toString().trim() !== "" || child.icon.name.toString().trim() !== "")
                    checkedHasImage = true
            }
            if ((child.icon && child.icon.name &&child.icon.name.toString().trim() !== "") ||
                    (child.icon && child.icon.source &&child.icon.source.toString().trim() !== "")) {
                control1.contentItem.hasIcons = true;
                needIcon = true;
            }
        }
        if(hasChecked){
            for (var s in children) {
                if(!(children instanceof MenuSeparator))
                {
                    var childs = control1.contentItem.contentItem.children[s];

                    if(childs.hasOwnProperty("hasChecked"))
                        childs.hasChecked = true
                }
            }
        }

        if(checkable){
            for (var k in children) {
                if(!(children instanceof MenuSeparator))
                {
                    var childk = control1.contentItem.contentItem.children[k];

                    var hasImageStr = false
                    if(childk && childk.icon && childk.icon.source && childk.icon.source.toString() !== "")
                        hasImageStr = true
                    if(!hasImageStr && childk && childk.icon && childk.icon.name && childk.icon.name.toString() !== "")
                        hasImageStr = true
                    var checked = childk.checkable && childk.checked
                    if(childk.hasOwnProperty("itemLeftSpace")){
                        if(checked)
                            childk.itemLeftSpace = 24 + childk.leftPadding;
                        else{
                            if(checkedHasImage && !hasImageStr)
                                childk.itemLeftSpace = 48 + childk.leftPadding;
                            else if(checkedHasImage && hasImageStr)
                                childk.itemLeftSpace = 24 + childk.leftPadding;
                            else if(!checkedHasImage && !hasImageStr)
                                childk.itemLeftSpace = 24 + childk.leftPadding;
                            else if(!checkedHasImage && hasImageStr)
                                childk.itemLeftSpace = childk.leftPadding;
                        }
                    }
                }
            }
        }
        else{
            if(needIcon){
                for (var j in control1.contentItem.contentItem.children) {
                    var childj = control1.contentItem.contentItem.children[j];
                    var b = ((childj.icon && childj.icon.name && childj.icon.name.toString().trim() !== "") ||
                            (childj.icon && childj.icon.source && childj.icon.source.toString().trim() !== ""));

                    if(childj.hasOwnProperty("itemLeftSpace"))
                    childj.itemLeftSpace = childj.leftPadding + (b ? 0 : 24);

                }
            }
        else{
                for (var m in control1.contentItem.contentItem.children) {
                    var childm = control1.contentItem.contentItem.children[m];

                    if(childm.hasOwnProperty("itemLeftSpace"))
                    childm.itemLeftSpace = 8//childm.leftPadding;
                }
            }
        }
        var maxWidth = 0;
        for (var l in control1.contentItem.contentItem.children) {
            if(!(control1.contentItem.contentItem.children[l] instanceof MenuSeparator)){
                // if(control1.contentItem.contentItem.children[l].text)
                maxWidth = Math.max(maxWidth, control1.contentItem.contentItem.children[l].implicitWidth)
            }
        }
        for (var nn in control1.contentItem.contentItem.children) {
            if(!(control1.contentItem.contentItem.children[nn] instanceof MenuSeparator)){
                control1.contentItem.contentItem.children[nn].implicitWidth = maxWidth;
            }
        }

        if(maxW === 0)
            maxW = maxWidth

        listview.implicitWidth = maxWidth;
        if(control.width > 0 && control.width !== listview.implicitWidth  + scrollbarWidth + control1.leftPadding + control1.rightPadding){
            control1.implicitWidth = control.width

        }
        else
            control1.implicitWidth = listview.implicitWidth  + scrollbarWidth + control1.leftPadding + control1.rightPadding
        //                        console.log("children control1..", listview.implicitWidth, control1.implicitWidth , popupwindow.width)
        //            control1.implicitWidth = Math.max(background ? background.implicitWidth : 0,
        //                                            contentItem ? maxW + control1.leftPadding + control1.rightPadding : 0)


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
        if(popupwindow.x + popupwindow.width > size.width + p.x){
            var x = size.width + p.x - popupwindow.width;
            if(x > 0)
                popupwindow.x = x;
        }
        return false

    }
}
