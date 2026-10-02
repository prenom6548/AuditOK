/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.ItemDelegate {
    id: control

    // 应用可选：property bool highlightOnly: true
    // true：仅 highlighted 走品牌高亮色（导航栏等）；默认 false：保持 highlighted && checked
    property var isHighlightOnly: {
        if (control.hasOwnProperty("highlightOnly"))
            return control.highlightOnly
        else
            return false
    }
    readonly property bool useHighlightColors: control.highlighted
                                              && (isHighlightOnly || control.checked)

    // 应用可选：property bool focusHandling: true 启用样式侧列表走焦辅助（NoFocus/take/hook）
    // 未设置 / false：不改 Qt/应用走焦，仍画焦点框
    property var isFocusHandling: {
        if (control.hasOwnProperty("focusHandling"))
            return control.focusHandling
        else
            return false
    }

    // ---- 焦点样式 ----
    // isFocusHandling：列表内 NoFocus；点选临时 ClickFocus；hook 开框等。
    // !isFocusHandling：不代管走焦，只按 visualFocus / 列表当前项画框。
    property bool takeListClickFocus: false
    // 缓存本列表的焦点控制器（即下方 listFocusHookComponent 创建的 hook），避免每次按键都扫 contentItem.children
    property var fc: null
    Binding on focusPolicy {
        when: isFocusHandling && ListView.view && !takeListClickFocus && !control.activeFocus
        value: Qt.NoFocus
    }
    onTakeListClickFocusChanged: {
        if (!isFocusHandling || !ListView.view)
            return
        if (takeListClickFocus) {
            focusPolicy = Qt.ClickFocus
            if (fc) {
                try {
                    // 自愈：无论谁置我 take，先清掉前一个持 take 者，保证同时至多一个
                    if (fc.takeItem && fc.takeItem !== control)
                        fc.takeItem.takeListClickFocus = false
                    fc.takeItem = control
                } catch (e) {
                    control.fc = null
                }
            }
        } else {
            if (!control.activeFocus)
                focusPolicy = Qt.NoFocus
            if (fc) {
                try {
                    if (fc.takeItem === control)
                        fc.takeItem = null
                } catch (e) {
                    control.fc = null
                }
            }
        }
    }

    function _releaseListClickFocus(view, focusReason) {
        // 必须先移走 activeFocus，再关 take / 设 NoFocus
        if (view)
            view.forceActiveFocus(focusReason)
        takeListClickFocus = false
        if (ListView.view)
            focusPolicy = Qt.NoFocus
    }

    function _delegateAtIndex(view, index) {
        if (!view)
            return null
        if (typeof view.itemAtIndex === "function") {
            var it = view.itemAtIndex(index)
            if (it)
                return it
        }
        if (!view.contentItem)
            return null
        var kids = view.contentItem.children
        for (var k = 0; k < kids.length; ++k) {
            var kid = kids[k]
            if (!kid)
                continue
            var kidIndex = kid.index
            if (kidIndex === undefined && kid.rowIndex !== undefined)
                kidIndex = kid.rowIndex
            if (kidIndex === index)
                return kid
        }
        return null
    }

    // 列表 ↑↓：跳过 disabled / 不可见 / 高度为 0 的项
    function _nextEnabledListIndex(view, from, step) {
        if (!view)
            return -1
        var i = from + step
        while (i >= 0 && i < view.count) {
            var item = _delegateAtIndex(view, i)
            if (!item)
                return i
            // 可选：业务侧 property bool focusNavigable: false 跳过（如导航栏 Tag）
            if (item.hasOwnProperty("focusNavigable") && item.focusNavigable === false) {
                i += step
                continue
            }
            if (item.enabled && item.visible && item.height > 0)
                return i
            i += step
        }
        return -1
    }

    Keys.onPressed: {
        // 与 Button 相同：只打标，不 accept
        if (event.key === Qt.Key_Space
                || event.key === Qt.Key_Return
                || event.key === Qt.Key_Enter)
            keyboardPressing = true

        if (!isFocusHandling) {
            event.accepted = false
            return
        }

        var view = ListView.view
        if (view && control.activeFocus && takeListClickFocus) {
            if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                // 只开焦点样式并还焦给 ListView；必须 accept，否则默认 Tab 链会带出列表
                // 不改 currentIndex——走焦仍由应用 / ListView 处理（下一键）
                _setAllListFocusEnable(true)
                _releaseListClickFocus(view, event.key === Qt.Key_Tab
                                       ? Qt.TabFocusReason : Qt.BacktabFocusReason)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                // 点选临时焦点下：换到下一个可用项并移交 take，样式仍关
                var step = event.key === Qt.Key_Down ? 1 : -1
                var next = _nextEnabledListIndex(view, view.currentIndex, step)
                if (next >= 0) {
                    takeListClickFocus = false
                    view.currentIndex = next
                    if (view.currentItem && view.currentItem.takeListClickFocus !== undefined) {
                        view.currentItem.takeListClickFocus = true
                        view.currentItem.forceActiveFocus(Qt.MouseFocusReason)
                    }
                }
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Left || event.key === Qt.Key_Right
                    || event.key === Qt.Key_Home || event.key === Qt.Key_End
                    || event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
                _releaseListClickFocus(view, Qt.MouseFocusReason)
                event.accepted = true
                return
            }
        }
        event.accepted = false
    }

    // 每个 ListView 挂一次（合并了样式开关聚合 + Tab 走焦 hook）：
    // 1) Tab/Backtab 进入列表时开样式
    // 2) 点选后焦点已在 ListView、样式关着时再 Tab，也开样式（不代管走焦）
    // 3) 维护 takeItem / anyListFocusEnable，让 delegate 不必每次按键都扫 contentItem.children
    Component {
        id: listFocusHookComponent
        Item {
            id: hookRoot
            objectName: "__ukuiItemDelegateFocusHook"
            width: 0
            height: 0

            // 当前持 take 焦点的 delegate（同时至多一个）；null 表示无人持焦
            property Item takeItem: null
            // 整表 listFocusEnable 是否还有任意项开着（isFocusHandling 模式下所有项同开同关，
            // 用一个 bool 即可；setAllListFocusEnable 时同步更新）
            property bool anyListFocusEnable: false

            function setAllListFocusEnable(enable) {
                var view = hookRoot.parent
                if (!view || !view.contentItem)
                    return
                var kids = view.contentItem.children
                for (var i = 0; i < kids.length; ++i) {
                    if (kids[i] && kids[i].listFocusEnable !== undefined)
                        kids[i].listFocusEnable = enable
                }
                hookRoot.anyListFocusEnable = enable
            }

            function listFocusStyleOff() {
                return !hookRoot.anyListFocusEnable
            }

            Connections {
                target: hookRoot.parent
                function onActiveFocusChanged() {
                    var view = hookRoot.parent
                    if (!view || !view.activeFocus)
                        return
                    if (view.focusReason !== Qt.TabFocusReason
                            && view.focusReason !== Qt.BacktabFocusReason)
                        return
                    hookRoot.setAllListFocusEnable(true)
                }
            }

            // Connections 无法连接 Keys.pressed；用 forwardTo 把 ListView 的键转到 hook
            Keys.onPressed: {
                var view = hookRoot.parent
                if (!view || !view.activeFocus) {
                    event.accepted = false
                    return
                }
                if (event.key !== Qt.Key_Tab && event.key !== Qt.Key_Backtab) {
                    event.accepted = false
                    return
                }
                if (!hookRoot.listFocusStyleOff()) {
                    event.accepted = false
                    return
                }
                // 列表已有焦点、样式被鼠标关掉后：Tab 只重新开框，不 accept
                hookRoot.setAllListFocusEnable(true)
                event.accepted = false
            }

            Component.onCompleted: {
                var view = hookRoot.parent
                if (!view)
                    return
                var fwd = view.Keys.forwardTo
                var arr = [hookRoot]
                if (fwd) {
                    for (var i = 0; i < fwd.length; ++i) {
                        if (fwd[i] && fwd[i] !== hookRoot)
                            arr.push(fwd[i])
                    }
                }
                view.Keys.forwardTo = arr
            }
        }
    }

    Component.onCompleted: {
        if (!isFocusHandling)
            return
        var view = ListView.view
        if (!view)
            return
        // 查找本 ListView 上是否已挂 hook（首个 delegate 创建后，后续 delegate 复用）
        for (var i = 0; i < view.data.length; ++i) {
            var d = view.data[i]
            if (d && d.objectName === "__ukuiItemDelegateFocusHook") {
                control.fc = d
                return
            }
        }
        var hook = listFocusHookComponent.createObject(view)
        control.fc = hook
    }

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             itemdelegate.implicitHeight + topInset + bottomInset)

    padding: itemdelegate.padding
    spacing: app.space

    icon.width: app.iconWidth
    icon.height: app.iconWidth
    hoverEnabled: true
    font: app.font
    palette: app.palette

