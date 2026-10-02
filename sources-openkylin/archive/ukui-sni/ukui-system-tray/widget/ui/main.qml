import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0

WidgetItem {
    property bool isHorizontal: (Widget.orientation === Types.Horizontal)
    property int pos: Widget.container.position
    property real ratio: parent.ratio ? parent.ratio : 1
    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: childrenRect.width
    Layout.preferredHeight: childrenRect.height
    Layout.minimumWidth: isHorizontal ? dropArea.controlPanelLength + dropArea.itemShort : -1
    Layout.minimumHeight: isHorizontal ? -1 : dropArea.controlPanelLength + dropArea.itemShort
    clip: true
    Widget.active: dropArea.panelActive

    TrayView {
        id: dropArea
        property bool panelActive: false
        property bool onTopBar: parent.height < 40
        scaleFactor: parent.ratio
        isHorizontal: parent.isHorizontal
        itemWidth: onTopBar ? 28 : isHorizontal ?
                                  scaleFactor * 28 : scaleFactor * 40
        itemHeight: onTopBar ? 28 : isHorizontal ?
                                   scaleFactor * 40 : scaleFactor * 28
        itemLong: onTopBar ? 28 : scaleFactor * 40
        itemShort: onTopBar ? 28 : scaleFactor * 28
        panelPos: parent.pos
    }

    onWidthChanged: {
        dropArea.changeFoldWindowPosition();
    }

    onHeightChanged: {
        dropArea.changeFoldWindowPosition();
    }
}
