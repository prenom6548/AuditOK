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
import QtQuick.Layouts 1.12
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

BaseButton {
    property bool isButtonHover: (baseArea.containsMouse && !posInCirle)
                                 || nameTooltip.containsMouse
                                 || statusNameTooltip.containsMouse
    property bool isButtonPress: baseArea.containsPress && !posInCirle
    property bool isIconHover: baseArea.containsMouse && posInCirle
    property bool isIconPress: baseArea.containsPress && posInCirle
    property bool posInCirle: false
    property bool posInIconArea: false
    property bool posInNameTooltip: false
    property bool posInStatusNameTooltip: false

    activeFocusOnTab: false
    focusTargets: widgetId === "" ? [buttonIcon] : [buttonIcon, swipeIcon]

    UkuiItems.DtThemeBackground {
        anchors.fill: parent
        radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax * 2 : Platform.GlobalTheme.kRadiusMax
        backgroundColor: dragMask ? Platform.GlobalTheme.kGray6
                                  : isButtonPress ? Platform.GlobalTheme.kGrayAlpha3
                                                  : isButtonHover ? Platform.GlobalTheme.kGrayAlpha2
                                                                  : Platform.GlobalTheme.kGrayAlpha1
        border.width: 1
        borderColor: Platform.GlobalTheme.kGrayAlpha1
        useStyleTransparency: false

        RowLayout {
            id: rowLayout
            anchors.fill: parent
            anchors.leftMargin: mainWindow.isTabletMode ? 24 : 10
            anchors.rightMargin: 4
            spacing: 8

            FocusScope {
                id: buttonIcon
                Layout.preferredWidth: mainWindow.isTabletMode ? 64 : 48
                Layout.preferredHeight: mainWindow.isTabletMode ? 64 : 48
                Layout.alignment: Qt.AlignVCenter
                activeFocusOnTab: true

                Accessible.role: Accessible.Button
                Accessible.name: name

                UkuiItems.DtThemeBackground {
                    id: iconBackground
                    anchors.fill: parent
                    useStyleTransparency: false
                    radius: width/2
                    backgroundColor: buttonColor === PluginColorRole.HighLight ? menuHighLightBackGroundColor()
                                                                               : menuNormalBackGroundColor()
                    border.width: root.focusFrameVisible && buttonIcon.activeFocus ? 2 : 1
                    borderColor: root.focusFrameVisible && buttonIcon.activeFocus ? Platform.GlobalTheme.kBrandFocus
                                                        : buttonColor === PluginColorRole.HighLight ? Platform.GlobalTheme.kGrayAlpha1
                                                                                                   : menuNormalBorderColor()

                    function menuHighLightBackGroundColor() {
                        return isIconPress ? Platform.GlobalTheme.kBrandClick
                                           : isIconHover ? Platform.GlobalTheme.kBrandHover
                                                         : Platform.GlobalTheme.kBrandNormal
                    }

                    function menuNormalBackGroundColor() {
                        return isIconPress ? Platform.GlobalTheme.kGrayAlpha3
                                           : isIconHover ? Platform.GlobalTheme.kGrayAlpha2
                                                         : Platform.GlobalTheme.kGrayAlpha1
                    }

                    function menuNormalBorderColor() {
                        return isIconPress ? Platform.GlobalTheme.kGrayAlpha3
                                           : isIconHover ? Platform.GlobalTheme.kGrayAlpha2
                                                         : Platform.GlobalTheme.kGrayAlpha1
                    }

                    UkuiItems.Icon {
                        id: iconBase
                        width: parent.width/2
                        height: parent.width/2
                        anchors.centerIn: parent
                        source: icon
                        // mode: UkuiItems.Icon.AutoHighlight
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
            }

            ColumnLayout {
                id: columnLayout
                Layout.preferredWidth: parent.width - buttonIcon.width - swipeIcon.width - parent.spacing * 2
                Layout.fillHeight: true
                spacing: 2

                UkuiItems.DtThemeText {
                    id: nameText
                    property bool posInName: false
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: (parent.height - parent.spacing) / 2
                    text: name
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    textColor: Platform.GlobalTheme.kFontPrimary
                    UkuiItems.Tooltip {
                        id: nameTooltip
                        anchors.fill: parent
                        posFollowCursor: true
                        mainText: name
                        active: parent.truncated
                    }

                    onPosInNameChanged: {
                        if (posInName) {
                            nameTooltip.showTooltip();
                        } else {
                            nameTooltip.hideImmediately();
                        }
                    }
                }

                UkuiItems.DtThemeText {
                    id: statusNameText
                    property bool posInstatusName: false
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: (parent.height - parent.spacing) / 2
                    text: statusName
                    pointSizeOffset: -1
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    textColor: Platform.GlobalTheme.kFontPrimary
                    UkuiItems.Tooltip {
                        id: statusNameTooltip
                        anchors.fill: parent
                        posFollowCursor: true
                        mainText: statusName
                        active: parent.truncated
                    }

                    onPosInstatusNameChanged: {
                        if (posInstatusName) {
                            statusNameTooltip.showTooltip();
                        } else {
                            statusNameTooltip.hideImmediately();
                        }
                    }
                }
            }

            FocusScope {
                id: swipeIcon
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                activeFocusOnTab: false
                visible: widgetId === "" ? false : true
                enabled: visible

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Go to sub‑page")

                UkuiItems.DtThemeBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.kGrayAlpha0
                    border.width: root.focusFrameVisible && swipeIcon.activeFocus ? 2 : 0
                    borderColor: Platform.GlobalTheme.kBrandFocus
                    radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal

                    UkuiItems.Icon {
                        anchors.fill: parent
                        source: LayoutMirroring.enabled ? "ukui-start-symbolic" : "pan-end-symbolic"
                        mode: UkuiItems.Icon.AutoHighlight
                    }
                }
            }
        }

        MouseArea {
            id: baseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                let iconPoint = baseArea.mapFromItem(columnLayout, 0, 0);
                posInIconArea = LayoutMirroring.enabled ? (mouse.x > iconPoint.x + columnLayout.width + 3) : (mouse.x < iconPoint.x - 3);

                if (posInIconArea) {
                    if (iconBase.sourceType === UkuiItems.Icon.LottieSource) {
                        if (iconBase.UkuiItems.Lottie.playing) {
                            iconBase.UkuiItems.Lottie.currentFrame = 0
                        } else {
                            iconBase.UkuiItems.Lottie.playing = true
                        }
                    }
                    active(ShortcutPlugin.Click);
                } else {
                    widgetPage.showWidget(widgetId, true, name, 250, false);
                }
            }
            onPressed: {
                nameText.posInName = false;
                statusNameText.posInstatusName = false;
            }
            onPositionChanged: {
                if (pressed) {
                    nameText.posInName = false;
                    statusNameText.posInstatusName = false;
                } else {
                    let nameRect = baseArea.mapFromItem(nameText, Qt.rect(0, 0, nameText.width, nameText.height));
                    let statusNameRect = baseArea.mapFromItem(statusNameText, Qt.rect(0, 0, statusNameText.width, statusNameText.height));
                    nameText.posInName = mainWindow.inTargetRect(mouse.x, mouse.y, nameRect);
                    statusNameText.posInstatusName = mainWindow.inTargetRect(mouse.x, mouse.y, statusNameRect);
                }

                let iconPoint = baseArea.mapFromItem(columnLayout, 0, 0);
                let circlePoint = baseArea.mapFromItem(buttonIcon, buttonIcon.width/2, buttonIcon.height/2);
                posInIconArea = LayoutMirroring.enabled ? (mouse.x > iconPoint.x + columnLayout.width + 3) : (mouse.x < iconPoint.x - 3);
                posInCirle = mainWindow.inTargetCircle(mouse.x, mouse.y, circlePoint.x, circlePoint.y, buttonIcon.width/2);
            }
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (buttonIcon.activeFocus) {
                if (iconBase.sourceType === UkuiItems.Icon.LottieSource) {
                    if (iconBase.UkuiItems.Lottie.playing) {
                        iconBase.UkuiItems.Lottie.currentFrame = 0
                    } else {
                        iconBase.UkuiItems.Lottie.playing = true
                    }
                }
                active(ShortcutPlugin.Click);
                event.accepted = true;
            } else if (swipeIcon.activeFocus) {
                widgetPage.showWidget(widgetId, true, name, 250, true);
                event.accepted = true;
            }
        }
    }
}