//    icon.color: control.palette.text
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var isAlternate:{
        if(control.hasOwnProperty("alternate")){
            if(control.alternate && index%2 === 1)
                return true
        }
        else
            return false
    }
    // 颜色选择辅助：把"透明/不透明"（+ 交替行）二维分支收敛成函数，避免超长三元链
    function _pickT(tColor, sColor) {
        return isTransaprent ? tColor : sColor
    }
    function _pickAlt(tAlt, t, sAlt, s) {
        return isTransaprent ? (isAlternate ? tAlt : t) : (isAlternate ? sAlt : s)
    }

    property var borderHColor: !control.enabled ? itemdelegate.disableBorderHColor : /*control.pressed ? itemdelegate.clickBorderHColor :
                               control.hovered ? itemdelegate.hoverBorderHColor : */itemdelegate.normalBorderHColor
    property var backHColor: !control.enabled ? itemdelegate.disableHBC : /*control.pressed ? itemdelegate.clickedHBC :
                             control.hovered ? itemdelegate.hoveredHBC :*/ itemdelegate.normalHBC

    // 焦点色优先于 pressed，避免 Enter/空格按下时焦点框被盖掉
    property var borderColor: !control.enabled ? _pickT(itemdelegate.disableTransparentBorderColor, itemdelegate.disableBorderColor) :
                               showFocusBorder ? app.focusBorderColor :
                               control.pressed ? _pickT(itemdelegate.clickedTransparentBorderColor, itemdelegate.clickBorderColor) :
                               control.hovered ? _pickT(itemdelegate.hoveredTransparentBorderColor, itemdelegate.hoverBorderColor) :
                                                 _pickT(itemdelegate.normalTransparentBorderColor, itemdelegate.normalBorderColor)
    property var backColor: !control.enabled ? _pickAlt(itemdelegate.disableAlternateTransparentBC, itemdelegate.disableTransparentBC, itemdelegate.disableAlternateBC, itemdelegate.disableBC) :
                             control.pressed ? _pickAlt(itemdelegate.clickedAlternateTransparentBC, itemdelegate.clickedTransparentBC, itemdelegate.clickedAlternatedBC, itemdelegate.clickedBC) :
                             control.hovered ? _pickAlt(itemdelegate.hoveredAlternateTransparentBC, itemdelegate.hoveredTransparentBC, itemdelegate.hoveredAlternatedBC, itemdelegate.hoveredBC) :
                                               _pickAlt(itemdelegate.normalAlternateTransparentBC, itemdelegate.normalTransparentBC, itemdelegate.normalAlternateBC, itemdelegate.normalBC)
    property var borderCheckedColor: !control.enabled ? _pickT(itemdelegate.disableCheckedTransparentBorderColor, itemdelegate.disableBorderCheckedColor) :
                                      //control.pressed ? _pickT(itemdelegate.clickedCheckedTransparentBorderColor, itemdelegate.clickBorderCheckedColor) :
                                      //control.hovered ? _pickT(itemdelegate.hoveredCheckedTransparentBorderColor, itemdelegate.hoverBorderCheckedColor) :
                                                        showFocusBorder ? app.focusBorderColor :
                                                        _pickT(itemdelegate.normalCheckedTransparentBorderColor, itemdelegate.normalBorderCheckedColor)
    property var backCheckedColor: !control.enabled ? _pickT(itemdelegate.disableCheckedTransparentBC, itemdelegate.disableCheckedBC) :
                                    //control.pressed ? _pickT(itemdelegate.clickedCheckedTransparentBC, itemdelegate.clickedCheckedBC) :
                                    //control.hovered ? _pickT(itemdelegate.hoveredCheckedTransparentBC, itemdelegate.hoveredCheckedBC) :
                                    _pickT(itemdelegate.normalCheckedTransparentBC, itemdelegate.normalCheckedBC)

    // 平铺：directFocusEnable ← visualFocus；鼠标按下清掉
    // 列表：listFocusEnable + 当前项 + ListView 有焦点才画框
    // isFocusHandling 只决定是否代管走焦，不影响「能不能画框」
    property bool directFocusEnable: false
    property bool listFocusEnable: app.focusEnable
    property bool keyboardPressing: false
    property bool showFocusBorder: {
        if (!(app.focusEnable))
            return false
        if (directFocusEnable)
            return true
        if (ListView.view)
            return listFocusEnable && ListView.view.activeFocus && ListView.isCurrentItem
        return false
    }

    // 走列表控制器（fc）做整表开关；fc 仅 isFocusHandling 模式存在，无 fc 时退化为本地扫描
    // 注意：不要调用 fc.setAllListFocusEnable——hook 由某个 delegate 的 Component.createObject
    // 创建，该 delegate 被 ListView 回收后方法/对象可能失效并抛 TypeError。
    function _setAllListFocusEnable(enable) {
        var view = ListView.view
        if (!view || !view.contentItem)
            return
        var kids = view.contentItem.children
        for (var i = 0; i < kids.length; ++i) {
            if (kids[i] && kids[i].listFocusEnable !== undefined)
                kids[i].listFocusEnable = enable
        }
        if (!fc)
            return
        try {
            fc.anyListFocusEnable = !!enable
        } catch (e) {
            control.fc = null
        }
    }

    // 同时至多一个 delegate 持 take，故只需复位 fc.takeItem；force 重置 focusPolicy 以保留原行为
    function _clearAllTakeListClickFocus() {
        if (!fc)
            return
        try {
            var t = fc.takeItem
            if (!t)
                return
            if (t.takeListClickFocus)
                t.takeListClickFocus = false
            if (t.focusPolicy !== Qt.NoFocus)
                t.focusPolicy = Qt.NoFocus
        } catch (e) {
            control.fc = null
        }
    }

    onVisualFocusChanged: {
        if (control.pressed)
            return
        if (!isFocusHandling) {
            // 不代管走焦：同 Button，跟 visualFocus 画框
            directFocusEnable = control.visualFocus
            return
        }
        if (ListView.view) {
            directFocusEnable = false
            if (control.visualFocus)
                _setAllListFocusEnable(true)
        } else {
            directFocusEnable = control.visualFocus
        }
    }

    onActiveFocusChanged: {
        if (!isFocusHandling) {
            if (!control.activeFocus)
                directFocusEnable = false
            return
        }
        if (!ListView.view && !control.activeFocus)
            directFocusEnable = false
        if (ListView.view && !control.activeFocus) {
            if (takeListClickFocus)
                takeListClickFocus = false
            if (focusPolicy !== Qt.NoFocus)
                focusPolicy = Qt.NoFocus
        }
    }

    onPressedChanged: {
        // 画框：鼠标按下消框（两种模式都做）
        if (control.pressed && !control.keyboardPressing) {
            directFocusEnable = false
            listFocusEnable = false
        }

        if (isFocusHandling && control.pressed && !control.keyboardPressing) {
            if (ListView.view) {
                _setAllListFocusEnable(false)
                _clearAllTakeListClickFocus()
                takeListClickFocus = true
                control.forceActiveFocus(Qt.MouseFocusReason)
                // 仅同步当前项，方便应用/ListView 后续导航；不代替走焦
                if (typeof index !== "undefined" && ListView.view.currentIndex !== index)
                    ListView.view.currentIndex = index
            } else {
                control.forceActiveFocus(Qt.MouseFocusReason)
            }
        }
        if (!control.pressed)
            control.keyboardPressing = false
    }

    ListView.onIsCurrentItemChanged: {
        if (!ListView.isCurrentItem || control.pressed || !ListView.view)
            return
        if (!isFocusHandling) {
            // 不代管：键盘换项时恢复画框
            listFocusEnable = app.focusEnable
            return
        }
        // 代管：仅当整表样式还开着才跟随（鼠标清完全部后不再因上下键开框）
        try {
            if (fc && fc.anyListFocusEnable)
                listFocusEnable = true
        } catch (e) {
            control.fc = null
        }
    }

    ParseInterface{
        id: parseInterface
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }

