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
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.12
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

BaseButton {
    id: buttonRoot

    activeFocusOnTab: false
    focusTargets: {
        var targets = [];
        if (iconClickable) targets.push(buttonIcon);
        if (sliderControl.enabled) targets.push(control);
        if (swipeIcon.visible) targets.push(swipeIcon);
        return targets;
    }
    property bool singleFocusTarget: focusTargets.length === 1
    property Item singleFocusItem: focusTargets.length === 1 ? focusTargets[0] : null
    property bool showSingleBorder: root.focusFrameVisible
                                    && buttonRoot.singleFocusTarget
                                    && buttonRoot.singleFocusItem
                                    && buttonRoot.singleFocusItem.activeFocus

    UkuiItems.DtThemeBackground {
        anchors.fill: parent
        radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax * 2 : Platform.GlobalTheme.kRadiusMax
        backgroundColor: Platform.GlobalTheme.kContainSecondaryAlphaNormal

        border.width: buttonRoot.showSingleBorder ? 2 : 1
        borderColor: buttonRoot.showSingleBorder ? Platform.GlobalTheme.kBrandFocus : Platform.GlobalTheme.kGrayAlpha1
        useStyleTransparency: false

        RowLayout {
            spacing: 8
            anchors.fill: parent
            anchors.leftMargin: mainWindow.isTabletMode ? 24 : 12
            anchors.rightMargin: mainWindow.isTabletMode ? 24 : 12

            FocusScope {
                id: buttonIcon
                Layout.preferredWidth: mainWindow.isTabletMode ? 32 : 24
                Layout.preferredHeight: mainWindow.isTabletMode ? 32 : 24
                Layout.alignment: Qt.AlignVCenter
                activeFocusOnTab: iconClickable

                Accessible.role: Accessible.Button
                Accessible.name: tooltip

                Keys.onPressed: {
                    if (iconClickable) {
                        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            active(ShortcutPlugin.Click);
                        }
                    }
                }

                UkuiItems.DtThemeBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.kGrayAlpha0
                    border.width: root.focusFrameVisible && buttonIcon.activeFocus && !buttonRoot.singleFocusTarget ? 2 : 0
                    borderColor: Platform.GlobalTheme.kBrandFocus
                    radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal

                    UkuiItems.Icon {
                        anchors.fill: parent
                        source: icon
                        mode: UkuiItems.Icon.AutoHighlight
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true;
                        enabled: iconClickable

                        UkuiItems.Tooltip {
                            anchors.fill: parent
                            posFollowCursor: true
                            active: parent.containsMouse
                            mainText: tooltip
                        }

                        onClicked: {
                            if (iconClickable) {
                                active(ShortcutPlugin.Click);
                            }
                        }
                    }
                }
            }

            FocusScope {
                id: sliderControl
                Layout.fillWidth: true;
                Layout.preferredHeight: mainWindow.isTabletMode ? 40 : 16;
                Layout.alignment: Qt.AlignVCenter;
                activeFocusOnTab: false
                enabled: !disabled;

                UkuiItems.TouchSlider {
                    id: control;
                    anchors.fill: parent
                    to: maxValue
                    value: buttonValue;
                    padding: 0;
                    stepSize:1;
                    activeFocusOnTab: true
                    backgroundColor: Platform.GlobalTheme.kContainSecondaryAlphaNormal;
                    borderColor: Platform.GlobalTheme.kGrayAlpha1;
                    enabled: sliderControl.enabled;

                    Accessible.role: Accessible.Slider
                    Accessible.name: tooltip

                    onPressedChanged: {
                        disableValueUpdate(pressed);
                        //松开鼠标重置buttonValue
                        if (!pressed) {
                            updateValue();
                        }
                    }

                    onValueChanged: {
                        setValue(value);
                    }

                    // 在鼠标点击后，还原焦点位置
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        onPressed: function(mouse) {
                            root.beginSliderPointerInteraction(mainWindow.activeFocusItem);
                            mouse.accepted = false;
                        }
                    }
                }

                UkuiItems.DtThemeBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.kGrayAlpha0
                    border.width: root.focusFrameVisible && control.activeFocus && !buttonRoot.singleFocusTarget ? 2 : 0
                    borderColor: Platform.GlobalTheme.kBrandFocus
                    radius: height / 2
                }
            }

            FocusScope {
                id: swipeIcon
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignVCenter
                activeFocusOnTab: false
                visible: parent.visible && (widgetId !== "") && !mainWindow.isTabletMode
                enabled: visible

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Go to sub‑page")

                UkuiItems.DtThemeBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.kGrayAlpha0
                    border.width: root.focusFrameVisible && swipeIcon.activeFocus && !buttonRoot.singleFocusTarget ? 2 : 0
                    borderColor: Platform.GlobalTheme.kBrandFocus
                    radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal

                    UkuiItems.Icon {
                        anchors.fill: parent
                        source: LayoutMirroring.enabled ? "ukui-start-symbolic" : "pan-end-symbolic"
                        mode: UkuiItems.Icon.AutoHighlight
                    }

                    MouseArea {
                        anchors.fill: parent;
                        onClicked: {
                            widgetPage.showWidget(widgetId, true, name, 250, false);
                        }
                    }


                }
                Keys.onPressed: {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        widgetPage.showWidget(widgetId, true, name, 250, true);
                    }
                }
            }
        }
    }
}
