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
 * Authors: zhouyong <zhouyong01@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.ukui.shortcut.audio 1.0
import org.ukui.audio.common 1.0
import org.ukui.audio.sinkcontrol 1.0
import org.ukui.audio.sourcecontrol 1.0

Item {
    id: qmlAudioAreaItem
    anchors.fill: parent
    anchors.top: parent.top
    anchors.topMargin: 56

    property bool qml_sinkVolumeBoostStatus: false

    SystemVolumeArea {
        id: systemVolume
        anchors.top: parent.top

        onSystemVolumeBtnClick: {
            console.log("AudioArea.qml onSystemVolumeBtnClick systemVolumeMuteStatus:",systemVolumeMuteStatus)
            UKUIAudioControl.startTask(Common.TaskID_SetSinkMute, -1
                                       , !systemVolumeMuteStatus)
        }

        onValueChanged: {
            console.log("AudioArea.qml onValueChanged value:",value)
            if (systemVolumeMuteStatus) {
                UKUIAudioControl.startTask(Common.TaskID_SetSinkMute, -1, false)
            }

            UKUIAudioControl.startTask(Common.TaskID_SetSinkVolume, -1, value)
        }
    }

    Rectangle {
        id: lineTop
        width: parent.width
        height: 1
        anchors.top: systemVolume.bottom
        color: Common.getThemePropertyByName("kline-normal")
    }

    AudioListArea {
        id: audioList
        anchors.top: lineTop.bottom
        audioListHeight: (parent.height - systemVolume.height - lineTop.height - lineBottom.height - moreAudioSetting.height)
    }

    Rectangle {
        id: lineBottom
        width: parent.width
        height: 1
        anchors.bottom: moreAudioSetting.top
        color: Common.getThemePropertyByName("kline-normal")
    }

    MoreAudioSettingArea {
        id: moreAudioSetting
        anchors.bottom: parent.bottom

        onMoreAudioSettingClick: {
            console.log("AudioArea.qml onMoreAudioSettingClick")
            UKUIAudioControl.startTask(Common.TaskID_JumpControlPanelAudio)
        }
    }

    Component.onCompleted: {
        console.log("AudioArea.qml onCompleted")
        UKUIAudioControl.startTask(Common.TaskID_GetSinkAvailablePortList)
        UKUIAudioControl.startTask(Common.TaskID_GetSinkInputList)
    }

    Connections {
        target: UKUIAudioSinkControl

        function onSinkVolumeValueChanged(sinkVolumeValue) {
            console.log("AudioArea.qml onSinkVolumeValueChanged sinkVolumeValue:",sinkVolumeValue,systemVolume.systemVolumeValue
                        ,systemVolume.systemVolumeSliderRange)
            if (systemVolume.systemVolumeSliderPressed) {
                return;
            }
            if (sinkVolumeValue > systemVolume.systemVolumeSliderRange) {
                return;
            }

            if (sinkVolumeValue !== systemVolume.systemVolumeValue) {
                systemVolume.systemVolumeValue = sinkVolumeValue
            }
        }

        function onSinkMuteStatusChanged(sinkMuteStatus) {
            console.log("AudioArea.qml onSinkMuteStatusChanged sinkMuteStatus:",sinkMuteStatus)
            systemVolume.systemVolumeMuteStatus = sinkMuteStatus
        }

        function onSinkVolumeStatusIconChanged(sinkVolumeStatusIcon) {
            console.log("AudioArea.qml onSinkVolumeStatusIconChanged sinkVolumeStatusIcon:",sinkVolumeStatusIcon)
            systemVolume.systemVolumeBtnSource = sinkVolumeStatusIcon
        }

        function onSinkVolumeBoostValueChanged(sinkVolumeBoostValue) {
            console.log("AudioArea.qml onSinkVolumeBoostValueChanged sinkVolumeBoostValue:"
                        ,qml_sinkVolumeBoostStatus,sinkVolumeBoostValue)
            if (qml_sinkVolumeBoostStatus) {
                systemVolume.systemVolumeSliderRange = sinkVolumeBoostValue
                UKUIAudioControl.startTask(Common.TaskID_GetSinkVolume)
            }
        }

        function onSinkVolumeBoostStatusChanged(sinkVolumeBoostStatus) {
            console.log("AudioArea.qml onSinkVolumeBoostStatusChanged sinkVolumeBoostStatus:",sinkVolumeBoostStatus)
            qml_sinkVolumeBoostStatus = sinkVolumeBoostStatus;
            if (sinkVolumeBoostStatus) {
                UKUIAudioControl.startTask(Common.TaskID_GetSinkVolumeBoostValue)
            }
        }

        function onSinkDeviceChanged(portName, cardName) {
            console.log("AudioArea.qml onSinkDeviceChanged:",portName,cardName)
            UKUIAudioSinkControl.updateSinkDefaultDevice(portName, cardName)
            UKUIAudioControl.startTask(Common.TaskID_GetSinkVolume)
        }

        function onSinkDeviceAdjust() {
            console.log("AudioArea.qml onSinkDeviceAdjust")
            UKUIAudioControl.startTask(Common.TaskID_GetSinkAvailablePortList)
        }
    }

    Connections {
        target: UKUIAudioControl

        function onTaskCompleted(taskID, retValue) {
            console.log("AudioArea.qml onTaskCompleted taskID:",taskID)
            if (taskID === Common.TaskID_Init) {

            } else if (taskID === Common.TaskID_GetSinkVolume) {
                UKUIAudioSinkControl.updateSinkVolume(retValue)
            } else if (taskID === Common.TaskID_SetSinkVolume) {

            } else if (taskID === Common.TaskID_GetSinkMute) {
                UKUIAudioSinkControl.updateSinkMute(retValue)
            } else if (taskID === Common.TaskID_SetSinkMute) {

            } else if (taskID === Common.TaskID_SetSinkInputVolume) {

            } else if (taskID === Common.TaskID_GetSourceVolume) {

            } else if (taskID === Common.TaskID_SetSourceVolume) {

            } else if (taskID === Common.TaskID_GetSinkAvailablePortList) {
                UKUIAudioSinkControl.updateSinkAvailablePortList(retValue)
                UKUIAudioControl.startTask(Common.TaskID_GetSinkDefaultDevice)
                UKUIAudioControl.startTask(Common.TaskID_GetSinkVolume, -1)

                if (UKUIAudioSinkControl.getEnabledSinkDeviceCount() > 0) {
                    systemVolume.systemVolumeBtnEnabled = true
                    systemVolume.systemVolumeSliderEnabled = true
                } else {
                    systemVolume.systemVolumeBtnEnabled = false
                    systemVolume.systemVolumeSliderEnabled = false
                }

            } else if (taskID === Common.TaskID_GetSourceAvailablePortList) {

            } else if (taskID === Common.TaskID_GetSinkInputList) {
                UKUIAudioSinkControl.updateSinkInputList(retValue)
            } else if (taskID === Common.TaskID_GetSourceInputList) {

            } else if (taskID === Common.TaskID_GetSinkDefaultDevice) {
                UKUIAudioSinkControl.updateSinkDefaultDevice(retValue)
            } else if (taskID === Common.TaskID_SetSinkDefaultDevice) {

            } else if (taskID === Common.TaskID_GetSourceDefaultDevice) {

            } else if (taskID === Common.TaskID_SetSourceDefaultDevice) {

            } else if (taskID === Common.TaskID_JumpControlPanelAudio) {

            } else if (taskID === Common.TaskID_JumpAppControlPanelAudio) {

            } else if (taskID === Common.TaskID_GetSinkVolumeBoostValue) {
                UKUIAudioSinkControl.updateSinkVolumeBoostValue(retValue)
            } else if (taskID === Common.TaskID_GetSinkVolumeBoostStatus) {
                UKUIAudioSinkControl.updateSinkVolumeBoostStatus(retValue)
            }
        }

        function onTaskError(taskID) {
            console.log("AudioArea.qml onTaskError taskID:",taskID)
            if (taskID === Common.TaskID_Init) {

            } else if (taskID === Common.TaskID_GetSinkVolume) {

            } else if (taskID === Common.TaskID_SetSinkVolume) {

            } else if (taskID === Common.TaskID_GetSourceVolume) {

            } else if (taskID === Common.TaskID_SetSourceVolume) {

            } else if (taskID === Common.TaskID_GetSinkAvailablePortList) {

            } else if (taskID === Common.TaskID_GetSourceAvailablePortList) {

            } else if (taskID === Common.TaskID_GetSinkInputList) {

            } else if (taskID === Common.TaskID_GetSourceInputList) {

            } else if (taskID === Common.TaskID_GetSinkDefaultDevice) {

            } else if (taskID === Common.TaskID_SetSinkDefaultDevice) {

            } else if (taskID === Common.TaskID_GetSourceDefaultDevice) {

            } else if (taskID === Common.TaskID_SetSourceDefaultDevice) {

            } else if (taskID === Common.TaskID_JumpControlPanelAudio) {

            } else if (taskID === Common.TaskID_JumpAppControlPanelAudio) {

            }
        }
    }
}


