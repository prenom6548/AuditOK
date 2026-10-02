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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import AppControls2 1.0 as AppControls2
import org.ukui.menu.core 1.0

Item {
    property bool isAppListShow: appList.visible
    property bool isHeaderShow
    property Item focusToWidgetPage
    property Item focusToSearchInput

    function resetFocus() {
        if (appList.visible) {
            appList.resetListFocus();
        } else {
            selectionPage.focus = true;
            labelPage.focus = true;
        }
    }

    onFocusChanged: {
        if (focus) {
            if (isHeaderShow) {
                appListHeader.forceActiveFocus();
            } else {
                appList.forceActiveFocus();
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        AppListHeader {
            id: appListHeader
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            header: AppPageBackend.appModel.header
            KeyNavigation.tab: appList.visible ? appList : labelPage
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            AppList {
                id: appList
                anchors.fill: parent
                visible: !selectionPage.visible
                model: AppPageBackend.appModel
                KeyNavigation.tab: focusToWidgetPage
                KeyNavigation.backtab: isHeaderShow ? appListHeader : focusToSearchInput

                onFocusChanged: {
                    if (focus) resetFocus();
                }

                view.onContentYChanged: {
                    if (view.contentY <= 0) {
                        appListHeader.title = "";
                    } else {
                        appListHeader.title = view.currentSection;
                    }
                }

                function positionLabel(label) {
                    let index = model.findLabelIndex(label);
                    if (index >= 0) {
                        view.positionViewAtIndex(index, ListView.Beginning)
                    }
                }

                onLabelItemClicked: {
                    labelPage.labelBottle = AppPageBackend.appModel.labelBottle;
                    let labelIndex = labelPage.labelBottle.labels.findIndex(item => item.label === displayName);
                    if (labelIndex >= 0) {
                        labelPage.currentIndex = labelIndex;
                    }
                    selectionPage.state = "viewShowed";
                }
            }

            MouseArea {
                property int stateDuration: 300
                id: selectionPage
                anchors.fill: parent
                visible: false
                hoverEnabled: true
                onContainsMouseChanged: {
                    if (!labelPage.focus) return;
                    labelPage.showBorder = !containsMouse;
                }
                onPositionChanged: {
                    if (!labelPage.focus) return;
                    labelPage.showBorder = !containsMouse;
                }

                onClicked: state = "viewHid";

                state: "viewHid"
                states: [
                    State {
                        name: "viewShowed"
                        changes: [
                            PropertyChanges {target: selectionPage; scale: 1; opacity: 1; focus: true }
                        ]
                    },
                    State {
                        name: "viewHid"
                        changes: [
                            PropertyChanges {target: selectionPage; scale: 1.5; opacity: 0; focus: false }
                        ]
                    }
                ]

                transitions: [
                    Transition {
                        from: "*"; to: "viewShowed"
                        SequentialAnimation {
                            ScriptAction { script: selectionPage.visible = true; }
                            NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: selectionPage.stateDuration}
                            ScriptAction { script: labelPage.focus = true; }
                        }
                    },
                    Transition {
                        from: "*"; to: "viewHid"
                        SequentialAnimation {
                            NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: selectionPage.stateDuration}
                            ScriptAction {
                                script: {
                                    let shouldShowListBorder = labelPage.showLabelPage && labelPage.showBorder;
                                    selectionPage.visible = false;
                                    labelPage.labelBottle = null;
                                    labelPage.focus = false;
                                    appList.focus = true;
                                    appList.showBorder = shouldShowListBorder;
                                }
                            }
                        }
                    }
                ]

                AppLabelPage {
                    id: labelPage
                    anchors.centerIn: parent
                    interactive: height > parent.height
                    KeyNavigation.tab: focusToWidgetPage
                    KeyNavigation.backtab: isHeaderShow ? appListHeader : focusToSearchInput
                    onFocusChanged: {
                        if (selectionPage.containsMouse) return;
                        labelPage.showBorder = focus;
                    }

                    onLabelSelected: (label) => {
                        selectionPage.state = "viewHid";
                        appList.listFocus = true;
                        var index = appList.model.findLabelIndex(label);
                        if (index >= 0) {
                            appList.positionLabel(label);
                            appList.viewIndex = index;
                            appList.view.currentIndex = index;
                            appList.boldGroupName = label;
                            appList.currentGroupName = label;
                        }
                    }

                    onModelChanged: {
                        currentIndex = -1;
                    }

                    onShowLabelPageChanged: {
                        if (showLabelPage) {
                            selectionPage.stateDuration = 300;
                            return;
                        } else {
                            Qt.callLater(() => {
                                selectionPage.stateDuration = 0;
                                selectionPage.state = "viewHid";
                            });
                        }
                    }
                }

                function hidePage() {
                    state = "viewHid";
                    visible = false;
                }

                Component.onCompleted: {
                    mainWindow.windowVisibleChanged.connect(hidePage)
                }

                Component.onDestruction: {
                    mainWindow.windowVisibleChanged.disconnect(hidePage)
                }
            }
        }
    }
}
