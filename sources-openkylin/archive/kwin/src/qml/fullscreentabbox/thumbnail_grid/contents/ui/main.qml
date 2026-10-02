/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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
import QtQuick.Controls 2.5
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

KWin.Switcher {
    id: tabBox
    currentIndex: listView.currentIndex

    PlasmaCore.Dialog {
        id: informativeTabBox
        property bool canStretchX: true
        property bool canStretchY: false

        property int textMargin: 2

        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
    	backgroundHints: PlasmaCore.Dialog.NoBackground
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

	    onVisibleChanged: {
            if (visible) {
                // 显示前更新size
                dialogMainItem.width = Math.min(Math.max(tabBox.screenGeometry.width * 0.2, listView.calculateMaxRowWidth()), tabBox.screenGeometry.width * 0.8);
                dialogMainItem.height = Math.min(Math.max(tabBox.screenGeometry.height * 0.2, listView.calcRowHeight() * listView.count), tabBox.screenGeometry.height * 0.8);
            }
        }

        mainItem: Rectangle {
            id: dialogMainItem
            property int optimalWidth: listView.maxRowWidth
            property int optimalHeight: listView.rowHeight * listView.count
            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.2, optimalWidth), tabBox.screenGeometry.width * 0.8)
            height: Math.min(Math.max(tabBox.screenGeometry.height * 0.2, optimalHeight), tabBox.screenGeometry.height * 0.8)
	        color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"|| tabBox.theme === "ukui" || tabBox.theme === "ukui-light") ? Qt.rgba(207/255, 207/255, 207/255, 1) : Qt.rgba(54/255, 54/255, 54/255, 1)
            focus: true

            // just to get the margin sizes
            PlasmaCore.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            // delegate
            Component {
                id: listDelegate
                Item {
                    id: delegateItem
                    width: listView.width
                    height: listView.rowHeight
                    property bool isLayoutLeftToRight: tabBox.isLayoutLeftToRight()

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            listView.currentIndex = index;
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft

                        QIconItem {
                            id: iconItem
                            icon: model.icon
                            width: units.iconSizes.medium
                            height: units.iconSizes.medium
                            state: QIconItem.ActiveState
                        }
                        PlasmaComponents.Label {
                            id: captionItem
                            horizontalAlignment: Text.AlignHCenter
                            text: listView.itemCaption(caption, minimized)
                            font.weight: Font.Bold
                            font.italic: minimized
                            elide: Text.ElideMiddle
                            color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"|| tabBox.theme === "ukui" || tabBox.theme === "ukui-light") ? "#000000" : "#FFFFFF"
                        }
                        PlasmaComponents.Label {
                            id: desktopNameItem
                            horizontalAlignment: Text.AlignHCenter
                            text: desktopName
                            font.italic: true
                            elide: Text.ElideMiddle
                            visible: tabBox.allDesktops
                            color: (tabBox.theme === "ukui-default" || tabBox.theme === "ukui-white"|| tabBox.theme === "ukui" || tabBox.theme === "ukui-light") ? "#000000" : "#FFFFFF"
                        }
                    }
                }
            }
            ListView {
                /**
                * Returns the caption with adjustments for minimized items.
                * @param caption the original caption
                * @param mimized whether the item is minimized
                * @return Caption adjusted for minimized state
                **/
                function itemCaption(caption, minimized) {
                    var text = caption;
                    if (minimized) {
                        text = "(" + text + ")";
                    }
                    return text;
                }
                function calculateMaxRowWidth() {
                    var width = 0;
                    var textElement = Qt.createQmlObject(
                        'import QtQuick 2.0;'
                        + 'Text {\n'
                        + '     text: "' + listView.itemCaption(tabBox.model.longestCaption(), true) + '"\n'
                        + '     font.bold: true\n'
                        + '     visible: false\n'
                        + '}',
                        listView, "calculateMaxRowWidth");
                    width = Math.max(textElement.width, width);
                    textElement.destroy();
                    return width + units.iconSizes.medium + hoverItem.margins.right + hoverItem.margins.left;
                }
                /**
                * Calculates the height of one row based on the text height and icon size.
                * @return Row height
                **/
                function calcRowHeight() {
                    var textElement = Qt.createQmlObject(
                        'import QtQuick 2.0;'
                        + 'Text {\n'
                        + '     text: "Some Text"\n'
                        + '     font.bold: true\n'
                        + '     visible: false\n'
                        + '}',
                        listView, "calcRowHeight");
                    var height = textElement.height;
                    textElement.destroy();
                    // icon size or two text elements and margins and hoverItem margins
                    return Math.max(units.iconSizes.medium, height*2 + informativeTabBox.textMargin * 3 + hoverItem.margins.top + hoverItem.margins.bottom);
                }
                id: listView
                model: tabBox.model
                keyNavigationWraps: true
                // the maximum text width + icon item width (32 + 4 margin) + margins for hover item + margins for background
                property int maxRowWidth: calculateMaxRowWidth()
                property int rowHeight: calcRowHeight()
                anchors {
                    fill: parent
                }
                clip: true
                delegate: listDelegate
                highlight: Rectangle {
                    color: "lightblue"
                }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                boundsBehavior: Flickable.StopAtBounds
                Connections {
                    target: tabBox
                    onCurrentIndexChanged: {listView.currentIndex = tabBox.currentIndex;}
                }
            }

            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: {
                if (event.key == Qt.Key_Up) {
                    listView.decrementCurrentIndex();
                } else if (event.key == Qt.Key_Down) {
                    listView.incrementCurrentIndex();
                } else if (event.key == Qt.Key_M) {
                    listView.incrementCurrentIndex();
                }
            }
        }
    }
}
