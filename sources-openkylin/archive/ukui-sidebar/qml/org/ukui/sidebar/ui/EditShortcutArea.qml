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
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

DropArea {
    property alias flowId: shortcutFlow
    property int editType

    EditShortcutFlow {
        id: shortcutFlow
        modelType: editType
    }

    onPositionChanged: drag => {
                           if (drag.source.rootId !== dropArea) return;
                           let pos = mapToItem(shortcutFlow, drag.x, drag.y);
                           let targetInfo = getTarget(pos.x, pos.y);
                           let target = targetInfo.index;
                           let offset = targetInfo.offset;
                           if (target === -1) return;
                           if (drag.source.type === shortcutFlow.modelType) {
                               if (drag.source.selectIndex === target) return;
                               moveItem(drag.source.selectIndex, target, offset);
                           } else {
                               dropArea.dragFromType = drag.source.type;
                               if (dropArea.inserted) {
                                   var i = target
                                   if (moveItem(drag.source.selectIndex, target, offset)) {
                                       dropArea.targetIndex = i;
                                       drag.source.selectIndex = i;
                                   }
                               } else {
                                   dropArea.targetIndex = target;
                                   dropArea.inserted = true;
                                   dropArea.indexInsert = drag.source.selectIndex;
                                   drag.source.selectIndex = target;
                                   dropArea.insertItem(target, drag.source.buttonType, dropArea.dragFromType);
                               }
                           }
                       }

    onDropped: {
        if (dropArea.inserted) {
            let isLast = dropArea.removeItem(dropArea.targetIndex, dropArea.dragFromType);
            dropArea.inserted = false;
            dropArea.dragging = false;
            if (!isLast) {
                shortcutFlow.buttonModel.model.setOrder(dropArea.targetId, dropArea.targetIndex);
            } else {
                shortcutFlow.buttonModel.model.changeState(dropArea.targetId, dropArea.dragFromType);
            }
        }
    }

    onEntered: {
        shortcutFlow.isExit = false;
        if (drag.source.type !== shortcutFlow.modelType) {
            if (!dropArea.inserted) {
                let target = shortcutFlow.buttonModel.items.count;
                dropArea.dragFromType = drag.source.type;
                dropArea.targetId = drag.source.buttonId;
                dropArea.targetIndex = target;
                dropArea.inserted = true;
                dropArea.indexInsert = drag.source.selectIndex;
                drag.source.selectIndex = target;
                dropArea.insertItem(target, drag.source.buttonType, dropArea.dragFromType);
            }
        }
    }

    onExited: {
        if (dropArea.inserted) {
            dropArea.removeItem(dropArea.targetIndex, dropArea.dragFromType);
            dropArea.inserted = false;
            drag.source.selectIndex = dropArea.indexInsert;
        } else {
            shortcutFlow.isExit = true;
        }
    }

    function getTarget(x, y) {
        return shortcutFlow.buttonModel.model.findTargetItem(x, y);
    }

    function moveItem(from, to, offset) {
        if (offset === -1) {
            shortcutFlow.buttonModel.items.move(from, to);
            shortcutFlow.changePosition();
            return true;
        }
        if (from > to) {
            if (offset === 0) {
                shortcutFlow.buttonModel.items.move(from, to);
                shortcutFlow.changePosition();
                return true;
            }
        } else {
            if (offset === 1) {
                shortcutFlow.buttonModel.items.move(from, to);
                shortcutFlow.changePosition();
                return true;
            }
        }
        return false;
    }
}
