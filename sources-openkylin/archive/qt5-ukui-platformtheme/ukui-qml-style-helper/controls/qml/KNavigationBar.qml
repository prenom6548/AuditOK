/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KNavigationBar。
 * 普通项 / 次级项：复用 qqc2style ItemDelegate（hover/高亮/禁用/焦点）；
 * 当前项：highlightOnly + highlighted（不用 checked）；Tag 无底。
 * 走焦：FocusScope + ListView + ItemDelegate focusHandling；↑↓ 跳过 Tag/禁用/折叠。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import org.ukui.qqc2style.private 1.0 as StylePrivate

FocusScope {
    id: control

    enum ItemType { StandardItem = 0, SubItem = 1, TagItem = 2 }
    enum DataType { ValueRole = 256, StringRole = 257, BoolRole = 258 }

    // [{ text, icon, type: 0/1/2, enabled, tag, tabValue, addVisible, extendVisible, expanded, addIcon }]
    property var items: []
    property bool tabValueVisible: false
    property int currentIndex: -1

    property font font: app.font
    property var palette: app.palette

    // Tab 进入本控件，落到内部 ListView
    activeFocusOnTab: true
    implicitWidth: 240
    implicitHeight: 400

    signal addTagClicked(string tag)
    signal extendClicked(string tag, int row)
    signal currentChanged(int index)

    StylePrivate.APPParameter {
        id: app
        onParametryChanged: control._reloadIcons()
        onIconThemeChanged: control._reloadIcons()
    }

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token
    readonly property color _fontPrimary: _hasToken
                                          ? ukui_platformtheme_token.kFontPrimary
                                          : (palette ? palette.windowText : "#000000")
    readonly property color _fontPrimaryDisable: _hasToken
                                                 ? ukui_platformtheme_token.kFontPrimaryDisable
                                                 : "#999999"
    readonly property color _fontSecondary: _hasToken
                                            ? ukui_platformtheme_token.kFontSecondary
                                            : "#8C8C8C"
    readonly property color _fontHighlight: _hasToken
                                            ? ukui_platformtheme_token.kFontWhite
                                            : "#FFFFFF"
    readonly property color _kWhite: _hasToken
                                     ? ukui_platformtheme_token.kWhite
                                     : "#FFFFFF"

    property int iconRev: 0
    function _reloadIcons() { iconRev++ }

    function _copyItems() {
        var a = []
        for (var i = 0; i < items.length; i++)
            a.push(Object.assign({}, items[i]))
        return a
    }

    function _setItems(a) { items = a }

    function _rowVisible(it) {
        if (!it)
            return false
        if (it.type === KNavigationBar.TagItem)
            return true
        return it.expanded !== false
    }

    function _resolveIconUrl(name, selected, enabled) {
        if (!name || name === "")
            return ""
        var s = String(name)
        var prefix = "image://imageProvider/"
        if (s.indexOf(prefix) === 0) {
            var path = s.substring(prefix.length)
            var slash = path.lastIndexOf("/")
            if (slash >= 0) {
                var model = path.substring(slash + 1)
                if (model === "normal" || model === "disenable" || model === "clicked"
                        || model === "hover" || model === "highlight" || model === "highDisbale"
                        || model === "hoveredWidthThemeColor" || model === "clickedWidthThemeColor"
                        || model === "normalWidthThemeColor" || model === "disableWidthThemeColor")
                    path = path.substring(0, slash)
            }
            s = path
        }
        var mode
        if (enabled === false)
            mode = selected ? "highDisbale" : "disableWidthThemeColor"
        else if (selected)
            mode = "highlight"
        else
            mode = "normalWidthThemeColor"
        return prefix + s + "/" + mode
    }

    function _tagIconUrl(name) {
        return _resolveIconUrl(name || "list-add-symbolic", false, true)
    }

    function _isSelectable(it) {
        if (!it || it.type === KNavigationBar.TagItem)
            return false
        if (it.enabled === false)
            return false
        return control._rowVisible(it)
    }

    function _nextSelectable(from, step) {
        var i = from + step
        while (i >= 0 && i < control.items.length) {
            if (control._isSelectable(control.items[i]))
                return i
            i += step
        }
        return -1
    }

    function _findTagIndex(a, tag) {
        for (var i = 0; i < a.length; i++) {
            if (a[i].type === KNavigationBar.TagItem && a[i].text === tag)
                return i
        }
        return -1
    }

    function _toggleGroup(tag, tagRow) {
        var a = _copyItems()
        var ti = _findTagIndex(a, tag)
        if (ti < 0)
            return
        var currentlyExpanded = a[ti].expanded !== false
        var nextExpanded = !currentlyExpanded
        a[ti].expanded = nextExpanded
        var clearCurrent = false
        for (var i = 0; i < a.length; i++) {
            if (a[i].type !== KNavigationBar.TagItem && a[i].tag === tag) {
                a[i].expanded = nextExpanded
                if (!nextExpanded && i === control.currentIndex)
                    clearCurrent = true
            }
        }
        _setItems(a)
        if (clearCurrent)
            control.currentIndex = -1
        control.extendClicked(tag, tagRow)
    }

    // FocusScope 获焦后把焦点交给 ListView
    onActiveFocusChanged: {
        if (activeFocus && !lv.activeFocus)
            lv.forceActiveFocus()
    }

    ListView {
        id: lv
        anchors.fill: parent
        model: control.items
        clip: true
        spacing: 4
        currentIndex: control.currentIndex
        boundsBehavior: Flickable.StopAtBounds
        // 由外层 FocusScope 进焦；ListView 持焦以驱动 ItemDelegate 焦点框
        activeFocusOnTab: false
        keyNavigationEnabled: false
        focus: true

        onActiveFocusChanged: {
            if (!activeFocus)
                return
            if (currentIndex < 0 || !control._isSelectable(control.items[currentIndex])) {
                var i = control._nextSelectable(-1, 1)
                if (i >= 0)
                    currentIndex = i
            }
        }

        // ListView 持焦时（Tab 进入、未点选 take）的 ↑↓，需跳过 Tag
        Keys.onPressed: {
            if (event.key === Qt.Key_Down) {
                var n = control._nextSelectable(currentIndex, 1)
                if (n >= 0)
                    currentIndex = n
                event.accepted = true
            } else if (event.key === Qt.Key_Up) {
                var p = control._nextSelectable(currentIndex, -1)
                if (p >= 0)
                    currentIndex = p
                event.accepted = true
            } else if (event.key === Qt.Key_Space
                       || event.key === Qt.Key_Return
                       || event.key === Qt.Key_Enter) {
                if (currentItem && currentItem.keyboardPressing !== undefined)
                    currentItem.keyboardPressing = true
                event.accepted = true
            }
        }
        Keys.onReleased: {
            if (event.key === Qt.Key_Space
                    || event.key === Qt.Key_Return
                    || event.key === Qt.Key_Enter) {
                if (currentItem && currentItem.keyboardPressing !== undefined)
                    currentItem.keyboardPressing = false
                event.accepted = true
            }
        }

        onCurrentIndexChanged: {
            if (control.currentIndex !== currentIndex)
                control.currentIndex = currentIndex
            control.currentChanged(currentIndex)
        }

        Connections {
            target: control
            function onCurrentIndexChanged() {
                if (lv.currentIndex !== control.currentIndex)
                    lv.currentIndex = control.currentIndex
            }
        }

        delegate: ItemDelegate {
            id: del
            width: lv.width

            property var rowData: modelData
            property int rowIndex: index
            readonly property bool isCurrent: ListView.isCurrentItem
            readonly property bool isTag: rowData && rowData.type === KNavigationBar.TagItem
            readonly property bool isSub: rowData && rowData.type === KNavigationBar.SubItem
            // 供 ItemDelegate focusHandling ↑↓ 跳过 Tag
            property bool focusNavigable: !isTag

            height: control._rowVisible(rowData)
                    ? (isTag ? (rowIndex === 0 ? 36 : 48) : 40)
                    : 0
            visible: height > 0

            padding: 0
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            property bool focusHandling: true
            property bool highlightOnly: true

            highlighted: !isTag && isCurrent
            checked: false
            hoverEnabled: !isTag
            enabled: isTag ? true : (rowData && rowData.enabled !== false)

            onClicked: {
                if (!isTag && rowData && rowData.enabled !== false)
                    lv.currentIndex = rowIndex
            }

            Binding {
                target: del.background
                property: "visible"
                value: !del.isTag
                when: del.background !== null
            }

            contentItem: Item {
                anchors.fill: parent

                // 普通 / 次级
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: del.isSub ? 32 : 16
                    anchors.rightMargin: 16
                    spacing: 8
                    visible: !del.isTag

                    Image {
                        id: rowIcon
                        visible: !del.isSub && !!del.rowData && !!del.rowData.icon
                                 && status === Image.Ready
                        sourceSize.width: 16
                        sourceSize.height: 16
                        Layout.preferredWidth: (!del.isSub && del.rowData && del.rowData.icon) ? 16 : 0
                        Layout.preferredHeight: 16
                        Layout.alignment: Qt.AlignVCenter
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        cache: false

                        // 主题 / 选中 / 禁用变化时重刷（避免 Connections 与 ItemDelegate 信号重名）
                        readonly property string _iconKey: [
                            control.iconRev,
                            del.rowData ? (del.rowData.icon || "") : "",
                            del.highlighted ? 1 : 0,
                            del.enabled ? 1 : 0
                        ].join(":")

                        on_IconKeyChanged: reload()
                        Component.onCompleted: reload()

                        function reload() {
                            if (del.isSub || !del.rowData || !del.rowData.icon) {
                                source = ""
                                return
                            }
                            var url = control._resolveIconUrl(del.rowData.icon, del.highlighted, del.enabled)
                            source = ""
                            source = url
                        }
                    }

                    Label {
                        text: (del.rowData && del.rowData.text) ? del.rowData.text : ""
                        font: control.font
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        color: !del.enabled
                               ? control._fontPrimaryDisable
                               : (del.highlighted
                                  ? control._fontHighlight
                                  : control._fontPrimary)
                    }

                    Label {
                        visible: !!(control.tabValueVisible
                                 && del.rowData
                                 && del.rowData.tabValue !== undefined
                                 && String(del.rowData.tabValue) !== "")
                        text: (del.rowData && del.rowData.tabValue) ? del.rowData.tabValue : ""
                        font: control.font
                        Layout.alignment: Qt.AlignVCenter
                        color: del.highlighted
                               ? control._kWhite
                               : Qt.rgba(control._fontPrimary.r,
                                         control._fontPrimary.g,
                                         control._fontPrimary.b, 0.4)
                    }
                }

                // Tag
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.topMargin: del.rowIndex === 0 ? 0 : 12
                    spacing: 8
                    visible: del.isTag

                    Label {
                        text: (del.rowData && del.rowData.text) ? del.rowData.text : ""
                        font: control.font
                        color: control._fontSecondary
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Item {
                        visible: !!(del.rowData && del.rowData.addVisible)
                        Layout.preferredWidth: visible ? 24 : 0
                        Layout.preferredHeight: 24
                        Layout.alignment: Qt.AlignVCenter
                        z: 1

                        Rectangle {
                            anchors.fill: parent
                            radius: 6
                            color: addMa.pressed
                                   ? Qt.rgba(control.palette.buttonText.r,
                                             control.palette.buttonText.g,
                                             control.palette.buttonText.b, 0.2)
                                   : (addMa.containsMouse
                                      ? Qt.rgba(control.palette.buttonText.r,
                                                control.palette.buttonText.g,
                                                control.palette.buttonText.b, 0.05)
                                      : "transparent")
                        }
                        Image {
                            id: addIconImg
                            anchors.centerIn: parent
                            sourceSize.width: 16
                            sourceSize.height: 16
                            smooth: true
                            cache: false
                            readonly property string _iconKey: control.iconRev + ":"
                                    + ((del.rowData && del.rowData.addIcon) ? del.rowData.addIcon : "list-add-symbolic")
                            on_IconKeyChanged: reload()
                            Component.onCompleted: reload()
                            function reload() {
                                var n = (del.rowData && del.rowData.addIcon)
                                        ? del.rowData.addIcon : "list-add-symbolic"
                                var url = control._tagIconUrl(n)
                                source = ""
                                source = url
                            }
                        }
                        MouseArea {
                            id: addMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: control.addTagClicked(del.rowData.text)
                        }
                        ToolTip.visible: addMa.containsMouse
                        ToolTip.text: qsTr("Add tag")
                        ToolTip.delay: 500
                    }

                    Item {
                        visible: !!(del.rowData && del.rowData.extendVisible)
                        Layout.preferredWidth: visible ? 24 : 0
                        Layout.preferredHeight: 24
                        Layout.alignment: Qt.AlignVCenter
                        z: 1

                        Rectangle {
                            anchors.fill: parent
                            radius: 6
                            color: extMa.pressed
                                   ? Qt.rgba(control.palette.buttonText.r,
                                             control.palette.buttonText.g,
                                             control.palette.buttonText.b, 0.2)
                                   : (extMa.containsMouse
                                      ? Qt.rgba(control.palette.buttonText.r,
                                                control.palette.buttonText.g,
                                                control.palette.buttonText.b, 0.05)
                                      : "transparent")
                        }
                        Image {
                            id: extIconImg
                            anchors.centerIn: parent
                            sourceSize.width: 16
                            sourceSize.height: 16
                            smooth: true
                            cache: false
                            readonly property string _iconKey: control.iconRev + ":"
                                    + ((del.rowData && del.rowData.expanded !== false) ? "up" : "down")
                            on_IconKeyChanged: reload()
                            Component.onCompleted: reload()
                            function reload() {
                                var n = (del.rowData && del.rowData.expanded !== false)
                                        ? "ukui-up.symbolic" : "ukui-down.symbolic"
                                var url = control._tagIconUrl(n)
                                source = ""
                                source = url
                            }
                        }
                        MouseArea {
                            id: extMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: control._toggleGroup(del.rowData.text, del.rowIndex)
                        }
                        ToolTip.visible: extMa.containsMouse
                        ToolTip.text: (del.rowData && del.rowData.expanded !== false)
                                      ? qsTr("Fold") : qsTr("Expand")
                        ToolTip.delay: 500
                    }
                }
            }
        }

        // 内容未超出高度时隐藏（UKUI ScrollBar：非 AlwaysOff 即显示）
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOff }
        ScrollBar.vertical: ScrollBar {
            policy: lv.contentHeight > lv.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
        }
    }

    // —— API（对齐 C++）——
    function addItem(it) {
        var a = _copyItems()
        var row = Object.assign({ type: KNavigationBar.StandardItem, expanded: true }, it)
        row.type = KNavigationBar.StandardItem
        a.push(row)
        _setItems(a)
    }
    function addSubItem(it) {
        var a = _copyItems()
        var row = Object.assign({ type: KNavigationBar.SubItem, expanded: true }, it)
        row.type = KNavigationBar.SubItem
        a.push(row)
        _setItems(a)
    }
    function addGroupItems(list, tag) {
        var a = _copyItems()
        a.push({ text: tag, type: KNavigationBar.TagItem, expanded: true,
                 addVisible: false, extendVisible: false })
        for (var i = 0; i < list.length; i++) {
            var row = Object.assign({ type: KNavigationBar.StandardItem, expanded: true }, list[i])
            row.type = KNavigationBar.StandardItem
            row.tag = tag
            row.expanded = true
            a.push(row)
        }
        _setItems(a)
    }
    function insertGroupItems(tag, row, it) {
        var a = _copyItems()
        var ti = _findTagIndex(a, tag)
        if (ti < 0)
            return
        var insertAt = ti + 1 + row
        var groupExpanded = a[ti].expanded !== false
        var n = Object.assign({ type: KNavigationBar.StandardItem }, it)
        n.type = KNavigationBar.StandardItem
        n.tag = tag
        n.expanded = groupExpanded
        if (insertAt > a.length)
            insertAt = a.length
        a.splice(insertAt, 0, n)
        _setItems(a)
    }
    function removeGroupItem(tag, predicate) {
        // predicate: object 引用匹配 text，或 string 匹配 text
        var a = _copyItems()
        var key = typeof predicate === "string" ? predicate
                : (predicate && predicate.text !== undefined ? predicate.text : "")
        for (var i = a.length - 1; i >= 0; i--) {
            if (a[i].tag === tag && a[i].type !== KNavigationBar.TagItem
                    && a[i].text === key) {
                a.splice(i, 1)
                break
            }
        }
        _setItems(a)
    }
    function addTag(tag) {
        var a = _copyItems()
        a.push({ text: tag, type: KNavigationBar.TagItem, expanded: true,
                 addVisible: false, extendVisible: false })
        _setItems(a)
    }
    function setTabValue(itemOrText, value) {
        var key = typeof itemOrText === "string" ? itemOrText
                : (itemOrText && itemOrText.text !== undefined ? itemOrText.text : "")
        var a = _copyItems()
        for (var i = 0; i < a.length; i++) {
            if (a[i].text === key && a[i].type !== KNavigationBar.TagItem) {
                a[i].tabValue = value
                break
            }
        }
        _setItems(a)
    }
    function setTabValueVisible(v) { tabValueVisible = v }
    function setAddIcon(tag, iconName) {
        var a = _copyItems()
        var ti = _findTagIndex(a, tag)
        if (ti >= 0)
            a[ti].addIcon = iconName
        _setItems(a)
    }
    function setAddTagVisible(tag, v) {
        var a = _copyItems()
        var ti = _findTagIndex(a, tag)
        if (ti >= 0)
            a[ti].addVisible = v
        _setItems(a)
    }
    function setExtendTagVisible(tag, v) {
        var a = _copyItems()
        var ti = _findTagIndex(a, tag)
        if (ti >= 0)
            a[ti].extendVisible = v
        _setItems(a)
    }
}
