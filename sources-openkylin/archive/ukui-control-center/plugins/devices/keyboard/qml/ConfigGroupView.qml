/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Column {
    id: configGroupView

    property alias groups: repeater.model
    property int itemHeight: 60
    property int itemCheckBoxHeight: 0
    property int itemLineEditHeight: 0
    property int itemSwitchHeight: 0
    property int itemSpacing: 2
    property bool autoSave: false
    property bool useCustomTranslate: false
    property bool enableBackground: true
    property bool groupTitlebold: true
    property var saveHandler: null
    property var openExternalHandler: null
    property var delayedShortcutUpdateHandler: null
    property int itemLayoutMargins: 0
    property int itemSpecialCheckBoxHeight: 37
    property string source: ""
    property bool enableListOption: false

    spacing: 0
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 24

    Repeater {
        id: repeater

        delegate: Column {
            spacing: 2
            width: configGroupView.width

            UkuiItems.DtThemeText {
                visible: modelData.description !== ""
                text: modelData.description
                pointSizeOffset: groupTitlebold ? 2 : 0
                anchors.left: parent.left
                anchors.leftMargin: groupTitlebold ? 16 : 0
            }

            Rectangle {
                width: configGroupView.width
                height: childrenRect.height
                radius: GlobalTheme.kRadiusMax
                color: "transparent"

                Column {
                    id: itemColumn

                    width: parent.width
                    spacing: 0
                    topPadding: configGroupView.enableBackground ? 8 : 0

                    Repeater {
                        id: itemsRepeater

                        model: modelData.items
                        Component.onCompleted: {
                            for (var i = itemsRepeater.model.length - 1; i >= 0; i--) {
                                var item = itemsRepeater.itemAt(i);
                                if (item && item.isItemValid === true) {
                                    item.isLast = true;
                                    break;
                                }
                            }
                        }

                        delegate: Item {
                            readonly property bool isFirst: index === 0
                            property bool isLast: false
                            readonly property bool isItemValid: loader.status === Loader.Ready
                            readonly property int topSpacing: model.index >= 1 ? ((modelData.name === "EnumerateWithTriggerKeys") ? 0 : configGroupView.itemSpacing) : 0

                            width: parent.width
                            height: spacingRect.height + loader.height

                            Rectangle {
                                id: spacingRect

                                width: parent.width
                                height: isItemValid ? topSpacing : 0
                                color: "transparent"
                            }

                            ConfigItemLoader {
                                id: loader

                                source: configGroupView.source
                                anchors.top: spacingRect.bottom
                                itemWidth: configGroupView.width
                                itemHeight: configGroupView.itemHeight
                                itemCheckBoxHeight: (modelData.name === "EnumerateWithTriggerKeys") ? itemSpecialCheckBoxHeight : configGroupView.itemCheckBoxHeight
                                itemLineEditHeight: configGroupView.itemLineEditHeight
                                itemLayoutMargins: configGroupView.itemLayoutMargins
                                itemSwitchHeight: configGroupView.itemSwitchHeight
                                anchors.horizontalCenter: parent.horizontalCenter
                                enableBackground: configGroupView.enableBackground
                                itemData: modelData
                                index: model.index
                                autoSave: configGroupView.autoSave
                                useCustomTranslate: configGroupView.useCustomTranslate
                                saveHandler: configGroupView.saveHandler
                                openExternalHandler: configGroupView.openExternalHandler
                                delayedShortcutUpdateHandler: configGroupView.delayedShortcutUpdateHandler
                                enableVCenter: modelData.name !== "EnumerateWithTriggerKeys"
                                enableListOption: configGroupView.enableListOption
                            }

                            UkuiItems.DtThemeBackground {
                                visible: configGroupView.enableBackground
                                useStyleTransparency: false
                                z: -1
                                anchors.fill: loader
                                radius: GlobalTheme.kRadiusNormal
                                clip: true

                                Rectangle {
                                    anchors.fill: parent
                                    radius: GlobalTheme.kRadiusNormal
                                    color: parent.color
                                    anchors.topMargin: isFirst ? 0 : -parent.radius
                                    anchors.bottomMargin: isLast ? 0 : -parent.radius
                                }

                            }

                        }

                    }

                }

            }

            Rectangle {
                width: configGroupView.width
                color: "transparent"
                height: configGroupView.enableBackground ? 38 : configGroupView.itemSpacing * 2
            }

        }

    }

}
