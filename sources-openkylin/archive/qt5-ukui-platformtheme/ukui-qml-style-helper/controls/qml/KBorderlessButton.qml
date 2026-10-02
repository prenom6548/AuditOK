/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KBorderlessButton.
 * 图标：image://imageProvider/（KyImageProvider）
 * 颜色：KyButton（C++，GlobalDTConfig）
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.AbstractButton {
    id: control

    property int paletteStyle: Private.KyButton.StyleBorderless

    readonly property bool _hasIcon: icon.name !== "" || icon.source.toString() !== ""
    readonly property int _iconSize: icon.width > 0 ? icon.width
                                : (icon.height > 0 ? icon.height : 16)
    readonly property int _gap: (_hasIcon && text !== "") ? btn.space : 0

    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    font: StylePrivate.APPParameter.font

    property string _toolTipText: ""

    ToolTip{
        delay: Qt.styleHints.mousePressAndHoldInterval
        text: _toolTipText
        visible: hovered && _toolTipText !== ""
    }

    leftPadding: btn.leftRightMargin
    rightPadding: btn.leftRightMargin
    topPadding: btn.upDownMargin
    bottomPadding: btn.upDownMargin

    StylePrivate.UKUIButton {
        id: btn
    }

    property alias buttonStyle: btn

    Private.KyButton {
        id: btnPalette
        styleType: control.paletteStyle
        enabled: control.enabled
        hovered: control.hovered
        pressed: control.pressed
        hasIcon: control._hasIcon
    }

    property alias buttonPalette: btnPalette

    StylePrivate.APPParameter {
        id: appParam
        onParametryChanged: {
            // 先刷新文字/边框色，再重载图标资源（同 URL 需先清空）
            btnPalette.refreshPalette()
            contentItem.updateIconSource()
        }
        onIconThemeChanged: contentItem.updateIconSource()
    }

    background: Item { width: 0; height: 0 }

    implicitWidth: Math.max(implicitContentWidth + leftPadding + rightPadding, 1)
    implicitHeight: Math.max(implicitContentHeight + topPadding + bottomPadding, 1)

    onTextChanged: contentItem.updateToolTip()
    onWidthChanged: contentItem.updateToolTip()
    onIconChanged: contentItem.updateIconSource()
    // hover/press/enable 只改 contentColor，ColorOverlay 自动跟随，无需重载图标

    contentItem: RowLayout {
        id: contentRow
        anchors.fill: parent

        Item { Layout.fillWidth: true }

        RowLayout {
            id: innerRow
            spacing: control._gap
            Layout.alignment: Qt.AlignVCenter

            // 对齐 C++ drawColoredPixmap(pixmap, fontColor)：底图只取图标资源，
            // 颜色用 ColorOverlay 跟 contentColor（主题/hover/press 随 KyButton 刷新）
            function updateIconSource() {
                var s = control.icon.source.toString()
                if (s === "")
                    s = control.icon.name
                if (s === "") {
                    iconImage.source = ""
                    return
                }
                var prefix = "image://imageProvider/"
                if (s.indexOf(prefix) === 0) {
                    var path = s.substring(prefix.length)
                    var slash = path.lastIndexOf("/")
                    if (slash >= 0) {
                        var model = path.substring(slash + 1)
                        if (model === "normal" || model === "disenable" || model === "clicked"
                                || model === "hover" || model === "highlight" || model === "highDisbale"
                                || model === "hoveredWidthThemeColor" || model === "clickedWidthThemeColor"
                                || model === "normalWidthThemeColor" || model === "disableWidthThemeColor")
                            path = path.substring(0, slash)
                    }
                    s = path
                }
                // 同 URL 不重载：先清空再设，保证切主题/图标主题时重新取图
                var url = prefix + s + "/normal"
                iconImage.source = ""
                iconImage.source = url
            }

            Item {
                id: iconBox
                visible: control._hasIcon && iconImage.status === Image.Ready
                Layout.preferredWidth: visible ? control._iconSize : 0
                Layout.preferredHeight: visible ? control._iconSize : 0
                Layout.alignment: Qt.AlignVCenter
                width: control._iconSize
                height: control._iconSize

                Image {
                    id: iconImage
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    cache: false
                    visible: false
                }
                ColorOverlay {
                    anchors.fill: parent
                    source: iconImage
                    // 与文字同色，对齐 C++ fontColor 着色
                    color: btnPalette.contentColor
                }
            }

            Text {
                id: label
                visible: control.text !== ""
                text: control.text
                font: control.font
                color: btnPalette.contentColor
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: control._hasIcon ? Text.AlignLeft : Text.AlignHCenter
                elide: Text.ElideRight
                maximumLineCount: 1
                wrapMode: Text.NoWrap
                Layout.alignment: Qt.AlignVCenter
                Layout.maximumWidth: {
                    if (control.width <= 0)
                        return implicitWidth
                    var available = control.width - control.leftPadding - control.rightPadding
                    if (iconBox.visible)
                        available -= control._iconSize + control._gap
                    return available > 0 ? available : implicitWidth
                }
                onImplicitWidthChanged: contentRow.updateToolTip()
                onWidthChanged: contentRow.updateToolTip()
                onTruncatedChanged: contentRow.updateToolTip()
            }

            Component.onCompleted: {
                updateIconSource()
                contentRow.updateToolTip()
            }
        }

        Item { Layout.fillWidth: true }

        function updateToolTip() {
            control._toolTipText = (control.text && label.truncated) ? control.text : ""
        }

        function updateIconSource() {
            innerRow.updateIconSource()
        }
    }
}
