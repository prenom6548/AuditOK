import QtQuick 2.12
import QtQuick.Window 2.12
//Qt Quick Controls模块提供了大量类似Qt Widgets模块的可重用组件。
import QtQuick.Controls 2.15
import org.username 1.0

 ApplicationWindow {
    visible: true

    minimumWidth: 1096
    maximumWidth: 1096
    minimumHeight: 648
    maximumHeight: 648
    height: 648
    width: 1096
    title:"新功能介绍"
    color: palette.base
    //flags: Qt.FramelessWindowHint

    x: (Screen.width / 2 - width / 2)
    y: (Screen.height / 2 - height / 2)


//    //标题部分
//    TitleWidget {
//        id:titleWidget
//        x:0
//        y:0
//        z:2
//    }

//    //右侧关闭按钮
//    QuitWidget{
//        id:quitWidget
//        x:968
//        y:8
//        z:2

//    }

    //主界面
    Mediatemplate {
        x:0
        y:80
    }
}
