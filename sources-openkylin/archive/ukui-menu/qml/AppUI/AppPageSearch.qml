import QtQuick 2.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.menu.utils 1.0


Item {
    property Item focusToPageContent
    property bool inputStatus: false
    property alias searchInputBar: searchInputBar

    UkuiItems.SearchLineEdit {
        id: searchInputBar
        property string providerId: "search"

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true;

        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        enableContextMenu: true

        Keys.onDownPressed: changeFocusToListView()
        Keys.onReturnPressed: changeFocusToListView()

        onTextChanged: {
            if (text === "") {
                AppPageBackend.group = PluginGroup.Display;
                inputStatus = false;
            } else {
                AppPageBackend.group = PluginGroup.Search;
                AppPageBackend.startSearch(text);
                inputStatus = true;
            }
        }

        function changeFocusToListView() {
            if (!mainWindow.isFullScreen) {
                normalUI.focus = true;
                focusToPageContent.focus = true;
                appPage.content.resetFocus();
            }
        }

        function textInputFocus() {
            textInput.forceActiveFocus();
        }

        function menuStateChanged () {
            text = root.currentSearchText;
        }

        Component.onCompleted: {
            menuStateChanged();
            mainWindow.fullScreenChanged.connect(menuStateChanged);
        }

        Component.onDestruction: {
            mainWindow.fullScreenChanged.disconnect(menuStateChanged);
        }
    }

    function changeToSearch(key) {
//        if (header.content === null) {
            searchInputBar.text = key;
            searchInputBar.textInputFocus();
//        }
    }
}
