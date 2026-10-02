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
    每个预览窗口的UI设计
*/
import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import "Controller.js" as Controller
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

Rectangle {
    id : root
    Accessible.name: "app preview root window"
    color : "transparent"

    /** windowType is the type of PreviewWindow,
    * the options are : none, app, desktop, creation.
    * Default to none.
    */
    property string windowType : "none"

    /** stateList is the list of state,
    * the options are : default, hover, drag, receive.
    * Default to default.
    */
    property var stateList : ["default", "hover", "drag", "receive"];
    state : "default"
    property string wid : ""
    property var x_wid : ""
    property var thumbHeight
    property var thumbWidth
    property real posX : 0
    property real posY : 0
    property double titleOpacity : 1
    property var controller : new Controller.AppController(appWindowListModel, root, multitaskViewManager, ["none"]);
    property var icon
    Drag.active: previewMouseArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData : { "text/plain": "app" } // 必须定义mimeData才能drop成功
    Drag.source : root
    Drag.hotSpot.x : thumbWidth * scaleDownValue / 2
    Drag.hotSpot.y : thumbHeight * scaleDownValue / 2
    property alias borderRectangle : previewArea
    property alias caption : caption.text

    // 定义缩放比例系数变量
    property double scaleValue : 0.0
    property double windowDragX : 0.0
    property double windowDragY : 0.0
    property double scaleDownValue : 0.38

    // 定义应用预览窗各间距、宽高
    property int spacingWidth: 2 * scaleFactor  // focus边框与预览图间距
    property int appThumbnailMargin: appBorderWidth + spacingWidth
    property int minWidth: thumbWidth + 2 * appThumbnailMargin
    width: Math.max(thumbWidth + 2 * appThumbnailMargin, minWidth)
    height: captionRow.height + previewArea.height + mainColumnLayout.spacing + 2 * appThumbnailMargin


    states : [
        State {
            name : "default"
            PropertyChanges {
                target : previewArea
                borderColor : root.parent.state == "tabActive" ? tabFocusBorderColor : defaultBorderColor
            }
            PropertyChanges { target : previewWindowCloseButton; visible : rootWindow.isTouchState ? true : false }
        },
        State {
            name : "hover"
            PropertyChanges {
                target : previewArea
                borderColor : root.parent.state == "tabActive" ? tabFocusBorderColor : hoverBorderColor
            }
            PropertyChanges { target : previewWindowCloseButton; visible : true }
        }
    ]

    MouseArea {
        id : previewMouseArea
        anchors.fill : parent
        hoverEnabled : true
        preventStealing : true
        drag.axis : Drag.XAndYAxis
        drag.target : root

        Accessible.role: Accessible.Button
        Accessible.name: "active-window-" + root.caption
        Accessible.description: "preview-window, click to active the app window"

        onEntered : {
            root.state = "hover"
        }
        onExited : {
            root.state = "default"
        }
        onPressed : {
            windowDragX = mouseX
            windowDragY = mouseY

            previewArea.grabToImage(function (result) {
                if (!result) {
                    console.warn("previewArea grabToImage failed: result is null or undefined.")
                    return;
                }
                root.Drag.imageSource = result.url
            }, Qt.size(root.thumbWidth * root.scaleDownValue, root.thumbHeight * root.scaleDownValue))
        }

        onClicked : {
            controller.onWindowClicked(wid);
        }

        onReleased: {
            root.Drag.drop();
        }

        ColumnLayout {
            id: mainColumnLayout
            anchors.fill: parent
            anchors.centerIn : parent
            spacing : 10 * scaleFactor  // 标题栏与预览图外边框间距

            RowLayout {
                id : captionRow
                spacing : Math.floor(8 * scaleFactor)
                Layout.fillHeight : true
                Layout.preferredHeight : iconAndCloseButtonHeight
                Layout.leftMargin : appBorderWidth + spacingWidth
                Layout.rightMargin: appBorderWidth
                opacity : titleOpacity
                layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft

                UkuiItem.Icon {
                    id: iconItem
                    width: iconAndCloseButtonHeight
                    height: width
                    source: icon
                }

                UkuiItem.DtThemeText {
                    id : caption
                    verticalAlignment: Qt.AlignVCenter
                    horizontalAlignment: isLayoutLeftToRight ? Qt.AlignLeft : Qt.AlignRight
                    elide : isLayoutLeftToRight ? Text.ElideRight : Text.ElideLeft
                    Layout.fillWidth : true
                    text : ""
                    textColor : UkuiPlatform.GlobalTheme.kFontWhite
                }

                CloseButton {
                    id : previewWindowCloseButton
                    width : iconAndCloseButtonHeight
                    height : width
                    radius : width / 2
                    windowType : "app"

                    Accessible.name: "close-window-" + caption.text
                    Accessible.description: "close-button, click to close the app window"

                    onExited: {
                        if (!previewMouseArea.containsMouse) {
                            root.state = "default"
                        }
                    }
                    onClicked: {
                        controller.removePreviewWindow(root.wid)
                    }
                }
            }//RowLayout

            UkuiItem.DtThemeBackground {
                id : previewArea
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight : thumbRectangleHeight + 2 * appThumbnailMargin
                color : "transparent"
                borderColor : tabFocusBorderColor
                border.width : appBorderWidth
                radius : UkuiPlatform.GlobalTheme.kRadiusWindow * scaleFactor

                Rectangle {
                    id : appWindowBack
                    anchors.centerIn: parent
                    height : thumbHeight
                    width : thumbWidth
                    radius : UkuiPlatform.GlobalTheme.kRadiusMenu * scaleFactor
                    color : Qt.rgba(1, 1, 1, 0.5)
                    clip : true
                    layer.enabled : appWindowBack.radius !== 0
                    layer.effect : OpacityMask {
                        maskSource : Rectangle {
                            height : thumbHeight
                            width : thumbWidth
                            radius : appWindowBack.radius
                        }
                    }
                    Loader {
                        id : appWindowLoader
                        property string thumbnailId : wid
                        height : thumbHeight
                        width : thumbWidth
                        anchors.centerIn: parent
                        source: isWlcom ? "ThumbnailWithWlcom.qml" : "ThumbnailWithKwin.qml"
                    }
                }
            }
        }//culumnLayout
    }

    function returnOriginal(x, y) {
        zoomInAnimation()
    }

    function zoomOutAnimation() {
        transformId.origin.x = windowDragX
        transformId.origin.y = windowDragY
        scaleValue = 0.38
        titleOpacity = 0
        scaleDownAnimation.restart()
    }

    function zoomInAnimation() {
        transformId.origin.x = windowDragX
        transformId.origin.y = windowDragY
        scaleValue = 1
        titleOpacity = 1
        scaleUpAnimation.restart()
    }


    transform : Scale{
        id : transformId
    }

    ParallelAnimation{
        id : scaleDownAnimation
        NumberAnimation {
            target : transformId
            property : "xScale"
            to : scaleValue
            duration : 300
            easing.type : Easing.InOutQuart
        }
        NumberAnimation{
            target : transformId
            property : "yScale"
            to :  scaleValue
            duration : 300
            easing.type : Easing.InOutQuart
        }
        NumberAnimation{
            target : root
            property : "opacity"
            to : titleOpacity
            duration : 300
            easing.type : Easing.InOutQuart
        }
    }

    ParallelAnimation{
        id : scaleUpAnimation
        NumberAnimation {
            target : transformId
            property : "xScale"
            to : scaleValue
            duration : 300
            easing.type : Easing.InOutQuart
        }
        NumberAnimation{
            target : transformId
            property : "yScale"
            to :  scaleValue
            duration : 300
            easing.type : Easing.InOutQuart
        }
        NumberAnimation{
            target : root
            property : "opacity"
            to : titleOpacity
            duration : 300
            easing.type : Easing.InOutQuart
        }

        NumberAnimation {
            target: root
            property: "x"
            duration: 300
            to: 0
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: root
            property: "y"
            duration: 300
            to: 0
            easing.type: Easing.InOutQuad
        }
    }
}
