/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KVLineFrame.
 * 对齐 klineframe.cpp：垂直 1px 分隔线，色 kline-normal，固定宽 1；
 * setFollowPalette(false) 冻结当前色，true 恢复随主题变化。
 */

import QtQuick 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate

Rectangle {
    id: root

    property bool followPalette: true

    implicitWidth: 1
    width: 1

    StylePrivate.ParseColorInterface { id: parseColor }
    StylePrivate.APPParameter { id: app }

    Component.onCompleted: root.color = parseColor.getDtColor("kline-normal")

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
    }

    function setFollowPalette(f) { followPalette = f }
}
