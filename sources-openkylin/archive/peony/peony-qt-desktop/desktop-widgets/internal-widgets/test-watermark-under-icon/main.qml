import QtQuick 2.15
import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    Rectangle {
        anchors.fill: parent
        color: "green"
        opacity: 0.5
    }
    Text {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: "测试水印\n图标之下\n不可拖拽"
    }
}
