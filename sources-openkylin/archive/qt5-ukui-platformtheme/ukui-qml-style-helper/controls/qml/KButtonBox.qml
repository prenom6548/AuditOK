/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KButtonBox.
 * 按钮容器：水平/垂直、无缝拼接成圆角胶囊、互斥、阴影、id 体系。
 * 子项可为 Component（Loader 新建）或已存在的按钮 Item 实例。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.12 as T
import QtGraphicalEffects 1.0
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Control {
    id: control

    property int orientation: Qt.Horizontal
    property int borderRadius: (typeof ukui_platformtheme_token !== "undefined"
                                && ukui_platformtheme_token
                                && ukui_platformtheme_token.kradiusNormal > 0)
                               ? ukui_platformtheme_token.kradiusNormal : 6
    property bool exclusive: false
    property bool checkable: false
    property bool shadowEnabled: false
    property bool showDividers: true // 按钮间分割线画在按钮自身边缘（无间隙）：N 个按钮 → N-1 条
    property var buttons: []
    property var _ids: [] // index -> id，默认 id = index（对齐 QButtonGroup addButton(btn,i)）

    font: app.font
    palette: app.palette

    // 对齐 C++：SetFixedSize，尺寸随内容；首尾两角各圆 borderRadius
    implicitWidth: orientation === Qt.Horizontal ? row.implicitWidth
                                                    : Math.max(row.implicitWidth, 36)
    implicitHeight: orientation === Qt.Horizontal ? Math.max(row.implicitHeight, 36)
                                                    : col.implicitHeight

    signal buttonClicked(var button)
    signal buttonPressed(var button)
    signal buttonReleased(var button)
    signal buttonToggled(var button, bool toggled)

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    // 子按钮共享 delegate：挂信号；圆角由 updateBorderRadius 按位置逐角设到按钮自身
    // （对齐 C++ updateBorderRadius：圆角在按钮自身，焦点框跟着按钮走，不被容器裁剪）
    Component {
        id: buttonDelegate
        Loader {
            id: btnLoader
            sourceComponent: modelData
            Layout.alignment: control.orientation === Qt.Horizontal ? Qt.AlignVCenter : Qt.AlignHCenter
            Layout.fillWidth: control.orientation === Qt.Vertical
            Layout.fillHeight: control.orientation === Qt.Horizontal
            onLoaded: control._onItemLoaded(btnLoader.item)
            // 用 Connections 的 onPressed/onReleased 等（QML 信号处理器）避免
            // item.pressed 是 bool 属性、与 pressed() 信号同名导致 .connect 失败
            Connections {
                target: btnLoader.item
                ignoreUnknownSignals: true
                function onClicked() { control._onClicked(btnLoader.item) }
                function onPressed() { control.buttonPressed(btnLoader.item) }
                function onReleased() { control.buttonReleased(btnLoader.item) }
                function onToggled(checked) { control._onToggled(btnLoader.item, checked) }
            }
        }
    }

    background: Item {}

    contentItem: Item {
        id: shadowRoot
        anchors.fill: parent

        // 阴影（对齐 C++ QGraphicsDropShadowEffect offset(0,2) blurRadius4 alpha0.1），不裁剪子按钮
        layer.enabled: control.shadowEnabled
        layer.effect: DropShadow {
            verticalOffset: 2
            horizontalOffset: 0
            radius: 4
            samples: 9
            color: app.isDark ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.1)
            transparentBorder: true
        }

        RowLayout {
            id: row
            visible: control.orientation === Qt.Horizontal
            anchors.fill: parent
            spacing: 0 // 对齐 C++ setSpacing(0)，按钮紧贴
            Repeater {
                id: rowRepeater
                model: control.orientation === Qt.Horizontal ? control.buttons : []
                delegate: buttonDelegate
            }
        }

        ColumnLayout {
            id: col
            visible: control.orientation === Qt.Vertical
            anchors.fill: parent
            spacing: 0
            Repeater {
                id: colRepeater
                model: control.orientation === Qt.Vertical ? control.buttons : []
                delegate: buttonDelegate
            }
        }
    }

    function _activeRepeater() {
        return orientation === Qt.Horizontal ? rowRepeater : colRepeater
    }

    function _allItems() {
        var rep = _activeRepeater()
        var arr = []
        for (var i = 0; i < rep.count; i++) {
            var ld = rep.itemAt(i)
            if (ld && ld.item)
                arr.push(ld.item)
        }
        return arr
    }

    function _itemAt(i) {
        var ld = _activeRepeater().itemAt(i)
        return ld && ld.item ? ld.item : null
    }

    function _indexOfItem(btn) {
        return _allItems().indexOf(btn)
    }

    function _idAt(i) {
        return (_ids.length > i && _ids[i] !== undefined) ? _ids[i] : i
    }

    // 给单个子按钮设四角半径：优先 setBorderRadius(bl,tl,tr,br)（KPushButton），
    // 回退 radius 单值（RoundButton 等，无法逐角只能近似）
    function _setChildRadius(btn, bl, tl, tr, br) {
        if (!btn)
            return
        if (typeof btn.setBorderRadius === "function")
            btn.setBorderRadius(bl, tl, tr, br)
        else if ('radius' in btn)
            btn.radius = Math.max(bl, tl, tr, br)
    }

    // 给子按钮设分割线（画在按钮自身边缘，无间隙）：
    // 水平 box → 非末按钮画右边竖线 dividerRight；垂直 box → 非末按钮画底边横线 dividerBottom
    function _setChildDivider(btn, on) {
        if (!btn)
            return
        if ('dividerRight' in btn)
            btn.dividerRight = (control.orientation === Qt.Horizontal) && on
        if ('dividerBottom' in btn)
            btn.dividerBottom = (control.orientation === Qt.Vertical) && on
    }

    // 对齐 C++ updateBorderRadius（kbuttonbox.cpp:91-132）：按按钮位置/方向给每个按钮设四角
    // 并按 showDividers 给非末按钮设分割线（N 个按钮 → N-1 条，画在按钮上无缝）
    function updateBorderRadius() {
        var rep = _activeRepeater()
        var n = rep.count
        var r = borderRadius
        for (var i = 0; i < n; i++) {
            var it = _itemAt(i)
            if (!it)
                continue
            var bl, tl, tr, br
            if (it.flat) { // flat 按钮：四角都圆（独立圆角按钮，不拼胶囊）
                bl = tl = tr = br = r
            } else if (i === 0) { // 首按钮
                if (orientation === Qt.Horizontal) { bl = r; tl = r; tr = 0; br = 0 }
                else { bl = 0; tl = r; tr = r; br = 0 }
            } else if (i === n - 1) { // 尾按钮
                if (orientation === Qt.Horizontal) { bl = 0; tl = 0; tr = r; br = r }
                else { bl = r; tl = 0; tr = 0; br = r }
            } else { // 中间
                bl = 0; tl = 0; tr = 0; br = 0
            }
            _setChildRadius(it, bl, tl, tr, br)
            _setChildDivider(it, control.showDividers && i < n - 1)
        }
    }

    function _onItemLoaded(item) {
        if (!item)
            return
        // 仅按钮有 checkable；分割线等非按钮子项跳过
        if ('checkable' in item)
            item.checkable = Qt.binding(function () { return control.checkable })
        control.updateBorderRadius() // 子项就绪后重算四角（位置可能变）
    }

    onBorderRadiusChanged: updateBorderRadius()
    onOrientationChanged: updateBorderRadius()
    onShowDividersChanged: updateBorderRadius()

    function _onClicked(btn) {
        if (exclusive && checkable) {
            var arr = _allItems()
            for (var i = 0; i < arr.length; i++)
                if (arr[i] !== btn && arr[i].checked)
                    arr[i].checked = false
        }
        buttonClicked(btn)
    }

    // 互斥：编程式 setChecked 也会触发 toggled，从而联动取消其它（对齐 QButtonGroup exclusive）
    function _onToggled(btn, checked) {
        if (exclusive && checkable && checked) {
            var arr = _allItems()
            for (var i = 0; i < arr.length; i++)
                if (arr[i] !== btn && arr[i].checked)
                    arr[i].checked = false
        }
        buttonToggled(btn, checked)
    }

    onExclusiveChanged: update() // 状态记录，无额外动作

    function setOrientation(o) { orientation = o }
    function addButton(btnComponent, index) {
        var arr = buttons
        if (index === undefined || index < 0 || index >= arr.length)
            arr.push(btnComponent)
        else
            arr.splice(index, 0, btnComponent)
        buttons = arr
    }
    function setButtonList(list) { buttons = list }
    function removeButton(indexOrButton) {
        var arr = buttons
        var idx = -1
        if (typeof indexOrButton === "number")
            idx = indexOrButton
        else {
            // 先按 Component 匹配（addButton 传入的），否则按已加载 Item 实例定位其 index
            idx = _indexOfButtonComponent(indexOrButton)
            if (idx < 0)
                idx = _indexOfItem(indexOrButton)
        }
        if (idx >= 0 && idx < arr.length)
            arr.splice(idx, 1)
        buttons = arr
    }
    function _indexOfButtonComponent(comp) {
        for (var i = 0; i < buttons.length; i++)
            if (buttons[i] === comp)
                return i
        return -1
    }
    function buttonList() { return _allItems() }
    function setBorderRadius(r) { borderRadius = r }
    function setExclusive(flag) { exclusive = flag }
    function setCheckable(flag) { checkable = flag }
    function setShadow(flag) { shadowEnabled = flag }

    // id 体系（对齐 C++ setId/id/button(int)/checkedButton/checkedId）
    function setId(button, id) {
        var i = _indexOfItem(button)
        if (i < 0)
            return
        _ids[i] = id
    }
    function id(button) {
        var i = _indexOfItem(button)
        return i < 0 ? -1 : _idAt(i)
    }
    function button(btnId) {
        var rep = _activeRepeater()
        for (var i = 0; i < rep.count; i++)
            if (_idAt(i) === btnId)
                return _itemAt(i)
        return null
    }
    function checkedButton() {
        var arr = _allItems()
        for (var i = 0; i < arr.length; i++)
            if (arr[i].checked)
                return arr[i]
        return null
    }
    function checkedId() {
        var btn = checkedButton()
        return btn ? id(btn) : -1
    }
}
