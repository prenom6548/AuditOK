import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.15
import org.ukui.systemTray 1.0
import org.ukui.quick.items 1.0 as UkuiItems

ListView {
    id: controlPanelView
    height: dropArea.isHorizontal ? dropArea.scaleFactor * 40 : count * dropArea.scaleFactor * 24 + (count - 1) * spacing;
    width: dropArea.isHorizontal ? count * dropArea.scaleFactor * 24 + (count - 1) * spacing : dropArea.scaleFactor * 40
    orientation: dropArea.isHorizontal ? ListView.Horizontal : ListView.Vertical
    layoutDirection: ListView.RightToLeft
    verticalLayoutDirection: ListView.Qt.BottomToTop
    spacing: 0
    interactive: false

    model: DelegateModel {
        id: panelModel
        model: ControlPanelModel

        delegate: MouseArea {
            width: dropArea.isHorizontal ? dropArea.scaleFactor * 24 : dropArea.scaleFactor * 40
            height: dropArea.isHorizontal ? dropArea.scaleFactor * 40 : dropArea.scaleFactor * 24
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true

            Accessible.role: Accessible.Button
            Accessible.description: "This is a ukui-system-tray button of ukui-sni"
            Accessible.name: model.Id === undefined ? "" : "ukui-system-tray_" + model.Id

            Component.onCompleted: {
                Qt.callLater(()=>{
                    Accessible.name = Qt.binding(function () {
                        return model.Id === undefined ? "" : "ukui-system-tray_" + model.Id;
                    });
                });
            }

            UkuiItems.Icon {
                id: icon
                anchors.centerIn: parent
                width: Math.round(dropArea.scaleFactor * (dropArea.onTopBar ? 20 : 16))
                height: Math.round(dropArea.scaleFactor * (dropArea.onTopBar ? 20 : 16))
                mode: UkuiItems.Icon.AutoHighlight
                source: model.Icon

                Component.onCompleted: {
                    Qt.callLater(()=>{
                        source = Qt.binding(function () {return model.Icon});
                    });
                }
            }

            UkuiItems.Tooltip {
                id: tooltip
                anchors.fill: parent
                mainText: model.ToolTipTitle === undefined ? "" : model.ToolTipTitle.length === 0 ? model.Title : model.ToolTipTitle
                posFollowCursor: false
                location: {
                    switch(dropArea.panelPos) {
                        case UkuiItems.Types.Bottom:
                            return UkuiItems.Dialog.BottomEdge;
                        case UkuiItems.Types.Left:
                            return UkuiItems.Dialog.LeftEdge;
                        case UkuiItems.Types.Top:
                            return UkuiItems.Dialog.TopEdge;
                        case UkuiItems.Types.Right:
                            return UkuiItems.Dialog.RightEdge;
                    }
                }
                margin: dropArea.isHorizontal ? (dropArea.height - control.height)/2 + 8 : (dropArea.width - control.width)/2 + 8

                Component.onCompleted: {
                    Qt.callLater(()=>{
                        mainText = Qt.binding(function () {
                            return model.ToolTipTitle === undefined ? "" : model.ToolTipTitle.length === 0 ? model.Title : model.ToolTipTitle;
                        });
                    });
                }
            }

            onClicked: {
                if (mouse.button === Qt.LeftButton) {
                    panelModel.model.activate(index);
                }
                if (mouse.button === Qt.RightButton) {
                    panelModel.model.showContextMenu(panelModel.modelIndex(index), showDropArea);
                    dropArea.callMenu = true;
                }
            }
        }
    }
}
