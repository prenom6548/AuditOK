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
import "childwindow"
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Item {
    id: root

    property string itemText: ""
    property string uniqueName: ""
    property string imIcon: ""
    property int itemIndex: -1
    property int itemCount: -1
    property bool configurable: false
    property Item parentContainer

    signal dragStarted()
    signal dragging(real yPos)
    signal dragEnded()
    signal editClicked(int index)
    signal moveUp(int index)
    signal deleteIM(int index)

    width: parent.width

    UkuiItems.StyleBackground {
        id: dragProxy

        parent: parentContainer
        width: root.width
        height: root.height
        visible: dragArea.drag.active
        opacity: 0.6

        RowLayout {
            anchors.fill: parent
            Layout.fillWidth: true

            UkuiItems.Icon {
                Layout.leftMargin: 16
                width: 24
                height: 24
                source: imIcon
                mode: UkuiItems.Icon.AutoHighlight
                Layout.alignment: Qt.AlignVCenter
            }

            UkuiItems.DtThemeText {
                id: proxyText

                Layout.leftMargin: 16
                text: root.itemText
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                Layout.rightMargin: 16
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 36
                Layout.preferredWidth: 36
                Layout.minimumHeight: 36
                Layout.minimumWidth: 36
                Layout.maximumHeight: 36
                Layout.maximumWidth: 36
                flat: true
                Accessible.name: qsTr("Option Button")

                icon {
                    name: "view-more-horizontal-symbolic"
                }

            }

        }

    }

    Rectangle {
        id: background

        anchors.fill: parent
        color: "transparent"

        MouseArea {
            id: dragArea

            anchors.fill: parent
            drag.target: dragProxy
            drag.threshold: 5
            onPressed: {
                root.z = 1;
                dragProxy.x = root.mapToItem(parentContainer, 0, 0).x;
                dragProxy.y = root.mapToItem(parentContainer, 0, 0).y;
                dragStarted();
            }
            onPositionChanged: {
                if (drag.active)
                    dragging(dragProxy.y);

            }
            onReleased: {
                root.z = 0;
                dragEnded();
            }
        }

        RowLayout {
            id: imLayouts

            anchors.fill: parent

            UkuiItems.Icon {
                Layout.leftMargin: 16
                width: 24
                height: 24
                source: imIcon
                mode: UkuiItems.Icon.AutoHighlight
                Layout.alignment: Qt.AlignVCenter
            }

            UkuiItems.DtThemeText {
                id: imText

                text: root.itemText
                height: 21
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.alignment: Qt.AlignVCenter
                Layout.bottomMargin: 4
            }

            Item {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }

            Button {
                id: optionButton

                parent: imLayouts
                Layout.rightMargin: 16
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 36
                Layout.preferredWidth: 36
                Layout.minimumHeight: 36
                Layout.minimumWidth: 36
                Layout.maximumHeight: 36
                Layout.maximumWidth: 36
                flat: true
                checkable: true
                onClicked: {
                    var disableMoveUp = itemIndex === 0 ? "moveup" : "";
                    var disableMoveDown = itemIndex === itemCount - 1 ? "movedown" : "";
                    var disableMoveSetting = !configurable ? "settings" : "";
                    var disableDelete = itemIndex === itemCount - 1 && itemIndex === 0 ? "delete" : "";
                    var disableActionList = [disableMoveUp, disableMoveDown, disableMoveSetting, disableDelete];
                    menuManager.setMenu(optionButton, itemIndex, disableActionList);
                    menuManager.showMenu();
                    checked = true;
                }
                Accessible.name: qsTr("Option Button")

                icon {
                    name: "view-more-horizontal-symbolic"
                }

            }

        }

    }

    Connections {
        function onMenuItemClicked(message, index) {
            if (index === itemIndex) {
                switch (message) {
                case "moveup":
                    imModel.moveUpInputMethod(index);
                    imModel.updateCurrentGroupInputMethodList();
                    break;
                case "movedown":
                    imModel.moveDownInputMethod(index);
                    imModel.updateCurrentGroupInputMethodList();
                    break;
                case "delete":
                    deleteIM(index);
                    break;
                case "settings":
                    if (uniqueName.toLowerCase().includes("com.sogou")) {
                        configManager.launchSogouConfigTool(uniqueName);
                    } else {
                        configManager.getImConfig(uniqueName);
                        var imConfigDialog = imConfigComponent.createObject(parent);
                        imConfigDialog.windowTitle = root.itemText;
                        imConfigDialog.uniqueName = root.uniqueName;
                        imConfigDialog.show();
                    }
                    break;
                default:
                    break;
                }
            }
        }

        function onMenuClosed() {
            optionButton.checked = false;
        }

        target: menuManager
    }

    Component {
        id: imConfigComponent

        InputMethodConfig {
            id: imConfig
        }

    }

}
