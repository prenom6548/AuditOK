/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
    虚拟桌面的UI和功能的处理，也是加载创建预览窗口
    虚拟桌面区域的窗口和窗口区域的窗口都来自一个"类”
*/
import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

FocusScope {
    id : root
    //虚拟桌面的各个参数
    property bool desktopHoverStatus : false
    //虚拟桌面标题栏高度
    property int desktopCaptionHeight : Math.floor(32 * scaleFactor)
    property int spacing : Math.floor(24 * scaleFactor)
    property int desktopRectWidth : desktopRect.height
    property int desktopRectHeight : desktopThumbnailHeight + 2 * workspaceBorderWidth + spacingWidth + desktopCaptionHeight
    property int listviewTopMargin : Math.floor(24 * scaleFactor)

    UkuiItem.DtThemeBackground {
        id : desktopRect
        anchors.fill : parent
        useStyleTransparency : true
        backgroundColor : UkuiPlatform.GlobalTheme.kContainGeneralNormal

        MouseArea {
            id : desktopMouseArea
            Accessible.name: "master desktop area"
            anchors.fill : parent
            hoverEnabled : true
            onEntered: {
                desktopHoverStatus =  true
            }
            onExited: {
                desktopHoverStatus = false
            }

            ListView {
                id : desktopListView
                Accessible.name: "master desktop listview"
                anchors.fill : parent
                anchors.topMargin : root.listviewTopMargin
                property bool initMultitask : true
                orientation : ListView.Vertical
                model : desktopListModel
                spacing : root.spacing
                activeFocusOnTab: false
                clip : true
                onActiveFocusChanged: {
                    if (desktopListView.activeFocus) {
                        currentIndex = 0
                    }
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_PageUp || event.key === Qt.Key_Home) {
                        if (currentIndex === 0) {
                            return
                        }
                        currentIndex = 0
                        if (desktopListView.contentY > 0){
                            contentMoveAnimation.to = 0
                            contentMoveAnimation.start()
                        }
                    } else if (event.key === Qt.Key_PageDown || event.key === Qt.Key_End) {
                        if (currentIndex === desktopListModel.rowCount() - 2) {
                            return
                        }
                        currentIndex = desktopListModel.rowCount() - 2
                        if (desktopListView.contentY < desktopListView.contentHeight - desktopListView.height) {
                            contentMoveAnimation.to = desktopListView.contentHeight - desktopListView.height
                            contentMoveAnimation.start()
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    id : deskVbar
                    hoverEnabled : true
                    active : hovered || pressed

                    //定义样式
                    contentItem : Rectangle {
                        implicitWidth : Math.floor(8 * scaleFactor)
                        radius : Math.floor(4 * scaleFactor)
                        visible : desktopHoverStatus && desktopListView.contentHeight > desktopListView.height
                        color : deskVbar.hovered || deskVbar.pressed ? UkuiPlatform.GlobalTheme.kComponentAlphaHover : UkuiPlatform.GlobalTheme.kComponentAlphaNormal
                    }
                }

                populate: Transition {
                    NumberAnimation {
                        properties : "scale,opacity"
                        from : 0
                        to : 1
                        duration : 400
                        easing.type : Easing.InOutQuad
                    }
                }

                add : Transition {
                    NumberAnimation {
                        properties : "scale,opacity"
                        from : 0
                        to : 1
                        duration : 300
                        easing.type : Easing.InOutQuad
                    }
                }
                remove : Transition {
                    NumberAnimation {
                        properties : "scale,opacity"
                        from : 1
                        to : 0
                        duration : 300
                        easing.type : Easing.InOutQuad
                    }
                }
                removeDisplaced : Transition {
                    NumberAnimation {
                        properties : "x,y"
                        duration : 300
                    }
                }

                move : Transition {
                    NumberAnimation {
                        property : "y"
                        duration : 200
                        easing.type : Easing.InOutQuad
                    }
                }
                moveDisplaced : Transition {
                    NumberAnimation {
                        property : "y"
                        duration : 200
                        easing.type : Easing.InOutQuad
                    }
                }

                function getWorkspaceItemOrNewWorkspaceItem(type) {
                    if(type === "newDesktop") {
                        return "NewDesktopButton.qml"
                    }
                    return "VirtualDesktopWindow.qml"
                }

                NumberAnimation {
                    id : moveToScrollEndAnimation
                    target : desktopListView
                    property : "contentY"
                    from : desktopListView.contentY
                    to : desktopListView.contentHeight + desktopRectHeight + root.spacing - desktopListView.height
                    duration : 300
                    easing.type : Easing.InOutQuart
                }

                function moveToScrollEnd() {
                    if (desktopListView.contentHeight + desktopRectHeight + root.spacing > desktopListView.height
                            && desktopListView.count < 16) {
                        moveToScrollEndAnimation.restart()
                    }
                }

                Connections {
                    target: desktopListModel
                    function onDesktopAppended(desktopIndex) {
                        desktopListView.moveToScrollEnd(desktopIndex)
                    }
                }

                delegate: Loader {
                    source : desktopListView.getWorkspaceItemOrNewWorkspaceItem(model.ItemType)
                    Binding {
                        target: item
                        property: "wid"
                        value: model.Uuid
                        when: "wid" in item
                    }
                }
            } // desktopListView
        }

        NumberAnimation {
            id : contentMoveAnimation
            target : desktopListView
            property : "contentY"
            from : desktopListView.contentY
            to : desktopListView.contentY
            duration : 300
            easing.type : Easing.InOutQuart
        }
    }
}
