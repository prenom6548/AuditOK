import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    property var cards: []
    property string keyword: ""
    property string pluginId: ""
    property color paneColor: "white"
    property color textColor: "black"
    property color secondaryTextColor: "#666666"
    property string fontFamily: ""
    property real fontPointSize: 10

    onKeywordChanged: flickable.contentY = 0
    onPluginIdChanged: flickable.contentY = 0

    function matchesEntry(entry) {
        var displayName = entry.displayName ? entry.displayName.toLowerCase() : ""
        return (keyword.length === 0 || displayName.indexOf(keyword) >= 0)
                && (pluginId.length === 0 || entry.widgetId === pluginId)
    }

    readonly property int matchingCardCount: {
        var count = 0
        for (var i = 0; i < cards.length; ++i) {
            if (matchesEntry(cards[i])) {
                ++count
            }
        }
        return count
    }

    Rectangle {
        anchors.fill: parent
        color: root.paneColor
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: Math.max(height, cardFlow.height + 8)
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        ScrollBar.vertical: ScrollBar {
            policy: flickable.contentHeight > flickable.height
                    ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
        }

        Flow {
            id: cardFlow
            x: 2
            y: 4
            width: Math.max(1, flickable.width - 12)
            spacing: 6

            Repeater {
                id: cardRepeater
                model: root.cards

                delegate: Item {
                    id: card

                    property var entry: modelData
                    property int pressedIndex: -1
                    property real pressX: 0
                    property real pressY: 0
                    property bool nativeDragStarted: false
                    property bool matchesFilter: root.matchesEntry(entry)

                    visible: matchesFilter
                    width: Math.max(entry.previewWidth + 24, 160)
                    height: previewSlot.height + 8 + titleLabel.implicitHeight
                            + (descriptionLabel.visible
                               ? 4 + descriptionLabel.implicitHeight : 0) + 8

                    Item {
                        id: previewSlot
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width
                        height: entry.previewSlotHeight

                        Item {
                            id: previewSurface
                            anchors.centerIn: parent
                            width: entry.previewWidth
                            height: entry.previewHeight
                            clip: true

                            property Item widgetItem: entry.widgetItem || null

                            function attachWidgetItem() {
                                if (!widgetItem)
                                    return
                                widgetItem.parent = previewSurface
                                widgetItem.x = 0
                                widgetItem.y = 0
                                widgetItem.width = entry.desktopWidth
                                widgetItem.height = entry.desktopHeight
                                widgetItem.transformOrigin = Item.TopLeft
                                widgetItem.scale = entry.previewScale
                            }

                            onWidgetItemChanged: attachWidgetItem()
                            Component.onCompleted: attachWidgetItem()

                            Rectangle {
                                anchors.fill: parent
                                visible: !previewSurface.widgetItem
                                radius: 12
                                color: root.paneColor
                                border.width: 1
                                border.color: root.secondaryTextColor

                                Text {
                                    anchors.centerIn: parent
                                    width: Math.max(1, parent.width - 12)
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                    color: root.secondaryTextColor
                                    text: entry.displayName
                                }
                            }

                        }
                    }

                    Text {
                        id: titleLabel
                        anchors.top: previewSlot.bottom
                        anchors.topMargin: 8
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.max(1, parent.width - 24)
                        height: implicitHeight
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        color: root.textColor
                        font.family: root.fontFamily
                        font.pointSize: root.fontPointSize
                        font.bold: true
                        text: entry.displayName

                        ToolTip.visible: titleHover.containsMouse && truncated
                        ToolTip.text: text
                        ToolTip.delay: 500

                        MouseArea {
                            id: titleHover
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            hoverEnabled: true
                        }
                    }

                    Text {
                        id: descriptionLabel
                        anchors.top: titleLabel.bottom
                        anchors.topMargin: entry.description.length > 0 ? 4 : 0
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.max(1, parent.width - 24)
                        height: entry.description.length > 0 ? implicitHeight : 0
                        visible: entry.description.length > 0
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        color: root.textColor
                        font.family: root.fontFamily
                        font.pointSize: Math.max(1, root.fontPointSize - 1)
                        text: entry.description

                        ToolTip.visible: descriptionHover.containsMouse && truncated
                        ToolTip.text: text
                        ToolTip.delay: 500

                        MouseArea {
                            id: descriptionHover
                            anchors.fill: parent
                            acceptedButtons: Qt.NoButton
                            hoverEnabled: true
                        }
                    }

                    MouseArea {
                        id: cardMouse
                        x: previewSlot.x + previewSurface.x
                        y: previewSlot.y + previewSurface.y
                        width: previewSurface.width
                        height: previewSurface.height
                        z: 100
                        acceptedButtons: Qt.LeftButton
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onPressed: function(mouse) {
                            card.pressedIndex = index
                            card.pressX = mouse.x
                            card.pressY = mouse.y
                            card.nativeDragStarted = false
                        }

                        onPositionChanged: function(mouse) {
                            if (!pressed || card.nativeDragStarted || card.pressedIndex < 0)
                                return
                            var distance = Math.abs(mouse.x - card.pressX)
                                    + Math.abs(mouse.y - card.pressY)
                            if (distance < Qt.styleHints.startDragDistance)
                                return
                            card.nativeDragStarted = true
                            var entryIndex = card.pressedIndex
                            widgetManagerDialog.startPreviewDrag(entryIndex, cardMouse)
                            card.pressedIndex = -1
                            card.nativeDragStarted = false
                        }

                        onClicked: {
                            if (!card.nativeDragStarted && card.pressedIndex >= 0)
                                widgetManagerDialog.addWidgetFromPreview(card.pressedIndex)
                            card.pressedIndex = -1
                        }

                        onCanceled: {
                            card.pressedIndex = -1
                            card.nativeDragStarted = false
                        }
                    }
                }
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.matchingCardCount === 0
        color: root.secondaryTextColor
        horizontalAlignment: Text.AlignHCenter
        textFormat: Text.PlainText
        text: root.keyword.length > 0
              ? qsTr("No desktop widgets match \"%1\".").arg(root.keyword)
              : qsTr("No desktop widgets found.\nInstall widget packages to get started.")
    }
}
