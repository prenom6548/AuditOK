/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import org.ukui.quick.container 1.0
import org.ukui.panel.shell 1.0

MouseArea {
    id: root
    width: height * 16 / 56 //默认比例
    hoverEnabled: true
    Accessible.name: "ukui-panel_Island_drag_control_rod"
    property Item targetIsland: null
    property string container: "panel"
    property bool dragToPanel: true
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction
    drag.target: root
    Drag.mimeData: {
        "Id": targetIsland.WidgetContainer.id
    }

    ContentWindow {
        id: islandDropArea
        screen: targetIsland.WidgetContainer.screen
        position: Types.Bottom
        useAvailableGeometry: false
        flags: Qt.Window | Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint
        windowType: WindowType.SystemWindow
        visible: root.Drag.active
        enableBlurEffect: false

        DropArea {
            id: dropArea
            width: targetIsland.WidgetContainer.screen.size.width
            height: targetIsland.WidgetContainer.screen.size.height

            onPositionChanged: (dragEvent) =>{
                root.dragToPanel = dragEvent.y > height / 2;
                if(root.container === "panel") {
                    IslandDragHelper.IslandDragRemovingFromPanel(!dragToPanel);
                } else if(root.container === "topBar") {
                    IslandDragHelper.IslandDragRemovingFromTopBar(dragToPanel);
                }
            }
        }
    }

    onPressed: {
        Drag.hotSpot = mapToItem(targetIsland, mouse.x, mouse.y)
        dragImageSource.sourceItem = targetIsland;
        dragImageSource.grabToImage(function (result) {
            Drag.imageSource = result.url;
            dragImageSource.sourceItem = null;
        });
    }

    drag.onActiveChanged: {
        if(drag.active) {
            backgroundBase.visible = false;
            IslandDragHelper.draggingIsland = root.targetIsland.WidgetContainer.id;
            Drag.active = drag.active;
            IslandDragHelper.dragging = drag.active;
        } else {
            Drag.active = drag.active;
            IslandDragHelper.dragging = drag.active;
            if(root.container === "panel" &&!root.dragToPanel ) {
                IslandDragHelper.IslandDragRemoveFromPanel(root.targetIsland.WidgetContainer.id);
            } else if(root.container === "topBar" && root.dragToPanel) {
                IslandDragHelper.IslandDragRemoveFromTopBar(root.targetIsland.WidgetContainer.id);
            }
            IslandDragHelper.draggingIsland = "";
        }
    }
    onContainsMouseChanged: {
        backgroundBase.visible = containsMouse
    }
    Component.onCompleted: {
        backgroundBase.visible = containsMouse? !drag.active : false;
    }

    ShaderEffectSource {
        id: dragImageSource
        width: targetIsland.width
        height: targetIsland.height
        sourceItem: null
        visible: false;
    }

    DtThemeBackground {
        id: backgroundBase
        anchors.fill: parent

        backgroundColor: GlobalTheme.windowActive
        useStyleTransparency: false

        Icon {
            id: iconBase
            anchors.centerIn: parent
            rotation: 90
            mode: Icon.AutoHighlight
            source: "open-menu-symbolic"
            width: parent.width * 0.75
            height: parent.width * 0.75
        }
    }
}

