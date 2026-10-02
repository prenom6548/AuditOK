import QtQuick 2.15
import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    // Rectangle {
    //     anchors.fill: parent
    //     color: "blue"
    //     opacity: 0.5
    // }
    Text {
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: "测试工作台\n图标并存\n可拖拽\n网格布局 instance id:" + Widget.instanceId + "\nscreen id:" + widget.container.instanceId
    }
}
