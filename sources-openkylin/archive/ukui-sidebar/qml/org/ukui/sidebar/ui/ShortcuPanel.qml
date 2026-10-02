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

FocusScope {
    id: panelRoot

    signal exitEdit()
    property var focusGroups: []

    activeFocusOnTab: false
    focus: true
    height: panelLayout.height + 38
    onExitEdit: {
        dropArea.isEdit = false;
    }

    //恢复焦点（焦点放在第一个可聚焦元素）
    function focusFirstItem() {
        rebuildFocusItems();
        if (focusGroups.length > 0 && focusGroups[0].length > 0) {
            focusGroups[0][0].forceActiveFocus();
        }
    }

    //重建焦点数据
    function rebuildFocusItems() {
        var validSpecs = validFocusGroupSpecs(focusGroupSpecs());
        clearFocusGroupNavigation(focusGroups);

        var rebuiltGroups = [];
        for (var i = 0; i < validSpecs.length; ++i) {
            var currentSpec = validSpecs[i];
            var nextSpec = validSpecs[(i + 1) % validSpecs.length];
            var previousSpec = validSpecs[(i + validSpecs.length - 1) % validSpecs.length];
            configureFocusGroup(currentSpec, nextSpec, previousSpec);
            rebuiltGroups.push(currentSpec.items);
        }

        focusGroups = rebuiltGroups;
    }

    function headerFocusGroups() {
        if (!loader.item) return [];

        if (dropArea.isEdit) {
            return loader.item.doneButton ? [[loader.item.doneButton]] : [];
        }
        return [[loader.item.editFocusTarget], [loader.item.settingsFocusTarget]];
    }

    //收集焦点项，并标记是否需要在组内使用Tab导航。
    function appendFocusGroupSpecs(specs, groups, usesTabNavigation) {
        for (var groupIndex = 0; groupIndex < groups.length; ++groupIndex) {
            specs.push({
                items: groups[groupIndex],
                usesTabNavigation: usesTabNavigation
            });
        }
    }

    //按当前页面状态收集焦点项
    function focusGroupSpecs() {
        var specs = [];
        appendFocusGroupSpecs(specs, headerFocusGroups(), false);

        if (!dropArea.isEdit) {
            appendFocusGroupSpecs(specs, dropArea.focusGroups, false);
            appendFocusGroupSpecs(specs, progressBarView.focusGroups, true);
        }
        return specs;
    }

    //过滤已销毁、不可见或不可用的焦点项
    function validFocusGroupSpecs(specs) {
        var validSpecs = [];
        for (var i = 0; i < specs.length; ++i) {
            var spec = specs[i];
            var sourceItems = spec.items;
            if (!sourceItems) {
                continue;
            }

            var validItems = [];
            for (var j = 0; j < sourceItems.length; ++j) {
                var focusItem = sourceItems[j];
                if (isValidFocusItem(focusItem)) {
                    validItems.push(focusItem);
                }
            }

            if (validItems.length > 0) {
                validSpecs.push({
                    items: validItems,
                    usesTabNavigation: spec.usesTabNavigation
                });
            }
        }
        return validSpecs;
    }

    function isValidFocusItem(item) {
        if (!item) return false;

        try {
            return item.visible && item.enabled && item.KeyNavigation;
        } catch (error) {
            return false;
        }
    }

    //清理上一轮焦点图中的导航关系。
    function clearFocusGroupNavigation(groups) {
        for (var groupIndex = 0; groupIndex < groups.length; ++groupIndex) {
            var group = groups[groupIndex];
            if (!group) {
                continue;
            }
            for (var itemIndex = 0; itemIndex < group.length; ++itemIndex) {
                clearKeyNavigation(group[itemIndex]);
            }
        }
    }

    function clearKeyNavigation(item) {
        try {
            if (!item || !item.KeyNavigation) {
                return;
            }
            item.KeyNavigation.tab = null;
            item.KeyNavigation.backtab = null;
            item.KeyNavigation.left = null;
            item.KeyNavigation.right = null;
        } catch (error) {
        }
    }

    //配置当前焦点组的Tab、Shift+Tab和方向键导航
    function configureFocusGroup(spec, nextSpec, previousSpec) {
        var group = spec.items;
        //跨组时进入下一组首项；上一组如果组内Tab切换，从其末项返回
        var nextTabItem = nextSpec.items[0];
        var previousGroup = previousSpec.items;
        var previousTabItem = previousSpec.usesTabNavigation
                              ? previousGroup[previousGroup.length - 1] : previousGroup[0];

        //progressbar先在组内逐项使用Tab，到两端后再跨组
        if (spec.usesTabNavigation) {
            for (var i = 0; i < group.length; ++i) {
                var focusItem = group[i];
                focusItem.KeyNavigation.tab = i + 1 < group.length
                                                ? group[i + 1] : nextTabItem;
                focusItem.KeyNavigation.backtab = i > 0
                                                    ? group[i - 1] : previousTabItem;
            }
            return;
        }

        //其他组的Tab直接跨组，组内使用左右方向键移动
        for (var j = 0; j < group.length; ++j) {
            var tabItem = group[j];
            tabItem.KeyNavigation.tab = nextTabItem;
            tabItem.KeyNavigation.backtab = previousTabItem;
        }

        for (var k = 0; k + 1 < group.length; ++k) {
            var firstItem = group[k];
            var secondItem = group[k + 1];
            //左右键适配镜像模式
            if (LayoutMirroring.enabled) {
                firstItem.KeyNavigation.left = secondItem;
                secondItem.KeyNavigation.right = firstItem;
            } else {
                firstItem.KeyNavigation.right = secondItem;
                secondItem.KeyNavigation.left = firstItem;
            }
        }
    }

    Connections {
        target: dropArea
        function onFocusGroupsChanged() {
            Qt.callLater(rebuildFocusItems);
        }
        function onIsEditChanged() {
            Qt.callLater(rebuildFocusItems);
        }
    }

    Connections {
        target: progressBarView
        function onFocusGroupsChanged() {
            Qt.callLater(rebuildFocusItems);
        }
    }

    ColumnLayout {
        id: panelLayout
        width: parent.width
        spacing: 16

        anchors.top: parent.top
        anchors.topMargin: 14
        anchors.bottomMargin: 24

        Item {
            Layout.preferredWidth: parent.width - 40
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignHCenter

            RowLayout {
                anchors.fill: parent
                spacing: 0

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    UkuiItems.DtThemeText {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                        Layout.alignment: Qt.AlignVCenter
                        pointSizeOffset: dropArea.isEdit ? 0 : 2
                        font.weight: dropArea.isEdit ? Font.Normal : Font.Bold

                        text: dropArea.isEdit ? qsTr("Added components") : qsTr("Shortcuts")
                        textColor: Platform.GlobalTheme.kFontPrimary
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                    }
                }

                Component {
                    id: editButtonArea

                    RowLayout {
                        property Item editFocusTarget: editButton
                        property Item settingsFocusTarget: sidebarSystemSetting

                        //kGrayAlpha0
                        property var kTransparent : Platform.GradientColor {
                            gradientDeg : 0
                            gradientStart : Qt.rgba(0/255, 0/255, 0/255, 0)
                            gradientEnd : Qt.rgba(0/255, 0/255, 0/255, 0)
                            gradientBackground : Qt.rgba(0/255, 0/255, 0/255, 0)
                        }
                        width: 68
                        height: 32
                        spacing: 4

                        UkuiItems.DtThemeButton {
                            id: editButton
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            activeFocusOnTab: true

                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("Edit")

                            background.radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal
                            background.border.width: root.focusFrameVisible && activeFocus ? 2 : 0
                            background.borderColor: Platform.GlobalTheme.kBrandFocus
                            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                                      : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                      : kTransparent

                            icon.source: editModel.editIconName()
                            icon.mode: UkuiItems.Icon.AutoHighlight

                            onClicked: {
                                dropArea.isEdit = !dropArea.isEdit;
                            }

                            Keys.onPressed: {
                                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                    dropArea.isEdit = !dropArea.isEdit;
                                }
                            }

                            UkuiItems.Tooltip {
                                anchors.fill: parent;
                                posFollowCursor: false;
                                mainText: qsTr("Click to enter editing mode");
                                location: UkuiItems.Dialog.BottomEdge;
                            }
                        }

                        UkuiItems.DtThemeButton {
                            id: sidebarSystemSetting
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            activeFocusOnTab: true

                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("System Settings")

                            icon.source: "system-settings-symbolic"
                            icon.mode: UkuiItems.Icon.AutoHighlight

                            background.border.width: root.focusFrameVisible && activeFocus ? 2 : 0
                            background.borderColor: Platform.GlobalTheme.kBrandFocus
                            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                                      : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                      : kTransparent
                            background.radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal

                            onClicked: {
                                appManager.launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", [], "ukui-control-center");
                            }

                            Keys.onPressed: {
                                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                    appManager.launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop",
                                                                      [],
                                                                      "ukui-control-center");
                                }
                            }

                            UkuiItems.Tooltip {
                                anchors.fill: parent;
                                posFollowCursor: false;
                                mainText: qsTr("System Settings");
                                location: UkuiItems.Dialog.BottomEdge;
                            }
                        }
                    }
                }

                Component {
                    id: backButton
                    FocusScope {
                        id: doneButton
                        width: 80
                        height: 32

                        activeFocusOnTab: true

                        UkuiItems.DtThemeBackground {
                            id: doneBackground
                            anchors.fill: parent
                            radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal
                            backgroundColor: control.containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                                   : control.containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                                           : Platform.GlobalTheme.kComponentAlphaNormal
                            border.width: root.focusFrameVisible && doneButton.activeFocus ? 2 : 1
                            borderColor: root.focusFrameVisible && doneButton.activeFocus ? Platform.GlobalTheme.kBrandFocus
                                                               : control.containsPress ? Platform.GlobalTheme.kLineComponentClick
                                                                                         : control.containsMouse ? Platform.GlobalTheme.kLineComponentHover
                                                                                                                 : Platform.GlobalTheme.kLineComponentNormal
                            UkuiItems.DtThemeText {
                                anchors.centerIn: parent
                                width: Math.min(implicitWidth, parent.width)
                                text: qsTr("Done")
                                textColor: Platform.GlobalTheme.kFontPrimary
                                pointSizeOffset: -2
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                            }
                        }

                        MouseArea {
                            id: control
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                dropArea.isEdit = !dropArea.isEdit;
                            }

                            UkuiItems.Tooltip {
                                anchors.fill: parent;
                                posFollowCursor: false;
                                mainText: qsTr("Click to exit editing mode");
                                location: UkuiItems.Dialog.BottomEdge;
                            }
                        }

                        Keys.onPressed: {
                            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                dropArea.isEdit = !dropArea.isEdit;
                            }
                        }
                    }
                }

                Loader {
                    id: loader
                    Layout.preferredWidth: item ? item.width : 0
                    Layout.preferredHeight: item ? item.height : 0
                    Layout.alignment: Qt.AlignVCenter

                    sourceComponent: dropArea.isEdit ? backButton : editButtonArea
                }
            }
        }

        EditButtonView {
            property bool isEdit: false
            id: dropArea
            Layout.preferredWidth: childrenRect.width
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignHCenter

            Connections {
                target: mainWindow
                function onVisibleChanged() {
                    if (!mainWindow.visible) dropArea.isEdit = false;
                }
            }
        }

        Item {
            Layout.preferredWidth: childrenRect.width
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignHCenter

            BaseButtonView {
                id: progressBarView
                anchors.centerIn: parent
                Layout.alignment: Qt.AlignHCenter
                columns: 1
                itemWidth: 356
                itemHeight: 56
                rowSpacing: 16
                buttoonType: ShortcutPlugin.ProgressBar
                buttonComponent: ProgressBar {}
            }
        }
    }

    Connections {
        target: loader
        function onLoaded() {
            Qt.callLater(rebuildFocusItems);
        }
    }

    Component.onCompleted: {
        Qt.callLater(rebuildFocusItems);
    }
}
