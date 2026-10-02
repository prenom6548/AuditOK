/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KProgressLabel。
 * 配色严格对齐 kprogresslabel.cpp：
 *  - 圆点：未完成 -> midlight-active + 白字；当前(isSucceed) 与 已完成(colorFlag) -> highlight-active
 *    已完成画 dialog-apply 勾，当前画步骤号
 *  - 连线：index < currentIndex -> highlight-active；否则 midlight-active；2px
 *  - 文本：当前 -> highlight-active；其余 brighttext-active @0.35
 *  - circleSize 默认 24x24
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import org.ukui.qqc2style.private 1.1 as StylePrivate

Item {
    id: root

    property var list1: []    // 上行文本（步骤号）
    property var list2: []    // 下行文本（可选，与 list1 等长）
    property int currentIndex: -1
    property size circleSize: Qt.size(24, 24)

    implicitWidth: 360
    implicitHeight: list1.length > 0 ? 80 : 24

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    signal currentChanged(int index)

    readonly property color _midlight: parseColor.getDtColor("midlight-active")
    readonly property color _highlight: parseColor.getDtColor("highlight-active")
    readonly property color _kwhite: parseColor.getDtColor("kwhite")

    function _textColor(i) {
        return i === root.currentIndex ? root._highlight : Qt.rgba(1, 1, 1, 0.35)
    }
    // brighttext-active @0.35 近似为白色@0.35（深浅色下 midlight 圆点配白字）

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // 第一行：圆点 + 连线
        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Repeater {
                model: root.list1.length
                RowLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        id: circle
                        width: root.circleSize.width
                        height: root.circleSize.height
                        radius: width / 2
                        Layout.alignment: Qt.AlignVCenter
                        // i < currentIndex -> 已完成(colorFlag)；i == currentIndex -> 当前(isSucceed)；i > -> 未完成
                        property bool _completed: index < root.currentIndex
                        property bool _current: index === root.currentIndex
                        color: (_completed || _current) ? root._highlight : root._midlight
                        antialiasing: true

                        Text {
                            // 当前画步骤号，已完成画勾图标
                            anchors.centerIn: parent
                            visible: !circle._completed
                            text: root.list1[index] || ""
                            font: app.font
                            color: root._kwhite
                        }

                        Image {
                            anchors.centerIn: parent
                            visible: circle._completed
                            source: visible ? "image://imageProvider/dialog-apply/normalWidthThemeColor" : ""
                            width: 16; height: 16
                            fillMode: Image.PreserveAspectFit; smooth: true; cache: false
                        }
                    }

                    // 连线（非末项）
                    Rectangle {
                        visible: index < root.list1.length - 1
                        Layout.fillWidth: true
                        Layout.preferredHeight: 2
                        Layout.alignment: Qt.AlignVCenter
                        color: index < root.currentIndex ? root._highlight : root._midlight
                        antialiasing: true
                    }
                }
            }
        }

        // 第二行：文本
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: root.circleSize.width / 2 - 6
            Layout.rightMargin: root.circleSize.width / 2 - 6
            spacing: 0

            Repeater {
                model: (root.list2.length > 0 ? root.list2 : root.list1).length
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: txt.implicitHeight
                    Text {
                        id: txt
                        anchors.centerIn: parent
                        text: (root.list2.length > 0 ? root.list2 : root.list1)[index] || ""
                        font: app.font
                        color: root._textColor(index)
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width
                    }
                }
            }
        }
    }

    function setStringList(l1, l2) {
        list1 = l1
        list2 = (l2 !== undefined && l2 !== null) ? l2 : []
    }
    function setCurrentIndex(i) { currentIndex = i; currentChanged(i) }
    function setCircleSize(s) { circleSize = s }
}
