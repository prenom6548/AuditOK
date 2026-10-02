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
import org.kylin.background 3.0 as KWin
import QtGraphicalEffects 1.0
//ukui-quick
import org.ukui.quick.platform 1.0 as UkuiPlatform

import AppWindowFilterModel 1.0

Rectangle {
    id : rootWindow
    color: "black"
    focus: true
    Keys.enabled: true
    property var isWlcom : multitaskViewManager.isWlcom()
    property var isVerticalLayout : multitaskViewManager.isMultitaskViewVerticalLayout()
    property var highlightDesktop : desktopListModel.highlightDesktop
    property rect screenRect : Qt.rect(x, y, width, height)
    property int screenIndex : 0
    property var scaleFactor : devicePixelRatio
    property bool isTouchState : multitaskViewModel.isTouchState
    // focus border color
    property var hoverBorderColor : UkuiPlatform.GlobalTheme.kWhite
    property var tabFocusBorderColor : UkuiPlatform.GlobalTheme.kBrandNormal
    property var defaultBorderColor : UkuiPlatform.GlobalTheme.kGrayAlpha0
    // focus border width
    property int iconAndCloseButtonHeight : 32 * scaleFactor
    property int appBorderWidth : 4 * scaleFactor
    property int workspaceBorderWidth : 3 * scaleFactor
    property int spacingWidth : 2 * scaleFactor
    property int thumbRectangleHeight : isVerticalLayout ? Math.floor(screenRect.height * 5 / 22) : Math.floor(screenRect.height * 5 / 23)
    // 工作区窗口缩放比
    property double desktopScale : isVerticalLayout ? 16 / 135 : 57 / 480
    // 工作区窗口的高度和宽度
    property int desktopThumbnailWidth : screenRect.width * desktopScale
    property int desktopThumbnailHeight : screenRect.height * desktopScale
    // 水平布局方向
    property bool isLayoutLeftToRight: Qt.application.layoutDirection === Qt.LeftToRight

    property bool shouldHaveTabFocus: false

    Loader {
        id: backgroundLoader
        anchors.fill: parent
        //active: !rootWindow.isWlcom
        sourceComponent: KWin.DesktopBackground {
            id : wholeDesktopBackground
            width : rootWindow.width
            height : rootWindow.height
            blurAlpha : 0.7
        }
    }

    Connections {
        target: rootWindow.isWlcom  && backgroundLoader.active ? multitaskViewManager : null
        function onBackgroundChanged() {
            backgroundLoader.item.update()
        }
    }

    DropArea {
        anchors.fill: parent
    }

    FocusScope {
        id: mainView
        anchors.fill: parent
        focus: true
        width: parent.width
        height: parent.height

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                multitaskViewManager.close()
            }
        }

        AppWindowFilterModel {
            id: appAreaModel
            desktopId: rootWindow.highlightDesktop
            screenId: rootWindow.screenIndex
            sourceModel: appWindowListModel
        }

        Loader {
            id: viewLoader
            sourceComponent: isVerticalLayout ? verticalLayoutView : horizontalLayoutView
            anchors.fill: parent
        }

        Component {
            id: verticalLayoutView
            Rectangle {
                color: "transparent"
                AppArea {
                    id : appArea1
                    anchors.left : parent.left
                    anchors.right : parent.right
                    anchors.top : parent.top
                    height : Math.floor(mainView.height * 111 / 135)
                    width: parent.width

                    Component.onCompleted : {
                        console.log("AppArea end! ! !")
                    }
                }
                DesktopArea {
                    id : desktopArea1
                    anchors.left : parent.left
                    anchors.right : parent.right
                    anchors.top : appArea1.bottom
                    anchors.bottom : parent.bottom
                }

                Component.onCompleted : {
                    console.log("verticalLayoutView end! ! !")
                }
            }
        }

        Component {
            id: horizontalLayoutView
            Rectangle {
                color: "transparent"
                AppArea {
                    id : appArea2
                    anchors.left : parent.left
                    anchors.bottom : parent.bottom
                    anchors.top : parent.top
                    height: parent.height
                    width : Math.floor(mainView.width * 410 / 480)

                    Component.onCompleted : {
                        console.log("MasterAppArea end! ! !")
                    }
                }

                MasterDesktopArea {
                    id : desktopArea2
                    anchors.left : appArea2.right
                    anchors.right : parent.right
                    anchors.top : parent.top
                    anchors.bottom : parent.bottom
                }

                Component.onCompleted : {
                    console.log("horizontalLayoutView end! ! !")
                }
            }
        }
    }
}
