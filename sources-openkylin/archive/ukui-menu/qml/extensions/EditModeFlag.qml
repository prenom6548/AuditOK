import QtQuick 2.15
import org.ukui.menu.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Loader {
    id: tag
    property bool isFavorited
    property bool isEntered: false
    visible: sourceComponent != null
    signal clicked()

    Component {
        id: editImage
        UkuiItems.DtThemeButton {
            width: 28
            height: 28
            icon.width: 16
            icon.height: 16
            background.backgroundColor: Platform.GlobalTheme.lightActive
            background.alpha: 1
            activeFocusOnTab: false

            onClicked: tag.clicked()
            onEntered: tag.isEntered = true
            onExited: tag.isEntered = false

            background.radius: width / 2
            icon.source: isFavorited ? "ukui-cancel-star-symbolic" : "non-starred-symbolic"
        }
    }

    sourceComponent: mainWindow.editMode && (type === DataType.Normal) ? editImage : null
}
