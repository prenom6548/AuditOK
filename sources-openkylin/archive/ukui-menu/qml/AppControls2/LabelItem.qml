import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

MouseArea {
    id: control
    hoverEnabled: visible

    property string displayName
    property bool labelItemShowBorder: false

    // ToolTip.text: comment
    // ToolTip.visible: control.containsMouse
    // ToolTip.delay: 500

    Loader {
        anchors.fill: parent
        sourceComponent: mainWindow.isFullScreen ? fullComponent : normalComponent
    }

    Component {
        id: normalComponent
        UkuiItems.DtThemeBackground {
            anchors.fill: parent
            radius: Platform.GlobalTheme.kRadiusMin
            useStyleTransparency: false
            backgroundColor: control.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                   : control.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                           : Platform.GlobalTheme.kContainGeneralAlphaNormal
            border.width: labelItemShowBorder ? 2 : 0
            borderColor: Platform.GlobalTheme.kBrandFocus

            RowLayout {
                anchors.fill: parent
                UkuiItems.DtThemeText {
                    id: labelText
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 12

                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    font.bold: true
                    elide: Text.ElideRight
                    text: control.displayName
                }

                UkuiItems.Icon {
                    visible: control.containsMouse
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.rightMargin: 16
                    mode: UkuiItems.Icon.AutoHighlight
                    source: "open-menu-symbolic"
                }
            }
        }
    }

    Component {
        id: fullComponent
        RowLayout {
            anchors.fill: parent
            spacing: 14
            UkuiItems.DtThemeText {
                id: labelText
                Layout.preferredWidth: contentWidth
                Layout.fillHeight: true
                Layout.leftMargin: 12

                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                font.bold: true
                elide: Text.ElideRight
                text: control.displayName
            }

            UkuiItems.DtThemeBackground {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                Layout.rightMargin: 16
                useStyleTransparency: false
                backgroundColor: Platform.GlobalTheme.kDivider
            }
        }
    }
}
