import QtQuick 2.12
import QtQuick.Controls 2.5

Label {
    property bool showTooltip: true  // 是否显示ToolTip
    elide: Text.ElideRight
    ToolTip.text: text
    ToolTip.visible: (showTooltip && truncated) ?  hovered : false
    ToolTip.delay: 500
}
