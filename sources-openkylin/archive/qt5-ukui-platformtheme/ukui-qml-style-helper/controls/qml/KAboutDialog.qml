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
 * Pure QML about dialog aligned with kdk::KAboutDialog layout; window chrome
 * reuses KDialog. Icon theme id, dpkg version lookup, mailto handling,
 * openKylin hiding support, kylin-user-guide, etc. are left to C++ via
 * properties and signals.
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

    readonly property int appIconLogicalSize: 96
    readonly property real _iconDpr: (screen && screen.devicePixelRatio > 0) ? screen.devicePixelRatio : 1.0
    readonly property int _iconSourcePixels: Math.min(512, Math.max(appIconLogicalSize, Math.round(appIconLogicalSize * _iconDpr)))

    /// Theme icon name (QIcon::name); refreshed when icon theme changes
    property string appIconId: ""
    property string appName: ""
    property string appVersion: ""
    /// Optional file / qrc icon; when set, used before theme
    property url appIconUrl: ""
    /// kdk::KAboutDialog::bodyText — long description (widgets often keep hidden)
    property string bodyText: ""
    property bool bodyTextVisible: false
    /// kdk::setAppSupport — plain or rich text; empty uses default Service & Support mailto line
    property string appSupport: ""
    /// When false, hide support row (e.g. openKylin in widget code)
    property bool supportSectionVisible: true
    property string supportEmail: "support@kylinos.cn"
    /// kdk::setAppPrivacyLabelVisible
    property bool privacyVisible: false
    /// Match kdk::ConvertFirstLetterCase on appName
    property bool capitalizeAppName: true

    readonly property string _titleText: capitalizeAppName ? _convertFirstLetterCase(appName) : appName

    readonly property string _appIconSource: {
        if (appIconUrl.toString().length > 0)
            return appIconUrl.toString()
        var id = appIconId.length ? appIconId : appName
        if (id.length > 0)
            return "image://imageProvider/" + id + "/" + (root.active ? "normal" : "disenable")
        return ""
    }

    /// 与 QLabel::linkActivated(url) 一致：点击富文本链接时传入 &lt;a href="..."&gt; 里的字符串（如 mailto:xxx）。
    signal supportLinkActivated(string link)
    signal privacyClicked()

    /// 旧 widget 曾用 mailto://addr，规范成 mailto:addr 便于 QDesktopServices::openUrl
    function _normalizeSupportHref(href) {
        if (!href || href.length === 0)
            return href
        if (href.indexOf("mailto://") === 0)
            return "mailto:" + href.substring("mailto://".length)
        return href
    }

    /// KyQuickDialog.openSupportLink：检查默认邮件应用，与 kaboutdialog.cpp 一致。
    function handleSupportLinkActivated(href) {
        var u = _normalizeSupportHref(href)
        root.kyQuickDialog.openSupportLink(u)
        supportLinkActivated(u)
    }

    function _convertFirstLetterCase(input) {
        if (!input || !input.length)
            return ""
        var sep = input.indexOf("-") >= 0 ? "-" : " "
        var parts = input.split(sep)
        var out = []
        for (var i = 0; i < parts.length; i++) {
            var w = parts[i]
            if (!w.length)
                continue
            out.push(w.charAt(0).toUpperCase() + w.slice(1))
        }
        return out.join(sep)
    }

    minimumWidth: 450
    maximumWidth: 450
    width: 450
    /// ApplicationWindow.height 是整窗高度；contentItem 在 header 下方，高度约为 height - header.height。
    /// 只设 content 高度而不加标题栏时，客户区会被少算一截，正文会被裁切。
    readonly property int _titleBarH: header ? header.height : 40
    readonly property real _bodyH: contentColumn.implicitHeight
    height: _titleBarH + _bodyH
    minimumHeight: _titleBarH + _bodyH
    // 勿与 height 同值锁死 maximum：privacyVisible 等变为 true 时 implicit 变高，
    // 部分环境下 maximum 更新滞后会导致客户区高度不够，底部（含隐私行）被裁掉。
    maximumHeight: _titleBarH + _bodyH + 800
    modality: Qt.ApplicationModal
    title: qsTr("About")
    property string titleIcon: appIconId.length ? appIconId : appName

    Component.onCompleted: {
        minButton.visible = false
        maxButton.visible = false
        menuButton.visible = false
    }

    APPParameter {
        id: styleParam
        onParametryChanged: {
            appIconImage.source = ""
            appIconImage.source = root._appIconSource
        }
        onIconThemeChanged: {
            appIconImage.source = ""
            appIconImage.source = root._appIconSource
        }
    }

    Item {
        id: aboutBody
        z: 100
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        implicitHeight: contentColumn.implicitHeight
        height: contentColumn.implicitHeight

        ColumnLayout {
            id: contentColumn
            width: parent.width - 50
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }

            Image {
                id: appIconImage
                Layout.alignment: Qt.AlignHCenter
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
                Layout.fillWidth: true
                Layout.preferredHeight: 24
            }

            Label {
                id: appNameLabel
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: 400
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: root._titleText
                font.pixelSize: Math.max(8, styleParam.font.pointSize * 1.7)
                font.weight: Font.Medium
                property string labelDTColor: "kfont-primary"
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }

            Label {
                id: appVersionLabel
                visible: root.appVersion.length > 0
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: 400
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: root.appVersion
                property string labelDTColor: "kfont-secondary"

            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
                visible: appVersionLabel.visible
            }

            // 行宽须随内容收缩（短邮箱不要用满 400px），否则 horizontalCenter 只是把一条「满宽左对齐」的条居中，看起来仍不居中
            Item {
                id: supportOuter
                visible: root.supportSectionVisible && root.appSupport.length === 0
                Layout.fillWidth: true
                implicitHeight: supportRow.implicitHeight

                readonly property real _lineCap: width > 0 ? Math.min(400, width) : 400

                RowLayout {
                    id: supportRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 0

                    Label {
                        id: supportBlock
                        Layout.alignment: Qt.AlignVCenter
                        wrapMode: Text.NoWrap
                        text: qsTr("Service & Support: ")
                        property string labelDTColor: "kfont-secondary"
                    }

                    Item {
                        id: emailWrap
                        Layout.alignment: Qt.AlignVCenter
                        readonly property real _emailColW: Math.min(
                            supportBlockEmail.implicitWidth,
                            Math.max(0, supportOuter._lineCap - supportBlock.implicitWidth - supportRow.spacing))
                        Layout.preferredWidth: _emailColW
                        Layout.preferredHeight: supportBlockEmail.contentHeight

                        Text {
                            id: supportBlockEmail
                            width: parent.width
                            wrapMode: Text.WordWrap
                            textFormat: Text.RichText
                            linkColor: palette.highlight
                            font: supportBlock.font
                            text: "<a href=\"mailto:" + root.supportEmail + "\">" + root.supportEmail + "</a>"
                            onLinkActivated: function (link) {
                                root.handleSupportLinkActivated(link)
                            }
                        }

                        MouseArea {
                            anchors.fill: supportBlockEmail
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onPressed: function (mouse) {
                                mouse.accepted = false
                            }
                        }
                    }
                }
            }

            Label {
                id: supportCustom
                visible: root.supportSectionVisible && root.appSupport.length > 0
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: 400
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                linkColor: palette.highlight
//                color: palette.windowText
                text: root.appSupport
                onLinkActivated: function (link) { root.handleSupportLinkActivated(link) }
                property string labelDTColor: "kfont-secondary"

            }

            Loader {
                id: privacyLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight : 0
                active: root.privacyVisible
                sourceComponent: Column {
                    width: contentColumn.width
                    spacing: 0
                    Item { width: parent.width; height: 16 }
                    Label {
                        id: privacyText
                        width: parent.width
                        height: contentHeight
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Privacy statement")
                        font.family: styleParam.font.family
                        font.pixelSize: Math.max(9, styleParam.font.pixelSize > 0
                            ? styleParam.font.pixelSize
                            : Math.round(styleParam.font.pointSize > 0 ? styleParam.font.pointSize * (4.0 / 3.0) : 12))
                        font.bold: styleParam.font.bold
                        font.italic: styleParam.font.italic
//                        font.underline: true
                        color: palette.highlight
                        wrapMode: Text.NoWrap
                        property string labelDTColor: "kfont-secondary"

//                        property string labelDTColor: "highlight-active"

                        MouseArea {
                            anchors.fill: parent
                            z: 1
                            cursorShape: Qt.PointingHandCursor
                            acceptedButtons: Qt.LeftButton
                            hoverEnabled: true
                            onClicked: root.kyQuickDialog.privacyClicked()
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
                visible: root.bodyTextVisible && root.bodyText.length > 0
            }

            ScrollView {
                id: bodyScroll
                visible: root.bodyTextVisible && root.bodyText.length > 0
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                clip: true
                Label {
                    width: contentColumn.width
                    wrapMode: Text.WordWrap
                    text: root.bodyText
                    color: palette.windowText
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 35
            }
        }
    }
}
