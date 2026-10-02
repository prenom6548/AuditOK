import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: headerDelegate
    width: headerDelegateWidth
    height: headerDelegateHeight + headerDelegateTopMargin

    property real headerDelegateWidth: 396
    property real headerDelegateHeight: 32
    property real headerDelegateTopMargin: 0
    property string headerDelegateBtnSource: ""

    property alias headerDelegateBtnVisible: appSettingsFocusItem.visible
    property alias headerDelegateBtnEnabled: headerDelegateBtn.enabled
    property alias headerDelegateTextStr: headerDelegateText.text

    signal headerDelegateBtnClick()

    Item {
        id: headerContent
        width: parent.width
        height: headerDelegateHeight
        y: headerDelegateTopMargin

        Label {
            id: headerDelegateText
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("")
        }

        Item {
            id: appSettingsFocusItem
            width: 32
            height: 32
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 16
            visible: false
            Accessible.name: qsTr("Application volume Settings")

            UkuiItems.DtThemeButton {
                id: headerDelegateBtn
                anchors.fill: parent
                enabled: false

                background.radius: 6
                property var kTransparent: Platform.GradientColor {
                    gradientDeg: 0
                    gradientStart: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 0)
                    gradientEnd: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 0)
                    gradientBackground: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 0)
                }
                background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                          : (containsMouse || appSettingsFocusItem.activeFocus)
                                                            ? Platform.GlobalTheme.kContainAlphaHover
                                                            : kTransparent
                icon.source: headerDelegateBtnSource
                icon.mode: UkuiItems.Icon.AutoHighlight

                onClicked: {
                    console.log("HeaderDelegate.qml headerDelegateBtn onClicked")
                    headerDelegate.headerDelegateBtnClick()
                }

                UkuiItems.Tooltip {
                    anchors.fill: parent
                    posFollowCursor: false
                    mainText: qsTr("Application volume Settings")
                    location: UkuiItems.Dialog.BottomEdge
                }
            }
        }
    }
}
