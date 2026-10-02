/*
 *
 *  Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.ukui.quick.items 1.0
import org.ukui.windowThumbnail 1.0
import org.ukui.quick.platform 1.0
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager

DtThemeBackground {
    id: container
    property int spacing: 8
    property var containerIcon
    property bool isList: false
    property bool containsMouse: false
    property bool containsPress: false
    property bool viewIsHorizontal: false
    property string containerTitle: ""

    signal closeButtonClicked()

    backgroundColor: GlobalTheme.baseActive
    //预览图非列表模式跟随主题透明度
    alpha: 0
    useStyleTransparency: !isList
    radius: GlobalTheme.kRadiusWindow
    borderColor: GlobalTheme.kLineNormal
    border.width: !isList ? 1 : 0

    Loader {
        id: layoutLoader
        DtThemeBackground {
            anchors.fill: parent
            backgroundColor: containsPress ? GlobalTheme.kGrayAlpha3 :
                                             containsMouse ? GlobalTheme.kGrayAlpha1 :
                                                             GlobalTheme.kGrayAlpha0
            useStyleTransparency: false
            radius: isList ? 4 : GlobalTheme.kRadiusWindow
        }
        sourceComponent: isList ? listLayout : thumbnailLayout
    }

    Component {
        id: thumbnailLayout
        ColumnLayout {
            id: baseLayout
            spacing: 0
            height: {
                if (viewIsHorizontal) {
                    return viewLoader.preChildrenSize;
                } else {
                    var preHeight = windowThumbnail.height + container.spacing * 3 + 24;
                    return Math.max(preHeight, minimumHeight);
                }
            }
            width: {
                if (viewIsHorizontal) {
                    var preWidth = windowThumbnail.width + container.spacing * 2;
                    return Math.max(preWidth, minimumWidth);
                } else {
                    return viewLoader.preChildrenSize;
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
                Layout.maximumHeight: 24
                Layout.margins: container.spacing
                Layout.bottomMargin: 0
                spacing: 0

                Icon {
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: 24
                    // 最小宽度下，保证图标和关闭按钮完全显示（24*2），左右margin也要对称显示
                    Layout.rightMargin: Math.min(baseLayout.width - 24*2 - container.spacing*2, container.spacing)
                    source: containerIcon
                }
                DtThemeText {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: containerTitle
                    textColor: GlobalTheme.kFontPrimary
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter

                    Tooltip {
                        anchors.fill: parent
                        mainText: parent.text
                        posFollowCursor: true
                        visible: parent.truncated
                    }
                }
                MouseArea {
                    id: closeButtonArea
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: 24
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    hoverEnabled: true
                    visible: container.containsMouse

                    DtThemeBackground {
                        Accessible.role: Accessible.Button
                        Accessible.description: "This is a close button of ukui-panel thumbnail window"
                        Accessible.name: "ukui-panel_thumbnail_window_close_button"
                        backgroundColor: closeButtonArea.containsPress ? GlobalTheme.kErrorClick :
                                                                         GlobalTheme.kErrorNormal
                        radius: GlobalTheme.kRadiusNormal
                        anchors.fill: parent
                        visible: parent.containsMouse

                        Tooltip {
                            anchors.fill: parent
                            mainText: qsTr("Close")
                            posFollowCursor: true
                        }
                    }
                    Icon {
                        anchors.centerIn: parent
                        width: 16
                        height: width
                        mode: parent.containsMouse ? Icon.Highlight : Icon.AutoHighlight
                        source: "window-close-symbolic"
                    }
                    onClicked: {
                        closeButtonClicked();
                    }
                }
            }

            WindowThumbnail {
                id: windowThumbnail
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: container.spacing
                Layout.preferredWidth: viewIsHorizontal ? itemWidth : viewLoader.preChildrenSize - container.spacing * 2
                Layout.preferredHeight: viewIsHorizontal ? viewLoader.preChildrenSize - 48 : itemHeight
                Layout.maximumHeight: maximumHeight - container.spacing * 3 - 24
                Layout.maximumWidth: maximumWidth - container.spacing * 2

                property real itemHeight
                property real itemWidth
                // 窗口visible改变
                winId: windowVisible ? modelData : ""

                function updateSize() {
                    if (winId === "") {
                        return;
                    }
                    var itemGeometry = WindowManager.geometry(winId);
                    if (viewIsHorizontal) {
                        itemWidth = itemGeometry.width * ((viewLoader.preChildrenSize - 48 ) / itemGeometry.height);
                    } else {
                        itemHeight = itemGeometry.height * ((viewLoader.preChildrenSize - container.spacing * 2) / itemGeometry.width);
                    }
                }
                onWinIdChanged: updateSize()
                onWidthChanged: updateSize()
                onHeightChanged: updateSize()
            }
        }
    }
    Component {
        id: listLayout
        Item {
            width: viewLoader.item.isIntervactive ? 220 : 232
            height: 36
            RowLayout {
                anchors.fill: parent
                anchors.margins: container.spacing

                Icon {
                    Layout.preferredHeight: 16
                    Layout.preferredWidth: 16
                    source: containerIcon
                }
                DtThemeText {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: containerTitle
                    textColor: GlobalTheme.kFontPrimary
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter

                    Tooltip {
                        anchors.fill: parent
                        mainText: parent.text
                        posFollowCursor: true
                        visible: parent.truncated
                    }
                }
                MouseArea {
                    id: listCloseButtonArea
                    Layout.preferredHeight: 16
                    Layout.preferredWidth: 16
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    hoverEnabled: true
                    visible: container.containsMouse

                    DtThemeBackground {
                        Accessible.role: Accessible.Button
                        Accessible.description: "This is a close button of ukui-panel thumbnail window"
                        Accessible.name: "ukui-panel_thumbnail_window_close_button"
                        backgroundColor: listCloseButtonArea.containsPress ? GlobalTheme.kErrorClick :
                                                                             GlobalTheme.kErrorNormal
                        radius: GlobalTheme.kRadiusMin
                        anchors.fill: parent
                        visible: parent.containsMouse

                        Tooltip {
                            anchors.fill: parent
                            mainText: qsTr("Close")
                            posFollowCursor: true
                        }
                    }
                    Icon {
                        anchors.fill: parent
                        mode: parent.containsMouse ? Icon.Highlight : Icon.AutoHighlight
                        source: "window-close-symbolic"
                    }
                    onClicked: {
                        closeButtonClicked();
                    }
                }
            }
        }
    }
}
