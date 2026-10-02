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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems

Rectangle {
    id: configPage

    property alias configGroups: groupView.groups
    property int itemHeight: 60
    property int itemCheckBoxHeight: 0
    property int itemLineEditHeight: 0
    property int itemSwitchHeight: 0
    property int itemLayoutMargins: 0
    property int itemSpacing: 2
    property bool autoSave: false
    property bool useCustomTranslate: false
    property bool enableBackground: true
    property bool groupTitlebold: true
    property var immediateSaveFunction: null
    property var openExternalFunction: null
    property var delayedShortcutUpdateHandler: null
    property string source: ""
    property bool enableListOption: false

    function debouncedSave() {
        if (!saveTimer.running && immediateSaveFunction)
            saveTimer.start();

    }

    function openExternalConfig(uri, name) {
    }

    implicitHeight: mainColumn.height
    color: "transparent"

    Flickable {
        id: flickable

        anchors.fill: parent
        contentWidth: mainColumn.width
        contentHeight: mainColumn.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        MouseArea {
            onWheel: () => {
                flickable.flick(0, wheel.angleDelta.y * 5);
            }
        }

        Column {
            id: mainColumn

            width: configPage.width
            spacing: 2

            ConfigGroupView {
                id: groupView

                source: configPage.source
                itemLayoutMargins: configPage.itemLayoutMargins
                width: configPage.width
                itemSpacing: configPage.itemSpacing
                itemHeight: configPage.itemHeight
                itemCheckBoxHeight: configPage.itemCheckBoxHeight
                itemLineEditHeight: configPage.itemLineEditHeight
                itemSwitchHeight: configPage.itemSwitchHeight
                enableBackground: configPage.enableBackground
                groupTitlebold: configPage.groupTitlebold
                autoSave: configPage.autoSave
                useCustomTranslate: configPage.useCustomTranslate
                saveHandler: configPage.immediateSaveFunction
                openExternalHandler: configPage.openExternalFunction
                delayedShortcutUpdateHandler: configPage.delayedShortcutUpdateHandler
                enableListOption: configPage.enableListOption
            }

        }

        ScrollBar.vertical: ScrollBar {
            id: scrollBar

            Accessible.name: qsTr("Config Page ScrollBar")
            hoverEnabled: true
            active: hovered || pressed
            orientation: Qt.Vertical
            size: flickable.height / flickable.contentHeight
            anchors.rightMargin: 4
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: flickable.contentHeight > flickable.height
        }

    }

    Timer {
        id: saveTimer

        interval: 500
        onTriggered: {
            if (immediateSaveFunction)
                immediateSaveFunction();

        }
    }

}
