/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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
import AppUI 1.0 as AppUI
import AppControls2 1.0 as AppControls2
import org.ukui.menu.core 1.0
import org.ukui.menu.utils 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: root
    clip: true
    property int animationDuration: menuSetting.get("animationDuration")
    property var normalGeometry: mainWindow.normalRect
    property string currentSearchText: ""
    property bool focusOnFoldButton: false

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true;

    Keys.onEscapePressed: {
        if (mainWindow.windowVisible) {
            panelFocusBridge.scheduleRestorePanelFocusOnHide();
            mainWindow.activeMenuWindow(false);
        }
    }

    MaskMouseArea {
        z: 1
        anchors.fill: parent
        acceptedButtons: {
            let button = Qt.NoButton;
            if (functionControl.disableMouseLeftButton) {
                button |= Qt.LeftButton
            }
            if (functionControl.disableMouseRightButton) {
                button |= Qt.RightButton
            }
            return button;
        }
        enabled: functionControl.disableMouseLeftButton | functionControl.disableMouseRightButton
    }

    Connections {
        target: mainWindow
        function onWindowVisibleChanged() {
            root.currentSearchText = "";
        }
    }

    Component.onCompleted: {
        mainWindow.fullScreenChanged.connect(switchMenuState);
        menuSetting.changed.connect(updateAnimationDuration);
    }

    function switchMenuState() {
        if (mainWindow.isFullScreen) {
            fullScreen.visible = true;
            normalScreen.visible = false;
            fullScreen.forceActiveFocus();
        } else {
            normalScreen.visible = true;
            fullScreen.visible = false;
            normalScreen.forceActiveFocus();
        }
    }

    function updateAnimationDuration(key) {
        if (key === "animationDuration") {
            root.animationDuration = menuSetting.get("animationDuration");
        }
    }

    UkuiItems.DtThemeBackground {
        id: backgroundMask
        x: 0
        y: 0
        height: root.height
        width: root.width
        radius: 0

        border.width: 0
        //TODO
        borderColor: Platform.GlobalTheme.textActive
        borderAlpha: 0.15
    }

    AppUI.NormalUI {
        id: normalScreen
        x: 0; y: 0
        width: parent.width; height: parent.height

        Keys.onPressed: {
            keyPressed(event);
        }

        Component.onCompleted: {
            visible = !mainWindow.isFullScreen;
        }
    }

    AppUI.FullScreenUI {
        id: fullScreen
        x: 0; y: 0
        width: parent.width; height: parent.height

        Keys.onPressed: {
            keyPressed(event);
        }

        Component.onCompleted: {
            visible = mainWindow.isFullScreen;
        }
    }
}
