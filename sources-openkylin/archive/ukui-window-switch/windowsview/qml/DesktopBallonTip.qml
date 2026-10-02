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
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */
import QtQuick 2.15

import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

import DesktopListModel 1.0

KBallonTip {
    id: ballonTip
    enum PanelPos { Bottom, Top, Left, Right }
    property string tipText: ""
    property int marginToPanel: 36
    iconType: KBallonTip.IconType.None
    leftMargin: marginToPanel
    topMargin: marginToPanel
    rightMargin: marginToPanel
    bottomMargin: marginToPanel
    position: tipPos()
    text: tipText
    screen: multitaskViewManager.currentScreen

    Connections {
        target: desktopListModel
        function onDataChanged() {
            updateTipText()
        }
        function onRowsRemoved() {
            updateTipText()
        }
        // 修复拖拽工作区交换位置后，立即点击工作区，提示内容与当前工作区名称不一致问题
        function onRowsMoved() {
            updateTipText()
        }
        function onCurrentDesktopChanged() {
            updateTipText()
        }
    }

    Connections {
        target: multitaskViewManager
        function onShowDesktopTipSignal() {
            if (ballonTip.windowVisible) {
                ballonTip.hide()
            }
            ballonTip.popup()
        }
    }

    Connections {
        target: multitaskViewManager
        function onWinTabVisibleChanged() {
            if (ballonTip.windowVisible && multitaskViewManager.winTabVisible) {
                ballonTip.hide()
            }
        }
    }

    function tipPos() {
        if (multitaskViewManager.desktopTipPos === DesktopBallonTip.PanelPos.Top) {
            return Types.TopCenter
        } else if (multitaskViewManager.desktopTipPos === DesktopBallonTip.PanelPos.Left) {
            return Types.LeftCenter
        } else if (multitaskViewManager.desktopTipPos === DesktopBallonTip.PanelPos.Right) {
            return Types.RightCenter
        } else {
            return Types.BottomCenter
        }
    }

    function updateTipText() {
        tipText = desktopListModel.getCurrentDesktopName()
    }

}
