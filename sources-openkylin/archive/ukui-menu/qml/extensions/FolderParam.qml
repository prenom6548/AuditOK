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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */
import QtQml 2.15
import QtQuick 2.15

QtObject {
    property bool isFullScreen: false
    property int viewRow: 1
    property real ratio: 1.0

    // 应用组底色
    // 2026.02.07: 根据最新设计稿调整，小屏宽度从388调整为372
    property int backgroundWidth: isFullScreen ? 728 * ratio : 372 * ratio
    // 全屏模式下, 应用组高度已经包含拖拽提示条的大小
    property int backgroundHeight: isFullScreen ? folderIconBase.height : folderIconBase.height + dragHintBarHeight * 2
    property int dragHintBarHeight: isFullScreen ? 32 * ratio : 20 * ratio
    property int dragHintBarIconSize: isFullScreen ? 32 * ratio : 16 * ratio
    property int itemIconSize: isFullScreen ? 69 * ratio : 48 * ratio

    // 展开后的应用组相关参数 （展开后的应用组cellWidth和cellHeight是根据外部参数确定的）
    // Height: isFullScreen ? (itemHeight: 160 + spacing: 16 + margins: 24) : (itemHeight:96 + spacing:0)
    property rect folderIconBase: isFullScreen ? Qt.rect(0, 0, Math.ceil(720 * ratio), Math.ceil(viewRow*176 * ratio + folderContentMargins*2)) :
                                                    Qt.rect(0, Math.ceil(82 * ratio), Math.ceil(336 * ratio), Math.ceil(viewRow * 96 * ratio))
    property int folderContentMargins: isFullScreen ? 12 * ratio : 0
    property int folderLabelSpacing: isFullScreen ? 8 * ratio : 2 * ratio
    property int folderLabelMagrins: isFullScreen ? 8 * ratio : 2 * ratio

    // 收起后的应用组相关参数
    property rect normalIconBase: isFullScreen ? Qt.rect(0, 0, 84, 84) : Qt.rect(0, 0, 40, 40)
    property int iconContentMargins: isFullScreen ? 8 * ratio : 3 * ratio
    property int iconLabelSpacing: 0
    property int iconLabelMagrins: 0

    // 页码相关参数
    property int pageIndicatorSize: isFullScreen ? 16 * ratio : 12 * ratio
    property int pageIndicatorSizeContainMouse: isFullScreen ? 12 * ratio : 8 * ratio
    property int pageIndicatorMargin: isFullScreen ? 14 * ratio : 6 * ratio

    // 应用组标题相关参数
    property int folderNameMargin: isFullScreen ? 12 * ratio : 30 * ratio
    property int folderNameHeight: isFullScreen ? 72 * ratio : 47 * ratio
}
