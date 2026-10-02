/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
import QtQuick.Controls 2.12
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiItems.DtThemeBackground {
    id: root
    property alias count: actionsListview.count
    property alias actions: actionsListview.model
    property alias spacing: actionsListview.spacing
    property alias view: actionsListview
    property int maring: 3

    width: count > 0 ? (actionsListview.width + maring*2) : 0

    radius: Platform.GlobalTheme.kRadiusNormal
    alpha: 0.03
    useStyleTransparency: false
    backgroundColor: Platform.GlobalTheme.windowTextActive

    border.width: 1
    borderAlpha: 0.06
    borderColor: Platform.GlobalTheme.windowTextActive

    ListView {
        id: actionsListview
        orientation: ListView.Horizontal
        height: root.height - maring * 2
        width: contentWidth
        x: root.maring; y: root.maring
        spacing: 2
        onFocusChanged: {
            currentIndex = 0;
        }
        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Left:
                if (currentIndex > 0) {
                    currentIndex--;
                    event.accepted = true;
                }
                break;
            case Qt.Key_Right:
                if (currentIndex < actionsListview.count - 1) {
                    currentIndex++;
                    event.accepted = true;
                }
                break;
            case Qt.Key_Enter:
            case Qt.Key_Return:
                actionsListview.model[currentIndex].trigger();
                event.accepted = true;
                break;
            default:
                break;
            }
        }


        delegate: Component {
            UkuiItems.DtThemeButton {
                width: height
                height: parent.height
                background.border.width: actionsListview.activeFocus && ListView.isCurrentItem ? 2 : 0
                background.borderColor: Platform.GlobalTheme.kBrandFocus
                activeFocusOnTab: false

                Accessible.role: Accessible.Button
                Accessible.name: modelData.toolTip

                UkuiItems.Tooltip {
                    anchors.fill: parent
                    mainText: modelData.toolTip
                    posFollowCursor: true
                    margin: 6
                    visible: modelData.toolTip !== ""
                }

                background.radius: Platform.GlobalTheme.kRadiusNormal
                background.backgroundColor: modelData.checked ? Platform.GlobalTheme.highlightActive
                                                              : containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                                              : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                              : Platform.GlobalTheme.kContainGeneralAlphaNormal

                icon.source: modelData.icon
                icon.mode: modelData.checked ? UkuiItems.Icon.Highlight : UkuiItems.Icon.AutoHighlight
                icon.width: background.width / 2 + 1
                icon.height: background.height / 2 + 1

                onClicked: {
                    // 执行action
                    modelData.trigger();
                }
            }
        }
    }
}
