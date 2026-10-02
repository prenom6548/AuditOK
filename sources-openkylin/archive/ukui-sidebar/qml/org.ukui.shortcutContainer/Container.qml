import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import org.ukui.quick.items 1.0

WidgetContainerItem {
    id: containerItem
    anchors.fill: parent

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        Repeater {
            id: repeater
            model: containerItem.widgetItemModel
            delegate: widgetLoaderComponent
        }
    }
    Component {
        id: widgetLoaderComponent
        Item {
            id: widgetParent
            clip: true

            property Item widgetItem: model.widgetItem
            Layout.fillWidth: widgetItem.Layout.fillWidth
            Layout.fillHeight: widgetItem.Layout.fillHeight
            Layout.minimumHeight: widgetItem.Layout.minimumHeight
            Layout.maximumHeight: widgetItem.Layout.maximumHeight
            Layout.minimumWidth : widgetItem.Layout.minimumWidth
            Layout.maximumWidth : widgetItem.Layout.maximumWidth
            Layout.preferredHeight: widgetItem.Layout.preferredHeight
            Layout.preferredWidth: widgetItem.Layout.preferredWidth

            onWidgetItemChanged: {
                if (widgetItem) {
                    widgetItem.parent = widgetParent
                    widgetItem.anchors.fill = Qt.binding(function () {return widgetParent})
                }
            }
        }
    }
}
