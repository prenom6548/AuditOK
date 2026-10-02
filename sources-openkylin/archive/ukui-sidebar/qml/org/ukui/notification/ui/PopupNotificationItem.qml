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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

MouseArea {
    id: root
    property alias tabletTips: tabletTips
    property alias pcTips: pcTips
    property int itemMargin: 16
    property alias background: background
    property var actions: model.actions
    property bool screenLock
    property int shadowSize: baseShadow.shadowSize
    property bool hideBorderShadow: false
    signal actionExecuted(string action)
    signal notificationClosed()
    signal clearButtonAreaEntered(bool isEnter)
    property bool isTablet

    hoverEnabled: true
    UkuiItems.DtThemeBackground {
        id: background

        Accessible.role: Accessible.Button
        Accessible.description: "This is a notification on notification center"
        Accessible.name: "notification_center_notification_item"

        useStyleTransparency: false
        backgroundColor: Platform.GlobalTheme.kContainGeneralNormal
        border.width: hideBorderShadow ? 0 : 1
        borderColor: Platform.GlobalTheme.kLineWindowActive
        height: itemColumn.height + itemMargin*2
        width: root.width
        radius: root.isTablet ? 24 : Platform.GlobalTheme.kRadiusMax

        UkuiItems.DtDropShadow {
            id: baseShadow
            anchors.fill: background
            visible: !hideBorderShadow
            shadowData: model.isStored ? Platform.GlobalTheme.kShadowMin : Platform.GlobalTheme.kShadowMenu
            shadowRadius: background.radius
        }

        Column {
            id: itemColumn
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: root.width - itemMargin*2
            height: childrenRect.height
            spacing: 8

            RowLayout {
                width: parent.width
                height: 24
                spacing: 8

                Loader {
                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.height
                    sourceComponent: Component {
                       UkuiItems.Icon {
                            source: model.image === undefined ? model.appIconName : model.image
                        }
                    }
                }

                UkuiItems.DtThemeText {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: model.appName
                    textColor: Platform.GlobalTheme.kFontSecondary

                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }
                UkuiItems.DtThemeText {
                    id: timeStamp
                    Layout.fillHeight: true
                    Layout.preferredWidth: contentWidth
                    text: dateTimeUtils.computeTimeOut(model.createTime)
                    visible: !clearButton.visible
                    textColor: Platform.GlobalTheme.kFontSecondary

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    Component.onCompleted: {
                        dateTimeUtils.timeRefresh.connect(refreshTimeStamp);
                        dateTimeUtils.dateUpdate.connect(refreshTimeStamp);
                        dateTimeUtils.timeUpdate.connect(refreshTimeStamp);
                    }
                    Component.onDestruction: {
                        dateTimeUtils.timeRefresh.disconnect(refreshTimeStamp);
                        dateTimeUtils.dateUpdate.disconnect(refreshTimeStamp);
                        dateTimeUtils.timeUpdate.disconnect(refreshTimeStamp)
                    }
                    function refreshTimeStamp() {
                        timeStamp.text = dateTimeUtils.computeTimeOut(model.createTime);
                    }
                }
                UkuiItems.DtThemeBackground {
                    id: clearButton

                    Accessible.role: Accessible.Button
                    Accessible.description: "This is a button on notification center"
                    Accessible.name: "notification_center_notification_close_button"

                    Layout.preferredWidth: 24
                    Layout.fillHeight: true
                    radius: 12
                    useStyleTransparency: false
                    backgroundColor: clearButtonMouse.containsPress ? Platform.GlobalTheme.kContainClick
                                                                    : clearButtonMouse.containsMouse ? Platform.GlobalTheme.kContainHover
                                                                                                     : Platform.GlobalTheme.kContainGeneralAlphaNormal
                    visible: !model.isStored || (model.isStored && root.containsMouse)

                    UkuiItems.Icon {
                        height: 16
                        width: 16
                        anchors.centerIn: parent
                        mode: UkuiItems.Icon.AutoHighlight
                        source: "window-close-symbolic"

                    }
                    MouseArea {
                        id: clearButtonMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.notificationClosed();
                        }
                        onEntered: {
                            root.clearButtonAreaEntered(true);
                        }
                        onExited: {
                            root.clearButtonAreaEntered(false);
                        }

                        UkuiItems.Tooltip {
                            anchors.fill: parent
                            mainText: qsTr("Close")
                            posFollowCursor: true
                        }
                    }
                }
            }

            ColumnLayout {
                width: parent.width - 32
                anchors.right: parent.right
                spacing: 4

                UkuiItems.DtThemeText {
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: contentHeight
                    text: model.summary
                    textColor: Platform.GlobalTheme.kFontPrimary

                    maximumLineCount: 2
                    font.bold: true
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignTop
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                    visible: model.summary !== ""

                    UkuiItems.Tooltip {
                        anchors.fill: parent
                        mainText: model.summary
                        posFollowCursor: true
                        active: parent.truncated
                    }
                }
                UkuiItems.DtThemeText {
                    property string linkActiveColor: Platform.GlobalTheme.linkActive.pureColor.toString();
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: parent.width
                    text: screenLock ?
                              model.showContentOnLockScreen ?
                                  model.processedBody : qsTr("one notification from %1").arg(model.appName) : model.processedBody
                    textColor: Platform.GlobalTheme.kFontPrimary
                    maximumLineCount: 2
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                    onLinkActivated: function(link) {
                        if (Qt.openUrlExternally(link)) {
                            if (!model.resident) {
                                root.notificationClosed();
                            }
                        } else {
                            console.log("onLinkActivated: link error, ", link, Qt.resolvedUrl(link));
                        }
                    }

                    Component.onCompleted: {
                        sourceModel.linkActiveColor = Qt.binding(()=>{ return linkActiveColor; });
                    }

                    UkuiItems.Tooltip {
                        anchors.fill: parent
                        mainText: groupModel.stripRichText(model.body)
                        posFollowCursor: true
                        active: parent.truncated
                    }
                }
                UkuiItems.DtThemeText {
                    id: pcTips
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: contentHeight
                    Layout.alignment: Qt.AlignLeft
                    textColor: Platform.GlobalTheme.kFontPrimaryDisable
                    visible: false

                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }

                RowLayout {
                    Layout.preferredWidth: parent.width
                    Layout.alignment: Qt.AlignLeft
                    height: actionView.count === 0 && !tabletTips.visible ? 0 : 36
                    spacing: 0

                    UkuiItems.DtThemeText {
                        id: tabletTips
                        Layout.fillHeight: true
                        Layout.preferredWidth: contentWidth
                        textColor: Platform.GlobalTheme.kFontPrimaryDisable
                        visible: false

                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }

                    Component {
                        id: actionButtonDelegate
                        MouseArea {
                            id: buttonMouseArea
                            hoverEnabled: true
                            property string buttonIcon: ""
                            property string buttonText: modelData.name
                            width: 98
                            height: ListView.view.height
                            enabled: modelData.isEnable

                            onClicked: {
                                root.actionExecuted(modelData.action);
                            }

                            UkuiItems.DtThemeBackground {
                                id: buttonBase
                                useStyleTransparency: false
                                backgroundColor: buttonMouseArea.containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                                               : buttonMouseArea.containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                                                               : Platform.GlobalTheme.kComponentAlphaNormal
                                borderColor: buttonMouseArea.containsPress ? Platform.GlobalTheme.kLineComponentClick
                                                                           : buttonMouseArea.containsMouse ? Platform.GlobalTheme.kLineComponentHover
                                                                                                           : Platform.GlobalTheme.kLineComponentNormal
                                anchors.fill: parent
                                radius: Platform.GlobalTheme.kRadiusNormal
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8

                                UkuiItems.Icon {
                                    id: themeIcon
                                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft // default
                                    Layout.preferredWidth: 16
                                    Layout.preferredHeight: 16
                                    source: buttonMouseArea.buttonIcon
                                    visible: buttonMouseArea.buttonIcon !== ""
                                    mode: UkuiItems.Icon.AutoHighlight
                                }

                                UkuiItems.DtThemeText {
                                    id: actionButtonText
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: contentHeight
                                    Layout.maximumHeight: parent.height
                                    text: buttonMouseArea.buttonText
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    textColor: Platform.GlobalTheme.buttonTextActive
                                }
                            }

                            UkuiItems.Tooltip {
                                anchors.fill: parent
                                posFollowCursor: true
                                mainText: actionButtonText.text
                                active: actionButtonText.truncated
                            }

                            states: State {
                                when: buttonMouseArea.activeFocus
                                PropertyChanges {
                                    target: buttonBase
                                    borderColor: Platform.GlobalTheme.highlightActive
                                    border.width: 2
                                }
                            }
                        }
                    }

                    ListView {
                        id: actionView
                        Layout.fillHeight: true
                        Layout.preferredWidth: contentWidth
                        spacing: 8
                        interactive: false
                        orientation: ListView.Horizontal

                        model: {
                            var acs = [];
                            for (var i = 0; i < actions.length; ++i) {
                                if (!actions[i].isDefault && acs.length < 2) {
                                    acs.push(actions[i]);
                                }
                            }

                            return acs;
                        }
                        delegate: actionButtonDelegate
                    }
                }
            }
        }
    }
}
