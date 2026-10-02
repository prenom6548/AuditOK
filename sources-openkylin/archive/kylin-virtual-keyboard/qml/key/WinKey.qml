/*
* Copyright 2022 KylinSoft Co., Ltd.
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

import "../js/utils.js" as Utils
import QtQuick 2.0

TemporarySwitchKey {
    id: win

    property string winKeyImgPath

    function updateWinState(winState) {
        virtualKeyboard.winState = winState;
    }

    keyLabel.visible: winKeyImgPath == ""
    Component.onCompleted: {
        temporarySwitchKeyReleased.connect(updateWinState);
        temporarySwitchKeyClicked.connect(virtualKeyboard.winClicked);
    }
    state: virtualKeyboard.winState
    states: [
        State {
            name: "NORMAL"

            PropertyChanges {
                target: keyBackground
                state: "NORMAL"
            }

            PropertyChanges {
                target: win
                winKeyImgPath: virtualKeyboard.getIconPath("super_logo.svg")
            }

        },
        State {
            name: "OPEN"

            PropertyChanges {
                target: keyBackground
                state: "OPEN"
            }

            PropertyChanges {
                target: win
                winKeyImgPath: virtualKeyboard.getIconPath("super_logo_clicked.svg")
            }

        }
    ]

    Binding {
        target: win
        property: "switchKeyState"
        value: virtualKeyboard.winState
    }

    Loader {
        width: virtualKeyboard.actionKeySize * 1.2
        height: virtualKeyboard.actionKeySize * 1.2
        anchors.verticalCenter: parent.verticalCenter
        sourceComponent: winKeyImgPath == "" ? undefined : winKeyComponent

        anchors {
            horizontalCenter: alignment == Text.AlignHCenter ? parent.horizontalCenter : undefined
            right: alignment == Text.AlignRight ? parent.right : undefined
            rightMargin: alignment == Text.AlignRight ? virtualKeyboard.keyIconAlignment : undefined
            left: alignment == Text.AlignLeft ? parent.left : undefined
            leftMargin: alignment == Text.AlignLeft ? virtualKeyboard.keyIconAlignment : undefined
        }

    }

    Component {
        id: winKeyComponent

        Image {
            source: winKeyImgPath
        }

    }

}
