import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.shortcut.audio 1.0
import org.ukui.audio.common 1.0
import org.ukui.audio.sinkcontrol 1.0
import org.ukui.audio.sinkmodel 1.0
import org.ukui.audio.sinkappvolumemodel 1.0

Item {
    id: audioList
    width: audioListWidth
    height: audioListHeight

    property real audioListWidth: 396
    property real audioListHeight: 305

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: mainColumn.height
        boundsBehavior: Flickable.StopAtBounds
        interactive: contentHeight > height

        ScrollBar.vertical: ScrollBar {
            policy: flickable.contentHeight > flickable.height
                    ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
        }

        ColumnLayout {
            id: mainColumn
            width: flickable.width
            spacing: 0

            HeaderDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                headerDelegateTextStr: qsTr("Output")
                headerDelegateTopMargin: 4
                headerDelegateBtnVisible: false
                headerDelegateBtnEnabled: false
            }

            ListView {
                id: deviceListView
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                interactive: false
                clip: true
                model: UKUIAudioSinkModel
                Accessible.name: qsTr("Audio device list")

                delegate: Loader {
                    width: deviceListView.width
                    height: item ? item.height : 0
                    property var modelData: model

                    sourceComponent: {
                        if (!modelData) {
                            return null
                        }
                        console.log("AudioListArea.qml device:", modelData.itemType,
                                    modelData.portName, modelData.portIcon, modelData.cardDesc,
                                    modelData.isActivePort, modelData.portEnabled)
                        switch (modelData.itemType) {
                        case UKUIAudioSinkModel.EmptyDeviceItem:
                            return emptyDelegateComponent
                        case UKUIAudioSinkModel.DeviceItem:
                            return deviceComponent
                        default:
                            return null
                        }
                    }
                }
            }

            HeaderDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                headerDelegateTextStr: qsTr("Application volume")
                headerDelegateTopMargin: 4
                headerDelegateBtnVisible: true
                headerDelegateBtnEnabled: true
                headerDelegateBtnSource: "application-system-symbolic"

                onHeaderDelegateBtnClick: {
                    console.log("AudioListArea.qml onHeaderDelegateBtnClick TaskID_JumpAppControlPanelAudio")
                    UKUIAudioControl.startTask(Common.TaskID_JumpAppControlPanelAudio)
                }
            }

            ListView {
                id: streamListView
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                interactive: false
                clip: true
                model: UKUIAudioSinkAppVolumeModel
                Accessible.name: qsTr("Application list")

                delegate: Loader {
                    width: streamListView.width
                    height: item ? item.height : 0
                    property var modelData: model

                    sourceComponent: {
                        if (!modelData) {
                            return null
                        }
                        console.log("AudioListArea.qml stream:", modelData.itemType,
                                    modelData.streamName, modelData.streamIconName,
                                    modelData.streamVolume, modelData.streamIndex,
                                    modelData.streamVolumeEnabled)
                        switch (modelData.itemType) {
                        case UKUIAudioSinkAppVolumeModel.EmptyStreamItem:
                            return emptyDelegateComponent
                        case UKUIAudioSinkAppVolumeModel.StreamItem:
                            return appVolumeComponent
                        default:
                            return null
                        }
                    }
                }
            }
        }
    }

    Component {
        id: deviceComponent
        DeviceDelegate {
            width: parent ? parent.width : 396
            deviceDelegateBtnSource: modelData.portIcon
            deviceDelegatePortDesTextStr: modelData.portLabel + qsTr("  ( ") + modelData.cardDesc + qsTr(" )")
            deviceDelegateBtnHighLighted: modelData.isActivePort
            deviceDelegatePortEnabled: modelData.portEnabled

            onActivated: {
                console.log("AudioListArea.qml onActivated:",
                            modelData.portName, modelData.portIcon, modelData.cardName)
                UKUIAudioControl.startTask(Common.TaskID_SetSinkDefaultDevice, -1,
                                           modelData.portName, modelData.cardName)
            }
        }
    }

    Component {
        id: appVolumeComponent
        VolumeDelegate {
            width: parent ? parent.width : 396
            volumeValue: modelData.streamVolume
            volumeDelegateImgSource: modelData.streamIconName
            volumeAppName: modelData.streamName
            volumeEnabled: modelData.streamVolumeEnabled

            onValueChanged: {
                console.log("AudioListArea.qml onValueChanged value:", value)
                UKUIAudioControl.startTask(Common.TaskID_SetSinkInputVolume, modelData.streamIndex, value)
                UKUIAudioControl.startTask(Common.TaskID_SendSinkInputVolumeChanged, modelData.streamIconName, value, false)
            }
        }
    }

    Component {
        id: emptyDelegateComponent
        EmptyDelegate {
            width: parent ? parent.width : 396
            emptyDelegateTextStr: modelData.title
        }
    }

    Component.onCompleted: {
        console.log("AudioListArea.qml onCompleted")
        UKUIAudioControl.startTask(Common.TaskID_GetSinkMute, -1)
        UKUIAudioControl.startTask(Common.TaskID_GetSinkVolumeBoostStatus)
    }
}
