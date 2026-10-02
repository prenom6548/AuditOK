/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: qiqi <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

DtThemeBackground {
    useStyleTransparency: false
    backgroundColor: GlobalTheme.baseActive
    alpha: 0.65
    radius: GlobalTheme.kRadiusMin
    border.width: 1
    borderColor: GlobalTheme.kLineNormal

    property var playStatus
    signal skipBackwardClicked()
    signal playStatusClicked()
    signal skipForwardClicked()

    RowLayout {
        anchors.fill: parent
        spacing: 0
        MouseArea {
            id: backward
            Accessible.role: Accessible.Button
            Accessible.description: "This is a mpris player button of thumbnail window"
            Accessible.name: "thumbnail_backward_button"
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            hoverEnabled: true
            onClicked: skipBackwardClicked()

            Icon {
                height: 16
                width: 16
                anchors.centerIn: parent
                mode: (backward.containsPress || backward.containsMouse) ? Icon.ForceHighlight
                                                                      : Icon.AutoHighlight
                dtThemeHighlightColor: backward.containsPress ? GlobalTheme.kBrandClick :
                                                                                        backward.containsMouse ? GlobalTheme.kBrandHover
                                                                                                                : GlobalTheme.kFontPrimary
                source: "media-skip-backward-symbolic"
            }
        }

        DtThemeBackground {
            useStyleTransparency: false
            Layout.preferredHeight: 16
            Layout.preferredWidth: 1
            backgroundColor: GlobalTheme.kDivider
        }

        MouseArea {
            id: playPause
            Accessible.role: Accessible.Button
            Accessible.description: "This is a mpris player button of thumbnail window"
            Accessible.name: "thumbnail_playPause_button"
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            hoverEnabled: true
            onClicked: playStatusClicked()
            Icon {
                height: 16
                width: 16
                anchors.centerIn: parent
                mode: (playPause.containsPress || playPause.containsMouse) ? Icon.ForceHighlight
                                                                      : Icon.AutoHighlight
                dtThemeHighlightColor: playPause.containsPress ? GlobalTheme.kBrandClick :
                                                                                        playPause.containsMouse ? GlobalTheme.kBrandHover
                                                                                                                : GlobalTheme.kFontPrimary
                source: playStatus === "Playing" ? "media-playback-pause-symbolic" : "ukui-play-full-symbolic"
            }
        }

        DtThemeBackground {
            useStyleTransparency: false
            Layout.preferredHeight: 16
            Layout.preferredWidth: 1
            backgroundColor: GlobalTheme.kDivider
        }

        MouseArea {
            id: forward
            Accessible.role: Accessible.Button
            Accessible.description: "This is a mpris player button of thumbnail window"
            Accessible.name: "thumbnail_forward_button"
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            hoverEnabled: true
            onClicked: skipForwardClicked()
            Icon {
                height: 16
                width: 16
                anchors.centerIn: parent
                mode: (forward.containsPress || forward.containsMouse) ? Icon.ForceHighlight
                                                                      : Icon.AutoHighlight
                dtThemeHighlightColor: forward.containsPress ? GlobalTheme.kBrandClick :
                                                                                        forward.containsMouse ? GlobalTheme.kBrandHover
                                                                                                                : GlobalTheme.kFontPrimary
                source: "media-skip-forward-symbolic"
            }
        }
    }
}
