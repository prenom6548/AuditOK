/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiItems.DtThemeBackground {
    backgroundColor: Platform.GlobalTheme.kContainSecondaryNormal
    borderColor: Platform.GlobalTheme.kLineNormal
    border.width: 1

    Item {
        anchors.fill: parent
        anchors.margins: parent.border.width
        clip: true
        NotificationCenterPC {
            id: messageBox;
            visible: !sidebarWindow.isTabletMode;
            anchors.fill: parent;
            anchors.leftMargin: 16;
            anchors.rightMargin: 16;
        }
    }
}
