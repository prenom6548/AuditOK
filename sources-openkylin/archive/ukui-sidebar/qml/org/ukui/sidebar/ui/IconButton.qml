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

import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

BaseButton {
    id: buttonRoot

    Accessible.role: Accessible.Button
    Accessible.name: name

    UkuiItems.DtThemeBackground {
        useStyleTransparency: false
        anchors.fill: parent
        radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax * 2 : Platform.GlobalTheme.kRadiusMax
        backgroundColor: buttonColor === PluginColorRole.HighLight ? iconHighLightBackGroundColor()
                                                                   : iconNormalBackGroundColor()
        border.width: root.focusFrameVisible && buttonRoot.activeFocus ? 2 : 1
        borderColor: root.focusFrameVisible && buttonRoot.activeFocus ? Platform.GlobalTheme.kBrandFocus
                                 : buttonColor === PluginColorRole.HighLight ? Platform.GlobalTheme.kGrayAlpha1
                                                                            : iconNormalBorderColor()

        function iconHighLightBackGroundColor() {
            return dragMask ? Platform.GlobalTheme.kBrandClick
                            : mouseArea.containsPress ? Platform.GlobalTheme.kBrandClick
                                                      : mouseArea.containsMouse ? Platform.GlobalTheme.kBrandHover
                                                                                : Platform.GlobalTheme.kBrandNormal
        }

        function iconNormalBackGroundColor() {
            return dragMask ? Platform.GlobalTheme.kGray6
                            : mouseArea.containsPress ? Platform.GlobalTheme.kGrayAlpha3
                                                      : mouseArea.containsMouse ? Platform.GlobalTheme.kGrayAlpha2
                                                                                : Platform.GlobalTheme.kGrayAlpha1
        }

        function iconNormalBorderColor() {
            return mouseArea.containsPress ? Platform.GlobalTheme.kGrayAlpha3
                                           : mouseArea.containsMouse ? Platform.GlobalTheme.kGrayAlpha2
                                                                     : Platform.GlobalTheme.kGrayAlpha1
        }

        MouseArea {
            id: mouseArea
            hoverEnabled: true
            anchors.fill: parent
            onClicked: {
                if (iconBase.sourceType === UkuiItems.Icon.LottieSource) {
                    if (iconBase.UkuiItems.Lottie.playing) {
                        iconBase.UkuiItems.Lottie.currentFrame = 0
                    } else {
                        iconBase.UkuiItems.Lottie.playing = true
                    }
                }
                active(ShortcutPlugin.Click)
            }

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: name
                posFollowCursor: true
                active: buttonName.truncated
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.topMargin: 8
            spacing: 4

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                UkuiItems.Icon {
                    id: iconBase
                    width: mainWindow.isTabletMode ? 32 : 24
                    height: mainWindow.isTabletMode ? 32 : 24
                    anchors.centerIn: parent
                    source: icon
                    mode: (buttonColor === PluginColorRole.HighLight) ? UkuiItems.Icon.Highlight :
                        (buttonColor === PluginColorRole.Disable) ? UkuiItems.Icon.Disabled : UkuiItems.Icon.AutoHighlight

                    function setLottiePlaybackPolicy() {
                        if (sourceType !== UkuiItems.Icon.LottieSource) {
                            return
                        }
                        UkuiItems.Lottie.hiddenPlaybackPolicy = UkuiItems.Lottie.ContinueWhenHidden
                    }

                    onSourceTypeChanged: setLottiePlaybackPolicy()
                    Component.onCompleted: setLottiePlaybackPolicy()
                }
            }

            UkuiItems.DtThemeText {
                id: buttonName
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight + 8

                text: name
                elide: Qt.ElideRight
                textColor: disabled ? Platform.GlobalTheme.kFontPrimaryDisable
                                    : buttonColor === PluginColorRole.HighLight ? Platform.GlobalTheme.kFontWhite
                                                                                : Platform.GlobalTheme.kFontPrimary

                pointSizeOffset: -1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
            }
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (iconBase.sourceType === UkuiItems.Icon.LottieSource) {
                if (iconBase.UkuiItems.Lottie.playing) {
                    iconBase.UkuiItems.Lottie.currentFrame = 0
                } else {
                    iconBase.UkuiItems.Lottie.playing = true
                }
            }
            active(ShortcutPlugin.Click);
            event.accepted = true;
        }
    }
}
