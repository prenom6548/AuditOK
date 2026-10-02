import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

MouseArea {
    id: control
    property bool editStatus: false
    property bool truncate: false
    property bool isSelect: false
    hoverEnabled: true

    UkuiItems.Tooltip {
        anchors.fill: parent
        mainText: name
        posFollowCursor: true
        margin: 6
        visible: !editStatus && truncate
    }

    onPositionChanged: {
        if (tip.isVisible) {
            if (tip.visible) {
                tip.hide();
            } else {
                tip.show(name);
            }
        }
    }

    states: State {
        when: control.activeFocus
        PropertyChanges {
            target: controlBase
            borderColor: Platform.GlobalTheme.highlightActive
            border.width: 2
        }
    }

    UkuiItems.DtThemeBackground {
        id: controlBase
        anchors.fill: parent
        radius: Platform.GlobalTheme.kRadiusMin
        useStyleTransparency: false
        alpha: isSelect ? 0.55 : control.containsPress ? 0.82 : control.containsMouse ? 0.55 : 0.00

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            spacing: 12

            FolderIcon {
                rows: 2; columns: 2
                spacing: 2; padding: 2
                icons: icon
                alpha: 0.12; radius: Platform.GlobalTheme.kRadiusMin
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
            }

            Item {
                id: renameArea
                Layout.fillWidth: true
                Layout.fillHeight: true

                Component {
                    id: unEditText
                    UkuiItems.DtThemeText {
                        id: textShow
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                        text: name
                        onWidthChanged: {
                            control.truncate = textShow.truncated
                        }
                    }
                }

                Component {
                    id: editText
                    UkuiItems.DtThemeBackground {
                        radius: Platform.GlobalTheme.kRadiusNormal
                        useStyleTransparency: false
                        backgroundColor: textChange.activeFocus ? Platform.GlobalTheme.baseActive : Platform.GlobalTheme.kContainGeneralAlphaNormal
                        border.width: 2
                        borderColor: textChange.activeFocus ? Platform.GlobalTheme.highlightActive : Platform.GlobalTheme.kGrayAlpha0

                        TextInput {
                            id: textChange
                            text: name
                            clip: true
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: buttonMouseArea.left
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            verticalAlignment: Text.AlignVCenter
                            selectByMouse: true
                            maximumLength: 14
                            color : Platform.GlobalTheme.textActive.pureColor
                            selectionColor : Platform.GlobalTheme.highlightActive.pureColor

                            function editStatusEnd() {
                                control.editStatus = false;
                                control.focus = true;
                            }

                            onEditingFinished: {
                                modelManager.getAppModel().renameFolder(id, text);
                                textChange.editStatusEnd();
                            }

                            Component.onCompleted: {
                                forceActiveFocus();
                            }
                        }

                        MouseArea {
                            id: buttonMouseArea
                            hoverEnabled: true
                            width: 16; height: width
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            visible: textChange.activeFocus

                            UkuiItems.Icon {
                                anchors.centerIn: parent
                                width: 16; height: width
                                source: "edit-clear-symbolic"
                            }

                            onClicked: {
                                textChange.text = name;
                                textChange.editStatusEnd();
                            }
                        }
                    }
                }

                Loader {
                    id: editLoader
                    anchors.fill: parent
                    sourceComponent: editStatus ? editText : unEditText
                }
            }
        }
    }
}
