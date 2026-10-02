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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.ukui.quick.platform 1.0

Text {
    property int pointSizeOffset: 0
    property int paletteTextRole: Theme.ButtonText
    property int paletteHighlightRole: Theme.HighlightedText
    property real alpha: 1
    property bool enableHighlight: false

    function getThemeColor(paletteRole) {
        return Theme.colorWithCustomTransparency(paletteRole, Theme.Active, alpha);
    }

    elide: Text.ElideRight
    width: implicitWidth
    color: enableHighlight ? getThemeColor(paletteHighlightRole) : getThemeColor(paletteTextRole)

    font {
        pointSize: {
            if ((Theme.fontSize + pointSizeOffset) < 0)
                return Theme.fontSize;
            else
                return Theme.fontSize + pointSizeOffset;
        }
        family: Theme.fontFamily
    }

}
