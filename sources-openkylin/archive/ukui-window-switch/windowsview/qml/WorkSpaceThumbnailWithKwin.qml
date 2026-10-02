/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.15
import org.ukui.windowThumbnail 1.0 as UkuiThumbnail
import AppWindowFilterModel 1.0

Item {
    id: root
    anchors.fill: parent
    property rect screenGeometry: desktopListModel.getFullScreenGeometry(desktopDragRect.desktopId, rootWindow.screenIndex)

    Repeater {
        id : desktopWindowRepeater
        model : AppWindowFilterModel {
            id: kwinWorkspaceThumbnailModel
            desktopId: desktopDragRect.desktopId
            screenId: rootWindow.screenIndex
            sourceModel: desktopListModel.isCurrentDesktopShowingDesktop() ? {} : appWindowListModel
        }

        delegate: UkuiThumbnail.XWindowThumbnail {
            width : Math.floor(model.Geometry.width * desktopScale)
            height : Math.floor(model.Geometry.height * desktopScale)
            visible : !(model.Minimized)
            x : Math.floor((model.Geometry.x - screenGeometry.x) * desktopScale)
            y : Math.floor((model.Geometry.y - screenGeometry.y) * desktopScale)
            // 改变窗口堆叠顺序：由于appWindowListModel中窗口顺序为最近打开顺序,因此最先绘制的窗口缩略图显示需要在最上层
            z : kwinWorkspaceThumbnailModel.rowCount() - index
            winId: model.Uuid
        }
    }
}
