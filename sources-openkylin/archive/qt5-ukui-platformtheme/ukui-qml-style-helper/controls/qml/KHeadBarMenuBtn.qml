/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0

Private.KWindowButton{
    id: root
    property string type: "menu"

    // 嵌套 Menu 的 visible 不能隐藏父菜单里“Theme”那一行；须把承载子菜单的 MenuItem 设为不可见。
    property bool themeMenuVisible: true
    function syncThemeMenuRowVisible() {
        var p = themeSubMenu.parent
        if (p && p.subMenu === themeSubMenu)
            p.visible = themeMenuVisible
    }
    onThemeMenuVisibleChanged: syncThemeMenuRowVisible()
    Component.onCompleted: Qt.callLater(syncThemeMenuRowVisible)

    // 暴露主菜单对象，供外部自定义
    property alias menu: mainMenu

    // 暴露各个菜单项，供外部连接信号或设置处理程序
    // 对应 C++ 中的: settingAction(), themeAction(), assistAction(), aboutAction(), feedbackAction(), quitAction()
    property alias settingItem: settingMenuItem
    property alias themeItem: themeSubMenu
    property alias helpItem: helpMenuItem
    property alias aboutItem: aboutMenuItem
    property alias feedbackItem: feedbackMenuItem
    property alias quitItem: quitMenuItem

    // 主题子菜单中的选项
    // 对应 C++ 中的: autoAction(), lightAction(), darkAction()
    property alias autoItem: autoMenuItem
    property alias lightItem: lightMenuItem
    property alias darkItem: darkMenuItem

    // 菜单项触发信号，供应用直接连接（对齐 C++ QAction::triggered）
    signal settingTriggered()
    signal helpTriggered()
    signal aboutTriggered()
    signal feedbackTriggered()
    signal quitTriggered()
    signal themeAutoTriggered()
    signal themeLightTriggered()
    signal themeDarkTriggered()

    Menu {
        id: mainMenu
        parent: root

        // Setting - 对应 d->m_pSettingAction
        MenuItem {
            id: settingMenuItem
            text: qsTr("Setting")
            onTriggered: root.settingTriggered()
        }

        // Theme 子菜单 - 对应 d->m_pThemeAction
        Menu {
            id: themeSubMenu
            title: qsTr("Theme")

            MenuItem {
                id: autoMenuItem
                text: qsTr("Auto")
                checkable: true
                autoExclusive: true
                onTriggered: root.themeAutoTriggered()
            }

            MenuItem {
                id: lightMenuItem
                text: qsTr("Light")
                checkable: true
                autoExclusive: true
                onTriggered: root.themeLightTriggered()
            }

            MenuItem {
                id: darkMenuItem
                text: qsTr("Dark")
                checkable: true
                autoExclusive: true
                onTriggered: root.themeDarkTriggered()
            }
        }

        // Help - 对应 d->m_pAssistAction
        MenuItem {
            id: helpMenuItem
            text: qsTr("Help")
            onTriggered: root.helpTriggered()
        }

        // About - 对应 d->m_pAboutAction
        MenuItem {
            id: aboutMenuItem
            text: qsTr("About")
            onTriggered: root.aboutTriggered()
        }

        // Feedback - 对应 d->m_pFeedbackAction
        MenuItem {
            id: feedbackMenuItem
            text: qsTr("Feedback")
            visible: Private.KyQuickDialog.feedbackItemVisible()
            onTriggered: {
                Private.KyQuickDialog.feedbackShow()
                root.feedbackTriggered()
            }
        }

        // Quit - 对应 d->m_pQuitAction
        MenuItem {
            id: quitMenuItem
            text: qsTr("Quit")
            onTriggered: root.quitTriggered()
        }
    }

    // 点击按钮时弹出菜单（对应 setPopupMode(QToolButton::InstantPopup)）
    onClicked: mainMenu.popup()
}


