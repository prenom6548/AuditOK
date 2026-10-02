import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: menuRoot

    property var host
    property var backend

    readonly property int menuImplicitWidth: ctxMenu.implicitWidth
    readonly property int menuImplicitHeight: ctxMenu.implicitHeight

    function openAt(x, y) {
        if (!host)
            return
        ctxMenu.popup(host, x, y)
    }

    Menu {
        id: ctxMenu

        MenuItem {
            text: qsTr("More widgets")
            icon.name: "preferences-desktop-apps-symbolic"
            onTriggered: {
                if (host && host.desktopShell && host._hasApi(host.desktopShell, "showWidgetManager"))
                    host.desktopShell.showWidgetManager()
            }
        }

        MenuItem {
            text: qsTr("Change timezone")
            icon.name: "preferences-system-time-symbolic"
            onTriggered: {
                if (backend)
                    backend.openTimezoneChooser()
            }
        }

        MenuSeparator {}

        MenuItem {
            id: sizeSmallItem
            text: qsTr("Small size")
            checkable: true
            checked: host && host.sizeMode === "small"
            enabled: host && (host.sizeMode === "small" || host._smallVariantAvailable)
            icon.source: "qrc:/org/ukui/desktopclock/icons/small-symbolic.svg"
            onTriggered: {
                if (host)
                    Qt.callLater(function() { host.switchToVariant("small") })
            }
        }

        MenuItem {
            id: sizeMediumItem
            text: qsTr("Medium size")
            checkable: true
            checked: host && host.sizeMode === "medium"
            enabled: host && (host.sizeMode === "medium" || host._mediumVariantAvailable)
            icon.source: "qrc:/org/ukui/desktopclock/icons/medium-symbolic.svg"
            onTriggered: {
                if (host)
                    Qt.callLater(function() { host.switchToVariant("medium") })
            }
        }

        MenuItem {
            id: sizeLargeItem
            text: qsTr("Large size")
            checkable: true
            checked: host && host.sizeMode === "large"
            enabled: host && (host.sizeMode === "large" || host._largeVariantAvailable)
            icon.source: "qrc:/org/ukui/desktopclock/icons/large-symbolic.svg"
            onTriggered: {
                if (host)
                    Qt.callLater(function() { host.switchToVariant("large") })
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Remove widget")
            enabled: host && host.canRemoveFromDesktop
            icon.name: "edit-delete-symbolic"
            onTriggered: {
                if (!host || !host.canRemoveFromDesktop)
                    return
                if (backend)
                    backend.removeInstanceConfig()
                if (host.desktopShell)
                    host.desktopShell.removeWidgetInstance(host.resolvedInstanceId)
            }
        }
    }
}
