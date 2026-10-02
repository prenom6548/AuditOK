/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KDirTabBar。
 * 对齐 C++ 交互：
 *  - setCurrentPath(path) 按路径拆段重建按钮
 *  - 点正文 → setCurrentPath(该段 uri) + tabBarClicked(uri)
 *  - 点子目录菜单 → setCurrentPath(子路径) + tabBarClicked(uri)
 *  - 箭头仅在有子目录时显示；菜单打开时为 ukui-down-symbolic
 *  - 默认通过 KyDirHelper 读取真实文件系统子目录（QDirIterator）
 *  - 可选 childrenProvider 覆盖默认列举（测试用）
 */

import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

RowLayout {
    id: root

    property string currentPath: ""
    property int visibleDepth: 5
    // 对齐 C++ textLength：显示名超过该字符数则省略
    property int textLength: 5
    property bool flat: true
    property bool backgroundVisible: true
    property bool translucent: true
    // 可选：function(path) → [{ text, icon?, uri? }, ...]；未设置则读真实文件系统
    property var childrenProvider: null

    readonly property string homePath: dirHelper.homePath

    // 当前路径拆出的段 [{ text, displayText, uri, icon, menuItems }]
    property var _segments: []
    readonly property int segmentCount: _segments.length
    property int expandedIndex: -1
    // setIcon(1-based) 覆盖；key 为 1-based 下标字符串
    property var _iconOverrides: ({})

    spacing: 8
    implicitHeight: 36

    Private.KyDirHelper { id: dirHelper }

    StylePrivate.APPParameter {
        id: app
        onParametryChanged: root._reloadIcons()
        onIconThemeChanged: root._reloadIcons()
    }

    signal tabBarClicked(string uri)

    property int iconRev: 0
    function _reloadIcons() { iconRev++ }

    function _normalizePath(path) {
        return dirHelper.normalizePath(String(path === undefined || path === null ? "" : path))
    }

    function _parentPath(path) {
        var p = _normalizePath(path)
        if (p === "" || p === "/")
            return "/"
        var slash = p.lastIndexOf("/")
        if (slash <= 0)
            return "/"
        return p.substring(0, slash)
    }

    function _baseName(path) {
        var p = _normalizePath(path)
        if (p === "" || p === "/")
            return p
        var slash = p.lastIndexOf("/")
        return p.substring(slash + 1)
    }

    function _joinPath(parent, name) {
        var p = _normalizePath(parent)
        var n = String(name)
        if (p === "/" || p === "")
            return "/" + n
        return p + "/" + n
    }

    function _elideName(name) {
        var s = String(name)
        if (root.textLength <= 0 || s.length <= root.textLength)
            return s
        return s.substring(0, root.textLength) + "…"
    }

    function _defaultFolderIcon(name) {
        switch (String(name)) {
        case "图片": return "folder-pictures"
        case "下载": return "folder-download"
        case "公共的": return "folder-publicshare"
        case "文档": return "folder-documents"
        case "桌面": return "user-desktop"
        case "模板": return "folder-templates"
        case "视频": return "folder-videos"
        case "音乐": return "folder-music"
        default: return "folder"
        }
    }

    function _normalizeChild(parentUri, item) {
        if (typeof item === "string") {
            return {
                text: item,
                uri: root._joinPath(parentUri, item),
                icon: root._defaultFolderIcon(item)
            }
        }
        if (typeof item !== "object" || item === null)
            return null
        var text = item.text !== undefined ? item.text
                  : (item.title !== undefined ? item.title : item.name)
        text = text !== undefined && text !== null ? String(text) : ""
        var uri = item.uri !== undefined && item.uri !== null && String(item.uri) !== ""
                ? root._normalizePath(item.uri)
                : root._joinPath(parentUri, text)
        var icon = item.icon !== undefined ? item.icon
                 : (item.iconName !== undefined ? item.iconName : root._defaultFolderIcon(text))
        return { text: text, uri: uri, icon: String(icon || "folder") }
    }

    function _childrenOf(path) {
        var list = []
        var raw
        if (typeof root.childrenProvider === "function")
            raw = root.childrenProvider(path)
        else
            raw = dirHelper.listSubDirectories(path)
        if (!raw)
            return list
        for (var i = 0; i < raw.length; i++) {
            var c = root._normalizeChild(path, raw[i])
            if (c)
                list.push(c)
        }
        return list
    }

    // 对齐 C++ setCurrentPath：自路径向上拆段，不含 "/" 本身
    function _segmentsFromPath(path) {
        var p = root._normalizePath(path)
        var segs = []
        if (p === "" || p === "/")
            return segs
        var tmp = p
        while (tmp !== "/" && tmp !== "") {
            var name = root._baseName(tmp)
            segs.unshift({
                text: name,
                displayText: root._elideName(name),
                uri: tmp,
                icon: root._defaultFolderIcon(name),
                menuItems: []
            })
            tmp = root._parentPath(tmp)
        }
        return segs
    }

    function _rebuild() {
        root.expandedIndex = -1
        var segs = root._segmentsFromPath(root.currentPath)
        for (var i = 0; i < segs.length; i++) {
            var key = String(i + 1)
            if (root._iconOverrides[key] !== undefined && root._iconOverrides[key] !== "")
                segs[i].icon = root._iconOverrides[key]
            segs[i].menuItems = root._childrenOf(segs[i].uri)
        }
        root._segments = segs
        root._reloadIcons()
    }

    function setCurrentPath(path) {
        var p = root._normalizePath(path)
        if (root.currentPath !== p)
            root.currentPath = p
        else
            root._rebuild()
    }

    onCurrentPathChanged: root._rebuild()
    onVisibleDepthChanged: root._reloadIcons()
    onTextLengthChanged: root._rebuild()
    onChildrenProviderChanged: root._rebuild()

    function _firstVisibleIndex() {
        var n = root._segments.length
        if (n <= root.visibleDepth)
            return 0
        return n - root.visibleDepth
    }

    function _navigateTo(uri) {
        root.setCurrentPath(uri)
        root.tabBarClicked(uri)
    }

    function collapse() { expandedIndex = -1 }

    // 对齐 C++：index 为 1-based
    function setIcon(index, iconName) {
        if (index < 1)
            return
        var next = {}
        for (var k in root._iconOverrides)
            next[k] = root._iconOverrides[k]
        next[String(index)] = iconName !== undefined && iconName !== null ? String(iconName) : ""
        root._iconOverrides = next
        root._rebuild()
    }

    function icon(index) {
        if (index < 1 || index > root._segments.length)
            return ""
        return root._segments[index - 1].icon
    }

    function setVisibleDepth(d) { visibleDepth = d }
    function setTextLength(n) { textLength = n }
    function setBackgroundVisible(v) { backgroundVisible = v }
    function setTranslucent(v) { translucent = v }

    Component.onCompleted: {
        if (root.currentPath !== "")
            root._rebuild()
    }

    Repeater {
        model: root._segments

        Private.KBreadCrumbItem {
            id: crumb
            visible: index >= root._firstVisibleIndex()
            text: modelData.displayText !== undefined ? modelData.displayText : modelData.text
            iconName: modelData.icon !== undefined ? modelData.icon : "folder"
            // 对齐 C++：仅当该段有子目录菜单时画箭头
            showSeparator: modelData.menuItems && modelData.menuItems.length > 0
            current: index === root._segments.length - 1
            flat: root.flat
            backgroundVisible: root.backgroundVisible
            translucent: root.translucent
            separatorInteractive: true
            expanded: index === root.expandedIndex
            iconRev: root.iconRev
            // 允许 Tab 走焦并显示焦点框；菜单收起用 dismissFocusChrome 避免误显
            tabFocusEnabled: true

            // 点正文：跳到该段路径（截断更深层级）
            onClicked: root._navigateTo(modelData.uri)

            // 点箭头：重新扫描真实目录后弹出菜单
            onSeparatorClicked: {
                var items = root._childrenOf(modelData.uri)
                if (!items.length) {
                    root.expandedIndex = -1
                    crumb.dismissFocusChrome()
                    return
                }
                if (root.expandedIndex === index) {
                    root.expandedIndex = -1
                    itemMenu.close()
                    crumb.dismissFocusChrome()
                    return
                }
                // 弹出前标记为鼠标路径，菜单抢走/归还焦点时不显示焦点框
                crumb.dismissFocusChrome()
                root.expandedIndex = index
                itemMenu.segmentUri = modelData.uri
                itemMenu.menuModel = items
                itemMenu.popup(crumb, 0, crumb.height)
            }

            Menu {
                id: itemMenu
                property string segmentUri: ""
                property var menuModel: []
                width: Math.max(crumb.width + 5, implicitWidth)
                onAboutToShow: {
                    width = Math.max(crumb.width + 5, implicitWidth)
                    // 弹出前再扫一次，保证下拉内容是最新文件系统状态
                    if (segmentUri !== "")
                        menuModel = root._childrenOf(segmentUri)
                }
                onAboutToHide: crumb.dismissFocusChrome()
                onClosed: {
                    if (root.expandedIndex === index)
                        root.expandedIndex = -1
                    // Popup 归还焦点可能在 closed 之后，再清一次
                    Qt.callLater(function () { crumb.dismissFocusChrome() })
                }

                Instantiator {
                    model: itemMenu.menuModel
                    delegate: MenuItem {
                        text: modelData.text !== undefined ? modelData.text : String(modelData)
                        icon.name: modelData.icon !== undefined ? modelData.icon : "folder"
                        onTriggered: {
                            var uri = modelData.uri !== undefined
                                      ? modelData.uri
                                      : root._joinPath(itemMenu.segmentUri, text)
                            root._navigateTo(uri)
                        }
                    }
                    onObjectAdded: itemMenu.insertItem(index, object)
                    onObjectRemoved: itemMenu.removeItem(object)
                }
            }
        }
    }
}
