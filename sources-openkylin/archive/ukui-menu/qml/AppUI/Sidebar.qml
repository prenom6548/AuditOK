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

import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5

import org.ukui.menu.core 1.0
import org.ukui.menu.utils 1.0
import org.ukui.menu.extension 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

Item {
    id: sidebarRoot
    property alias foldButton: fullScreenButton
    property Item focusToAppPage
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 4

        UkuiItems.DtThemeButton {
            id: fullScreenButton
            visible: !isLiteMode && functionControl.showFoldButton
            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                      : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                      : Platform.GlobalTheme.kContainGeneralAlphaNormal
            background.borderColor: Platform.GlobalTheme.kBrandFocus
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            KeyNavigation.tab: userInfoButton

            Accessible.role: Accessible.Button
            Accessible.name: mainWindow.isFullScreen ? qsTr("Contract") : qsTr("Expand")

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: mainWindow.isFullScreen ? qsTr("Contract") : qsTr("Expand")
                posFollowCursor: true
                margin: 6
            }

            onClicked: {
                if (mainWindow.isFullScreen) {
                    root.currentSearchText = searchInputBar.text;
                } else {
                    root.currentSearchText = appPage.search.searchInputBar.text;
                }
                mainWindow.isFullScreen = !mainWindow.isFullScreen;
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return
                        || event.key === Qt.Key_Space) {
                    root.focusOnFoldButton = true;
                    if (mainWindow.isFullScreen) {
                        root.currentSearchText = searchInputBar.text;
                    } else {
                        root.currentSearchText = appPage.search.searchInputBar.text;
                    }
                    mainWindow.isFullScreen = !mainWindow.isFullScreen;
                }
            }

            background.radius: Platform.GlobalTheme.kRadiusMin
            icon.mode: UkuiItems.Icon.AutoHighlight
            icon.source: mainWindow.isFullScreen ? "view-restore-symbolic" : "view-fullscreen-symbolic"
        }

        Item {
            id: blankSpace
            Layout.fillHeight: true
            Layout.fillWidth: true
            SidebarButtonUtils {
                id: totalUtils
            }
        }

        MouseArea {
            id: userInfoButton
            property int spacingFromMenu: 16

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("user")

            KeyNavigation.down: computerButton
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: fullScreenButton

            UkuiItems.DtThemeBackground {
                anchors.fill: parent

                border.width: parent.activeFocus ? 2 : 0
                backgroundColor: parent.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                                 : parent.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                        : Platform.GlobalTheme.kContainGeneralAlphaNormal
                borderColor: Platform.GlobalTheme.kBrandFocus
                useStyleTransparency: false
                radius: Platform.GlobalTheme.kRadiusMin

                UkuiItems.Icon {
                    anchors.centerIn: parent
                    width: 24
                    height: 24
                    mode: UkuiItems.Icon.AutoHighlight
                    forceRound: true
                    source: totalUtils.iconFile
                }
            }

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: totalUtils.realName
                posFollowCursor: true
                margin: 6
            }

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

        UkuiItems.DtThemeButton {
            id: computerButton
            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                      : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                      : Platform.GlobalTheme.windowTextActive
            background.alpha: containsPress ? 1 : containsMouse ? 1 : 0.0
            background.borderColor: Platform.GlobalTheme.kBrandFocus

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            KeyNavigation.down: setButton
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: fullScreenButton

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("computer")

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: qsTr("Computer")
                posFollowCursor: true
                margin: 6
            }

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

            background.radius: Platform.GlobalTheme.kRadiusMin
            icon.mode: UkuiItems.Icon.AutoHighlight
            icon.source: "computer-symbolic"
        }

        UkuiItems.DtThemeButton {
            id: setButton
            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                      : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                      : Platform.GlobalTheme.windowTextActive
            background.alpha: containsPress ? 1 : containsMouse ? 1 : 0.0
            background.borderColor: Platform.GlobalTheme.kBrandFocus

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            KeyNavigation.down: powerButton
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: fullScreenButton

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("settings")

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: qsTr("Settings")
                posFollowCursor: true
                margin: 6
            }

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

            background.radius: Platform.GlobalTheme.kRadiusMin
            icon.mode: UkuiItems.Icon.AutoHighlight
            icon.source: "applications-system-symbolic"
        }

        UkuiItems.DtThemeButton {
            id: powerButton
            property int spacingFromMenu: 16
            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                      : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                      : Platform.GlobalTheme.windowTextActive
            background.alpha: containsPress ? 1 : containsMouse ? 1 : 0.0
            background.borderColor: Platform.GlobalTheme.kBrandFocus

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            KeyNavigation.tab: focusToAppPage
            KeyNavigation.backtab: fullScreenButton

            acceptedButtons: Qt.LeftButton | Qt.RightButton
            background.radius: Platform.GlobalTheme.kRadiusMin
            icon.mode: UkuiItems.Icon.AutoHighlight
            icon.source: powerButtonBase.icon

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("power")

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: powerButtonBase.toolTip
                posFollowCursor: true
                margin: 6
            }

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
                var buttonBottomMargin = sidebarRoot.height - mapToItem(sidebarRoot, 0, height).y; // 计算按钮底部距离sidebar底部的距离
                powerButtonBase.clicked(false, buttonPosition.x + spacingFromMenu, buttonPosition.y + buttonBottomMargin, mainWindow.isFullScreen);
            }
        }
    }
}
