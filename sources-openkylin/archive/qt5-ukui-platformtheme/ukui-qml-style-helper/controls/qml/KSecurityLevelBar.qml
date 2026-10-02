/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KSecurityLevelBar.
 * 三段彩条 + 等级文字。颜色对齐 ksecuritylevelbar.cpp：
 *  Low -> kerror-normal, Medium -> kwarning-normal, High -> ksuccess-normal,
 *  未点亮段 -> kcomponent-normal。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    id: control

    enum SecurityLevel { Low = 0, Medium = 1, High = 2 }

    property int securityLevel: KSecurityLevelBar.SecurityLevel.Low
    property color _labelColor: getDtColor("kfont-primary")
    property var _segColors: [getDtColor("kerror-normal"), getDtColor("kcomponent-normal"), getDtColor("kcomponent-normal")]

    implicitWidth: 300
    implicitHeight: 30

    signal levelChanged(int level)

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    Connections {
        target: app
        function onParametryChanged() { updateColors() }
        function onIsDarkChanged() { updateColors() }
    }

    onSecurityLevelChanged: updateSegColors()

    Component.onCompleted: updateColors()

    function getDtColor(colorName) {
        return parseColor.getDtColor(colorName)
    }

    function segColor(index) {
        if (securityLevel === KSecurityLevelBar.SecurityLevel.Low)
            return index === 0 ? getDtColor("kerror-normal") : getDtColor("kcomponent-normal")
        if (securityLevel === KSecurityLevelBar.SecurityLevel.Medium)
            return index <= 1 ? getDtColor("kwarning-normal") : getDtColor("kcomponent-normal")
        return getDtColor("ksuccess-normal")
    }

    function updateSegColors() {
        _segColors = [segColor(0), segColor(1), segColor(2)]
    }

    function updateColors() {
        _labelColor = getDtColor("kfont-primary")
        updateSegColors()
    }

    readonly property string _levelText: {
        switch (securityLevel) {
        case KSecurityLevelBar.SecurityLevel.Low: return qsTr("Low")
        case KSecurityLevelBar.SecurityLevel.Medium: return qsTr("Medium")
        case KSecurityLevelBar.SecurityLevel.High: return qsTr("High")
        }
        return ""
    }

    Text {
        id: label
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: control._levelText
        font: app.font
        color: control._labelColor
    }

    Row {
        anchors.left: label.right
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        spacing: 5

        Repeater {
            model: 3
            Rectangle {
                width: 32
                height: 8
                radius: height / 2
                color: control._segColors[index]
                antialiasing: true
            }
        }
    }

    function setSecurityLevel(level) {
        securityLevel = level
        levelChanged(level)
    }
}
