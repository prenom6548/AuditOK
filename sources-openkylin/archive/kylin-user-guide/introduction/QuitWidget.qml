import QtQuick 2.0

//右侧关闭按钮
Item {
    id:quitWidget
    width: 24
    height: 24

    //普通态
    Image{
        width: 24
        height: 24

        x:0
        y:0
        visible: true

        id:iconLabelQuit
        source: "/Icon/quit.png"

    }

    //悬停态
    Image{
        width: 24
        height: 24

        x:0
        y:0
        visible: false

        id:iconLabelQuitHover
        source: "/Icon/exit-hover.svg"

    }

    //按下态
    Image{
        width: 24
        height: 24

        x:0
        y:0
        visible: false

        id:iconLabelQuitPressed
        source: "/Icon/exit-press.svg"

    }


    MouseArea{
        anchors.fill: parent//鼠标检测的作用域
        hoverEnabled: true//开启鼠标滑过检测
        onEntered: {
            //鼠标进入
            iconLabelQuit.visible = false
            iconLabelQuitHover.visible = true
            iconLabelQuitPressed.visible = false
        }
        onExited: {
            //鼠标离开
            iconLabelQuit.visible = true
            iconLabelQuitHover.visible = false
            iconLabelQuitPressed.visible = false
        }

        onClicked: {
            //点击
            iconLabelQuit.visible = false
            iconLabelQuitHover.visible = false
            iconLabelQuitPressed.visible = true
            Qt.quit();
        }
    }
}
