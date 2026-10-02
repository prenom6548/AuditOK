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
    此文件是虚拟桌面的预览窗口的内容设置
*/

import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Window 2.12
import org.kylin.background 3.0 as KWin
//import org.kde.kwin 3.0 as KWinComponents
import QtGraphicalEffects 1.0
import "Controller.js" as Controller
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

import DesktopListModel 1.0

Item {
    id : root
    width : isVerticalLayout ? desktopRectWidth : desktopRect.width
    height : isVerticalLayout ? desktopRect.height - 8 : desktopRectHeight
    activeFocusOnTab: index === 0 ? true : false
    focus : true
    property string windowType : "desktop"
    property string wid : ""
    property int desktopIndex : index + 1
    // Using the Drag attached property, any Item can be made a source of drag and drop events within a scene
    Drag.active : previewMouseArea.drag.active
    Drag.hotSpot.x : width / 2
    Drag.hotSpot.y : height / 2
    property bool windowDragActive : previewMouseArea.drag.active
    property bool nowDragActive: false
    property var controller : new Controller.DesktopController(desktopListModel, this, multitaskViewManager, ["app","desktop"])
    property real posX : 0
    property real posY : 0
    // 用户自定义工作区名称
    property string customName: ""

    function playEnterAnimation() {
        root.scale = 0
        root.opacity = 0
        enterAnimation.restart()
    }

    NumberAnimation {
        id: enterAnimation
        target: root
        properties: "scale,opacity"
        to: 1
        duration: 300
        easing.type: Easing.InOutQuad
    }

    states: [
        State {
            name : "tabAndDragActive"
            when: root.activeFocus && root.windowDragActive && (multitaskViewManager.shouldShowTabBorder)
            PropertyChanges { target: previewArea; borderColor: tabFocusBorderColor }
            ParentChange { target: root; parent: desktopRect }
        },
        State {
            name : "tabActive"
            when: root.activeFocus && (multitaskViewManager.shouldShowTabBorder)
            PropertyChanges { target: previewArea; borderColor: tabFocusBorderColor }
        },
        State {
            name : "dragActive"
            when : nowDragActive
            ParentChange { target: root; parent: desktopRect }
        }
    ]

    Keys.onPressed: {
        if (rootWindow.isWlcom) {
            multitaskViewManager.keyNavigationDataCollect(event.key)
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            controller.setCurrentDesktop(root.wid)
            controller.onWindowClicked(root.wid)
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            controller.closeMultitaskView()
        } else if (event.modifiers & Qt.MetaModifier) {
            if (event.key === Qt.Key_Tab) {
                controller.closeMultitaskView()
            }
        } else if (event.key === Qt.Key_Tab) {
            if(event.modifiers & Qt.ShiftModifier && appAreaModel.rowCount() === 0) {
                if (multitaskViewModel.logicalScreenCount > 1) {
                    multitaskViewManager.activatePreviousTaskView(rootWindow.screenIndex)
                    shouldHaveTabFocus = false
                }
            }
            if (index === 0) {
                var lastIsNewDesktop = desktopListModel.data(
                        desktopListModel.index(desktopListView.count - 1, 0), DesktopListModel.ItemType) === desktopListView.newDesktop
                desktopListView.currentIndex = lastIsNewDesktop ? (desktopListView.count - 1) : 0
                multitaskViewManager.setShouldShowTabBorder(true)
            }
        }
    }

    onActiveFocusChanged: {
        if (index === 0 && root.activeFocus) {
            desktopListView.currentIndex = 0
            var offsetPosition = isVerticalLayout ? desktopListView.contentX : desktopListView.contentY
            if (offsetPosition > 0) {
                contentMoveAnimation.to = 0
                contentMoveAnimation.start()
            }
            multitaskViewManager.activeAreaChangeDataCollect("Workspace Preview Area")
        }
    }

    // 工作区位置移动触发下方信号
    onWindowDragActiveChanged : {
        // 需要保证让 dragRectangle 先恢复原来的 parent 再移动到合适的位置
        // 因为直接使用 windowDragActive 变量并无法实现上述控制顺序，所以
        // 需要额外的变量 nowDragActive 来实现 ParentChange，然后再移动到
        // 合适的位置
        if (nowDragActive != windowDragActive) {
            nowDragActive = windowDragActive
        }
        if (windowDragActive) {
            opacity = 0.75
        } else {
            // 工作区取消拖动后需要返回原来的位置，如果拖拽产
            // 生了工作区的位置交换也没有关系，也不会影响动效
            opacity = 1
            this.posX = 0
            this.posY = 0
            controller.rejectSource(root)
        }
    }

    Item {
        id : desktopDragRect
        anchors.fill: parent
        anchors.centerIn : parent
        state : "default"
        property var desktopId: root.wid

        states : [
            State {
                name : "default"
                PropertyChanges {
                    target : previewArea
                    borderColor : desktopDragRect.getWorkspaceDefaultBorderColor()
                }
                PropertyChanges { target : closeButton; visible : rootWindow.isTouchState && desktopListView.count > 2 }
                PropertyChanges { target : editButton; visible : false }
            },
            State {
                name : "hover"
                PropertyChanges {
                    target : previewArea
                    borderColor : desktopDragRect.getWorkspaceHoverBorderColor()
                }
                // 工作区名称输入状态不显示按钮
                // 工作区剩一个时不显示删除按钮，加上“新建工作区”共2个item，因此判断count > 2时显示关闭按钮
                PropertyChanges { target : closeButton; visible : !captionEdit.editable && desktopListView.count > 2}
                PropertyChanges { target : editButton; visible : true && !captionEdit.editable}
            }
        ]

        MouseArea {
            id : previewMouseArea
            anchors.fill : parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled : true
            preventStealing : true
            drag.axis : isVerticalLayout ? Drag.XAxis : Drag.YAxis
            drag.target : root

            Accessible.role: Accessible.Button
            Accessible.name: "desktop-" + root.desktopIndex
            Accessible.description: "preview-desktop, click to enter the desktop"

            onEntered : {
                controller.setHoverStatus();
            }
            onExited : {
                controller.setDefaultStatus()
            }
            onClicked : {
                if (mouse.button == Qt.RightButton) {
                    desktopListModel.openDesktopMenu(root.wid, root)
                } else {
                    controller.onWindowClicked(root.wid)
                }
            }

            ColumnLayout {
                id : mainVerticalLayout
                anchors.fill: parent
                anchors.centerIn : parent
                spacing : spacingWidth

                RowLayout {
                    id : captionRow
                    spacing : 0
                    Layout.fillHeight : true
                    Layout.rightMargin: workspaceBorderWidth
                    layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft
                    Layout.preferredHeight: desktopCaptionHeight
                    Layout.maximumHeight: root.height - mainVerticalLayout.spacing - previewArea.height

                    LineEdit {
                        id: captionEdit
                        editable: false
                        enableLayoutMirroring: !isLayoutLeftToRight
                        Layout.fillWidth : true
                        Layout.maximumWidth: (editable || !editButton.visible) ? desktopRectWidth : contentTextWidth + Math.floor(8 * scaleFactor)
                        Layout.leftMargin : workspaceBorderWidth
                        Layout.preferredHeight: Math.min(desktopCaptionHeight, captionRow.height)
                        isDefaultText: customName.length === 0

                        onTextEditingFinished: {
                            if (itemDisplayText != defaultText) {
                                customName = itemDisplayText
                            } else {
                                //清空输入则恢复默认名称
                                customName = captionEdit.isDefaultText ? "" : defaultText
                            }
                            //在输入状态按回车键保存时，要求仅保存名称，不退出任务视图界面
                            //工作区名称编辑结束，需要将焦点还给root，否则会影响工作区键盘导航
                            root.forceActiveFocus()
                        }
                    }
                    Binding {
                        id: defaultTextBinding
                        target: captionEdit
                        property: "defaultText"
                        value: qsTr("Desktop") + " " + root.desktopIndex
                    }
                    Binding {
                        target: captionEdit
                        property: "itemDisplayText"
                        value: customName.length === 0 ? captionEdit.defaultText : customName
                    }

                    Rectangle {
                        id : editButton
                        implicitWidth : Math.floor(16 * scaleFactor)
                        implicitHeight: width
                        color : "transparent"
                        Accessible.role: Accessible.Button
                        Accessible.name: "edit-desktop-name-" + root.wid
                        Accessible.description: "push-button, click to rename the desktop"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                captionEdit.editable = true
                            }
                        }
                        UkuiItem.Icon {
                            id: editIcon
                            anchors.centerIn: parent
                            width: parent.width
                            height: width
                            source: "document-edit-symbolic"
                            mode: UkuiItem.Icon.AutoHighlight
                        }
                    }

                    Item {
                        // 填充剩余空间
                        Layout.fillWidth : true
                        Layout.minimumWidth: Math.floor(8 * scaleFactor)
                        visible: editButton.visible
                    }

                    CloseButton {
                        id : closeButton
                        implicitWidth : Math.min(iconAndCloseButtonHeight, captionRow.height)
                        implicitHeight : width
                        radius : width / 2
                        windowType : "desktop"

                        Accessible.name: "close-desktop-" + root.desktopIndex
                        Accessible.description: "close-button, click to remove the desktop"

                        onClicked : {
                            // 删除工作区时解除默认名称绑定,避免删除时显示“工作区0” fix issue #IBBJ3F
                            // 剩余[1个工作区+"新建工作区"]时不会删除"工作区1"，属性绑定不能解除 fix bug #354325
                            if (desktopListModel.rowCount() > 2)
                                defaultTextBinding.when = false
                            controller.removePreviewWindow(root.wid)
                        }
                        onExited: {
                            if (!previewMouseArea.containsMouse) {
                                root.setDefaultStatus()
                            }
                        }
                    }
                }//RowLayout

                UkuiItem.DtThemeBackground {
                    id : previewArea
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.preferredHeight: desktopThumbnailHeight + 2 * border.width
                    color : "transparent"
                    borderColor : defaultBorderColor
                    border.width : workspaceBorderWidth
                    radius : UkuiPlatform.GlobalTheme.kRadiusWindow * scaleFactor

                    Rectangle {
                        id: workspace
                        anchors.centerIn: parent
                        width : desktopThumbnailWidth
                        height : desktopThumbnailHeight
                        color : "black"
                        radius : UkuiPlatform.GlobalTheme.kRadiusMenu * scaleFactor
                        clip : true
                        visible : true
                        layer.enabled : true  // 始终启用分层以优化复杂内容的渲染
                        // 固定纹理尺寸，避免 item 因动画缩放导致纹理反复重建
                        // 效果：GPU 只需分配一次显存，后续尺寸变化只需在着色器中调整采样坐标，无需重新创建纹理
                        layer.textureSize : Qt.size(desktopThumbnailWidth, desktopThumbnailHeight)
                        layer.effect : OpacityMask {
                            maskSource : Rectangle {
                                width : desktopThumbnailWidth
                                height : desktopThumbnailHeight
                                radius : workspace.radius
                                visible: false
                            }
                        }
                        KWin.DesktopBackground {
                            id : desktopThumbnailBackground
                            anchors.centerIn : parent
                            width : desktopThumbnailWidth
                            height : desktopThumbnailHeight
                            blurAlpha : 0
                        }

                        Loader {
                            id : workSpaceThumbnailLoader
                            width : desktopThumbnailWidth
                            height : desktopThumbnailHeight
                            source: isWlcom ? "WorkSpaceThumbnailWithWlcom.qml" : "WorkSpaceThumbnailWithKwin.qml"
                        }

                        Connections {
                            target: rootWindow.isWlcom ? multitaskViewManager : null
                            function onBackgroundChanged() {
                                desktopThumbnailBackground.update()
                            }
                        }
                    }

                    UkuiItem.DtThemeBackground {
                        id : desktopMaskColor
                        visible : false
                        anchors.fill: workspace
                        backgroundColor : UkuiPlatform.GlobalTheme.kGrayAlpha4
                        radius : workspace.radius
                    }
                }
            }//culumnLayout
        }

        // 应用程序预览窗口拖动到虚拟桌面，虚拟桌面区域收到下方信号
        DropArea {
            id : previewDropArea
            anchors.fill : parent
            visible: !root.windowDragActive
            onEntered : {
                if (drag.source.windowType == "app") {
                    desktopMaskColor.visible = true
                }
                if (drag.source.wid !== root.wid) {
                    controller.onSourceEntered(drag.source, drag.source.wid, root.desktopIndex);
                }
            }
            onExited : {
                desktopMaskColor.visible = false
            }
            onDropped : {
                desktopMaskColor.visible = false
                controller.onSourceDropped(drop.source, rootWindow.highlightDesktop, root.wid);
            }
        }

        function getWorkspaceDefaultBorderColor()
        {
            if (root.activeFocus && (multitaskViewManager.shouldShowTabBorder)) {
                return tabFocusBorderColor
            } else if (highlightDesktop === root.wid) {
                return hoverBorderColor
            } else {
                return defaultBorderColor
            }
        }

        function getWorkspaceHoverBorderColor()
        {
            if (root.activeFocus && (multitaskViewManager.shouldShowTabBorder)) {
                return tabFocusBorderColor
            } else {
                return hoverBorderColor
            }
        }
    }

    NumberAnimation on x{
        id : movePreX
        duration : 0
        running : false
    }

    NumberAnimation on y{
        id : movePreY
        duration : 0
        running : false
    }

    function returnOriginal(x,y)
    {
        movePreX.to = 0
        movePreY.to = 0
        movePreX.from = x
        movePreY.from = y
        movePreX.stop()
        movePreY.stop()
        movePreX.start()
        movePreY.start()
    }

    function setDefaultStatus() {
        desktopDragRect.state = "default"
    }

    function setHoverStatus() {
        desktopDragRect.state = "hover"
    }

    Component.onCompleted: {
        if (shouldHaveTabFocus && root.activeFocusOnTab && appAreaModel.rowCount() === 0) {
            // 无应用窗口时获取tab焦点
            root.forceActiveFocus()
        }
    }

    Connections {
        target: desktopListModel
        enabled: highlightDesktop === root.wid

        // 工作区重命名
        function onRenameDesktopSignal(item) {
            if (item !== root) {
                return
            }
            if (!captionEdit.activeFocus) {
                captionEdit.forceActiveFocus()
            }
            captionEdit.editable = true
        }
    }
}
