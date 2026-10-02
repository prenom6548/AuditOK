import QtQuick 2.0
import QtGraphicalEffects 1.15
import QtMultimedia 5.15

//Qt Quick Controls模块提供了大量类似Qt Widgets模块的可重用组件。
import QtQuick.Controls 2.0

import org.username 1.0

Item {
    id:mainWid
    anchors.fill: parent
    width: 1096
    height: 648
    z:1

    property int playNum : 1
    property bool ones : true
    signal buttonClick(var t1)
    signal videoNum(var t1)

    ProcessManager {
        id: processmanager
    }

    function maxPlayNum() {
        return processmanager.isOpenkylin() ? 6 : 5
    }

    Rectangle {
        id: progressBarCover
        width: 1096
        height: 4
        x: 0
        y: 633
        z: 7
        color: palette.base
    }

    ProgressBar{
        id : videoProgressBar
        from: 0.0
        to:100
        value: 0
        width: 1096
        height: 4
        x:0
        y:644
        z:6

        background: Rectangle {   //背景项
              implicitWidth: videoProgressBar.width
              implicitHeight: videoProgressBar.height
              color: "#DDDDDD"
              radius: 3 //圆滑度
          }

          contentItem: Item {   //内容项
              Rectangle {
                  width: videoProgressBar.visualPosition * videoProgressBar.width
                  height: videoProgressBar.height
                  radius: 2
                  color: "#3790FA"
              }
          }
    }


    //! 加载四个视频(无动画)
    Item {
        id:mediaPlayer
//        anchors.fill: parent
        width: 880
        height: 494
        x:108
        y:110
        z:3

        MediaPlayer {
            id: player01
            autoPlay: true
            loops: MediaPlayer.Infinite  // 无限循环
            playlist: Playlist {
                id: playlist
            }

            //! 视频播放的停止信号,停止后先看播放的视频是第几个
            //! 然后停留在这个视频的首帧
            onStopped: {
                updateEndImage()
            }

        }

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            source: player01
            visible: false  // 隐藏原始 VideoOutput，用 OpacityMask 显示
        }
        // 圆角矩形（作为遮罩）
        Rectangle {
            id: roundedRect
            anchors.fill: parent
            radius: 12  // 圆角半径 12px
            visible: false  // 仅作为遮罩模板，不直接显示
        }

        // 使用 OpacityMask 将视频裁剪为圆角
        OpacityMask {
            anchors.fill: videoOutput
            source: videoOutput
            maskSource: roundedRect
        }
    }

    //功能主要是设置视频最后一帧
    Image {
        id: videoEndImage
        visible: false
        width: 880
        height: 494
        x:108
        y:110
    }

    //功能主要是设置视频开始一帧
    Image {
        id: videoBeginImage
        visible: false
        width: 880
        height: 494
        x:108
        y:110
        source: (processmanager.isOpenkylin())
                ? "qrc:/media/01-openkylin.jpg"
                : "/media/picture/01-开始帧.png"
    }

    //! 左侧文字描述
    Item {
        id: leftTextItem
        width: 341;
        height: 315
        z:4

        // 交互 Text
        InteractionTextWidget {
            id: interactionText
            visible: false
            color: "transparent"

            //移动位置
            x:108
            y:-80
        }

        // 智能屏保 Text
        SmartScreensaverTextWidget {
            id: smartScreensaverText
            visible: false
            color: "transparent"

            //移动位置
            x:108
            y:-80
        }

        // 控制台 Text
        ConsoleTextWidget {
            id: consoleText
            visible: false
            color: "transparent"

            //移动位置
            x:108
            y:-80
        }

        // AI Text
        SmartAssistantTextWidget {
            id: aiAssistanttText
            visible: false
            color: "transparent"

            //移动位置
            x:108
            y:-80
        }

        // 文件管理器 Text
        FileManagerTextWidget {
            id: fileManagerText //fileManagerText
            visible: false
            color: "transparent"

            //移动位置
            x:108
            y:-80
        }

        // 磐石架构 Text
        RockArchitectureTextWidget {
            id: rockarchitectureText
            visible: false
            color: "transparent"

            //移动位置
            x:108
            y:-80
        }
    }

    //切换按钮
    Item {
        id: switchButtonItem

        z:3
        // 右切换按钮
        Button{
            id:rIcon

            //移动位置
            x:1036
            y:280
            width: 48
            height: 48
            flat: true
            contentItem: Item {
                anchors.fill: parent

                Image {
                    id: riconImage
                    source: "/Icon/righticon.svg"
                    sourceSize.width: 32
                    sourceSize.height: 32
                    anchors.centerIn: parent
                    visible: false
                }

                ColorOverlay {
                    anchors.fill: riconImage
                    source: riconImage
                    color: {
                        if (!rIcon.enabled) {
                            return "lightgray"
                        } else if (rIcon.hovered) {
                            return "black"
                        } else {
                            return "gray"
                        }
                    }

                    Behavior on color {
                        ColorAnimation { duration: 50 }
                    }
                }
            }
            onClicked: {
                // 根据 mainWid.playNum 来判断视频播放第几个
                // 如果超过现有视频，按钮置灰，不发送点击事件
                if (mainWid.playNum < maxPlayNum()) {
                    rIcon.enabled = true
                    lIcon.enabled = true
                    mainWid.buttonClick("right");
                    if ((mainWid.playNum + 1) > maxPlayNum()) {
                        rIcon.enabled = false
                    }
                }
            }
        }

        // 左切换按钮
        Button {
            id:lIcon

            //移动位置
            x:14
            y:280
            width: 48
            height: 48
            flat: true
            enabled: false
            contentItem: Item {
                anchors.fill: parent

                Image {
                    id: liconImage
                    source: "/Icon/lefticon.svg"
                    sourceSize.width: 32
                    sourceSize.height: 32
                    anchors.centerIn: parent
                    visible: false
                }

                ColorOverlay {
                    anchors.fill: liconImage
                    source: liconImage
                    color: {
                        if (!lIcon.enabled) {
                            return "lightgray"
                        } else if (lIcon.hovered) {
                            return "black"
                        } else {
                            return "gray"
                        }
                    }

                    Behavior on color {
                        ColorAnimation { duration: 50 }
                    }
                }
            }

            onClicked: {
                // 根据 mainWid.playNum 来判断视频播放第几个
                // 如果超过现有视频，按钮置灰，不发送点击事件
                if (mainWid.playNum >1) {
                    lIcon.enabled = true
                    rIcon.enabled = true
                    mainWid.buttonClick("left");
                    if (mainWid.playNum <=1) {
                        lIcon.enabled = false
                    }
                }
            }
        }
    }

    //根据当前播放的视频插入视频最后一帧图片
    function updateEndImage(){
        //贴图片,根据标志位
        if (mainWid.playNum === 1) {
            if (processmanager.isOpenkylin()) {
                return
            }
            videoEndImage.source = "/media/picture/01-end.png"

        }
        videoEndImage.visible = true

    }

    //播放切换动作
    function switchAction (t1) {

        if (t1 === "right") {
            mainWid.playNum ++

        } else {
            mainWid.playNum --
        }
        //发送信号给文字去更新
        mainWid.videoNum(mainWid.playNum);

        //播放视频
        playMedia(mainWid.playNum)

        console.log(mainWid.playNum)

        //更新滑动条数据
        changeProgressBar(mainWid.playNum)
    }

    //根据全局变量 playNum 来选择播放的视频
    function playMedia(){

        videoBeginImage.visible = false

        //先隐藏最后一帧图片
        videoEndImage.visible = false

        //上一个视频暂停
        player01.stop()

        //playlist 列表清除
        player01.playlist.clear()

        //根据当前要播放的标志位，加载对应播放路径
        if (mainWid.playNum === 1) {
            if (processmanager.isOpenkylin()) {
                videoBeginImage.source = "qrc:/media/01-openkylin.jpg"
                videoBeginImage.visible = true
            } else {
                player01.playlist.addItem("qrc:/media/01.mp4")
                //播放视频
                player01.play()
            }
        } else if (mainWid.playNum === 2) {
            if (processmanager.isOpenkylin()) {
                videoBeginImage.source = "qrc:/media/02-openkylin.jpg"
            } else {
                videoBeginImage.source = "qrc:/media/02.png"
            }
            videoBeginImage.visible = true
        } else if (mainWid.playNum === 3) {
            if (processmanager.isOpenkylin()) {
                videoBeginImage.source = "qrc:/media/03-openkylin.jpg"
            } else {
                videoBeginImage.source = "qrc:/media/03.png"
            }
            videoBeginImage.visible = true
        } else if (mainWid.playNum === 4) {
            if (processmanager.isOpenkylin()) {
                videoBeginImage.source = "qrc:/media/04-openkylin.jpg"
            } else {
                videoBeginImage.source = "qrc:/media/04.png"
            }
            videoBeginImage.visible = true
        } else if (mainWid.playNum === 5) {
            if (processmanager.isOpenkylin()) {
                videoBeginImage.source = "qrc:/media/05-openkylin.jpg"
            } else {
                videoBeginImage.source = "qrc:/media/05.png"
            }
            videoBeginImage.visible = true
        } else if (mainWid.playNum === 6) {
            videoBeginImage.source = "qrc:/media/06-openkylin.jpg"
            videoBeginImage.visible = true
        }
    }

    //根据信号改变文案
    function changeText (t1) {

        if (t1 === 1) {
            //交互
            interactionText.visible = true
            smartScreensaverText.visible = false
            consoleText.visible = false
            aiAssistanttText.visible = false
            fileManagerText.visible = false
            rockarchitectureText.visible = false
        } else if (t1 === 2) {
            //智能屏保页面
            interactionText.visible = false
            smartScreensaverText.visible = true
            consoleText.visible = false
            aiAssistanttText.visible = false
            fileManagerText.visible = false
            rockarchitectureText.visible = false
        } else if (t1 === 3) {
            //控制台页面
            interactionText.visible = false
            smartScreensaverText.visible = false
            consoleText.visible = true
            aiAssistanttText.visible = false
            fileManagerText.visible = false
            rockarchitectureText.visible = false
        } else if (t1 === 4) {
            //AI
            interactionText.visible = false
            smartScreensaverText.visible = false
            consoleText.visible = false
            aiAssistanttText.visible = true
            fileManagerText.visible = false
            rockarchitectureText.visible = false
        } else if (t1 === 5) {
            //文件管理器
            interactionText.visible = false
            smartScreensaverText.visible = false
            consoleText.visible = false
            aiAssistanttText.visible = false
            fileManagerText.visible = true
            rockarchitectureText.visible = false
        } else if (t1 === 6) {
            //磐石架构
            interactionText.visible = false
            smartScreensaverText.visible = false
            consoleText.visible = false
            aiAssistanttText.visible = false
            fileManagerText.visible = false
            rockarchitectureText.visible = true
        }
    }

    function changeProgressBar (t1) {
        videoProgressBar.value = (t1 - 1) * 100 / (maxPlayNum() - 1)
    }


    //! 播放第一个视频,并且关联 buttonClick 信号 与 switchAction 槽函数
    Component.onCompleted: {
        playMedia()
        interactionText.visible = true                    //首先展示欢迎页面的文字
        videoBeginImage.visible = true                    //首先展示欢迎页面的文字
        mainWid.buttonClick.connect(switchAction)     //连接视频切换
        mainWid.videoNum.connect(changeText)          //连接文案改变槽
    }
}
