/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import "configitem"
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

ConfigPageTemplate {
    function debouncedSave() {
        configManager.saveGlobalConfig();
    }

    anchors.fill: parent
    anchors.topMargin: 8
    width: parent.width
    itemHeight: 60
    configGroups: configManager.globalConfig
    autoSave: true
    useCustomTranslate: true
    immediateSaveFunction: debouncedSave
    itemLayoutMargins: 16

    Connections {
        function onGlobalConfigChanged() {
        }

        target: configManager
    }

}
