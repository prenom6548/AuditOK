import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.ukui.menu.core 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

FocusScope {
    id: normalUI

    Component.onCompleted: {
        resetToDefaultState();
        mainWindow.windowVisibleChanged.connect(resetToDefaultState);
        mainWindow.fullScreenChanged.connect(menuStateChanged);
    }
    Component.onDestruction: {
        mainWindow.windowVisibleChanged.disconnect(resetToDefaultState);
        mainWindow.fullScreenChanged.disconnect(menuStateChanged);
    }

    function resetToDefaultState() {
        normalUI.focus = true;
        if (mainWindow.windowVisible) {
            appPage.search.changeToSearch("");
            appPage.search.forceActiveFocus();
        }
        // 退出编辑模式
        mainWindow.editMode = false;
    }
    function keyPressed(event) {
        // 任意字符键焦点切换到搜索（0-9 a-z）
        if ((0x2f < event.key && event.key < 0x3a)||(0x40 < event.key && event.key < 0x5b)) {
            focus = true;
            appPage.search.forceActiveFocus();
        }
    }
    function menuStateChanged () {
        if (!mainWindow.isFullScreen) {
            if (root.focusOnFoldButton) {
                root.focusOnFoldButton = false;
                sidebar.foldButton.forceActiveFocus();
            } else {
                resetToDefaultState();
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        z: 10

        AppPage {
            id: appPage
            Layout.preferredWidth: 312
            Layout.fillHeight: true
            content.focusToWidgetPage: widgetPage

            onFocusChanged: {
                if (focus) appPage.search.forceActiveFocus();
            }
        }

        UkuiItems.DtThemeBackground {
            Layout.preferredWidth: 1
            Layout.fillHeight: true

            backgroundColor: Platform.GlobalTheme.kDivider
            useStyleTransparency: false
        }

        // 收藏、最近插件页面：宽度为396
        WidgetPage {
            id: widgetPage
            Layout.fillWidth: true
            Layout.fillHeight: true
            focusToSidebar: sidebar.foldButton
        }

        UkuiItems.DtThemeBackground {
            Layout.preferredWidth: 1
            Layout.fillHeight: true

            backgroundColor: Platform.GlobalTheme.kDivider
            useStyleTransparency: false
        }

        Sidebar {
            id: sidebar
            Layout.preferredWidth: 56
            Layout.fillHeight: true
            focusToAppPage: appPage
        }
    }
}
