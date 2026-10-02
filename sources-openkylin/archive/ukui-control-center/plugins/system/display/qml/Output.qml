/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

import QtQuick 2.1
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import org.kde.kscreen 1.0

QMLOutput {

    id: root;

    signal clicked();
    signal primaryTriggered(string self);
    signal enabledToggled(string self);
    signal mousePressed();
    signal mouseReleased(bool isReleased);
    signal positionChanged(bool isReleased);
    signal rotationChanged(bool isReleased);
    signal widthChanged(bool isReleased);
    signal heightChanged(bool isReleased);

    property bool isDragged: monitorMouseArea.drag.active;
    property bool isDragEnabled: root.dragEnable;
    property bool isToggleButtonVisible: false;
    property bool hasMoved: false;
    property var saveWidth: 0.0;
    property var saveHeight: 0.0;

    // 读屏报名：预览图 + 屏名；不设 onPressAction，避免获焦误选中
    Accessible.name: {
        var screenName = root.isCloneMode ? qsTr("Unity") : root.outputName;
        return qsTr("Preview") + " " + screenName;
    }
    Accessible.role: Accessible.Button;
    Accessible.focusable: true;

    width: monitorMouseArea.width;
    height: monitorMouseArea.height;

    visible: opacity > 0 && ((isCloneMode && isCloneModeShow) || !isCloneMode);
    opacity: output.connected ? 1.0 : 0.0;

    Component.onCompleted: {
        root.updateRootProperties();
    }

    SystemPalette {
        id: palette;
    }

    MouseArea {
        id: monitorMouseArea;
        width: {
            if (allowResetSize === false && saveWidth > 0.0) {
            } else {
                if (output.rotation === KScreenOutput.None || output.rotation === KScreenOutput.Inverted) {
                    saveWidth = root.currentOutputWidth * screen.outputScale
                } else {
                    saveWidth = root.currentOutputHeight * screen.outputScale
                }
            }
            return saveWidth;
        }

        height: {
            if (allowResetSize === false && saveHeight > 0.0) {
            } else {
                if (output.rotation === KScreenOutput.None || output.rotation === KScreenOutput.Inverted) {
                    saveHeight = root.currentOutputHeight * screen.outputScale
                } else {
                    saveHeight = root.currentOutputWidth * screen.outputScale
                }
            }
            return saveHeight;
        }

        anchors.centerIn: parent;
        //是否激活时的透明度
        opacity: root.output.enabled ? 1.0 : 0.3;
        transformOrigin: Item.Center;
        rotation: 0;

        hoverEnabled: true;
        preventStealing: true;

        drag {
            target: root.output.enabled && root.isDragEnabled && !root.isCloneMode ? root : null;
            axis: Drag.XandYAxis;
            minimumX: 0;
            maximumX: screen.maxScreenSize.width - root.width;
            minimumY: 0;
            maximumY: screen.maxScreenSize.height - root.height;

            filterChildren: false;
        }

        drag.onActiveChanged: {
            /* If the drag is shorter then the animation then make sure
             * we won't end up in an inconsistent state */
            if (dragActiveChangedAnimation.running) {
                dragActiveChangedAnimation.complete();
            }

            dragActiveChangedAnimation.running = true;
        }

        onPressed: root.clicked();
        onReleased: root.mouseReleased(true)
        onRotationChanged: root.rotationChanged(false);
        onWidthChanged: root.widthChanged(false);
        onHeightChanged: root.heightChanged(false);

        /* FIXME: This could be in 'Behavior', but MouseArea had
         * some complaints...to tired to investigate */
        PropertyAnimation {
            id: dragActiveChangedAnimation;

            target: monitor;
            property: "opacity";
            from: monitorMouseArea.drag.active ? 0.7 : 1.0
            to: monitorMouseArea.drag.active ? 1.0 : 0.7
            duration: 100;
            easing.type: "OutCubic";
        }

        Behavior on opacity {
            PropertyAnimation {
                property: "opacity";
                easing.type: "OutCubic";
                duration: 250;
            }
        }

        Behavior on rotation {
            RotationAnimation {
                easing.type: "OutCubic"
                duration: 250;
                direction: RotationAnimation.Shortest;
            }
        }

        Behavior on width {
            PropertyAnimation {
                property: "width";
                easing.type: "OutCubic";
                duration: 150;
            }
        }

        Behavior on height {
            PropertyAnimation {
                property: "height";
                easing.type: "OutCubic";
                duration: 150;
            }
        }

        Rectangle {
            id: monitor;
            anchors.fill: parent;
            //圆角
            radius: 8;
            // 仅点选 isSelected 蓝底；Tab 的 activeFocus 不改选中色
            color: if (root.isSelected || root.isCloneMode) {
                       return "#3D6BE5";
                   } else {
                       if (isDarkMode) {
                           return "#37373B";
                       } else {
                           return "#E6E6E6";
                       }
                   }
            smooth: true;
            clip: true;
            border {
                color: if (root.isSelected) {
                           return "#3498DB";
                       } else {
                           if (isDarkMode) {
                               return "#37373B";
                           } else {
                               return "#AED6F1";
                           }
                       }
                width: 1;

                Behavior on color {
                    PropertyAnimation {
                        duration: 150;
                    }
                }
            }

            Rectangle {
                id: posLabel;
                y: 4;
                x: 4;
                width: childrenRect.width + 5;
                height: childrenRect.height + 2;
                radius: 8;
                opacity: root.output.enabled && monitorMouseArea.drag.active ? 1.0 : 0.0;
                visible: opacity != 0.0;

                color: "#101010";

                Text {
                    id: posLabelText;
                    text: root.outputX + "," + root.outputY;
                    color: "white";
                    y: 2;
                    x: 2;
                }
            }

            Label {
                id: labelVendor;
                y: ((parent.height - orientationPanel.height) / 2) - (implicitHeight / 2)

                anchors {
                    left: parent.left;
                    right: parent.right;
                    leftMargin: 5;
                    rightMargin: 5;
                }
                text: if (root.isCloneMode) {
                        return ("Unity");
                      } else {
                        return root.outputName;
                      }

                anchors {
                    verticalCenter: parent.verticalCenter;
                    left: parent.left;
                    right: parent.right;
                }
                horizontalAlignment: Text.AlignHCenter;
                color: if (root.isSelected || isDarkMode) {
                           return "#A9A9A9";
                       } else {
                           if (isDarkMode) {
                               return "#D9D9D9";
                           } else {
                               return "#262626";
                           }
                       }
                font.pixelSize: 12;
                elide: Text.ElideRight;
            }
        }
        Item {
            id: orientationPanelContainer;

            anchors.fill: monitor;

            visible: false

            Rectangle {
                id: orientationPanel;
                height: 10;
                //底部颜色
                color:  palette.highlight ;
                smooth: true;

                Behavior on color {
                    PropertyAnimation {
                        duration: 150;
                    }
                }
            }
        }

        OpacityMask {
            anchors.fill: orientationPanelContainer;
            source: orientationPanelContainer;
            maskSource: monitor;
        }
    }

    // 键盘焦点框：浅色黑框、深色反白，仅 activeFocus，不触发选中切换
    Rectangle {
        id: keyboardFocusFrame;
        anchors.fill: monitorMouseArea;
        anchors.margins: -3;
        radius: 10;
        color: "transparent";
        border.color: isDarkMode ? "#FFFFFF" : "#000000";
        border.width: 2;
        visible: root.activeFocus;
        z: 1000;
    }

    Behavior on opacity {
        PropertyAnimation {
            duration: 200;
            easing.type: "OutCubic";
        }
    }
}
