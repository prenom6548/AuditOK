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

import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

Item {
    id : root
    property bool closeButtonVisble : false
    property var screenIndex : null
    state : "default"
    property string wid : ""
    property var x_wid
    //当前预览窗口在桌面上的未缩放前的实际宽度
    property int preViewNoScaledWidth : -1
    //当前预览窗口在桌面上的未缩放前的实际高度
    property int preViewNoScaledHeight : -1
    //当前预览窗口在桌面上的未缩放前的X坐标
    property int preViewNoScaledXCoordinate : -1
    //当前预览窗口在桌面上的未缩放前的Y坐标
    property int preViewNoScaledYCoordinate : -1
    property string captionText : ""
    property real thumbAnchorRootMapGloableX : 0
    property real thumbAnchorRootMapGloableY : 0
    property int thumbnailRadius : UkuiPlatform.GlobalTheme.kRadiusWindow * tabletScaleFactor
    property var icon
    //drag event
    property bool dragActive: previewWindowMouseArea.drag.active
    property real dargBeginTime : Date.now()

    states :  [
        State {
            name : "default"
            PropertyChanges { target : thumbnailWindow; border.width : 0 }
            PropertyChanges { target : previewWindowCloseButton; visible : false }
        },
        State {
            name : "hover"
            PropertyChanges { target : thumbnailWindow; border.width : borderWidth }
            PropertyChanges { target : previewWindowCloseButton; visible : true }
        }
    ]

    //控制白色边框是否显示
    MouseArea {
        id : previewWindowMouseArea
        anchors.fill : parent
        hoverEnabled : true
        drag.target: root
        drag.axis: Drag.YAxis
        drag.maximumY: root.height / 10

        Accessible.role: Accessible.Button
        Accessible.name: "active-window-" + root.caption
        Accessible.description: "preview-window, click to active the app window"

        onEntered : {
            if (!thumbnailWindow.thumbClicked) {
                root.state = "hover"
            }
        }
        onExited: {
            root.state = "default"
        }
        onClicked: {
            appWindowClicked()
        }

        ColumnLayout {
            id : columnLayout
            spacing : iconToThumbSpacing
            anchors.centerIn : parent
            Layout.fillHeight : true
            width : root.width

            RowLayout {
                id : captionRow
                spacing : Math.floor(12 * tabletScaleFactor)
                Layout.fillHeight : true
                layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft

                UkuiItem.Icon {
                    id: iconItem
                    width: iconAndCloseButtonHeight
                    height: width
                    source: icon
                }

                UkuiItem.DtThemeText {
                    id : caption
                    Layout.fillWidth : true
                    text : captionText
                    fontSizeMode : Text.HorizontalFit
                    horizontalAlignment: isLayoutLeftToRight ? Qt.AlignLeft : Qt.AlignRight
                    elide : isLayoutLeftToRight ? Text.ElideRight : Text.ElideLeft
                    textColor : UkuiPlatform.GlobalTheme.kFontWhite
                }

                TabletViewCloseButton {
                    id : previewWindowCloseButton
                    Accessible.name: "close-window-" + captionText
                    Accessible.description: "close-button, click to close the app window"

                    onExited: {
                        if (!previewWindowMouseArea.containsMouse) {
                            root.state = "default"
                        }
                    }
                    onClicked: {
                        removePreviewWindow(wid)
                    }
                }
            }//RowLayout captionRow

            UkuiItem.DtThemeBackground {
                id : thumbnailWindow
                height : root.height - iconToThumbSpacing - iconAndCloseButtonHeight
                width : root.width
                color : "transparent"
                radius : root.thumbnailRadius
                borderColor : UkuiPlatform.GlobalTheme.kWhite
                border.width : borderWidth
                property bool thumbClicked: false // 记录是否是点击缩略图

                states: [
                    State {
                        name : "mousestate"
                        when : thumbnailWindow.thumbClicked
                        ParentChange {
                            target : thumbnailBackground
                            parent : tabletViewMain
                        }
                    }
                ]

                UkuiItem.DtThemeBackground {
                    id : thumbnailBackground
                    /*
                    * 此处必须使用 x, y, width, height 进行位置设置，且禁止使用 anchors.centerIn 和 anchors.fill 等
                    * 因为 thumbnailBackground 在动画过程中会改变 parent，所以如果使用锚点会导致 thumbnailBackground
                    * 突然参考新的 parent 改变大小。设置 width 和 height 时，禁止使用 parent.width 和 parent.height
                    * 也是如此。
                    */
                    x : borderWidth
                    y : borderWidth
                    width : thumbnailWindow.width - 2 * borderWidth
                    height : thumbnailWindow.height - 2 * borderWidth
                    // anchors.centerIn : parent
                    useStyleTransparency: true
                    backgroundColor : UkuiPlatform.GlobalTheme.kContainGeneralNormal
                    radius : root.thumbnailRadius !== 0 ? root.thumbnailRadius - borderWidth : 0
                    Loader {
                        id: thumbnail
                        width : Math.floor(thumbWidth())
                        height : Math.floor(thumbHeight())
                        anchors.centerIn : parent
                        property string thumbnailId : wid
                        source: isWlcom ? "ThumbnailWithWlcom.qml" : "ThumbnailWithKwin.qml"
                        clip : true
                        layer.enabled : root.thumbnailRadius !== 0
                        layer.effect : OpacityMask {
                            maskSource : Rectangle {
                                width : thumbnail.width
                                height : thumbnail.height
                                radius : root.thumbnailRadius
                            }
                        }

                        function thumbWidth(){
                            var factor = model.height / thumbnailBackground.height;
                            var width = model.width / factor;
                            return Math.min(thumbnailBackground.width, width);
                        }

                        function thumbHeight(){
                            var factor = thumbnailBackground.width * model.height
                            var height = factor / model.width;
                            return Math.round(Math.min(thumbnailBackground.height, height))
                        }
                    }

                }
            }
        }
    }

    onDragActiveChanged: {
        if (dragActive) {
            enlargeWindowAnimation.start()
            appWindowReturnBackAnimation.stop()
            dargBeginTime = Date.now()
        } else {
            var subTime = Date.now() - dargBeginTime
            var subDistance = -root.y
            var velocity = Math.abs(subDistance) / subTime;
            // console.log("移动距离: " + subDistance , "窗口的三分之二距离: ", root.height * 2 / 3)
            // console.log("速度为: " + velocity)
            // 平均速度或者位移距离
            if ((velocity > 0.5 && subDistance > 0 ) || subDistance >= root.height * 2 / 3) {
                slideUpToCloseWindowAnimation.start()
            } else {
                appWindowReturnBackAnimation.start()
            }
        }
    }

    Connections {
        target : tabletViewMain
        function onClearAllWindowButtonClickSignal() {
            clearAllWindowAnimation.start()
        }
        function onBlankAreaClickSignal() {
            spaceClickedPreWindowAnimation.start()
        }
    }

    /***************************************动画***************************/
    // 全部清除窗口的动效
    ParallelAnimation {
        id : clearAllWindowAnimation
        NumberAnimation {
            target: root
            properties: "y"
            to: Math.floor(root.y - root.height * 2)
            duration: 300
            easing.type: Easing.InOutQuart
        }

        NumberAnimation {
            target: root
            properties: "opacity"
            to: 0
            duration: 300
            easing.type: Easing.InOutQuart
        }

        onFinished : {
            removePreviewWindow(wid)
        }
    }

    // 点击空白处关闭多任务视图动效（小窗口逐渐在原地消失）
    NumberAnimation {
        id : spaceClickedPreWindowAnimation
        target : root
        property : "opacity"
        duration : 300
        easing.type : Easing.InOutQuart
        from : 1
        to : 0
        onFinished: {
            closeMultitaskView()
        }
    }

    // 上滑关闭窗口动效
    SmoothedAnimation {
        id : slideUpToCloseWindowAnimation
        target : root
        property : "y"
        //因为需要上滑完全离开屏幕 尤其第二行预览窗口，故需要-（预览窗口距离屏幕顶端的距离+预览窗口本身高度）
        to : Math.floor(0 - root.parent.y - root.height)
        velocity : 1100
        reversingMode:  SmoothedAnimation.Eased
        onFinished: {
            removePreviewWindow(wid)
        }
    }
    // 上滑窗口 放大窗口
    NumberAnimation {
        id : enlargeWindowAnimation
        target: root
        property : "scale"
        duration : 200
        to : 1.05
    }

    // 取消上滑后，恢复到原始位置的动画
    ParallelAnimation {
        id : appWindowReturnBackAnimation
        SpringAnimation {
            target : root
            property : "y"
            to : 0
            easing.type : Easing.InOutQuart
            damping : 0.15                      // 弹簧阻尼值。默认值为0，取值0~1.0。控制
            spring : 2                          // 弹性。有效值范围是0~5.0，默认值为0(禁用了spring的动作)。
        }

        NumberAnimation {
            id : restoreWindowSizeAnimation
            target: root
            property : "scale"
            duration : 200
            to : 1
        }
    }

    // 点击应用程序预览窗口激活应用程序时的动画特效，只针对平板模式
    ParallelAnimation {
        id : activateWindowAnimationForTabletMode
        NumberAnimation {
            target : thumbnailBackground
            property : "x"
            from : thumbAnchorRootMapGloableX
            to : preViewNoScaledXCoordinate
            duration : 1000
            easing.type : Easing.InOutQuart
        }
        NumberAnimation {
            target : thumbnailBackground
            property : "y"
            from : thumbAnchorRootMapGloableY
            to : preViewNoScaledYCoordinate
            duration : 1000
            easing.type : Easing.InOutQuart
        }
        NumberAnimation {
            target : thumbnailBackground
            property : "width"
            to : preViewNoScaledWidth
            duration : 1000
            easing.type : Easing.InOutQuart
        }
        NumberAnimation {
            target : thumbnailBackground
            property : "height"
            to :  preViewNoScaledHeight
            duration : 1000
            easing.type : Easing.InOutQuart
        }
        onFinished: {
            closeMultitaskView()
        }
    }

    /***************************************函数*************************/
    function closeMultitaskView() {
        multitaskViewManager.close()
    }

    function removePreviewWindow(wid)
    {
        console.log("removePreviewWindow", wid)
        if (appWindowListModel.hasModalChildWindow(wid)) {
            appWindowListModel.activateAppWindow(wid)
            if (multitaskViewModel.isTabletMode()) {
                activateWindowAnimationForTabletMode.start()
            }
            // If window has modal child window, calls close force.
            multitaskViewManager.close();
        } else {
            // delete app
            appWindowListModel.removeAppWindow(wid);
        }
    }

    function appWindowClicked()
    {
        if (multitaskViewModel.isTabletMode) {
            thumbAnchorRootMapGloableX = mapToGlobal(thumbnailBackground.x, thumbnailBackground.y).x
            thumbAnchorRootMapGloableY = mapToGlobal(thumbnailBackground.x, thumbnailBackground.y).y
            thumbnailWindow.thumbClicked = true
            captionRow.visible = false
            thumbnailWindow.border.width = 0
            previewWindowCloseButton.visible = false
            appWindowListModel.activateAppWindow(wid)
            activateWindowAnimationForTabletMode.start()
        } else {
            appWindowListModel.activateAppWindow(wid);
            closeMultitaskView()
        }
    }
}
