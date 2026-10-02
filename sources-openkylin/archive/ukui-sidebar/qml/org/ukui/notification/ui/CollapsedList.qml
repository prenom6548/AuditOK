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

import QtQuick 2.0
import QtQml 2.12
import QtQml.Models 2.12
import QtQuick.Layouts 1.12
import org.ukui.notification.ui 1.0
import org.ukui.notification.model 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: listRoot
    property bool busy: false
    property bool enableAnimation: false

    height: childrenRect.height
    clip: true

    Component.onCompleted: {
        groupCacheProxyModel.sourceModel = groupModel;
        groupCacheProxyModel.setRootIndex(groupCacheProxyModel.getRootIndex(model.groupIndex));
        groupDelegateModel.setSourceModel(groupCacheProxyModel);
    }

    Item {
        id: contentItem
        property int nextHeight: 0
        property int disableUpdateHeight: 0

        width: parent.width
        height: {
            if (disableUpdateHeight) {
                return height;
            }
            return nextHeight;
        }

        function moveItems(skipIndex) {
            for (let i = children.length - 1; i >= 0; --i) {
                if (i === skipIndex) {
                    continue;
                }

                let item = children[i];
                item.y = 0;
                if (item.height <= nextHeight) {
                    item.scale = 1;
                    item.opacity = 1;

                } else {
                    let scale = (nextHeight / item.height) - 0.1;
                    item.scale = scale
                    item.opacity = scale;
                }
            }
        }

        function findChildIndex(itemsIndex) {
            for (let i = children.length - 1; i >= 0; --i) {
                if (children[i].DelegateModel.itemsIndex === itemsIndex) {
                    return i;
                }
            }

            return -1;
        }

        function addItem(newItem) {
            listRoot.busy = true;

            let h = newItem.height;
            disableUpdateHeight = (h <= height);
            nextHeight = h;

            moveItems(-1);
            newItem.parent = this;
        }

        function itemAdded() {
            disableUpdateHeight = false;
            listRoot.busy = false;
        }

        function removeItem(itemsIndex) {
            let index = findChildIndex(itemsIndex);
            if (index !== -1) {
                listRoot.busy = true;
                disableUpdateHeight = true;
                let item = children[index];

                if (index === (children.length - 1)) {
                    let ci = findChildIndex(itemsIndex + 1);
                    if (ci > -1) {
                        nextHeight = children[ci].height;
                        disableUpdateHeight = (nextHeight <= height);
                        moveItems(index);
                    }
                }

                item.remove();
            }
        }

        function itemRemoved() {
            disableUpdateHeight = false;
            // 最上层的item
            nextHeight = childAt(width / 2, 5).height;
            moveItems(-1);
            listRoot.busy = false;
        }
    }

    DelegateModel {
        id: groupDelegateModel

        function setSourceModel(sourceModel) {
            if (sourceModel === undefined || sourceModel === null) {
                return;
            }

            model = sourceModel;
            model.itemRemoved.connect(onItemRemoved);
        }

        function onItemRemoved(index) {
            for (let i = 0; i < items.count; ++i) {
                if (index === i) {
                    let obj = items.get(i);
                    if (obj.inPersistedItems) {
                        contentItem.removeItem(obj.itemsIndex);

                    } else {
                        groupCacheProxyModel.removeItem(obj.model.id);
                    }
                    break;
                }
            }
        }

        items.onChanged: function (removed, inserted) {
            // Fixme: 添加多条消息的顺序问题
            for (let i = 0; i < inserted.length; ++i) {
                let start = inserted[i].index;
                let end = start + inserted[i].count - 1;

                while (start <= end) {
                    contentItem.addItem(items.create(end));
                    --end;
                }
            }
        }

        delegate: listItem
    }

    GroupCacheProxyModel {
        id: groupCacheProxyModel
    }

    Component {
        id: listItem
        Item {
            id: listItemRoot

            // 0: normal, 1: added, 2: removed
            property int flag: 1
            property var id: model.id
            property bool enableAnimation: false
            property bool animationControl: listRoot.enableAnimation && enableAnimation && itemLoader.active
            z: groupDelegateModel.items.count - listItemRoot.DelegateModel.itemsIndex

            width: contentItem.width
            height: childrenRect.height
            transformOrigin: Item.Top

            Component.onCompleted: {
                y = -height;
                opacity = 0.35;
            }

            function remove() {
                flag = 2;
                y = -height;
                opacity = 0.35;

                if (!animationControl) {
                    listItemDeleted();
                }
            }

            function listItemAdded() {
                listItemRoot.flag = 0;
                contentItem.itemAdded();
            }

            function listItemDeleted() {
                listItemRoot.parent = null;
                contentItem.itemRemoved();
                groupCacheProxyModel.removeItem(listItemRoot.id);
            }

            onParentChanged: {
                if (parent === contentItem) {
                    enableAnimation = true;
                    y = 0;
                    opacity = 1;

                    if (!animationControl) {
                        listItemAdded();
                    }

                } else {
                    enableAnimation = false;
                }
            }

            Behavior on y {
                enabled: animationControl
                SequentialAnimation {
                    NumberAnimation { duration: 500; easing.type: Easing.Bezier; easing.bezierCurve: [0.25, 0.1, 0, 1, 1, 1] }
                    ScriptAction {
                        script: {
                            if (listItemRoot.flag === 1) {
                                listItemRoot.listItemAdded();

                            } else if (listItemRoot.flag === 2) {
                                listItemRoot.listItemDeleted();
                            }
                        }
                    }
                }
            }

            Behavior on scale {
                enabled: animationControl
                NumberAnimation { duration: 500; easing.type: Easing.Bezier; easing.bezierCurve: [0.25, 0.1, 0, 1, 1, 1] }
            }

            Behavior on opacity {
                enabled: animationControl
                SequentialAnimation {
                    NumberAnimation { duration: 500; easing.type: Easing.Bezier;  }
                    ScriptAction {
                        script: { listItemRoot.opacity = 1; }
                    }
                }
            }

            Loader {
                id: itemLoader
                width: parent.width
                height: childrenRect.height
                active: listItemRoot.DelegateModel.itemsIndex < 5
                sourceComponent: PopupNotificationItem {
                    // width: parent.width
                    height: childrenRect.height
                    background.radius: 25
                    background.alpha: 1
                    background.borderAlpha: 0.1
                    background.border.width: listItemRoot.DelegateModel.itemsIndex < 2 ? 1 : 0

                    tabletTips.text: qsTr("%1 more notifications").arg(groupDelegateModel.count - 1)
                    tabletTips.visible: groupDelegateModel.count > 1
                    isTablet: true

                    onClicked: {
                        if (!listRoot.busy) {
                            sourceModel.execAction(listItemRoot.id, "default");
                        }
                    }

                    onActionExecuted: function (action) {
                        if (!listRoot.busy) {
                            sourceModel.execAction(listItemRoot.id, action);
                        }
                    }

                    onNotificationClosed: {
                        if (!listRoot.busy) {
                            sourceModel.closeNotification(listItemRoot.id);
                        }
                    }
                }
            }
        }
    }
}
