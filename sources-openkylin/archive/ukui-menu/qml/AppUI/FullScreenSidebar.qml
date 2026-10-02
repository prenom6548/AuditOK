/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import AppControls2 1.0 as AppControls2

import org.ukui.menu.core 1.0
import org.ukui.menu.utils 1.0
import org.ukui.menu.extension 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

Item {
    property Item focusToAppPage
    property alias userButton: userInfoButton
    onFocusChanged: {
        if (focus) {
            if (actionsRepeater.visible) {
                actionsRepeater.forceActiveFocus();
            } else {
                userInfoButton.forceActiveFocus();
            }
        }
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.bottomMargin: 28
        spacing: 4

        // 应用列表模式选择按钮
        Item {
            id: actionsItemBase
            // Layout.topMargin: 40
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            activeFocusOnTab: true
            KeyNavigation.tab: userInfoButton
            KeyNavigation.backtab: fullScreenButton

            Column {
                id: mainLayout
                width: parent.width
                height: childrenRect.height
                spacing: 8

                Repeater {
                    id: actionsRepeater
                    model: AppPageBackend.appModel.header.actions
                    visible: count > 0

                    property int selectIndex: -1
                    onFocusChanged: {
                        selectIndex = focus ? 0 : -1
                    }
                    Keys.onPressed: {
                        switch (event.key) {
                            case Qt.Key_Up:
                                if (selectIndex > 0) {
                                    selectIndex--;
                                    event.accepted = true;
                                }
                                break;
                            case Qt.Key_Down:
                                if (selectIndex < actionsRepeater.count - 1) {
                                    selectIndex++;
                                    event.accepted = true;
                                }
                                break;
                            case Qt.Key_Enter:
                            case Qt.Key_Return:
                                actionsRepeater.model[selectIndex].trigger();
                                event.accepted = true;
                                break;
                            default:
                                break;
                        }
                    }

                    delegate: AppControls2.LabelButton {
                        width: parent.width
                        height: 36

                        Accessible.role: Accessible.Button
                        Accessible.name: modelData.text

                        layoutSpacing: 4
                        isHighlight: modelData.checked
                        sourceData: {"icon": modelData.icon, "text": modelData.text, "tooltip": modelData.toolTip}

                        background.border.width: index === actionsRepeater.selectIndex ? 2 : 0
                        background.backgroundColor: modelData.checked ? Platform.GlobalTheme.highlightActive
                                                                        : containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                                                        : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                                        : Platform.GlobalTheme.kContainGeneralAlphaNormal

                        onClicked: {
                            // 执行action
                            modelData.trigger();
                        }
                    }
                }
            }
        }

        Item {
            id: blankSpace
            Layout.fillHeight: true
            Layout.fillWidth: true
            SidebarButtonUtils {
                id: totalUtils
            }
        }

        AppControls2.LabelButton {
            id: userInfoButton
            property int spacingFromMenu: 16
            iconForceRound: true
            layoutSpacing: 6
            sourceData: {"icon": totalUtils.iconFile, "text": totalUtils.realName, "tooltip": totalUtils.realName}

            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("user")

            KeyNavigation.down: computerButton
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: actionsRepeater.visible ? actionsRepeater : fullScreenButton

            onPressed: {
                if (mouse.button === Qt.LeftButton || mouse.button === Qt.RightButton) {
                    openUserMenu();
                }
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    openUserMenu();
                } else if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                    event.accepted = false;
                }
            }

            function openUserMenu() {
                var buttonPosition = mapToGlobal(width, height);
                totalUtils.openUsersInfoMenu(buttonPosition.x + spacingFromMenu, buttonPosition.y + spacingFromMenu, mainWindow.isFullScreen);
            }
        }

        AppControls2.LabelButton {
            id: computerButton
            sourceData: {"icon": "computer-symbolic", "text": qsTr("Computer"), "tooltip": qsTr("Computer")}

            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            KeyNavigation.down: setButton
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: actionsRepeater.visible ? actionsRepeater : fullScreenButton

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("computer")

            onClicked: {
                totalUtils.openPeonyComputer();
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    totalUtils.openPeonyComputer();
                } else if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                    event.accepted = false;
                }
            }
        }

        AppControls2.LabelButton {
            id: setButton
            sourceData: {"icon": "applications-system-symbolic", "text": qsTr("Settings"), "tooltip": qsTr("Settings")}

            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            KeyNavigation.down: powerButton
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: actionsRepeater.visible ? actionsRepeater : fullScreenButton

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("settings")

            onClicked: {
                totalUtils.openControlCenter();
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    totalUtils.openControlCenter();
                } else if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                    event.accepted = false;
                }
            }
        }

        AppControls2.LabelButton {
            id: powerButton
            property int spacingFromMenu: 16
            sourceData: {"icon": powerButtonBase.icon, "text": powerButtonBase.toolTip, "tooltip": powerButtonBase.toolTip}

            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: actionsRepeater.visible ? actionsRepeater : fullScreenButton

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("power")

            PowerButton {
                id: powerButtonBase
            }

            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    openPowerMenu();
                } else if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                    event.accepted = false;
                }
            }

            onPressed: {
                if (mouse.button === Qt.RightButton || mouse.button === Qt.LeftButton) {
                    openPowerMenu();
                }
            }

            function openPowerMenu() {
                var buttonPosition = mapToGlobal(width, height);
                powerButtonBase.clicked(false, buttonPosition.x + spacingFromMenu, buttonPosition.y + spacingFromMenu, mainWindow.isFullScreen);
            }
        }
    }
}
