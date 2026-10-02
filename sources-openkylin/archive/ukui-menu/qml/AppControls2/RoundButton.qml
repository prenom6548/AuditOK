import QtQuick 2.15
import org.ukui.menu.core 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

MouseArea {
    id: buttonMouseArea
    hoverEnabled: true
    property string buttonIcon: ""
    property alias highlight: themeIcon.highLight
    property alias autoHighLight: themeIcon.autoHighLight

    UkuiItems.DtThemeBackground {
        id: buttonBase
        useStyleTransparency: false
        backgroundColor: Platform.GlobalTheme.textActive
        anchors.fill: parent
        radius: height / 2
        alpha: buttonMouseArea.containsPress ? 0.20 : buttonMouseArea.containsMouse ? 0.16 : 0.10
    }

    UkuiItems.Icon {
        id: themeIcon
        anchors.centerIn: parent
        width: 16; height: width
        source: buttonIcon
        mode: UkuiItems.Icon.AutoHighlight
    }
    states: State {
        when: buttonMouseArea.activeFocus
        PropertyChanges {
            target: buttonBase
            borderColor: Platform.GlobalTheme.highlightActive
            border.width: 2
        }
    }
}
