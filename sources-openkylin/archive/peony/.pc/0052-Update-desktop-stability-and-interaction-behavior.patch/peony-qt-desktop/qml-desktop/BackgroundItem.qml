import QtQuick 2.15
import org.ukui.peony.desktop.pag 1.0

Item {
    id: root

    //外部属性
    property string frontImagePath: ""
    property string backImagePath: ""
    property size frontImageSourceSize: Qt.size(0, 0)
    property size backImageSourceSize: Qt.size(0, 0)
    property string backgroundOption: "scaled"
    property bool usePureColor: false
    property string bgColor: "black"
    property rect screenGeometry: Qt.rect(0, 0, 0, 0)

    //内部属性
    property real animationDuration: 1000
    property rect sourceRect: Qt.rect(0, 0, 0, 0)
    property bool pagFirstFrameReady: false
    property bool isFirstStartup: true

    ParallelAnimation {
        id: animation
        NumberAnimation {
            target: backImage
            property: "opacity"
            from: 1
            to: 0
            duration: animationDuration
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            target: frontImage
            property: "opacity"
            from: 0
            to: 1
            duration: animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Image {
        id: backImage
        anchors.fill: parent
        source: backImagePath
        sourceSize: backImageSourceSize
        fillMode: getFillMode(backgroundOption)
        opacity: 0
        visible: !root.usePureColor && (!root.isPagWallpaper || (!root.pagFirstFrameReady && !root.isFirstStartup))
        onStatusChanged: root.tryWallpaperTransition()
    }

    Image {
        id: frontImage
        anchors.fill: parent
        source: frontImagePath
        sourceSize: frontImageSourceSize
        fillMode: getFillMode(backgroundOption)
        opacity: 1
        visible: !root.usePureColor && (!root.isPagWallpaper || (!root.pagFirstFrameReady && !root.isFirstStartup))
        onStatusChanged: root.tryWallpaperTransition()
    }

    Rectangle {
        id: colorBackground
        anchors.fill: parent
        color: root.bgColor
        visible: root.usePureColor
    }

    // 静态壁纸对应的同名 .pag 路径（去除 file:// 前缀）；若文件不存在或当前显示模式不适合则为空串
    readonly property string pagLocalPath: {
        // 居中/平铺/跨区模式下语义与全屏动画不兼容，不播放 PAG
        var opt = backgroundOption.toLowerCase()
        if (opt === "centered" || opt === "wallpaper" || opt === "spanned")
            return ""

        var path = frontImagePath
        if (path.startsWith("file://"))
            path = path.substring(7)
        if (path.endsWith(".pag"))
            return path
        var dot = path.lastIndexOf(".")
        var pagPath = dot !== -1 ? path.substring(0, dot) + ".pag" : path + ".pag"
        // 仅主屏播放 PAG，非主屏直接显示静态壁纸
        return (backgroundManager.fileExists(pagPath) && platformWindow.isPrimary) ? pagPath : ""
    }

    // 存在对应的 .pag 文件时为 true
    readonly property bool isPagWallpaper: pagLocalPath !== ""
    
    // PAG 壁纸 Loader：isPagWallpaper 为 false 时自动卸载并释放 GL 资源
    Loader {
        id: pagLoader
        anchors.fill: parent
        active: root.isPagWallpaper && !root.usePureColor
        sourceComponent: pagComponent
    }

    Component {
        id: pagComponent

        PAGView {
            id: pagViewObj
            anchors.fill: parent
            loop: false
            // 首帧未就绪时保持不可见，确保 firstFrameReady 与首帧纹理在同一 Wayland 帧中原子呈现，
            // 避免出现"PAG 已显示但桌面图标尚未出现"的一帧闪烁。
            //opacity: root.pagFirstFrameReady ? 1 : 0

            onFirstFrameReady: {
                root.pagFirstFrameReady = true
                root.isFirstStartup = false
            }

            onFinished: {
                // 播放完毕：隐藏 PAGView，显示底层静态壁纸
                // PAGView 保持 loaded 状态，解锁时 restart() 可直接重播
                root.pagFirstFrameReady = false
            }

            Component.onCompleted: {
                if (root.pagLocalPath !== "") {
                    root.pagFirstFrameReady = false
                    pagViewObj.setScaleModeFromOption(root.backgroundOption)
                    var ok = setFile(root.pagLocalPath)
                    if (!ok) {
                        console.warn("BackgroundItem: PAGView failed to load:", root.pagLocalPath)
                        root.isFirstStartup = false
                    }
                }
            }

            Connections {
                target: root
                function onPagLocalPathChanged() {
                    if (root.pagLocalPath !== "") {
                        root.isFirstStartup = false
                        root.pagFirstFrameReady = false
                        pagViewObj.setScaleModeFromOption(root.backgroundOption)
                        pagViewObj.setFile(root.pagLocalPath)
                    }
                }
                function onBackgroundOptionChanged() {
                    pagViewObj.setScaleModeFromOption(root.backgroundOption)
                }
            }
        }
    }

    // 解锁屏幕时重播 PAG 动画；图标入场动效由 DesktopView 监听 PAG firstFrameReady 后重新触发
    Connections {
        target: windowManager
        function onStartInitAnimaRequest() {
            if (pagLoader.item) {
                root.pagFirstFrameReady = false
                pagLoader.item.restart()
            }
        }
    }

    Connections {
        target: backgroundManager
        function onForceUpdated() {
            backImage.cache = false
            frontImage.cache = false
            backImage.cache = true
            frontImage.cache = true
        }
    }

    //获取填充模式
    function getFillMode(option) {
        switch(option.toLowerCase()) {
            case "stretched": return Image.Stretch;
            case "scaled": return Image.PreserveAspectCrop;
            case "centered": return Image.Pad;
            case "wallpaper": return Image.Tile;
            case "zoom": return Image.PreserveAspectFit;
            case "spanned": return handleSpanned();
            default: return Image.PreserveAspectCrop;
        }
    }

    //处理跨区
    function handleSpanned() {
        if (backgroundOption != "spanned")
            return Image.Stretch

        var frontRect = backgroundManager.calculateSpannedSourceRect(screenGeometry, frontImagePath)
        var backRect = backgroundManager.calculateSpannedSourceRect(screenGeometry, backImagePath)

        backImage.sourceClipRect = backRect
        frontImage.sourceClipRect = frontRect

        console.log("handleSpanned rect", backRect, frontRect, frontImagePath, backImagePath)
        return Image.Stretch
    }

    //更新对齐
    function updateImageAlignment() {
        if (backgroundOption == "wallpaper") {
            backImage.horizontalAlignment = Image.AlignLeft
            backImage.verticalAlignment = Image.AlignTop
            frontImage.horizontalAlignment = Image.AlignLeft
            frontImage.verticalAlignment = Image.AlignTop
        } else {
            backImage.horizontalAlignment = Image.AlignHCenter
            backImage.verticalAlignment = Image.AlignVCenter
            frontImage.horizontalAlignment = Image.AlignHCenter
            frontImage.verticalAlignment = Image.AlignVCenter
        }
    }

    onIsPagWallpaperChanged: {
        if (!isPagWallpaper)
            pagFirstFrameReady = false
    }

    //清除切换跨区后sourceClipRect属性
    function updateImageSourceClipRect() {
        if (backgroundOption != "spanned") {
            backImage.sourceClipRect = undefined
            frontImage.sourceClipRect = undefined
        }
    }

    function tryWallpaperTransition() {
        if (root.usePureColor)
            return
        if (root.isPagWallpaper)
            return
        // New image failed to decode: keep previous wallpaper visible instead of a black layer.
        if (frontImage.status === Image.Error && backImage.status === Image.Ready) {
            backImage.opacity = 1
            frontImage.opacity = 0
            return
        }
        if (backImage.status === Image.Ready && frontImage.status === Image.Ready) {
            // Same URI on both layers (e.g. startup): skip cross-fade to avoid a visible flash.
            if (backImagePath === frontImagePath && frontImagePath.length > 0) {
                backImage.opacity = 0
                frontImage.opacity = 1
                return
            }
            if (backImagePath.length > 0 && frontImagePath.length > 0) {
                animation.restart()
            }
        }
    }

    onFrontImagePathChanged: {
        if (!root.isPagWallpaper) {
            tryWallpaperTransition()
            updateImageAlignment()
        }
    }

    onBackImagePathChanged: {
        if (!root.isPagWallpaper) {
            tryWallpaperTransition()
            updateImageAlignment()
        }
    }

    onScreenGeometryChanged: {
        handleSpanned()
    }

    onBackgroundOptionChanged: {
        console.log("onBackgroundOptionChanged", backgroundOption)
        frontImage.fillMode = getFillMode(backgroundOption)
        backImage.fillMode = getFillMode(backgroundOption)
        updateImageAlignment()
        updateImageSourceClipRect()
        backgroundManager.forceUpdated()
    }

}
