/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12

import org.ukui.menu.core 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

GridView {
    property var labelBottle: null
    property int labelColum: labelBottle === null ? 0 : labelBottle.column
    property int labelRow: Math.ceil(count / labelColum)
    property bool showLabelPage: labelBottle !== null && labelBottle.visible
    property int selectIndex: -1
    property bool showBorder: true

    signal labelSelected(string label)

    onFocusChanged: {
        selectIndex = focus ? 0 : -1;
    }

    // 默认为小屏幕下尺寸
    width: 200
    height: childrenRect.height
    cellWidth: width / labelColum
    cellHeight: 40

    model: labelBottle === null ? [] : labelBottle.labels
    delegate: MouseArea {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
        focus: true
        hoverEnabled: true

        Accessible.role: Accessible.Button
        Accessible.name: modelData.label

        UkuiItems.Tooltip {
            anchors.fill: parent
            mainText: modelData.label
            posFollowCursor: true
            active: labelArea.showTooltip
        }

        onClicked: {
            GridView.view.labelSelected(modelData.label);
            GridView.view.currentIndex = model.index;
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                if (!showBorder) return;
                GridView.view.labelSelected(modelData.label);
            }
        }

        UkuiItems.DtThemeBackground {
            id: labelArea
            property bool showTooltip: labelText.visible ? labelText.truncated : false
            property bool isSelect: {
                if (mainWindow.isFullScreen) {
                    return selectIndex === index;
                } else {
                    return parent.GridView.isCurrentItem;
                }
            }

            anchors.fill: parent

            radius: Platform.GlobalTheme.kRadiusNormal
            useStyleTransparency: false
            backgroundColor: parent.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                  : parent.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                         : Platform.GlobalTheme.kContainGeneralAlphaNormal

            border.width: isSelect && showBorder ? 2 : 0
            borderColor: Platform.GlobalTheme.kBrandFocus

            UkuiItems.DtThemeText {
                id: labelText
                anchors.fill: parent
                visible: modelData.type === LabelItem.Text
                text: modelData.display
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            UkuiItems.Icon {
                anchors.centerIn: parent
                width: parent.height/2
                height: parent.height/2
                visible: modelData.type === LabelItem.Icon
                source: modelData.type === LabelItem.Icon ? modelData.display : ""
                mode: UkuiItems.Icon.AutoHighlight
            }
        }
    }

    Keys.onRightPressed: {
        if (!showBorder) return;
        if (mainWindow.isFullScreen) return;
        if (currentIndex < 0) {
            currentIndex = 0;
        } else {
            if (currentIndex === count - 1) {
                currentIndex = 0;
            } else {
                currentIndex++;
            }
        }
    }
    Keys.onLeftPressed: {
        if (!showBorder) return;
        if (mainWindow.isFullScreen) return;
        if (currentIndex === 0) {
            currentIndex = count - 1;
        } else {
            if (currentIndex < 0) {
                currentIndex = 0;
            } else {
                currentIndex--;
            }
        }
    }
    Keys.onDownPressed: {
        if (!showBorder) return;
        if (mainWindow.isFullScreen) {
            if (selectIndex < count - 1) {
                selectIndex++;
            }
        } else {
            if (currentIndex < 0) {
                currentIndex = 0;
            } else {
                if (currentIndex + labelColum < count) {
                    currentIndex = currentIndex + labelColum;
                } else {
                    currentIndex = currentIndex - Math.floor(currentIndex / labelColum) * labelColum;
                }
            }
        }
    }
    Keys.onUpPressed: {
        if (!showBorder) return;
        if (mainWindow.isFullScreen) {
            if (selectIndex > 0) {
                selectIndex--;
            }
        } else {
            if (currentIndex < 0) {
                currentIndex = 0;
            } else {
                if (currentIndex - labelColum >= 0) {
                    currentIndex = currentIndex - labelColum;
                } else {
                    currentIndex = currentIndex + (labelRow - 1) * labelColum < count ? currentIndex + (labelRow - 1) * labelColum
                                                                                      : currentIndex + (labelRow - 2) * labelColum;
                }
            }
        }
    }
    Keys.onPressed: (event) => {
        if (!mainWindow.isFullScreen) return;
        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            labelSelected(model[selectIndex].label);
            currentIndex = selectIndex;
        }
    }
}
