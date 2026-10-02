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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQml.Models 2.15
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    property var focusGroups: []
    property bool inserted: false //是否插入了一个item
    property bool dragging: false //是否有item在被拖拽
    property int targetIndex      //拖拽目标位置
    property int indexInsert      //item在之前页面中的位置
    property int dragFromType     //之前页面的类型
    property string targetId      //拖拽目标id

    function refreshFocusItems() {
        focusGroups = addArea.flowId.focusGroups;
    }

    Connections {
        target: addArea.flowId
        function onFocusGroupsChanged() {
            refreshFocusItems();
        }
    }

    UkuiItems.Dialog {
        id: editWindow
        flags: Qt.WindowDoesNotAcceptFocus
        visualParent: panel
        enableWindowBlur: true
        mainItem: editWindowContaint
        type: Platform.WindowType.SystemWindow
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        minimumWidth: editWindowContaint.width
        maximumWidth: minimumWidth
        minimumHeight: editWindowContaint.height
        maximumHeight: minimumHeight

        location: {
            if (LayoutMirroring.enabled) {
                return mainWindow.panelPos === 3 ? UkuiItems.Types.RightEdge : UkuiItems.Types.LeftEdge;
            } else {
                return mainWindow.panelPos === 2 ? UkuiItems.Types.LeftEdge : UkuiItems.Types.RightEdge;
            }
        }
        margin: 8
        visible: dropArea.isEdit

        ExcludedPanel {
            id: editWindowContaint

            width: panel.width
            height: panel.height

            Connections {
                target: mainWindow
                function onPositionChanged(){
                    editWindow.changeEditWindowPosition();
                }
            }
        }

        function changeEditWindowPosition() {
            if (editWindow.visible) {
                editWindow.updatePosition();
            }
        }

        Component.onCompleted: {
            setStates(Platform.WindowState.Focusable | Platform.WindowState.Activatable, false);
        }
    }

    EditShortcutArea {
        id: addArea
        editType: PluginState.Add

        anchors.centerIn: parent
        width: flowId.width
        height: flowId.height

        Item {
            visible: (addArea.flowId.buttonModel.items.count === 0) && (!dropArea.isEdit)
            anchors.fill: parent
            UkuiItems.DtThemeText {
                property color argColor: Platform.GlobalTheme.kBrandNormal.pureColor
                property string argString: qsTr("enter editing mode")
                anchors.centerIn: parent
                width: contentWidth > parent.width ? parent.width : contentWidth
                textColor: Platform.GlobalTheme.kFontSecondary
                text: qsTr("There are currently no quick control options available, please %1 to add").arg('<font color="' + argColor + '">' + argString + '</font>')
                wrapMode: Text.Wrap

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        dropArea.isEdit = !dropArea.isEdit;
                    }
                }
            }
        }
    }

    Component.onCompleted: Qt.callLater(refreshFocusItems)

    //通用方法
    function insertItem(to, type, modelType) {
        if (modelType === PluginState.NotAdd) {
            addArea.flowId.buttonModel.items.insert(to, {itemType: type});
            addArea.flowId.changePosition();
        } else {
            editWindowContaint.area.flowId.buttonModel.items.insert(to, {itemType: type});
            editWindowContaint.changePosition();
        }
    }

    function removeItem(from, modelType) {
        let isLast = false;
        if (modelType === PluginState.NotAdd) {
            if (addArea.flowId.buttonModel.items.count - 1 === from) {
                isLast = true;
            }
            addArea.flowId.buttonModel.items.remove(from);
            addArea.flowId.changePosition();
        } else {
            if (editWindowContaint.area.flowId.buttonModel.items.count - 1 === from) {
                isLast = true;
            }
            editWindowContaint.area.flowId.buttonModel.items.remove(from);
            editWindowContaint.changePosition();
        }
        return isLast;
    }
}
