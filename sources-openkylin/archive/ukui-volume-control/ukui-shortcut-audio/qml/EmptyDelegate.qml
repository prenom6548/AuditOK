import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.audio.common 1.0

Item {
    id: emptyDelegate
    width: emptyDelegateWidth
    height: emptyDelegateHeight

    property real emptyDelegateWidth: 396
    property real emptyDelegateHeight: 64
    property alias emptyDelegateTextStr: emptyDelegateText.text

    Label {
        id: emptyDelegateText
        width: 306
        anchors.centerIn: parent
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.NoWrap
        color: Common.getThemePropertyByName("kfont-secondary")
        text: qsTr("")

        ToolTip {
            text: emptyDelegateText.text
            visible: hoverHandler.hovered && emptyDelegateText.truncated
            delay: 500
            property var maxWidth: 500
            property var wrapMode: Text.WrapAnywhere
        }

        HoverHandler { id: hoverHandler}
    }
}
