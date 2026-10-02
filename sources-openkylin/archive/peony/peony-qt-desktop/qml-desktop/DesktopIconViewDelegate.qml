import QtQuick 2.15

Rectangle {
    id: DesktopIconViewDelegate
    width: 120
    height: 120
    radius: 6
    property string uri: model.display
    property point pos: model.position
    Item {
        id: viewIconRect
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: 64
        height: 64
        x: pos.x*width
        y: pos.y*height

        Text {
            text: uri;
            anchors.horizontalCenter: viewIconRect.horizontalCenter
            anchors.top: viewIconRect.bottom
            anchors.topMargin: 5
            width: parent.width - 8
            wrapMode: Text.WrapAnywhere
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
        }
//                    ThemeIcon {
//                        source: model.decoration
//                        anchors.fill: parent
//                    }
//                    ThemeIcon {
//                        source: "emblem-personal-symbolic"
//                        width: 16
//                        height: 16
//                        anchors.horizontalCenter: parent.right
//                        anchors.verticalCenter: parent.bottom
//                    }
    }
}
//            delegate: Rectangle {
//                x: pos.x*width
//                y: pos.y*height
//                width: 120
//                height: 120
//                radius: 6
//                property string uri: model.display
//                property point pos: model.position

//                Item {
//                    id: viewIconRect
//                    anchors.top: parent.top
//                    anchors.topMargin: 10
//                    anchors.horizontalCenter: parent.horizontalCenter
//                    anchors.verticalCenter: parent.verticalCenter
//                    width: 64
//                    height: 64
////                    ThemeIcon {
////                        source: model.decoration
////                        anchors.fill: parent
////                    }
////                    ThemeIcon {
////                        source: "emblem-personal-symbolic"
////                        width: 16
////                        height: 16
////                        anchors.horizontalCenter: parent.right
////                        anchors.verticalCenter: parent.bottom
////                    }
//                }

////                Image { source: "file:///usr/share/icons/ukui-icon-theme-default/24x24/apps/accessories-calculator.png" }
//                Text {
//                    text: uri;
//                    anchors.horizontalCenter: viewIconRect.horizontalCenter
//                    anchors.top: viewIconRect.bottom
//                    anchors.topMargin: 5
//                    width: parent.width - 8
//                    wrapMode: Text.WrapAnywhere
//                    elide: Text.ElideRight
//                    horizontalAlignment: Text.AlignHCenter
//                    maximumLineCount: 2
//                }

////                TextField {
////                    id: textField
////                    text: uri
////                    anchors.horizontalCenter: viewIconRect.horizontalCenter
////                    anchors.top: viewIconRect.bottom
////                    anchors.topMargin: 5
////                    width: parent.width - 8
////                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
////                    selectByMouse: true
////                    visible: isEditing  // 当处于编辑状态时显示
////                    onEditingFinished: {
////                        isEditing = false  // 编辑完成后隐藏 TextField
////                        // 更新模型数据
////                        //myProxyModel.setProperty(index, "uri", textField.text)
////                    }

////                }
////                onClicked: {
////                    realDoEdit = true;

////                       if (editTriggerTimer.running) {
////                           var remainingTime = editTriggerTimer.interval - editTriggerTimer.remainingTime();
////                           var doubleClickInterval = Qt.styleHints.mouseDoubleClickInterval();

////                           if (remainingTime < (3000 - doubleClickInterval) && remainingTime > 0) {

////                               editTriggerTimer.triggered.connect(onEditTriggerTimeout);
////                           } else if (remainingTime >= (3000 - doubleClickInterval)) {
////                               // 模拟双击事件（注意：QML 中没有直接的双击事件，但可以通过这种方式模拟）
////                               // 这里应该调用一个处理双击的函数，但由于 QML 没有直接的对应项，所以留空或替换为实际逻辑
////                               // mouseDoubleClickEvent(mouse); // 这个函数在 QML 中不存在，需要自定义处理
////                           } else {
////                               realDoEdit = false;
////                               editTriggerTimer.stop();
////                           }
////                       } else {
////                           editTriggerTimer.start();
////                           editTriggerTimer.triggered.connect(onEditTriggerTimeout);
////                       }
////                    // 创建编辑器
////                    createEditor(index);
////                }

////                onDoubleClicked: {
////                    // 处理双击事件（如果需要的话）
////                    // 注意：QML 中的 MouseArea 有 onDoubleClicked 信号，但上面的逻辑是为了模拟在 C++ 中处理单击和双击的方式
////                }

//                // 支持手动拖拽修改位置
//                MouseArea {
//                    anchors.fill: parent
//                    drag.target: parent

//                    onReleased: {
//                        //clearOtherSelectState()
////                        // 拖拽后更新模型中的坐标
////                        customModel.setProperty(index, "x", parent.x)
//                    }
//                }
//            }
//        }

        // 框选矩形
//        Rectangle {
//            id: selectionRect
//            visible: false
//            color: "lightblue"
//            opacity: 0.3
//            border.color: "blue"
//            border.width: 1
//        }

        // 框选功能的 MouseArea
//        MouseArea {
//            anchors.fill: parent
//            onPressed: {
//                // 记录框选起始点
//                selectionRect.x = mouseX;
//                selectionRect.y = mouseY;
//                selectionRect.width = 0;
//                selectionRect.height = 0;
//                selectionRect.visible = true;
//            }
//            onPositionChanged: {
//                // 更新框选矩形的大小
//                selectionRect.width = mouseX - selectionRect.x;
//                selectionRect.height = mouseY - selectionRect.y;
//            }
//            onReleased: {
//                // 框选结束，检测矩形框内的子项
//                selectionRect.visible = false;
//                var selectionRectGeometry = Qt.rect(
//                    Math.min(selectionRect.x, selectionRect.x + selectionRect.width),
//                    Math.min(selectionRect.y, selectionRect.y + selectionRect.height),
//                    Math.abs(selectionRect.width),
//                    Math.abs(selectionRect.height)
//                );

//                // 遍历所有子项，检测是否在框选矩形内
//                var ll = repeater.children.length
//                for (var i = 0; i < repeater.children.length; i++) {
//                    var child = repeater.children[i];
//                    if (child.x >= selectionRectGeometry.x &&
//                        child.y >= selectionRectGeometry.y &&
//                        child.x + child.width <= selectionRectGeometry.x + selectionRectGeometry.width &&
//                        child.y + child.height <= selectionRectGeometry.y + selectionRectGeometry.height) {
//                        child.selected = true;  // 选中子项
//                    } else {
//                        child.selected = false;  // 取消选中
//                    }
//                }
//             }
//        }
