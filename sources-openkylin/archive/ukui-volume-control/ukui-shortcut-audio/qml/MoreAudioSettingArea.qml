/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.audio.common 1.0

Item {
    id: moreAudioSetting
    width: moreAudioSettingWidth
    height: moreAudioSettingHeight

    property real moreAudioSettingWidth: 396
    property real moreAudioSettingHeight: 56

    signal moreAudioSettingClick()

    Rectangle {
        id: moreAudioSettingRec
        width: parent.width
        height: parent.height
        color: "#00000000"

        Item {
            id: moreAudioSettingBtn
            anchors.fill: parent
            Accessible.name: moreAudioSettingText.text

            Label {
                id: moreAudioSettingText
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                text: qsTr("More sound settings")
                color: (moreAudioSettingBtn.activeFocus || moreAudioSettingTextHover.hovered)
                       ? Common.getThemePropertyByName("highlight-active")
                       : Common.getThemePropertyByName("kfont-strong")

                HoverHandler {
                    id: moreAudioSettingTextHover
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("MoreAudioSettingArea.qml onClicked")
                        moreAudioSetting.moreAudioSettingClick()
                    }
                }
            }
        }
    }
}
