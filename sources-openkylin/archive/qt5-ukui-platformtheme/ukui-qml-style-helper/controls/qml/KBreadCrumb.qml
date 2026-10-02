/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KBreadCrumb。
 * 使用 KBreadCrumbItem；默认无背景交互（文字高亮），可开 backgroundVisible。
 */

import QtQuick 2.12
import QtQuick.Layouts 1.15
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

RowLayout {
    id: root

    enum KBreadCrumbType { FlatBreadCrumb = 0, CubeBreadCrumb = 1 }

    property int breadCrumbType: KBreadCrumb.FlatBreadCrumb
    // 支持 string，或 { text: "...", icon: "dialog-info" }
    property var tabs: []
    property int currentIndex: 0
    property string iconName: ""
    property bool flat: true
    property bool backgroundVisible: false
    property bool translucent: true

    spacing: 8
    implicitHeight: 36

    StylePrivate.APPParameter {
        id: app
        onParametryChanged: root._reloadIcons()
        onIconThemeChanged: root._reloadIcons()
    }

    signal currentChanged(int index)

    property int iconRev: 0
    function _reloadIcons() { iconRev++ }

    function _tabText(item) {
        if (typeof item === "object" && item !== null) {
            if (item.text !== undefined)
                return item.text
            if (item.title !== undefined)
                return item.title
            return ""
        }
        return item !== undefined && item !== null ? String(item) : ""
    }

    function _tabIconName(index, item) {
        if (typeof item === "object" && item !== null) {
            var ic = item.icon !== undefined ? item.icon
                   : (item.iconName !== undefined ? item.iconName : item.iconSource)
            if (ic !== undefined && ic !== null && String(ic) !== "")
                return String(ic)
        }
        if (index === 0 && root.iconName !== "")
            return root.iconName
        return ""
    }

    Repeater {
        model: root.tabs

        Private.KBreadCrumbItem {
            text: root._tabText(modelData)
            iconName: root._tabIconName(index, modelData)
            showSeparator: index < root.tabs.length - 1
            current: index === root.currentIndex
            flat: root.flat
            backgroundVisible: root.backgroundVisible
            translucent: root.translucent
            separatorInteractive: false
            expanded: false
            iconRev: root.iconRev
            onClicked: {
                root.currentIndex = index
                root.currentChanged(index)
            }
        }
    }

    function setIcon(name) { iconName = name }
    function setFlat(f) {
        flat = f
        breadCrumbType = f ? KBreadCrumb.FlatBreadCrumb : KBreadCrumb.CubeBreadCrumb
    }
    function setBackgroundVisible(v) { backgroundVisible = v }
    function setTranslucent(v) { translucent = v }
}
