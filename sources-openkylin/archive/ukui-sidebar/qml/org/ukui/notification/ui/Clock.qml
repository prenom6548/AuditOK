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

import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Rectangle {
    height: timeLayout.height
    color: "transparent"

    ColumnLayout {
        id: timeLayout
        width: parent.width;
        height: childrenRect.height;
        spacing: 10
        Layout.margins: 0
        UkuiItems.DtThemeText {
            id: time
            Layout.maximumHeight: 80

            Layout.alignment: Qt.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            text: dateTimeUtils.currentTime()
            font.family: "stxihei"
            font.pixelSize: 80

            textColor: Platform.GlobalTheme.kFontWhite
            Component.onCompleted: {
                dateTimeUtils.timeUpdate.connect(timeUpdate)
            }
            function timeUpdate(timeStr) {
                time.text = timeStr;
            }
        }
        UkuiItems.DtThemeText {
            id:dateWeek
            Layout.maximumHeight: 32

            property string date: ""
            property string week: ""

            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "stxihei"
            font.pixelSize: 24
            textColor: Platform.GlobalTheme.kFontWhite

            Component.onCompleted: {
                date = dateTimeUtils.currentDate();
                week = dateTimeUtils.currentWeekDay();
                text = date + " " + week;
                dateTimeUtils.dateUpdate.connect(dateUpdate);
                dateTimeUtils.weekDayUpdate.connect(weekDayUpdate);
            }
            function dateUpdate(dateStr) {
                dateWeek.date = dateStr;
                dateWeek.text = dateStr + " " + dateWeek.week
            }
            function weekDayUpdate(weekDayStr) {
                dateWeek.week = weekDayStr;
                dateWeek.text = dateWeek.date + " " + weekDayStr
            }
        }
    }
}
