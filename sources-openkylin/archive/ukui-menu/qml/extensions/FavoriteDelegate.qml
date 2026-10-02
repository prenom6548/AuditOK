import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import org.ukui.menu.core 1.0
import org.ukui.menu.extension 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems
import AppControls2 1.0 as AppControls2

UkuiItems.DtThemeBackground {
    id: iconItem
    property bool hold: false
    property bool readyToHold: false
    property alias delegateLayout: itemLayout
    property alias mergePrompt: mergePrompt
    property bool isFullScreen: false
    radius: Platform.GlobalTheme.kRadiusMax
    useStyleTransparency: false
    backgroundColor: (hold || readyToHold) ? Platform.GlobalTheme.kContainGeneralAlphaNormal
                          : control.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                  : control.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                          : Platform.GlobalTheme.kContainGeneralAlphaNormal

    property int visualIndex
    property int aniDuration: 250
    Binding { target: itemLoader; property: "visualIndex"; value: visualIndex }

    // 合并提示框 52*52 ，backgroundColor的改变在状态切换的时候变
    UkuiItems.DtThemeBackground {
        id: mergePrompt
        height: width
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        useStyleTransparency: false
        alpha: 0
        z: -1
    }
    function keyPressed() {
        itemLoader.item.itemClicked(Qt.LeftButton);
        if (type === DataType.Folder) {
            folderLoader.forceActiveFocus()
        }
    }
    DropArea {
        id: delegateDropArea
        anchors.fill: parent

        // drag.source [itemLoader]
        onEntered: {
            // 拖拽对象为folder 或 左侧列表应用时
            if (!drag.source.sourceId) {
                drag.accepted = false;
                return;
            }

            if (!drag.source.isFolder && drag.source.sourceId !== model.id) {
                favoriteView.mergeToAppId = model.id;
                favoriteView.isMergeToFolder = (model.type === DataType.Folder);
                favoriteView.dragTypeIsMerge = true;

                // 添加应用合并动画
                itemLoader.item.state = "prepareToMerge";
            }
        }
        onExited: {
            if (!drag.source.sourceId) {
                return;
            }

            itemLoader.item.state = "normal";
        }

        onDropped: {
            if (type === DataType.Folder)  itemLoader.item.state = "normal";
        }

        DropArea {
            id: moveDropArea
            width: loaderBase.width
            height: loaderBase.height
            anchors.topMargin: isFullScreen ? 16 : 8
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter

            onEntered: {
                // 拖拽对象为folder 或 左侧列表应用时
                if (!drag.source.sourceId) {
                    drag.accepted = false;
                    return;
                }
                itemLoader.item.state = "normal";

                favoriteView.dragTypeIsMerge = false;
                favoriteView.viewModel.items.move(drag.source.visualIndex, iconItem.visualIndex);
            }
        }
    }

    MouseArea {
        id: control
        property bool isTouch: false
        anchors.fill: parent
        hoverEnabled: true
        pressAndHoldInterval: 300
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        drag.target: itemLoader
        drag.onActiveChanged: {
            if (control.isTouch && !iconItem.readyToHold) return;
            if (drag.active) {
                beginDrag();
            }
        }

        onClicked: {
            if (isFullScreen) {
                fullScreenUI.focus = false;
            } else {
                normalUI.focus = false;
            }

            itemLoader.item.itemClicked(mouse.button);
        }
        onPressAndHold: {
            if (!control.isTouch) return;
            if (mouse.button === Qt.LeftButton) {
                iconItem.readyToHold = true;
            }
        }
        onPressed: {
            if (mouse.button === Qt.RightButton) {
                iconItem.readyToHold = false;
                favoriteModel.openMenu(index);
                return;
            }

            itemLoader.grabToImage(function(result) {
                itemLoader.Drag.imageSource = result.url;
            })
            if (mouse.source === Qt.MouseEventSynthesizedBySystem ||
                    mouse.source === Qt.MouseEventSynthesizedByQt) {
                control.isTouch = true;
            } else {
                control.isTouch = false;
            }
        }
        onReleased: {
            iconItem.readyToHold = false;
        }
        UkuiItems.Tooltip {
            anchors.fill: parent
            posFollowCursor: true
            visible: iconText.truncated && control.containsMouse && !(iconItem.hold || iconItem.readyToHold)
            mainText: model.name
        }

        function beginDrag() {
            if (isFullScreen) {
                fullScreenUI.focus =false;
            } else {
                normalUI.focus = false;
            }

            iconItem.hold = true;
            favoriteView.exchangedStartIndex = itemLoader.visualIndex;
            itemLoader.sourceId = model.id;

            // 开始拖拽
            itemLoader.Drag.active = true;
            itemLoader.Drag.startDrag(Qt.CopyAction);
        }
    }

    ColumnLayout {
        id: itemLayout
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.bottomMargin: 14
        spacing: 4

        Item {
            id: loaderBase
            Layout.preferredWidth: isFullScreen ? 96 : 48
            Layout.preferredHeight: width
            Layout.alignment: Qt.AlignHCenter

            Accessible.role: Accessible.Button
            Accessible.name: model.name

            Loader {
                id: itemLoader
                anchors.fill: parent

                Drag.dragType: Drag.None
                Drag.proposedAction: Qt.CopyAction
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2
                Drag.source: itemLoader
                Drag.mimeData: {
                    "source": "ukui-menu",
                    "url": scheme + model.id
                }

                Drag.onActiveChanged: {
                    if (Drag.active) {
                        itemLoader.opacity = 0;
                    } else {
                        itemLoader.item.state = "normal";
                        iconItem.hold = false;
                        itemLoader.opacity = 1;
                    }
                }

                property int visualIndex: 0
                property string sourceId: ""
                property string scheme: isFolder ? "appFolder://" : "app://"
                property bool isFolder: model.type === DataType.Folder
                property var icon: model.icon ? model.icon : ""
                property bool isFavorite: true
                sourceComponent: {
                    if (type === DataType.Normal) {
                        return appIconComponent;
                    }
                    if (type === DataType.Folder) {
                        return folderIconComponent;
                    }
                }
            }
        }


        UkuiItems.DtThemeText {
            id: iconText
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            Layout.maximumHeight: contentHeight

            text: name
            elide: Text.ElideRight
            textColor: Platform.GlobalTheme.textActive
            horizontalAlignment: Text.AlignHCenter
            opacity: !(iconItem.hold || iconItem.readyToHold)

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }

    Component {
        id: appIconComponent
        Item {
            state: "normal"
            states: [
                State {
                    name: "prepareToMerge"
                    PropertyChanges {
                        target: iconImage
                        x: 7; y: 7
                        width: loaderBase.width / 3
                        height: loaderBase.height / 3
                    }
                    PropertyChanges {
                        target: mergePrompt
                        backgroundColor: Platform.GlobalTheme.kContainAlphaClick
                        alpha: 1
                    }
                },
                State {
                    name: "normal"
                    PropertyChanges {
                        target: iconImage
                        x: 0; y: 0
                        width: loaderBase.width
                        height: loaderBase.height
                    }
                    PropertyChanges {
                        target: mergePrompt
                        backgroundColor: Platform.GlobalTheme.kContainGeneralAlphaNormal
                        alpha: 0
                    }
                }
            ]

            transitions: [
                Transition {
                    NumberAnimation { properties: "width, height, x, y, alpha"; duration: aniDuration; easing.type: Easing.InOutCubic }
                }
            ]
            UkuiItems.Icon {
                id: iconImage
                source: icon
                addUnifiedIconBox: true
            }
            function itemClicked(mouseButton) {
                console.log("ICON_CLICK!", "DESKTOPFILE:", id);
                if (mouseButton === Qt.LeftButton) {
//                    var data = {"id": model.id};
//                    send(data);
                    appManager.launchApp(id);
                }
            }
        }
    }

    Component {
        id: folderIconComponent
        Item {
            states: [
                State {
                    name: "iconHide"
                    PropertyChanges { target: folderIcon; opacity: 0 }
                },
                State {
                    name: "iconShow"
                    PropertyChanges { target: folderIcon; opacity: 1 }
                },
                State {
                    name: "prepareToMerge"
                    PropertyChanges {
                        target: mergePrompt
                        backgroundColor: Platform.GlobalTheme.kContainAlphaClick
                        alpha: 1
                    }
                },
                State {
                    name: "normal"
                    PropertyChanges {
                        target: mergePrompt
                        backgroundColor: Platform.GlobalTheme.kContainGeneralAlphaNormal
                        alpha: 0
                    }
                }
            ]

            transitions: [
                Transition {
                    to: "iconShow"
                    NumberAnimation { property: "opacity"; duration: aniDuration; easing.type: Easing.InOutCubic }
                }
            ]

            AppControls2.FolderIcon {
                id: folderIcon
                height: width
                anchors.centerIn: parent
                alpha: 0.10
                icons: icon
                columns: rows

                width: isFullScreen ? 84: 40
                rows: isFullScreen ? 4 : 2
                spacing: isFullScreen ? 4 : 2
                padding: isFullScreen ? 8 : 2
                radius: isFullScreen ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal
            }
            function itemClicked(mouseButton) {
                if (mouseButton === Qt.LeftButton) {
                    var x = mapToGlobal(folderIcon.x, folderIcon.y).x;
                    var y = mapToGlobal(folderIcon.x, folderIcon.y).y
                    openFolderSignal(id, name, x, y);
                    // 执行隐藏动画，并且当前图标消失且鼠标区域不可用
                    state = "iconHide";
                    control.enabled = false;
                    control.hoverEnabled = false;
                }
            }
        }
    }

    // folderFunction
    function resetOpacity() {
        // 将state为"iconHide"的应用组，恢复到初始状态
        if (type === DataType.Folder) {
            itemLoader.item.state = "iconShow";
        }
        control.enabled = true;
        control.hoverEnabled = true;
    }

    onHoldChanged: {
        if (!hold) {
            readyToHold = false;
            if (favoriteView.dragTypeIsMerge && (model.id !== favoriteView.mergeToAppId) && (model.type === DataType.Normal)) {
                opacity = 0;
                if (favoriteView.isMergeToFolder) {
                    favoriteModel.addAppToFolder(model.id, favoriteView.mergeToAppId);
                } else {
                    favoriteModel.addAppsToNewFolder(model.id, favoriteView.mergeToAppId);
                }

            } else if (favoriteView.exchangedStartIndex !== itemLoader.visualIndex) {
                favoriteModel.exchangedAppsOrder(favoriteView.exchangedStartIndex, itemLoader.visualIndex);
            }
        }
    }
}
