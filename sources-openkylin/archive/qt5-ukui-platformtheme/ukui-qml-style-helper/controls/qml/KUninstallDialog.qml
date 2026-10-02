/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Pure QML shell matching kdk::KUninstallDialog layout; supply appName,
 * appVersion, optional appDisplayName and appIconUrl from C++ (icon path,
 * Chinese name from uksc db, etc.). Window chrome and behavior reuse KDialog.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Window 2.12
import org.ukui.platformtheme.controls 1.0 as UkuiPrivate
import org.ukui.qqc2style.private 1.0

UkuiPrivate.KDialog {
    id: root

    readonly property int appIconLogicalSize: 48
    /// imageProvider 在 requestedSize 为空时会落到 16px 默认尺寸，必须按逻辑尺寸 × DPR 请求像素图
    readonly property real _iconDpr: (screen && screen.devicePixelRatio > 0) ? screen.devicePixelRatio : 1.0
    readonly property int _iconSourcePixels: Math.min(512, Math.max(appIconLogicalSize, Math.round(appIconLogicalSize * _iconDpr)))

    /// Desktop / package id used for window title icon and theme icon lookup
    property string appName: ""
    property string appVersion: ""
    /// Empty: use appName; C++ may set localized display name (e.g. from uksc db)
    property string appDisplayName: ""
    /// Resolved icon URL (file://, qrc: …). Empty: try theme icon for appName
    property url appIconUrl: ""

    readonly property string _resolvedDisplayName: appDisplayName.length ? appDisplayName : appName

    signal uninstallClicked()

    width: 550
    height: 450
    minimumWidth: 550
    maximumWidth: 550
    minimumHeight: 450
    maximumHeight: 450
    modality: Qt.ApplicationModal
    Component.onCompleted: {
        minButton.visible = false
        maxButton.visible = false
        menuButton.visible = false
    }
    title: qsTr("uninstall")
    property string titleIcon: appName

    readonly property string _appIconSource: {
        if (appIconUrl.toString().length > 0)
            return appIconUrl.toString()
        if (appName.length > 0)
            return "image://imageProvider/" + appName + "/" + (root.active ? "normal" : "disenable")
        return ""
    }

    APPParameter {
        id: styleParam
        onParametryChanged:{
            appIconImage.source = ""
            appIconImage.source = root._appIconSource
        }
        onIconThemeChanged: {
            appIconImage.source = ""
            appIconImage.source = root._appIconSource
        }
    }

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                Item {
                    Layout.preferredWidth: 124
                }
                Image {
                    id: appIconImage
                    Layout.preferredWidth: root.appIconLogicalSize
                    Layout.preferredHeight: root.appIconLogicalSize
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                    sourceSize.width: root._iconSourcePixels
                    sourceSize.height: root._iconSourcePixels
                    source: root._appIconSource
                }
                Item {
                    Layout.preferredWidth: 12
                }
                Label {
                    id: appTitleLabel
                    Layout.preferredWidth: 331
                    Layout.maximumWidth: 331
                    text: root._resolvedDisplayName
                    wrapMode: Text.NoWrap
                    elide: Text.ElideRight
                    font.pointSize: Math.max(8, styleParam.font.pointSize * 2.5)
                }
                Item {
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                Item {
                    Layout.preferredWidth: 184
                }
                ColumnLayout {
                    spacing: 0

                    Label {
                        text: qsTr("deb name:") + root.appName
                        wrapMode: Text.NoWrap
                        elide: Text.ElideRight
                        Layout.maximumWidth: 329
                    }
                    Label {
                        id: debVersionLabel
                        property string fullVersionText: qsTr("deb version:") + root.appVersion
                        text: fullVersionText
                        width: 329
                        maximumLineCount: 1
                        wrapMode: Text.NoWrap
                        elide: Text.ElideRight

                        MouseArea {
                            id: debVersionHover
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                        }

                        ToolTip.delay: 500
                        ToolTip.visible: debVersionHover.containsMouse && debVersionLabel.truncated
                        ToolTip.text: fullVersionText
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.preferredHeight: 25
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                Item {
                    Layout.preferredWidth: 184
                }
                Button {
                    text: qsTr("uninstall")
                    Layout.preferredWidth: 122
                    Layout.preferredHeight: 40
                    onClicked: root.uninstallClicked()
                }
                Item {
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
