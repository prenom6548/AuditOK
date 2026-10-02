/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

GridLayout {
    id: baseView

    property var focusGroups: []
    property int itemWidth
    property int itemHeight
    property int buttoonType: ShortcutPlugin.Icon
    property Component buttonComponent: undefined

    function refreshFocusItems() {
        var groups = [];
        for (var i = 0; i < buttonView.count; ++i) {
            var delegate = buttonView.itemAt(i);
            if (delegate && delegate.focusTargets && delegate.focusTargets.length > 0) {
                groups.push(delegate.focusTargets);
            }
        }
        focusGroups = groups;
    }

    flow: GridLayout.LeftToRight
    width: columns * (itemWidth + columnSpacing) - columnSpacing
    height: rows * (itemHeight + rowSpacing) - rowSpacing

    Repeater {
        id: buttonView

        model: editModel.prosessBarModel()
        delegate: Item {
            property var focusTargets: buttonLoader.item && buttonLoader.item.focusTargets
                                      ? buttonLoader.item.focusTargets : []
            width: baseView.itemWidth
            height: baseView.itemHeight
            Layout.rowSpan: 1
            Layout.columnSpan: 1

            Loader {
                id: buttonLoader
                anchors.fill: parent
                anchors.centerIn: parent

                // properties
                property int index: model.index
                property int buttonColor: model.color
                property int buttonValue: model.value
                property int maxValue: model.maxValue
                property bool disabled: model.disabled
                property string icon: model.icon
                property string name: model.name
                property string statusName: model.statusName
                property string widgetId: model.widgetId
                property string tooltip: model.tooltip
                property bool iconClickable: model.iconClickable

                sourceComponent: baseView.buttonComponent

                function activeButton(act) {
                    console.log(tooltip, "active", " action:", act)
                    buttonView.model.active(index, act)
                }
                function setValue(val) {
                    console.log(tooltip, "setValue", " value:", val)
                    buttonView.model.setValue(index, val)
                }
                function updateValue() {
                    buttonView.model.updateValue(index)
                }
                function setDisableValueUpdate(disable) {
                    buttonView.model.disableValueUpdate(disable)
                }

                onLoaded: {
                    item.active.connect(activeButton)
                    item.updateValue.connect(updateValue)
                    item.disableValueUpdate.connect(setDisableValueUpdate)
                    item.setValue.connect(setValue)
                    Qt.callLater(baseView.refreshFocusItems)
                }

                Component.onDestruction: {
                    item.active.disconnect(activeButton)
                    item.updateValue.disconnect(updateValue)
                    item.disableValueUpdate.disconnect(setDisableValueUpdate)
                    item.setValue.disconnect(setValue)
                    Qt.callLater(baseView.refreshFocusItems)
                }
            }

            Connections {
                target: buttonLoader.item
                function onFocusTargetsChanged() {
                    Qt.callLater(baseView.refreshFocusItems);
                }
            }
        }
        onCountChanged: {
            baseView.rows = count % baseView.columns === 0 ?
                        count / baseView.columns :
                        count / baseView.columns + 1
            Qt.callLater(baseView.refreshFocusItems);
        }
    }

    Component.onCompleted: Qt.callLater(refreshFocusItems)
}
