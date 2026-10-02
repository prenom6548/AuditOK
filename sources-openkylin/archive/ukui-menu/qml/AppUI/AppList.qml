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

import QtQuick 2.15
import QtQml 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import AppControls2 1.0 as AppControls2
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.menu.core 1.0

AppListView {
    id: appListView
    signal labelItemClicked(string displayName)
    property bool isTouch: false

    function resetListFocus() {
        listFocus = true;
    }

    onContainsMouseChanged: {
        showBorder = false;
    }
    onPositionChanged: {
        showBorder = false;
    }

    Item {id: dragItem}

    delegate: Component {
        Loader {
            focus: true
            width: ListView.view ? ListView.view.width : 0
            height: appListView.itemHeight

            property int index: model.index
            property int type: model.type
            property string id: model.id
            property string name: model.name
            property string icon: model.icon
            property string comment: model.comment // label tooltip
            property string desktopName: model.desktopName
            property bool favorite: model.favorite
            property string group: model.group

            sourceComponent: Component {
                Item {
                    id: appItemBase

                    Accessible.role: Accessible.Button
                    Accessible.name: name

                    AppControls2.AppItem {
                        id: appItem
                        focus: true
                        width: appListView.view ? appListView.view.width : 0
                        height: appListView.itemHeight
                        anchors.centerIn: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        isRecentInstalled: model.recentInstall
                        appItemShowBorder: appListView.appItemShowBorder && appListView.showBorder

                        Drag.hotSpot.x: 48 / 2
                        Drag.hotSpot.y: 48 / 2
                        Drag.supportedActions: Qt.CopyAction
                        Drag.dragType: Drag.Automatic
                        Drag.mimeData: {
                            "source": "ukui-menu",
                            "url": "app://" + id,
                            "favorite": favorite
                        }
                        drag.target: appListView.isTouch ? null : dragItem
                        drag.onActiveChanged: {
                            Drag.active = drag.active;
                        }

                        Drag.onDragFinished: function (dropAction) {
                            appItem.Drag.active = false;
                        }

                        onClicked: {
                            console.log("ICON_CLICK!", "DESKTOPFILE:", id);
                            if (mouse.button === Qt.LeftButton) {
                                appManager.launchApp(id);
                                return;
                            }
                        }

                        onPressed: {
                            if (mouse.button === Qt.RightButton) {
                                if (appListView.isTouch) {
                                    appListView.isTouch = false;
                                    Drag.active = false;
                                    Drag.cancel();
                                }
                                Qt.callLater(() => {
                                    appListView.model.openMenu(index, MenuInfo.AppList);
                                });
                                return;
                            }

                            normalUI.focus = false;
                            // 在启动拖拽前进行截图，避免开始拖拽后没有图标
                            grabImage();
                            if (mouse.source === Qt.MouseEventSynthesizedBySystem ||
                                    mouse.source === Qt.MouseEventSynthesizedByQt) {
                                appListView.isTouch = true;
                            } else {
                                appListView.isTouch = false;
                            }
                        }

                        onPressAndHold: {
                            // 触摸长按时启动拖拽
                            if (isTouch) {
                                Drag.active =  true;
                            }
                        }
                    }

                    function grabImage() {
                        var icon = mouseGrabImage.createObject(appItem, {width: 48, height: 48})
                        if (icon !== null) {
                            icon.addUnifiedIconBox = true
                            icon.source = model.icon;
                            icon.grabToImage(function(result) {
                                appItem.Drag.imageSource = result.url
                                icon.destroy();
                            });
                            icon.visible = false;
                        }
                    }

                    Component {
                        id: mouseGrabImage
                        UkuiItems.Icon { }
                    }


                    Keys.onPressed: {
                        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            if (!showBorder) return;
                            if (!appListView.appItemShowBorder && boldGroupName !== "") {
                                labelItemClicked(boldGroupName);
                                return;
                            }
                            appManager.launchApp(id);
                        }
                    }
                }
            }
        }
    }

    view.section.property: "group"
    view.section.delegate: Component {
        AppControls2.LabelItem {
            width: ListView.view.width
            height: appListView.itemHeight
            focus: true
            displayName: section
            labelItemShowBorder: displayName === boldGroupName && appListView.showBorder
            onClicked: labelItemClicked(displayName);
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    labelItemClicked(displayName);
                }
            }
        }
    }
}
