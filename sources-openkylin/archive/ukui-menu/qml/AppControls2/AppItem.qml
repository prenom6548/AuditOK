import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import "../extensions" as Extension
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

MouseArea {
    id: control
    property bool isSelect: false
    property bool isRecentInstalled: false
    property bool appItemShowBorder: false
    hoverEnabled: visible
    pressAndHoldInterval: 300
    states: State {
        when: control.activeFocus
        PropertyChanges {
            target: controlBase
            borderColor: Platform.GlobalTheme.kBrandFocus
            border.width: appItemShowBorder ? 2 : 0
        }
    }

    UkuiItems.Tooltip {
        anchors.fill: parent
        mainText: name
        posFollowCursor: true
        margin: 6
        visible: content.textTruncated
    }

    UkuiItems.DtThemeBackground {
        id: controlBase
        anchors.fill: parent
        radius: Platform.GlobalTheme.kRadiusMin
        useStyleTransparency: false
        backgroundColor: isSelect ? Platform.GlobalTheme.kContainAlphaClick
                                  : control.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                          : control.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                  : Platform.GlobalTheme.kContainGeneralAlphaNormal

        RowLayout {
            anchors.fill: parent
            spacing: 2

            IconLabel {
                id: content
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true

                iconHeight: 32; iconWidth: iconHeight
                appName: name; appIcon: icon
                appDesktopName: desktopName
                display: Display.TextBesideIcon
                spacing: 12
            }

            Item {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: tagLabel.visible ? 28 : isRecentInstalled ? 8 : 0
                Layout.maximumHeight: width
                Layout.rightMargin: tagLabel.visible ? 4 : 10

                Extension.EditModeFlag {
                    id: tagLabel
                    anchors.fill: parent
                    isFavorited: favorite
                    onClicked: {
                        appManager.changeFavoriteState(id, !isFavorited);
                    }
                }

                UkuiItems.DtThemeBackground {
                    id: tagPoint
                    visible: tagLabel.sourceComponent != null ? false : isRecentInstalled
                    anchors.centerIn: parent
                    width: 8; height: width
                    radius: width / 2
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.highlightActive
                }
            }
        }
    }
}
