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

import AppControls2 1.0 as AppControls2
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.menu.core 1.0
import org.ukui.menu.utils 1.0
import org.ukui.menu.extension 1.0

Item {
    property Item focusToSidebar
    onFocusChanged: {
        if (focus) widgetList.forceActiveFocus();
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 5
        spacing: 5

        Item {
            // header
            Layout.fillWidth: true
            Layout.preferredHeight: 32

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 28
                spacing: 0

                ListView {
                    id: widgetList
                    property int selectIndex: -1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    KeyNavigation.tab: widgetLoader

                    clip: true
                    spacing: 0
                    interactive: false
                    orientation: ListView.Horizontal
                    function send(data) {
                        if (currentItem !== null) {
                            model.send(currentIndex, data);
                        }
                    }
                    onCurrentIndexChanged: {
                        if (currentItem !== null) {
                            currentItem.select();
                        }
                    }

                    onFocusChanged: {
                        selectIndex = focus ? 0 : -1
                    }
                    Keys.onPressed: {
                        switch (event.key) {
                        case Qt.Key_Left:
                            if (selectIndex > 0) {
                                selectIndex--;
                                event.accepted = true;
                            }
                            break;
                        case Qt.Key_Right:
                            if (selectIndex < widgetList.count - 1) {
                                selectIndex++;
                                event.accepted = true;
                            }
                            break;
                        case Qt.Key_Enter:
                        case Qt.Key_Return:
                            currentIndex = selectIndex;
                            event.accepted = true;
                            break;
                        default:
                            break;
                        }
                    }

                    model: WidgetModel {}
                    delegate: Component {
                        // 插件信息
                        UkuiItems.DtThemeBackground {
                            useStyleTransparency: false
                            backgroundColor: pluginMouseArea.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                                           : pluginMouseArea.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                                           : Platform.GlobalTheme.kContainGeneralAlphaNormal
                            radius: Platform.GlobalTheme.kRadiusMin
                            borderColor: Platform.GlobalTheme.kBrandFocus
                            border.width: index === widgetList.selectIndex ? 2 : 0

                            property var extensionData: model.data
                            property var extensionOptions: model.options
                            width: Math.max(56, styleText.implicitWidth + 12*2) // 文字左右边距为12
                            height: ListView.view ? ListView.view.height : 0

                            KeyNavigation.down: widgetLoader
                            Keys.onPressed: {
                                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                                    widgetList.currentIndex = model.index;
                                    EventTrack.sendClickEvent("switch_plugin", "Sidebar", {"plugin": model.name});
                                }
                            }

                            onExtensionDataChanged: {
                                if (widgetLoader.source === model.main) {
                                    widgetLoader.item.extensionData = extensionData;
                                }
                            }

                            Accessible.role: Accessible.Button
                            Accessible.name: model.name

                            function select() {
                                if (widgetLoader.source !== model.main) {
                                    widgetLoader.setSource(model.main, {extensionData: extensionData});
                                }
                            }

                            UkuiItems.DtThemeText {
                                id: styleText
                                height: parent.height
                                width: implicitWidth
                                anchors.centerIn: parent

                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.weight: Font.Medium

                                textColor: parent.ListView.isCurrentItem ? Platform.GlobalTheme.kFontPrimary: Platform.GlobalTheme.kFontPrimaryDisable
                                text: model.name
                            }

                            MouseArea {
                                id: pluginMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    widgetList.currentIndex = model.index;
                                    EventTrack.sendClickEvent("switch_plugin", "Sidebar", {"plugin": model.name});
                                }
                            }
                        }
                    }
                }

                Loader {
                    id: widgetMenuLoader
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }

        Loader {
            id: widgetLoader
            // 加载插件区域
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            KeyNavigation.tab: focusToSidebar

            onSourceChanged: {
                focus = false;
            }

            onLoaded: {
                item.send.connect(widgetList.send);
                // sidebarLayout.updateSidebarLayout(widgetList.currentItem.extensionOptions);
                updateMenu();
                item.extensionMenuChanged.connect(updateMenu);
            }

            function updateMenu() {
                if (item === null) {
                    return;
                }
                if (item.extensionMenu !== null) {
                    widgetMenuLoader.sourceComponent = item.extensionMenu;
                } else {
                    widgetMenuLoader.sourceComponent = undefined;
                }
            }
        }
    }

    Component.onCompleted: {
        if (widgetList.count > 0) {
            widgetList.currentIndex = 0;
        }
    }
}
