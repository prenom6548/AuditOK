/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KLunarDateEdit / KCustomDateEdit.
 * 尺寸、圆角、悬停/焦点边框及小日历配色与 Widgets 实现保持一致。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Window 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate
import org.ukui.platformtheme.controls 1.0 as Private
import "LunarCalendar.js" as Lunar

T.Control {
    id: control

    property date date: new Date()
    property bool showLunar: false
    property bool calendarPopupEnabled: true
    property int borderRadius: 6
    property int activeSection: -1 // 当前激活字段：0 年、1 月、2 日
    property date displayedMonth: new Date(date.getFullYear(), date.getMonth(), 1)
    property var calendarDays: []

    implicitWidth: 140
    implicitHeight: 36
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    padding: 0

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property bool dark: app.isDark
    readonly property color componentColor: parseColor.getDtColor("kcomponent-normal")
    readonly property color highlightColor: parseColor.getDtColor("highlight-active")
    readonly property color brandColor: parseColor.getDtColor("kbrand-normal")
    readonly property color buttonTextColor: parseColor.getDtColor("buttontext-active")
    readonly property color controlTextColor: enabled ? buttonTextColor
                                                       : parseColor.getDtColor("buttontext-disable")
    readonly property color currentTextColor: dark ? "#ffffff" : "#000000"
    readonly property bool inputFocused: activeSection >= 0 || calendar.opened

    onDateChanged: {
        displayedMonth = new Date(date.getFullYear(), date.getMonth(), 1)
        rebuildCalendar()
    }
    onEnabledChanged: {
        if (!enabled && calendar.opened) {
            calendar.close()
            activeSection = -1
        }
    }
    onDisplayedMonthChanged: rebuildCalendar()
    Component.onCompleted: rebuildCalendar()

    function copyDate(value) {
        return new Date(value.getFullYear(), value.getMonth(), value.getDate())
    }

    function sameDate(first, second) {
        return first.getFullYear() === second.getFullYear()
                && first.getMonth() === second.getMonth()
                && first.getDate() === second.getDate()
    }

    function rebuildCalendar() {
        var first = new Date(displayedMonth.getFullYear(), displayedMonth.getMonth(), 1)
        var firstWeekday = (first.getDay() + 6) % 7 // Monday is the first column.
        var start = new Date(first.getFullYear(), first.getMonth(), 1 - firstWeekday)
        var result = []
        for (var i = 0; i < 42; ++i) {
            var value = new Date(start.getFullYear(), start.getMonth(), start.getDate() + i)
            result.push({ value: value, currentMonth: value.getMonth() === displayedMonth.getMonth()
                        && value.getFullYear() === displayedMonth.getFullYear() })
        }
        calendarDays = result
    }

    function setDate(value) {
        if (value instanceof Date && !isNaN(value.getTime()) && !sameDate(value, date))
            date = copyDate(value)
    }

    function setShowLunar(value) { showLunar = value }
    function getDate() { return copyDate(date) }
    function showCalendar() {
        positionCalendar()
        calendar.open()
    }

    function calendarOverlayParent() {
        var item = control
        while (item) {
            if (item.overlay)
                return item.overlay
            item = item.parent
        }
        return Window.window ? (Window.window.overlay || Window.window.contentItem) : null
    }

    function positionCalendar() {
        var overlay = calendarOverlayParent()
        if (!overlay)
            return
        // 锚点与编辑框重合，使点击编辑框内部不会被视为弹窗外部点击。
        if (calendarAnchor.parent !== overlay)
            calendarAnchor.parent = overlay
        if (calendar.parent !== calendarAnchor)
            calendar.parent = calendarAnchor

        var position = control.mapToItem(overlay, 0, 0)
        calendarAnchor.x = position.x
        calendarAnchor.y = position.y
        calendar.x = Math.max(-calendarAnchor.x,
                              Math.min(0, overlay.width - calendar.width - calendarAnchor.x))
        calendar.y = position.y + control.height + 4 + calendar.height <= overlay.height
                || position.y - calendar.height - 4 < 0 ? control.height + 4 : -calendar.height - 4
    }

    function navigateMonth(delta) {
        var year = displayedMonth.getFullYear()
        var month = displayedMonth.getMonth() + delta
        var target = new Date(year, month, 1)
        var lastDay = new Date(target.getFullYear(), target.getMonth() + 1, 0).getDate()
        var day = Math.min(date.getDate(), lastDay)
        setDate(new Date(target.getFullYear(), target.getMonth(), day))
    }

    function datePartAt(position) {
        return position <= 4 ? 0 : position <= 7 ? 1 : 2
    }

    function selectedDatePart() {
        var position = dateInput.selectionStart < dateInput.selectionEnd
                ? dateInput.selectionStart : dateInput.cursorPosition
        return datePartAt(position)
    }

    function selectDatePart(part) {
        var start = part === 0 ? 0 : part === 1 ? 5 : 8
        var end = part === 0 ? 4 : part === 1 ? 7 : 10
        dateInput.select(start, end)
    }

    function activateDatePart(part) {
        activeSection = part
        if (!showLunar)
            selectDatePart(part)
        // 日历面板可由使用方关闭，字段激活和编辑仍可正常进行。
        if (calendarPopupEnabled)
            showCalendar()
    }

    function adjustDate(delta) {
        var part = selectedDatePart()
        activeSection = part
        var target
        if (part === 0) {
            var year = date.getFullYear() + delta
            var lastDay = new Date(year, date.getMonth() + 1, 0).getDate()
            target = new Date(year, date.getMonth(), Math.min(date.getDate(), lastDay))
        } else if (part === 1) {
            var month = new Date(date.getFullYear(), date.getMonth() + delta, 1)
            var monthLastDay = new Date(month.getFullYear(), month.getMonth() + 1, 0).getDate()
            target = new Date(month.getFullYear(), month.getMonth(), Math.min(date.getDate(), monthLastDay))
        } else {
            target = new Date(date.getFullYear(), date.getMonth(), date.getDate() + delta)
        }
        setDate(target)
        if (calendarPopupEnabled)
            showCalendar()
        Qt.callLater(function() { selectDatePart(part) })
    }

    function spinButtonIcon(name) {
        var icon = name
        if (!app.themeHasIcon(icon))
            icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/" + name + ".svg"
        return "image://imageProvider/" + icon + "/" + (dark ? "highlight" : "normal")
    }

    function acceptText() {
        var match = /^(\d{4})\/(\d{1,2})\/(\d{1,2})$/.exec(dateInput.text)
        if (!match) {
            dateInput.text = Qt.formatDate(date, "yyyy/MM/dd")
            return
        }
        var value = new Date(Number(match[1]), Number(match[2]) - 1, Number(match[3]))
        if (value.getFullYear() === Number(match[1]) && value.getMonth() === Number(match[2]) - 1
                && value.getDate() === Number(match[3]))
            setDate(value)
        else
            dateInput.text = Qt.formatDate(date, "yyyy/MM/dd")
    }

    function syncDateFromText() {
        // 仅在完整日期输入后同步，避免删除数字时被自动补零。
        var match = /^(\d{4})\/(\d{2})\/(\d{2})$/.exec(dateInput.text)
        if (!match)
            return
        var value = new Date(Number(match[1]), Number(match[2]) - 1, Number(match[3]))
        if (value.getFullYear() === Number(match[1]) && value.getMonth() === Number(match[2]) - 1
                && value.getDate() === Number(match[3]))
            setDate(value)
    }

    background: Rectangle {
        color: control.componentColor
        radius: control.borderRadius
        border.color: control.highlightColor
        border.width: control.inputFocused ? 2 : (control.hovered ? 1 : 0)
        antialiasing: true
    }

    contentItem: Item {
        TextInput {
            id: dateInput
            anchors.left: parent.left
            anchors.right: popupButton.left
            anchors.leftMargin: 8
            anchors.rightMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            visible: !control.showLunar
            enabled: control.enabled
            color: control.controlTextColor
            selectionColor: control.palette.highlight
            selectedTextColor: control.palette.highlightedText
            font.family: app.font.family
            font.pixelSize: 14
            selectByMouse: true
            verticalAlignment: TextInput.AlignVCenter
            inputMethodHints: Qt.ImhDigitsOnly
            validator: RegExpValidator {
                regExp: /^\d{0,4}(\/\d{0,2})?(\/\d{0,2})?$/
            }
            text: Qt.formatDate(control.date, "yyyy/MM/dd")
            onTextChanged: control.syncDateFromText()
            onEditingFinished: control.acceptText()
            onActiveFocusChanged: {
                if (activeFocus) {
                    if (control.activeSection < 0)
                        control.activateDatePart(0)
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    var position = dateInput.cursorPosition
                    control.activateDatePart(control.datePartAt(position))
                }
            }
        }

        Text {
            anchors.left: parent.left
            anchors.right: popupButton.left
            anchors.leftMargin: 10
            anchors.rightMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            visible: control.showLunar
            text: Lunar.lunarMonthDay(control.date)
            color: control.controlTextColor
            font.family: app.font.family
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        Item {
            id: popupButton
            width: 24
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            readonly property int borderInset: control.inputFocused ? 2 : (control.hovered ? 1 : 0)

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: popupButton.borderInset
                anchors.top: parent.top
                anchors.topMargin: popupButton.borderInset
                anchors.bottom: parent.verticalCenter
                clip: true

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: popupButton.height
                    radius: control.borderRadius
                    color: upMouseArea.pressed ? parseColor.getDtColor("kcomponent-click")
                                               : upMouseArea.containsMouse ? parseColor.getDtColor("kcomponent-hover")
                                                                           : "transparent"
                }
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: popupButton.borderInset
                anchors.top: parent.verticalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: popupButton.borderInset
                clip: true

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: popupButton.height
                    radius: control.borderRadius
                    color: downMouseArea.pressed ? parseColor.getDtColor("kcomponent-click")
                                                 : downMouseArea.containsMouse ? parseColor.getDtColor("kcomponent-hover")
                                                                             : "transparent"
                }
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.top
                anchors.verticalCenterOffset: popupButton.height / 4
                width: 16
                height: 16
                sourceSize.width: 16
                sourceSize.height: 16
                source: control.spinButtonIcon("ukui-up-symbolic")
                cache: false
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.bottom
                anchors.verticalCenterOffset: -popupButton.height / 4
                width: 16
                height: 16
                sourceSize.width: 16
                sourceSize.height: 16
                source: control.spinButtonIcon("ukui-down-symbolic")
                cache: false
            }

            MouseArea {
                id: upMouseArea
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.verticalCenter
                enabled: control.enabled
                hoverEnabled: true
                onClicked: control.adjustDate(1)
            }

            MouseArea {
                id: downMouseArea
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.verticalCenter
                anchors.bottom: parent.bottom
                enabled: control.enabled
                hoverEnabled: true
                onClicked: control.adjustDate(-1)
            }
        }

        MouseArea {
            anchors.left: parent.left
            anchors.right: popupButton.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            enabled: control.enabled && control.showLunar
            onClicked: {
                control.activateDatePart(0)
            }
        }
    }

    Item {
        id: calendarAnchor
        width: control.width
        height: control.height
    }

    Private.KPopup {
        id: calendar
        parent: calendarAnchor
        x: 0
        y: control.height + 4
        width: 300
        height: 336
        padding: 24
        modal: false
        focus: false
        acceptFocus: false
        focusTarget: control.showLunar ? control : dateInput
        animated: false
        transparent: false
        closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent

        onAboutToShow: control.positionCalendar()
        onClosed: control.activeSection = -1

        Column {
            width: 252
            height: 288
            spacing: 0

            Item {
                width: parent.width
                height: 36

                Repeater {
                    model: ["go-first.symbolic", "ukui-start-symbolic",
                            "ukui-end-symbolic", "go-first-rtl.symbolic"]
                    delegate: Item {
                        width: 36
                        height: 36
                        x: index < 2 ? index * 36 : parent.width - (4 - index) * 36
                        Image {
                            anchors.centerIn: parent
                            width: 16
                            height: 16
                            sourceSize.width: 16
                            sourceSize.height: 16
                            source: "image://imageProvider/" + modelData + "/normal"
                            cache: false
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                var delta = index === 0 ? -12 : index === 1 ? -1 : index === 2 ? 1 : 12
                                control.navigateMonth(delta)
                            }
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: Qt.formatDate(control.displayedMonth, "yyyy年M月")
                    color: control.currentTextColor
                    font.family: app.font.family
                    font.pixelSize: 14
                }
            }

            Row {
                width: parent.width
                height: 36
                Repeater {
                    model: ["一", "二", "三", "四", "五", "六", "日"]
                    delegate: Text {
                        width: 36
                        height: 36
                        text: modelData
                        color: control.currentTextColor
                        font.family: app.font.family
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Grid {
                width: parent.width
                columns: 7
                rows: 6
                rowSpacing: 0
                columnSpacing: 0

                Repeater {
                    model: control.calendarDays
                    delegate: Item {
                        id: dayItem
                        width: 36
                        height: 36
                        property var day: modelData
                        property bool selected: control.sameDate(day.value, control.date)
                        property bool hovered: mouseArea.containsMouse
                        property color textColor: selected ? "#ffffff" : hovered ? "#fafafa"
                                                  : day.currentMonth ? control.currentTextColor : "#c8c8c8"

                        Rectangle {
                            anchors.fill: parent
                            visible: dayItem.selected || dayItem.hovered
                            radius: 6
                            color: dayItem.selected ? control.brandColor : "#b4b4b4"
                            antialiasing: true
                        }

                        Text {
                            width: parent.width
                            y: control.showLunar ? 3 : 0
                            height: control.showLunar ? 21 : parent.height
                            text: dayItem.day.value.getDate()
                            color: dayItem.textColor
                            font.family: app.font.family
                            font.pixelSize: control.showLunar ? 13 : 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            visible: control.showLunar
                            width: parent.width
                            y: 19
                            height: 14
                            text: Lunar.lunarDay(dayItem.day.value)
                            color: dayItem.selected || dayItem.hovered ? dayItem.textColor : "#b3b3b3"
                            font.family: app.font.family
                            font.pixelSize: 9
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                control.setDate(dayItem.day.value)
                                calendar.close()
                            }
                        }
                    }
                }
            }
        }
    }
}
