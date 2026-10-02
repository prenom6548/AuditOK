/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtGraphicalEffects 1.12
import org.ukui.windowThumbnail 1.0
import org.ukui.quick.platform 1.0

Item {
    id: thumbnailSourceItem
    clip: true

    property string uuid
    property string outputId
    property bool fixHeight: false
    property bool fixWidth: false

    Loader {
        id: thumbnailLoader
        anchors.fill: parent
        visible: windowThumbnailConfig.realTimeThumbnailEnable
        sourceComponent: windowThumbnailConfig.kywlcomWindowThumbnailEnable
                         ? workspaceThumbnailComponent
                         : undefined

        Component {
            id: workspaceThumbnailComponent
            WorkspaceThumbnailItem {
                id: workspaceItem
                width: thumbnailLoader.width
                height: thumbnailLoader.height
                uuid: thumbnailSourceItem.uuid
                outputId: thumbnailSourceItem.outputId
                fixHeight: thumbnailSourceItem.fixHeight
                fixWidth: thumbnailSourceItem.fixWidth

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: workspaceItem.width
                        height: workspaceItem.height
                        Rectangle {
                            x: (workspaceItem.width - workspaceItem.paintedWidth) / 2
                            y: (workspaceItem.height - workspaceItem.paintedHeight) / 2
                            width: workspaceItem.paintedWidth
                            height: workspaceItem.paintedHeight
                            radius: GlobalTheme.kRadiusMin
                        }
                    }
                }
            }
        }
    }
}