//    contentItem: IconLabel {
//        spacing: control.spacing
//        mirrored: control.mirrored
//        display: control.display
//        alignment: control.display === IconLabel.IconOnly || control.display === IconLabel.TextUnderIcon ? Qt.AlignCenter : Qt.AlignLeft

//        icon: control.icon
//        text: control.text
//        font: app.font
//        color: getStartColor(control.enabled ? itemdelegate.normalTextColor : itemdelegate.disableTextColor)
//    }

    contentItem: IconLabelContent {
        id: iconLabel
        controlRoot: control
        //        anchors.verticalCenter: control.verticalCenter

        leftSpace: control.mirrored ? control.spacing : control.leftPadding
        // 高亮色：默认 highlighted&&checked；highlightOnly 时仅 highlighted
        textColor: !control.enabled
                   ? (useHighlightColors ? itemdelegate.disableHTextColor : itemdelegate.disableTextColor)
                   : (useHighlightColors ? itemdelegate.normalHTextColor : itemdelegate.normalTextColor)
        //x: 0// control.text ? (control.mirrored ? control.leftPadding : control.width - width - control.rightPadding) : control.leftPadding + (control.availableWidth - width) / 2
        //anchors.centerIn: control
        implicitWidth: iconLabel.adjustWidth(control.leftPadding + control.rightPadding) + indicator.implicitWidth + control.spacing
    }

    background: Rectangle {
        anchors.fill: control
        radius: itemdelegate.radius
        border.width: showFocusBorder ? app.focusBorderWidth : itemdelegate.borderWidth
        border.color: getStartColor(useHighlightColors ? borderHColor
                                      : control.checked ? borderCheckedColor : borderColor)
//        implicitWidth: 100
//        implicitHeight: 40
//        visible: control.down || useHighlightColors || control.visualFocus
        color: gradientRec.visible ? "transparent"
               : getStartColor(useHighlightColors ? backHColor
                               : control.checked ? backCheckedColor : backColor)

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            border.width: parent.border.width
            border.color: parent.border.color
            gradient:  Gradient {
                id:gradient
                GradientStop {
                    position: 0.0
                    color: getStartColor(useHighlightColors ? backHColor
                                         : control.checked ? backCheckedColor : backColor)
                }

                GradientStop {
                    position: 1.0
                    color: getEndColor(useHighlightColors ? backHColor
                                       : control.checked ? backCheckedColor : backColor)
                }
            }
            visible: !parseInterface.isSolidPattern(useHighlightColors ? backHColor
                                                      : control.checked ? backCheckedColor : backColor)
        }
    }

    StylePrivate.UKUIItemDelegate{
        id: itemdelegate
    }

    StylePrivate.APPParameter{
        id: app
    }
}
