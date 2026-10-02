/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import QtQuick.Controls 2.5
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

// https://techbase.kde.org/Development/Tutorials/KWin/WindowSwitcher
// https://github.com/KDE/kwin/blob/master/tabbox/switcheritem.h
KWin.Switcher {
    id: tabBox
    currentIndex: thumbnailGridView.currentIndex

    PlasmaCore.Dialog {
        id: dialog
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
       // visualParent: tabBox

        backgroundHints: PlasmaCore.Dialog.NoBackground
        //opacity: 0.1
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        onVisibleChanged: {
            if (visible) {
                dialogMainItem.calculateColumnCount();
            } else {
                thumbnailGridView.highCount = 0;
            }
        }


        mainItem: Rectangle {

            id: dialogMainItem
            focus: true
            color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"
                   || tabBox.theme === "ukui" || tabBox.theme === "ukui-light")? Qt.rgba(245/255, 245/255, 245/255, 0.45) : Qt.rgba(35/255, 36/255, 38/255, 0.45)


            property int maxWidth: tabBox.screenGeometry.width
            property int maxHeight: tabBox.screenGeometry.height * 0.8
            property real screenFactor: tabBox.screenGeometry.width / tabBox.screenGeometry.height
            property int maxGridColumnsByWidth: Math.floor(maxWidth / thumbnailGridView.cellWidth)

            property int gridColumns: maxGridColumnsByWidth
            property int gridRows: Math.ceil(thumbnailGridView.count / gridColumns)
            property int optimalWidth: thumbnailGridView.cellWidth * gridColumns
            property int optimalHeight: thumbnailGridView.cellHeight * gridRows
            property bool canStretchX: false
            property bool canStretchY: false
           // width: Math.min(Math.max(thumbnailGridView.cellWidth, optimalWidth), maxWidth)
            width:  maxWidth
            height: Math.min(Math.max(thumbnailGridView.cellHeight+ 80 * units.devicePixelRatio, optimalHeight + 80 * units.devicePixelRatio), maxHeight)

            clip: true

            // Simple greedy algorithm
            function calculateColumnCount() {
//                // respect screenGeometry
//                const c = Math.min(thumbnailGridView.count, maxGridColumnsByWidth);

//                const residue = thumbnailGridView.count % c;
//                if (residue == 0) {
//                    gridColumns = c;
//                    return;
//                }

//                // start greedy recursion
//                gridColumns = columnCountRecursion(c, c, c - residue);
            }

            // Step for greedy algorithm
            function columnCountRecursion(prevC, prevBestC, prevDiff) {
                const c = prevC - 1;

                // don't increase vertical extent more than horizontal
                // and don't exceed maxHeight
                if (prevC * prevC <= thumbnailGridView.count + prevDiff ||
                        maxHeight < Math.ceil(thumbnailGridView.count / c) * thumbnailGridView.cellHeight) {
                    return prevBestC;
                }
                const residue = thumbnailGridView.count % c;
                // halts algorithm at some point
                if (residue == 0) {
                    return c;
                }
                // empty slots
                const diff = c - residue;

                // compare it to previous count of empty slots
                if (diff < prevDiff) {
                    return columnCountRecursion(c, c, diff);
                } else if (diff == prevDiff) {
                    // when it's the same try again, we'll stop early enough thanks to the landscape mode condition
                    return columnCountRecursion(c, prevBestC, diff);
                }
                // when we've found a local minimum choose this one (greedy)
                return columnCountRecursion(c, prevBestC, diff);
            }

            // Just to get the margin sizes
//            PlasmaCore.FrameSvgItem {
//                id: hoverItem
//                imagePath: "widgets/viewitem"
//                prefix: "hover"
//                visible: false


//            }

            GridView {
                id: thumbnailGridView
                Accessible.name: "tabbox thumbnail gridview"
                anchors.fill: parent


                anchors.bottomMargin: 40 * units.devicePixelRatio
                anchors.topMargin: 40 * units.devicePixelRatio
                anchors.leftMargin: count > dialogMainItem.maxGridColumnsByWidth ? (dialogMainItem.width-dialogMainItem.maxGridColumnsByWidth * cellWidth)/2 :
                                                                                   (dialogMainItem.width-count * cellWidth)/2
                anchors.rightMargin: count > dialogMainItem.maxGridColumnsByWidth ? (dialogMainItem.width-dialogMainItem.maxGridColumnsByWidth * cellWidth)/2 :
                                                                                    (dialogMainItem.width-count * cellWidth)/2

                LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
                model: tabBox.model




                property int iconSize: 24 * units.devicePixelRatio
                property int captionRowHeight: 32 * units.devicePixelRatio // The close button is 30x30 in Breeze
                property int thumbnailWidth: (tabBox.screenGeometry.width-80)/4
               // property int thumbnailHeight: thumbnailWidth * (1.0/dialogMainItem.screenFactor)
                property int thumbnailHeight: tabBox.screenGeometry.height/22*5
                cellWidth: 8 * units.devicePixelRatio + thumbnailWidth + 8 * units.devicePixelRatio
                cellHeight: 8 * units.devicePixelRatio + captionRowHeight + thumbnailHeight + 8 * units.devicePixelRatio


                keyNavigationWraps: true
                highlightMoveDuration: 0

                // Allow expansion on increasing count
                property int highCount: 0
                onCountChanged: {
                    if (highCount < count) {
                        dialogMainItem.calculateColumnCount();
                        highCount = count;
                    }
                    if (count <= dialogMainItem.maxGridColumnsByWidth) {

                    }

                }

                delegate: Item {
                    id: thumbnailGridItem
                    width: thumbnailGridView.cellWidth
                    height: thumbnailGridView.cellHeight
                    property bool isLayoutLeftToRight: Qt.application.layoutDirection === Qt.LeftToRight

                    MouseArea {
                        anchors.fill: parent
                        Accessible.role: Accessible.Button
                        Accessible.name: "active-window-" + model.caption
                        Accessible.description: "preview-window, click to active the app window"

                        onClicked: {
                            thumbnailGridItem.select();
                        }
                        hoverEnabled: true

                        onEntered: {
                            thumbnailBackItem.border.width= 2 * units.devicePixelRatio;

                        }

                        onExited: {(tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"
                                    || tabBox.theme === "ukui" || tabBox.theme === "ukui-light")
                            thumbnailBackItem.border.width= 0;
                        }
                    }
                    function select() {
                        thumbnailGridView.currentIndex = index;
                        thumbnailGridView.currentIndexChanged(thumbnailGridView.currentIndex);
                        tabBox.selectWindow();
                    }

                    ColumnLayout {
                        z: 0
                        spacing: 0
                        anchors.fill: parent
                        anchors.leftMargin: 8 * units.devicePixelRatio
                        anchors.topMargin: 8 * units.devicePixelRatio
                        anchors.rightMargin: 8 * units.devicePixelRatio
                        anchors.bottomMargin: 8 * units.devicePixelRatio

                        RowLayout {
                            id: captionRow
                            spacing: 4 * units.devicePixelRatio
                            layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft

                            QIconItem {
                                id: iconItem
                                Layout.minimumHeight: thumbnailGridView.iconSize
                                Layout.minimumWidth: thumbnailGridView.iconSize
                                Layout.maximumHeight: Layout.minimumHeight
                                Layout.maximumWidth: Layout.minimumWidth
                                icon: model.icon
                                state: index == thumbnailGridView.currentIndex ? QIconItem.ActiveState : QIconItem.DefaultState
                            }

                            Label {
                                id: label
                                Layout.fillWidth: true
                                height: 20
                                color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"
                                        || tabBox.theme === "ukui" || tabBox.theme === "ukui-light") ? "#000000" : "#FFFFFF"
                                text: model.caption
                                horizontalAlignment: isLayoutLeftToRight ? Qt.AlignLeft : Qt.AlignRight
                                elide : isLayoutLeftToRight ? Text.ElideRight : Text.ElideLeft
                            }

                        }

                        // KWin.ThumbnailItem needs a container
                        // otherwise it will be drawn the same size as the parent ColumnLayout
                        Rectangle {
                            id: thumbnailBackItem
                            Layout.fillWidth: true
                            Layout.fillHeight: true
//                            color: Qt.rgba(255,255,255, 0.5)
                            color: "transparent"
                            border.color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"
                                           || tabBox.theme === "ukui" || tabBox.theme === "ukui-light") ? "#262626" : "#FFFFFF"
                            border.width: 0;
                            radius: tabBox.themeRadius * units.devicePixelRatio;


                            // Cannot draw anything (like an icon) on top of thumbnail
                            KWin.ThumbnailItem {
                                id: thumbnailItem
                                anchors.fill: parent
                                anchors.leftMargin: 4 * units.devicePixelRatio
                                anchors.topMargin: 4 * units.devicePixelRatio
                                anchors.rightMargin: 4 * units.devicePixelRatio
                                anchors.bottomMargin: 4 * units.devicePixelRatio

                                wId: windowId
                                clip: true
                            }
                        }
                    }
                } // GridView.delegate

                highlight: Rectangle {
                    id: highlightItem
                    color: "transparent"
                    border.color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"
                                   || tabBox.theme === "ukui" || tabBox.theme === "ukui-light") ? "#262626" : "#FFFFFF"
                    border.width: 4 * units.devicePixelRatio;
                    radius: tabBox.themeRadius * units.devicePixelRatio;

                }

                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {
                        thumbnailGridView.currentIndex = tabBox.currentIndex;
                    }
                }
            } // GridView

            Keys.onPressed: {
                if (event.key == Qt.Key_Left) {
                    thumbnailGridView.moveCurrentIndexLeft();
                } else if (event.key == Qt.Key_Right) {
                    thumbnailGridView.moveCurrentIndexRight();
                } else if (event.key == Qt.Key_Up) {
                    thumbnailGridView.moveCurrentIndexUp();
                } else if (event.key == Qt.Key_Down) {
                    thumbnailGridView.moveCurrentIndexDown();
                } else {
                    return;
                }

                thumbnailGridView.currentIndexChanged(thumbnailGridView.currentIndex);
            }
        } // Dialog.mainItem
    } // Dialog
}
