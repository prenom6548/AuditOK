/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.sidebar.shortcut.utils 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiItems.StyleBackground {
    id: root;

    function updateLayout() {
//        height = layoutConfig.height(LayoutComponent.SidebarUserInfo);
        radius = layoutConfig.radius(LayoutComponent.SidebarUserInfo);

        userInfoLayout.spacing = layoutConfig.spacing(LayoutComponent.SidebarUserInfo);
        userInfoLayout.anchors.leftMargin = layoutConfig.margin(LayoutComponent.SidebarUserInfo, 0);
        userInfoLayout.anchors.rightMargin = layoutConfig.margin(LayoutComponent.SidebarUserInfo, 2);

        userIcon.Layout.preferredWidth = layoutConfig.width(LayoutComponent.SidebarUserInfoIcon);
        userIcon.Layout.preferredHeight = layoutConfig.height(LayoutComponent.SidebarUserInfoIcon);

        powerButton.Layout.preferredWidth = layoutConfig.width(LayoutComponent.SidebarUserInfoPowerIcon);
        powerButton.Layout.preferredHeight = layoutConfig.height(LayoutComponent.SidebarUserInfoPowerIcon);
    }

    /**
     * 实现动态切换布局的一种方法
     * > 手动监听layoutConfig的layoutChanged信号
     *   手动对界面的各个部件的布局属性进行修改
     *   缺点：如果组件数量很多，需要书写大量的js代码,没有利用qt自定义组件的属性绑定功能
     **/
    Component.onCompleted: {
        updateLayout();
        layoutConfig.layoutChanged.connect(updateLayout);
    }

    AccountInformation {
        id: accountInfo;
    }

    RowLayout {
        id: userInfoLayout;

        anchors.fill: parent;

        ThemeIcon {
            id: userIcon;

            Layout.alignment: Qt.AlignVCenter;

            radius: height / 2;
            source: accountInfo.iconFile;

            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    accountInfo.openUserCenter();
                }
            }
        }

        Item {
            id: userInfo;
            Layout.fillWidth: true;
            Layout.fillHeight: true;

            UkuiItems.StyleText {
                id: userName;
                text: accountInfo.realName;
                elide: Text.ElideRight;
                font.bold: true;
                font.pixelSize: 20;

                width: parent.width;
                anchors.bottom: parent.verticalCenter;
            }

            UkuiItems.StyleText {
                id: userType;
                text: accountInfo.accountType;
                elide: Text.ElideRight;
                font.pixelSize: 16;

                width: parent.width;
                anchors.top: parent.verticalCenter;
            }
        }

        Rectangle {
            id: powerButton;

            PowerButton {
                id: powerButtonBase;
            }

            Layout.alignment: Qt.AlignVCenter;

            radius: height / 2;
            color: powerButtonBase.baseColor;

            ThemeIcon {
                id: powerIcon;

                anchors.centerIn: parent;
                width: Math.floor(parent.width / 2);
                height: width;

                radius: height / 2;
                source: powerButtonBase.icon;
            }

            MouseArea {
                anchors.fill: parent;
                property bool hovered: false;

                hoverEnabled: true;
                ToolTip.visible: hovered;
                ToolTip.text: powerButtonBase.toolTip;

                onClicked: {
                    powerButtonBase.clicked();
                }

                onPressed: {
                    powerButton.color = powerButtonBase.highLightColor;
                }

                onReleased: {
                    powerButton.color = powerButtonBase.baseColor;
                }

                onEntered: {
                    hovered = true;
                    powerButton.color = powerButtonBase.highLightColor;
                }

                onExited: {
                    hovered = false;
                    powerButton.color = powerButtonBase.baseColor;
                }
            }
        }
    }
}
