import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15

import org.ukui.systemTray 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

DropArea {
    property real scaleFactor
    property real itemWidth
    property real itemHeight
    property real itemLong
    property real itemShort
    property real controlPanelWidth: isHorizontal ? controlPanelLength : itemLong
    property real controlPanelHeight: isHorizontal ? itemLong : controlPanelLength
    property int controlPanelLength: scaleFactor * getControlPanelLength()
    property bool isHorizontal: true
    property int panelPos
    property int dragType
    property bool inserted: false
    property int targetIndex
    property int indexInFold
    property bool isOverLength: false
    property bool callMenu: false
    property int showCount: 11
    property int maxLength: dropArea.isHorizontal
                            ? parent.parent.Layout.maximumWidth
                            : parent.parent.Layout.maximumHeight

    height: isHorizontal ? parent.height : flowArea.height
    width: isHorizontal ? flowArea.width : parent.width

    MouseArea {
        z: 1
        anchors.fill: parent
        acceptedButtons: {
            let button = Qt.NoButton;
            if (FunctionControl.disableMouseLeftButton) {
                button |= Qt.LeftButton
            }
            if (FunctionControl.disableMouseRightButton) {
                button |= Qt.RightButton
            }
            return button;
        }
        enabled: FunctionControl.disableMouseLeftButton | FunctionControl.disableMouseRightButton
    }

    onItemShortChanged: {
        trayview.viewWidth = trayview.calculateLength();
    }

    onControlPanelLengthChanged: {
        getShowCount();
        trayview.viewWidth = trayview.calculateLength();
    }

    onPanelPosChanged: {
        foldWindow.hide();
        trayview.isFold = false;
    }

    onMaxLengthChanged: {
        getShowCount();
    }

    GroupModel {
        id: showModel
        modelType: ItemGroupModel.Show
        showCount: dropArea.showCount
    }

    GroupModel {
        id: foldModel
        modelType: ItemGroupModel.Fold
        showCount: dropArea.showCount
    }

    function getShowCount() {
        dropArea.showCount = (maxLength - (controlPanelLength - dropArea.itemShort)) / (dropArea.itemShort + trayview.viewSpacing) - 2;
    }

    function getControlPanelLength() {
        if (!ControlPanelModel.enable) return 28;
        switch(ControlPanelModel.rowCount) {
            case 0:
            case 1:
                return 28;
            case 2:
                return 60;
            case 3:
                return 80;
            default:
                return 0;
        }
    }

    Item {id: dragItem}

    Flow {
        id: flowArea
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        flow: dropArea.isHorizontal ? GridLayout.LeftToRight : GridLayout.TopToBottom
        spacing: trayview.viewSpacing

        DropArea {
            width: foldItem.visible ? dropArea.itemWidth : 0
            height: foldItem.visible ? dropArea.itemHeight : 0

            onEntered: {
                let source = drag.getDataAsString("source");
                if (source !== "ukui-system-tray") {
                    drag.accepted = false;
                }
            }
            onDropped: {
                if (drag.source.rootId !== dropArea) return;
                if (dragType === 1) {
                    if (trayModel.count > dropArea.showCount) return;
                    ItemModel.setOrderInGroup(foldModel, foldModel.index(foldBase.sourceIndex,0), 0)
                    ItemModel.setSeparateIndex(showModel.currentSeparateIndex + 1);
                }
                if (dragType === 0) {
                    ItemModel.setOrderInGroup(showModel, showModel.index(trayview.sourceIndex, 0), trayModel.items.count - 1);
                    ItemModel.setSeparateIndex(showModel.currentSeparateIndex - 1);
                    dropArea.panelActive = false;
                }
            }

            UkuiItems.DtThemeBackground {
                id: foldItem
                visible: !foldBase.isEmpty
                clip: true
                useStyleTransparency: false
                anchors.fill: parent
                backgroundColor: foldControl.containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                           : foldControl.containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                                       : Platform.GlobalTheme.kContainGeneralAlphaNormal

                radius: dropArea.onTopBar ? 0 : Platform.GlobalTheme.kRadiusNormal

                TrayIcon {
                    id: foldControl
                    Accessible.role: Accessible.Button
                    Accessible.description: "This is a foldControl button on ukui-system-tray of ukui-sni"
                    Accessible.name: "foldControl-button"
                    width: dropArea.itemWidth
                    height: dropArea.itemHeight
                    appIcon: showModel.foldIcon()
                    iconRotation: appIconRotation()
                    hoverEnabled: true
                    enabled: foldItem.visible
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    UkuiItems.Tooltip {
                        anchors.fill: parent
                        mainText: trayview.isFold ? qsTr("Folding display icon") : qsTr("Expand hidden icon")
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
                        margin: dropArea.isHorizontal ? (dropArea.height - foldControl.height)/2 + 8 : (dropArea.width - foldControl.width)/2 + 8
                    }

                    onClicked: {
                        if (mouse.button === Qt.LeftButton) {
                            foldWindowReply();
                        }
                    }

                    function appIconRotation() {
                        if (dropArea.panelPos === UkuiItems.Types.Left) {
                            return trayview.isFold ? 180 : 0;
                        } else if (dropArea.panelPos === UkuiItems.Types.Top) {
                            return trayview.isFold ? 270 : 90;
                        } else if (dropArea.panelPos === UkuiItems.Types.Right) {
                            return trayview.isFold ? 0 : 180;
                        } else if (dropArea.panelPos === UkuiItems.Types.Bottom) {
                            return trayview.isFold ? 90 : 270;
                        }
                        return 0;
                    }

                    function foldWindowReply() {
                        if (trayview.isFold) {
                            foldWindow.hide();
                            trayview.isFold = false;
                        } else {
                            if (!foldBase.isEmpty) {
                                updateFoldWindowPosition();
                                foldWindow.show();
                            }
                            trayview.isFold = true;
                        }
                    }

                    function updateFoldWindowPosition() {
                        if (dropArea.panelPos === UkuiItems.Types.Left) {
                            foldWindow.slideWindow.fromEdge = Platform.WindowProxy.LeftEdge
                            foldWindow.location = UkuiItems.Types.LeftEdge
                        } else if (dropArea.panelPos === UkuiItems.Types.Top) {
                            foldWindow.slideWindow.fromEdge = Platform.WindowProxy.TopEdge
                            foldWindow.location = UkuiItems.Types.TopEdge
                        } else if (dropArea.panelPos === UkuiItems.Types.Right) {
                            foldWindow.slideWindow.fromEdge = Platform.WindowProxy.RightEdge
                            foldWindow.location = UkuiItems.Types.RightEdge
                        } else if (dropArea.panelPos === UkuiItems.Types.Bottom) {
                            foldWindow.slideWindow.fromEdge = Platform.WindowProxy.BottomEdge
                            foldWindow.location = UkuiItems.Types.BottomEdge
                        }
                        foldWindow.margin = dropArea.isHorizontal ? (dropArea.height - foldItem.height)/2 + 8 : (dropArea.width - foldItem.width)/2 + 8;
                    }
                }


                UkuiItems.Dialog {
                    id: foldWindow
                    visualParent: foldItem
                    mainItem: foldBase
                    enableWindowBlur: true
                    type: Platform.WindowType.SystemWindow
                    slideWindow.offset: 0

                    minimumWidth: foldBase.width
                    maximumWidth: minimumWidth
                    minimumHeight: foldBase.height
                    maximumHeight: minimumHeight

                    FoldArea {
                        id: foldBase
                        foldWindowVisible: foldWindow.visible
                        onFoldAreaDragFinshed: fromIndex => {
                                                   if (dropArea.inserted) {
                                                       if (dropArea.dragType === 1) {
                                                           dropArea.inserted = false;
                                                           if (trayview.itemAtIndex(dropArea.targetIndex).isFixed) return;
                                                           trayModel.items.remove(dropArea.targetIndex);
                                                           ItemModel.setOrderBetweenGroups(foldModel,
                                                                                           foldModel.index(dropArea.isOverLength ? fromIndex + 1 : fromIndex, 0),
                                                                                           showModel,
                                                                                           dropArea.targetIndex);
                                                           ItemModel.setSeparateIndex(showModel.currentSeparateIndex + 1);
                                                       }
                                                   }
                                               }

                        onIsEmptyChanged: {
                            foldItem.visible = !foldBase.isEmpty;
                            foldWindow.hide();
                            trayview.isFold = false;
                        }
                    }

                    onVisibleChanged: {
                        dropArea.panelActive = foldWindow.visible;
                    }

                    onActiveFocusItemChanged: {
                        if (foldControl.pressed) return;
                        if (trayview.isFold) {
                            if (activeFocusItem === null) {
                                foldWindow.hide();
                                trayview.isFold = false;
                            }
                        }
                    }
                }
            }
        }

        ListView {
            id: trayview
            property string idSelect: ""
            property bool isFold: false
            property bool hold: false
            property int sortIndex
            property int sourceIndex
            property real viewWidth: 0
            property int viewSpacing: dropArea.onTopBar ? 6 : 0
            height: dropArea.isHorizontal ? dropArea.itemHeight : viewWidth
            width: dropArea.isHorizontal ? viewWidth : dropArea.itemWidth
            orientation: dropArea.isHorizontal ? ListView.Horizontal : ListView.Vertical
            layoutDirection: ListView.RightToLeft
            verticalLayoutDirection: ListView.Qt.BottomToTop
            spacing: viewSpacing
            interactive: false

            onViewSpacingChanged: {
                trayview.viewWidth = trayview.calculateLength();
                dropArea.getShowCount();
            }

            Component.onCompleted: {
                var i = trayview.calculateLength();
                viewWidth = i;
            }

            displaced: Transition {
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            trayview.viewWidth = trayview.calculateLength();
                        }
                    }
                    NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 200 }
                    NumberAnimation { properties: "z"; to: 1; easing.type: Easing.OutQuad; duration: 1 }
                    ScriptAction {
                        script: {
                            trayview.viewWidth += 1;
                            trayview.viewWidth -= 1;
                        }
                    }
                }
            }
            move: Transition {
                ScriptAction {
                    script: {
                        trayview.viewWidth = trayview.calculateLength();
                    }
                }
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 200 }
                NumberAnimation { properties: "z"; to: 0; easing.type: Easing.OutQuad; duration: 1 }
            }

            ListView.onAdd: SequentialAnimation {
                ScriptAction {
                    script: {
                        trayview.viewWidth = trayview.calculateLength();
                    }
                }
            }
            ListView.onRemove: SequentialAnimation {
                ScriptAction {
                    script: {
                        trayview.viewWidth = trayview.calculateLength();
                    }
                }
            }

            model: DelegateModel {
                id: trayModel
                model: showModel

                groups: DelegateModelGroup {
                    name: "foldItems"
                    includeByDefault: false
                }
                delegate: Item {
                    id: showDropArea
                    property int visualIndex: DelegateModel.itemsIndex
                    property bool isFixed: model.Fixed === undefined ? false : model.Fixed
                    property bool isControlPanel: ControlPanelModel.rowCount > 1 && model.Id === "ukui-sidebar"

                    width: isControlPanel ? dropArea.controlPanelWidth : dropArea.itemWidth
                    height: isControlPanel ? dropArea.controlPanelHeight : dropArea.itemHeight
                    clip: true

                    Binding { target: control; property: "selectIndex"; value: visualIndex}
                    UkuiItems.DtThemeBackground {
                        id: controlBase
                        clip: true
                        useStyleTransparency: false
                        anchors.fill: parent
                        backgroundColor: trayview.hold ? Platform.GlobalTheme.kContainGeneralAlphaNormal
                                                       : control.containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                               : control.containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                                       : Platform.GlobalTheme.kContainGeneralAlphaNormal

                        radius: dropArea.onTopBar ? 0 : Platform.GlobalTheme.kRadiusNormal

                        TrayIcon {
                            id: control
                            Accessible.role: Accessible.Button
                            Accessible.description: "This is a ukui-system-tray button of ukui-sni"
                            Accessible.name: model.Id === undefined ? "" : "ukui-system-tray_" + model.Id
                            property int selectIndex: 0
                            property var rootId: dropArea
                            anchors.fill: parent
                            appStatus: model.Status === undefined ? "" : model.Status
                            appAttentionMovieName: model.AttentionMovieName === undefined ? "" : model.AttentionMovieName
                            appIcon: model.Icon
                            appAttentionIcon: model.AttentionIcon
                            appOverlayIcon: model.OverlayIcon
                            showControlPanel: showDropArea.isControlPanel

                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            Drag.active: false
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            Drag.dragType: Drag.Automatic
                            Drag.mimeData: {
                                "source": "ukui-system-tray"
                            }
                            drag.target: model.Fixed ? null : dragItem
                            drag.onActiveChanged: {
                                if (drag.active) {
                                    toStartDrag();
                                    control.opacity = 0;
                                }
                                Drag.active = drag.active;
                            }

                            UkuiItems.Tooltip {
                                id: tooltip
                                anchors.fill: parent
                                mainText: model.ToolTipTitle === undefined ? "" : model.ToolTipTitle.length === 0 ? model.Title : model.ToolTipTitle
                                posFollowCursor: false
                                visible: !control.showControlPanel
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
                            }

                            onClicked: {
                                if (mouse.button === Qt.LeftButton) {
                                    trayModel.model.onActivate(trayModel.modelIndex(index));
                                }
                            }

                            onPressed: {
                                if (model.Fixed) return;
                                if (mouse.button === Qt.RightButton) {
                                    if (trayview.hold) return;
                                    trayModel.model.onContextMenu(trayModel.modelIndex(index), showDropArea);
                                    dropArea.callMenu = true;
                                    return;
                                }

                                if (mouse.button === Qt.LeftButton) {
                                    control.grabToImage(function(result) {
                                        control.Drag.imageSource = result.url;
                                    })
                                }
                            }

                            Drag.onDragFinished: {
                                trayview.hold = false;
                                control.opacity = 1
                                foldItem.visible = !foldBase.isEmpty;
                                dropArea.panelActive = false;
                                if (dropArea.dragType === 0) {
                                    if (trayview.itemAtIndex(visualIndex).isFixed) return;
                                    ItemModel.setOrderInGroup(showModel, showModel.index(trayview.sourceIndex, 0), visualIndex);
                                }
                            }
                            function toStartDrag() {
                                foldWindow.hide();
                                trayview.isFold = false;
                                trayview.hold = true;
                                trayview.sourceIndex = selectIndex;
                                dropArea.dragType = 0;
                                if (!foldItem.visible) foldItem.visible = true;
                                dropArea.panelActive = true;
                            }
                        }
                    }
                }
                onCountChanged: {
                    trayview.viewWidth = trayview.calculateLength();
                }
            }

            function calculateLength() {
                if (ControlPanelModel.rowCount > 1) {
                    update();
                    return trayModel.count * dropArea.itemShort + (trayModel.count - 1) * trayview.viewSpacing
                            + (dropArea.controlPanelLength - dropArea.itemShort);
                } else {
                    return trayModel.count * dropArea.itemShort + (trayModel.count - 1) * trayview.viewSpacing;
                }
            }
        }
    }

    function changeFoldWindowPosition() {
        if (foldWindow.visible) {
            foldWindow.updatePosition();
        }
    }

    onPositionChanged: drag => {
                           let source = drag.getDataAsString("source");
                           if ((source !== "ukui-system-tray") || (drag.source.rootId !== dropArea)) {
                               drag.accepted = false;
                               return;
                           }
                           var target = getTarget(drag.x, drag.y);
                           if (target === -1) return;
                           if (trayview.itemAtIndex(target).isFixed) {
                               drag.accepted = false;
                               return;
                           }
                           if (dropArea.dragType === 0) {
                               trayModel.items.move(drag.source.selectIndex, target);
                           } else if (dropArea.dragType === 1) {
                               dropArea.targetIndex = target;
                               if (dropArea.inserted) {
                                   var i = target
                                   trayModel.items.move(drag.source.selectIndex, target)
                                   drag.source.selectIndex = i
                               } else {
                                   dropArea.inserted = true;
                                   dropArea.indexInFold = drag.source.selectIndex;
                                   if (trayModel.count > dropArea.showCount) {
                                       dropArea.isOverLength = true;
                                       showModel.currentSeparateIndex--;
                                       foldModel.currentSeparateIndex--;
                                   } else {
                                       dropArea.isOverLength = false;
                                   }
                                   drag.source.selectIndex = target;
                                   trayModel.items.insert(target, {});
                               }
                           }
                       }
    onExited: {
        if (dropArea.inserted) {
            if (dropArea.isOverLength) {
                showModel.currentSeparateIndex++;
                foldModel.currentSeparateIndex++;
            }
            trayModel.items.remove(dropArea.targetIndex);
            dropArea.inserted = false;
            drag.source.selectIndex = dropArea.indexInFold;
        }
    }

    function getTarget(x, y) {
        switch (dropArea.panelPos) {
        case UkuiItems.Types.Top:
        case UkuiItems.Types.Bottom:
            return trayview.indexAt((x + trayview.contentX - trayview.x), y);
        case UkuiItems.Types.Left:
        case UkuiItems.Types.Right:
            return trayview.indexAt(x, y + trayview.contentY - trayview.y);
        default:
            return -1;
        }
    }

    function menuStateChange(state) {
        if (dropArea.callMenu) {
            dropArea.panelActive = foldWindow.visible ? true : state;
            dropArea.callMenu = state;
        }
    }

    Component.onCompleted: {
        ItemModel.menuStateChanged.connect(menuStateChange);
    }
    Component.onDestruction: {
        ItemModel.menuStateChanged.disconnect(menuStateChange);
    }
}
