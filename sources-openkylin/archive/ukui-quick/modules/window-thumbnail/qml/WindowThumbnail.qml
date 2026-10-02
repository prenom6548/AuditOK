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
 * Authors: qiqi <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtGraphicalEffects 1.12
import org.ukui.windowThumbnail 1.0
import org.ukui.quick.platform 1.0

Item {
    id: thumbnailSourceItem
    clip: true

    property string winId
    property bool fixHeight: false
    property bool fixWidth: false

    Loader {
        id: thumbnailLoader
        height: parent.height
        width: parent.width
        visible: windowThumbnailConfig.realTimeThumbnailEnable
        sourceComponent: {
            if(windowThumbnailConfig.kywlcomWindowThumbnailEnable) {
                return kywlcomWindowThumbnailComonent;
            } else if(windowThumbnailConfig.xThumbnailEnable) {
                return x11ThumbnailComponent;
            }
        }

        Component {
            id: x11ThumbnailComponent
            XWindowThumbnail {
                id: xWindowItem
                winId: Number.parseInt(thumbnailSourceItem.winId)
                fixHeight: thumbnailSourceItem.fixHeight
                fixWidth: thumbnailSourceItem.fixWidth
                width: parent.width
                height: parent.height

                // 预览图圆角
                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: xWindowItem.width
                        height: xWindowItem.height
                        Rectangle {
                            x: (xWindowItem.width - xWindowItem.paintedWidth) / 2
                            y: (xWindowItem.height - xWindowItem.paintedHeight) / 2
                            width: xWindowItem.paintedWidth
                            height: xWindowItem.paintedHeight
                            radius: GlobalTheme.kRadiusMin
                        }
                    }
                }
            }
        }
        // Component {
        //     id: piperWireThumbnailComponent
        //     PipeWireThumbnail {
        //         fixHeight: thumbnailSourceItem.fixHeight
        //         fixWidth: thumbnailSourceItem.fixWidth
        //     }
        // }
        Component {
            id: kywlcomWindowThumbnailComonent
            WindowThumbnailItem {
                id: kywlcomItem
                uuid: thumbnailSourceItem.winId
                fixHeight: thumbnailSourceItem.fixHeight
                fixWidth: thumbnailSourceItem.fixWidth

                // 预览图圆角
                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: kywlcomItem.width
                        height: kywlcomItem.height
                        Rectangle {
                            x: (kywlcomItem.width - kywlcomItem.paintedWidth) / 2
                            y: (kywlcomItem.height - kywlcomItem.paintedHeight) / 2
                            width: kywlcomItem.paintedWidth
                            height: kywlcomItem.paintedHeight
                            radius: GlobalTheme.kRadiusMin
                        }
                    }
                }
            }
        }
    }
}
