/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KHLineFrame。
 * 对齐 klineframe.cpp：水平 1px 分隔线，色 kline-normal，固定高 1；
 * setFollowPalette(false) 冻结当前色（不再随主题变化），true 恢复随主题。
 */

import QtQuick 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate

Rectangle {
    id: root

    property bool followPalette: true

    implicitHeight: 1
    height: 1

    StylePrivate.ParseColorInterface { id: parseColor }
    StylePrivate.APPParameter { id: app }

    Component.onCompleted: root.color = parseColor.getDtColor("kline-normal")

    // 主题变化：仅 followPalette 时刷新；false 时保持当前色（冻结，对齐 C++ changeTheme 守卫）
    Connections {
        target: app
        function onParametryChanged() {
            if (root.followPalette)
                root.color = parseColor.getDtColor("kline-normal")
        }
    }

    onFollowPaletteChanged: {
        if (followPalette)
            root.color = parseColor.getDtColor("kline-normal")
        // 置 false：不动 color，冻结当前值
    }

    function setFollowPalette(f) { followPalette = f }
}
