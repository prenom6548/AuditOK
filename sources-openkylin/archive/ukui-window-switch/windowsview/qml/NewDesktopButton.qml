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
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import "Controller.js" as Controller
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

Item {
    id : root
    width : isVerticalLayout ? desktopRectWidth : desktopRect.width
    height : isVerticalLayout ? desktopRect.height - 8 : desktopRectHeight
    activeFocusOnTab: true
    focus : true
    property string windowType : "creation"
    property var newWorkspaceController : new Controller.CreationController(desktopListModel, root, multitaskViewManager, ["app","desktop"]);
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

    onActiveFocusChanged: {
        if (root.activeFocus) {
            desktopListView.currentIndex = desktopListView.count - 1
            if (isVerticalLayout && desktopListView.contentX + desktopListView.contentXoffset < desktopListView.contentWidth - desktopListView.width) {
                contentMoveAnimation.to = isLayoutLeftToRight ? desktopListView.contentWidth - desktopListView.width : -desktopListView.contentWidth
                contentMoveAnimation.start()
            }
            if (!isVerticalLayout && desktopListView.contentY < desktopListView.contentHeight - desktopListView.height) {
                contentMoveAnimation.to = desktopListView.contentHeight + listviewTopMargin - desktopListView.height
                contentMoveAnimation.start()
            }
            multitaskViewManager.activeAreaChangeDataCollect("New Workspace Button Area")
        }
    }

    Keys.onPressed: {
        if (rootWindow.isWlcom) {
            multitaskViewManager.keyNavigationDataCollect(event.key)
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space)
        {
            newWorkspaceController.onWindowClicked()
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            newWorkspaceController.closeMultitaskView()
        } else if (event.modifiers & Qt.MetaModifier) {
            if (event.key === Qt.Key_Tab) {
                newWorkspaceController.closeMultitaskView()
            }
        } else if (event.key === Qt.Key_Tab) {
            if (multitaskViewModel.logicalScreenCount > 1) {
                multitaskViewManager.activateNextTaskView(rootWindow.screenIndex)
                shouldHaveTabFocus = false
            }

            if (appAreaModel.rowCount() === 0) {
                desktopListView.currentIndex = 0
                multitaskViewManager.setShouldShowTabBorder(true)
            }
        }
    }

    states: State {
        name: "active"
        when: root.activeFocus && (multitaskViewManager.shouldShowTabBorder)
        PropertyChanges { target: createDesktopButtonBorder; borderColor : tabFocusBorderColor}
    }

    Item {
        anchors.fill: parent
        anchors.centerIn : parent

        MouseArea {
            id : previewMouseArea
            anchors.fill : parent
            preventStealing : true

            Accessible.role: Accessible.Button
            Accessible.name: "new-desktop"
            Accessible.description: "push-button, click to create a new desktop"

            onClicked : {
                newWorkspaceController.onWindowClicked()
            }
        }

        ColumnLayout {
            id : mainVerticalLayout
            anchors.fill: parent
            anchors.centerIn : parent
            spacing : spacingWidth

            UkuiItem.DtThemeText {
                id : caption
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: isLayoutLeftToRight ? Qt.AlignLeft : Qt.AlignRight
                elide : isLayoutLeftToRight ? Text.ElideRight : Text.ElideLeft
                Layout.fillWidth : true
                Layout.fillHeight : true
                Layout.preferredHeight: desktopCaptionHeight
                Layout.maximumHeight: root.height - mainVerticalLayout.spacing - createDesktopButtonBorder.height
                Layout.leftMargin : workspaceBorderWidth
                text : qsTr("New Desktop")

                UkuiItem.Tooltip {
                    anchors.fill: parent
                    mainText: caption.text
                    posFollowCursor: true
                    active: caption.truncated
                }

                MouseArea {
                    id: captionArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }

            UkuiItem.DtThemeBackground {
                id : createDesktopButtonBorder
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: desktopThumbnailHeight + 2 * border.width
                color : "transparent"
                border.width : workspaceBorderWidth
                borderColor : defaultBorderColor
                radius : UkuiPlatform.GlobalTheme.kRadiusWindow * scaleFactor
                
                UkuiItem.DtThemeBackground {
                    id : createDesktopButton
                    width : desktopThumbnailWidth
                    height : desktopThumbnailHeight
                    anchors.centerIn : parent
                    radius : UkuiPlatform.GlobalTheme.kRadiusMenu * scaleFactor
                    backgroundColor : newDesktopDropArea.containsDrag ? UkuiPlatform.GlobalTheme.kGrayAlpha4 : UkuiPlatform.GlobalTheme.kComponentAlphaNormal

                    UkuiItem.Icon {
                        anchors.centerIn: parent
                        width: Math.floor(32 * scaleFactor)
                        height: width
                        source: "list-add-symbolic"
                        mode: UkuiItem.Icon.AutoHighlight
                    }

                    DropArea {
                        id : newDesktopDropArea
                        property bool hasDrop : false
                        property var dropSource
                        anchors.fill : parent
                        onDropped : {
                            hasDrop = true
                            dropSource = drop.source
                            desktopListModel.appendDesktop()
                        }
                        Connections {
                            target: desktopListModel
                            enabled: newDesktopDropArea.hasDrop
                            function onDesktopAppended(desktop) {
                                dropSourceToDesktop(desktop)
                            }
                            function onDesktopNewToAppended(desktop) {
                                dropSourceToDesktop(desktop)
                            }
                            function dropSourceToDesktop(desktop) {
                                newWorkspaceController.onSourceDropped(newDesktopDropArea.dropSource, rootWindow.highlightDesktop, desktop)
                                newDesktopDropArea.hasDrop = false
                                newDesktopDropArea.dropSource = ''
                            }
                        }
                    }
                }
            }
        }//culumnLayout
    }
}
