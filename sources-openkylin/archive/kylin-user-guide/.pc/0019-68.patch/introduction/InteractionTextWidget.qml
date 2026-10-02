import QtQuick 2.0
import QtQuick.Controls 2.15
import org.username 1.0

// Accessibility text
Rectangle {
    id: interactionText
    width: 900;
    height: 235

    ProcessManager {
        id: processmanager
                Component.onCompleted: {
            console.log("ProcessManager 初始化完成")
            updateTextColor()
        }
    }

    // 根据主题更新文本颜色的函数
    function updateTextColor() {
        var themeStyle = processmanager.getThemeStyle()
        console.log("当前主题样式:", themeStyle)
        var isDarkTheme = (themeStyle === "ukui-black" || themeStyle === "ukui-dark")
            var textColor = isDarkTheme ? "white" : "black"
        centerText.color = textColor
        bottomText.color = textColor

        console.log("字体颜色设置为:", textColor, "是否为深色主题:", isDarkTheme)
    }

    Text {
        id: centerText
        width: parent.width
        x:80

        text: qsTr("Lightweight vision · Breathing experience")
        font.bold: true
        wrapMode: Text.WordWrap
        font.pixelSize: 20
        //下方的label
        Text {
            id: bottomText
            x:0
            y:6+centerText.height
            width: 900
            text: qsTr("46 silky sliding effects and millimeter-level interactive aesthetics.")
            elide: Text.ElideRight  // 超出宽度显示...
            clip: true  // 裁剪超出部分
            maximumLineCount: 1  // 限制为单行
            font.pixelSize: 16
            // 添加悬浮提示
            ToolTip.visible: maBottom.containsMouse && truncated
            ToolTip.text: text
            MouseArea {
                id: maBottom
                anchors.fill: parent
                hoverEnabled: true
            }
        }
        anchors.centerIn: parent
    }

    Connections {
        target: processmanager
                onStyleNameChanged: function(message) {
            updateTextColor()
        }
    }

    Component.onCompleted: {
        Qt.callLater(function() {
            if (processmanager) {
                updateTextColor()
            }
        })
    }
}
