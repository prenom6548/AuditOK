import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property date targetTime // 目标时间点
    property string relativeText: "" // 显示的相对时间文本
    readonly property int updateInterval: 60000 // 每分钟更新一次
    property string fontClo

    Timer {
        id: updateTimer
        interval: root.updateInterval
        repeat: true
        running: true
        onTriggered: updateRelativeTime()
    }

    onTargetTimeChanged: {
        updateRelativeTime()
    }

    Component.onCompleted: {
        if (targetTime) {
            updateRelativeTime();
        }
    }

    function updateRelativeTime() {
        var now = new Date();
        var diff = now - targetTime;
        var seconds = Math.floor(diff / 1000);
        var minutes = Math.floor(seconds / 60);
        var hours = Math.floor(minutes / 60);
        var days = Math.floor(hours / 24);

        if (minutes < 1) {
            relativeText = qsTr("just now");
        } else if (minutes < 60) {
            relativeText = minutes + (minutes === 1 ? qsTr(" minute ago") : qsTr(" minutes ago"));
        } else if (hours < 24) {
            relativeText = hours + (hours === 1 ? qsTr(" hour ago") : qsTr(" hours ago"));
        } else if (days < 3) {
            relativeText = days + (days === 1 ? qsTr(" day ago") : qsTr(" days ago"));
        } else {
            relativeText = qsTr("3 days ago");
        }
        //console.log("targetTime",targetTime)
        //console.log("minutes=",minutes)
    }

    Label {
        id: test
        height: 0
        width: 0
    }

    Label {
        anchors.fill: parent
        anchors.right: parent.right
        text: relativeText
        //width: 100
        color: fontClo
        font.pixelSize: test.font.pixelSize - 1
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter

        Rectangle {
            id: backWindow
            anchors.fill: parent
            color: 'blue'
            opacity: tray.transparency
        }
    }
}
