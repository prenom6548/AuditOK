import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.15

import org.ukui.systemTray 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

DropArea {
    property bool isEmpty: foldAreaModel.count > 0 ? false : true
    property int sourceIndex
    property bool foldWindowVisible: false
    signal foldAreaDragFinshed(int selectIndex)
    width: foldAreaModel.count > 5 ? foldView.cellWidth * 5 + 8 : foldAreaModel.count * foldView.cellWidth + 8
    height: (foldAreaModel.count % 5) === 0 ?
                Math.floor((foldAreaModel.count / 5)) * foldView.cellHeight + 8 : Math.floor((foldAreaModel.count / 5) + 1) * foldView.cellHeight + 8

    onEntered: {
        let source = drag.getDataAsString("source");
        if (source !== "ukui-system-tray") {
            drag.accepted = false;
        }
    }

    UkuiItems.DtThemeBackground {
        anchors.fill: parent
        backgroundColor: Platform.GlobalTheme.baseActive

        GridView {
            id: foldView
            property string idSelect: ""
            property bool hold: false
            anchors.fill: parent
            anchors.topMargin: 4
            anchors.leftMargin: 4
            cellWidth: dropArea.scaleFactor * 32
            cellHeight: dropArea.scaleFactor * 40
            interactive: false

            moveDisplaced: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 200 }
                NumberAnimation { properties: "z"; to: 1; easing.type: Easing.OutQuad; duration: 1 }
            }

            model: DelegateModel {
                id: foldAreaModel
                model: foldModel

                onCountChanged: {
                    if (!trayview.isFold) foldView.forceLayout();
                }

                delegate: DropArea {
                    property int vIndex: DelegateModel.itemsIndex
                    id: foldDropArea
                    width: foldView.cellWidth
                    height: foldView.cellHeight
                    clip: true

                    GridView.onAdd: SequentialAnimation {
                        NumberAnimation {target: foldDropArea; property: "scale"; from: 0; to: 1; duration: 250; easing.type: Easing.InOutQuad}
                    }

                    onDropped: {
                        if (drag.source.rootId !== dropArea) return;
                        ItemModel.setOrderInGroup(foldModel, foldModel.index(foldBase.sourceIndex, 0), drag.source.selectIndex);
                    }

                    onEntered: {
                        let source = drag.getDataAsString("source");
                        if ((source !== "ukui-system-tray") || (drag.source.rootId !== dropArea)) {
                            drag.accepted = false;
                            return;
                        }
                        foldAreaModel.items.move(drag.source.selectIndex, vIndex)
                    }

                    Binding { target: controlFold; property: "selectIndex"; value: vIndex}
                    UkuiItems.DtThemeBackground {
                        id: controlBaseFold
                        clip: true
                        useStyleTransparency: false
                        anchors.fill: parent
                        backgroundColor: foldView.hold ? Platform.GlobalTheme.kContainGeneralAlphaNormal
                                                       : controlFold.containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                                   : controlFold.containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                                               : Platform.GlobalTheme.kContainGeneralAlphaNormal

                        radius: Platform.GlobalTheme.kRadiusNormal

                        TrayIcon {
                            id: controlFold
                            Accessible.role: Accessible.Button
                            Accessible.description: "This is a ukui-system-tray button of ukui-sni"
                            Accessible.name: model.Id === undefined ? "" : "ukui-system-tray_" + model.Id
                            property int selectIndex: 0
                            property var rootId: dropArea
                            anchors.fill: parent
                            appStatus: model.Status
                            appAttentionMovieName: model.AttentionMovieName
                            appIcon: model.Icon
                            appAttentionIcon: model.AttentionIcon
                            appOverlayIcon: model.OverlayIcon

                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            Drag.active: false
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            Drag.dragType: Drag.Automatic
                            Drag.mimeData: {
                                "source": "ukui-system-tray"
                            }
                            drag.target: controlFold;
                            drag.onActiveChanged: {
                                if (drag.active) {
                                    toStartDrag();
                                    controlFold.opacity = 0;
                                } else {
                                    x = 0;
                                    y = 0;
                                }
                                Drag.active = drag.active;
                            }
                            UkuiItems.Tooltip {
                                id: tooltip
                                anchors.fill: parent
                                mainText: model.ToolTipTitle.length === 0 ? model.Title : model.ToolTipTitle
                                posFollowCursor: false
                                active: foldWindowVisible
                                location: {
                                    switch (dropArea.panelPos) {
                                        case UkuiItems.Types.Bottom:
                                        case UkuiItems.Types.Left:
                                        case UkuiItems.Types.Right:
                                            return UkuiItems.Dialog.BottomEdge;
                                        case UkuiItems.Types.Top:
                                            return UkuiItems.Dialog.TopEdge;
                                    }
                                }
                                margin: 8
                            }

                            onClicked: {
                                if (mouse.button === Qt.LeftButton) {
                                    foldAreaModel.model.onActivate(foldAreaModel.modelIndex(index));
                                }
                            }

                            onPressed: {
                                if (mouse.button === Qt.RightButton) {
                                    if(foldView.hold) return;
                                    foldAreaModel.model.onContextMenu(foldAreaModel.modelIndex(index), foldDropArea);
                                    return;
                                }

                                if (mouse.button === Qt.LeftButton) {
                                    controlFold.grabToImage(function(result) {
                                        controlFold.Drag.imageSource = result.url;
                                    })
                                }
                            }
                            onReleased: {
                            }
                            Drag.onDragFinished: {
                                foldView.hold = false;
                                if (dropArea.inserted) {
                                    foldAreaDragFinshed(foldBase.sourceIndex);
                                } else {
                                    controlFold.parent = controlBaseFold;
                                    controlFold.opacity = 1
                                    if (foldBase.sourceIndex === vIndex) return;
                                    ItemModel.setOrderInGroup(foldModel, foldModel.index(foldBase.sourceIndex, 0), vIndex);
                                }
                            }
                            function toStartDrag() {
                                foldView.hold = true;
                                x = controlFold.mapToItem(foldView,0,0).x;
                                y = controlFold.mapToItem(foldView,0,0).y;
                                foldBase.sourceIndex = vIndex;
                                dropArea.dragType = 1;
                            }
                        }
                    }
                }
            }
        }
    }
}
