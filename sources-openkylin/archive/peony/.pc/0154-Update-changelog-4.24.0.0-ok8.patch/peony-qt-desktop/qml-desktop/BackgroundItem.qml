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
        // 静态壁纸作为底层兜底，PAG 壁纸时由不透明的 PAGView 覆盖
        visible: !root.usePureColor
        onStatusChanged: root.tryWallpaperTransition()
    }

    Image {
        id: frontImage
        anchors.fill: parent
        source: frontImagePath
        sourceSize: frontImageSourceSize
        fillMode: getFillMode(backgroundOption)
        opacity: 1
        visible: !root.usePureColor
        onStatusChanged: {
            root.tryWallpaperTransition()
            root.handleFrontImageStatusChanged()
        }
    }

    Rectangle {
        id: colorBackground
        anchors.fill: parent
        color: root.bgColor
        visible: root.usePureColor
    }

    // 静态壁纸对应的同名 .pag 路径。它只是解锁瞬间的候选动画
    // 普通启动、壁纸切换和显示模式变化都不会触发加载
    readonly property string pagLocalPath: {
        var opt = backgroundOption.toLowerCase()
        if (opt !== "stretched" && opt !== "scaled" && opt !== "zoom")
            return ""

        var path = frontImagePath
        if (path.startsWith("file://"))
            path = path.substring(7)
        if (path.endsWith(".pag"))
            return path
        var dot = path.lastIndexOf(".")
        var pagPath = dot !== -1 ? path.substring(0, dot) + ".pag" : path + ".pag"
        return (backgroundManager.fileExists(pagPath) && platformWindow.isPrimary) ? pagPath : ""
    }

    // DesktopView 的冷启动逻辑不应等待 PAG；动画仅作为解锁时的覆盖层
    readonly property bool isPagWallpaper: false

    Loader {
        id: pagLoader
        anchors.fill: parent
        active: false
        sourceComponent: pagComponent
        onLoaded: root.startPendingUnlockOnView()
    }

    Component {
        id: pagComponent
        PAGView {
            anchors.fill: parent
            opacity: 0
            onPrepared: {
                // PrepareHidden(0) 已在透明度为零时完成呈现
                if (generation !== root.unlockGeneration)
                    return
                if (root.pagSessionState === root.pagPreloading) {
                    root.pagSessionState = root.pagPreparedWhileLocked
                    return
                }
                if (root.pagSessionState === root.pagWaitingAfterUnlock)
                    root.startPreparedPag(generation)
            }
            onFinished: {
                if (generation !== root.unlockGeneration)
                    return
                opacity = 0
                root.pagFirstFrameReady = false
                root.pagSessionState = root.pagIdle
            }
            onFailed: {
                if (generation !== root.unlockGeneration)
                    return
                opacity = 0
                root.pagFirstFrameReady = false
                if (root.pagSessionState === root.pagPreloading ||
                        root.pagSessionState === root.pagPreparedWhileLocked)
                    root.pagSessionState = root.pagPreloadInvalid
                else
                    root.pagSessionState = root.pagIdle
                console.warn("BackgroundItem: unlock PAG failed:", reason)
            }
            onSafeToUnload: {
                if (generation !== root.loaderGeneration)
                    return
                if (root.pagSessionState === root.pagPreloading ||
                        root.pagSessionState === root.pagPreparedWhileLocked)
                    root.pagSessionState = root.pagPreloadInvalid
                root.safeLoaderGeneration = generation
                Qt.callLater(root.tryUnloadSafeLoader)
            }
        }
    }

    readonly property int pagIdle: 0
    readonly property int pagPreloading: 1
    readonly property int pagPreparedWhileLocked: 2
    readonly property int pagWaitingAfterUnlock: 3
    readonly property int pagPlaying: 4
    readonly property int pagPreloadInvalid: 5

    property double unlockGeneration: 0
    property int pagSessionState: pagIdle
    property bool pendingUnlock: false
    property string pendingUnlockPath: ""
    property string pendingUnlockOption: ""
    property double pendingUnlockGeneration: 0
    property bool pendingSkipStabilityDelay: false
    property double loaderGeneration: 0
    property double safeLoaderGeneration: 0
    property bool lockedPreloadActive: false
    property bool preloadRestartPending: false

    function cancelUnlockPag() {
        pendingUnlock = false
        pendingUnlockPath = ""
        pendingUnlockOption = ""
        pendingUnlockGeneration = 0
        pendingSkipStabilityDelay = false
        pagSessionState = pagIdle
        if (pagLoader.item)
            pagLoader.item.opacity = 0
        pagFirstFrameReady = false
        if (pagLoader.item)
            pagLoader.item.cancelPlayback()
        Qt.callLater(root.tryUnloadSafeLoader)
    }

    function startPreparedPag(generation) {
        if (generation !== unlockGeneration || !pagLoader.item ||
                pagSessionState === pagPlaying)
            return
        pagLoader.item.opacity = 1
        pagFirstFrameReady = true
        pagSessionState = pagPlaying
        pagLoader.item.beginVisiblePlayback(generation)
    }

    function requestPagSession(generation, preload) {
        cancelUnlockPag()
        unlockGeneration = generation
        if (usePureColor || pagLocalPath === "")
            return
        pagSessionState = preload ? pagPreloading : pagWaitingAfterUnlock
        pendingUnlock = true
        pendingUnlockPath = pagLocalPath
        pendingUnlockOption = backgroundOption
        pendingUnlockGeneration = generation
        pendingSkipStabilityDelay = preload
        tryStartPendingUnlock()
    }

    function restartLockedPreload() {
        cancelUnlockPag()
        if (!lockedPreloadActive || !platformWindow.isPrimary ||
                preloadRestartPending)
            return

        preloadRestartPending = true
        var generation = unlockGeneration
        Qt.callLater(function() {
            preloadRestartPending = false
            if (lockedPreloadActive && platformWindow.isPrimary &&
                    generation === unlockGeneration)
                requestPagSession(generation, true)
        })
    }

    function tryUnloadSafeLoader() {
        if (!pagLoader.active || pendingUnlock ||
                safeLoaderGeneration !== loaderGeneration)
            return
        if (pagLoader.item && pagLoader.item.active)
            return
        pagLoader.active = false
        loaderGeneration = 0
        safeLoaderGeneration = 0
    }

    function tryStartPendingUnlock() {
        if (!pendingUnlock)
            return
        if (frontImage.status === Image.Error) {
            cancelUnlockPag()
            return
        }
        if (frontImage.status !== Image.Ready)
            return
        if (root.usePureColor || !platformWindow.isPrimary ||
                root.pagLocalPath !== pendingUnlockPath ||
                root.backgroundOption !== pendingUnlockOption) {
            cancelUnlockPag()
            return
        }

        pagLoader.active = true
        if (pagLoader.item)
            startPendingUnlockOnView()
    }

    function startPendingUnlockOnView() {
        if (!pendingUnlock || !pagLoader.item)
            return
        if (frontImage.status !== Image.Ready || root.usePureColor ||
                !platformWindow.isPrimary ||
                root.pagLocalPath !== pendingUnlockPath ||
                root.backgroundOption !== pendingUnlockOption) {
            cancelUnlockPag()
            return
        }
        var path = pendingUnlockPath
        var option = pendingUnlockOption
        var generation = pendingUnlockGeneration
        var skipStabilityDelay = pendingSkipStabilityDelay
        pendingUnlock = false
        pendingUnlockPath = ""
        pendingUnlockOption = ""
        pendingUnlockGeneration = 0
        pendingSkipStabilityDelay = false
        loaderGeneration = generation
        safeLoaderGeneration = 0
        if (!pagLoader.item.prepareUnlock(path, option, generation, true, true,
                                          skipStabilityDelay)) {
            pagSessionState = skipStabilityDelay ? pagPreloadInvalid : pagIdle
            console.warn("BackgroundItem: unlock PAG request became invalid:", path)
        }
    }

    function handleFrontImageStatusChanged() {
        if (frontImage.status === Image.Error && pendingUnlock) {
            cancelUnlockPag()
            return
        }
        if (frontImage.status === Image.Ready)
            tryStartPendingUnlock()
    }

    Connections {
        target: windowManager
        function onUnlockAnimationPreloadRequested(generation) {
            // 锁屏阶段只准备首帧，解锁时复用已准备的 PAG 会话
            root.lockedPreloadActive = true
            if (generation === root.unlockGeneration &&
                    platformWindow.isPrimary &&
                    (root.pagSessionState === root.pagPreloading ||
                     root.pagSessionState === root.pagPreparedWhileLocked))
                return
            root.requestPagSession(generation, true)
        }
        function onUnlockAnimationRequested(generation) {
            root.lockedPreloadActive = false
            if (generation === root.unlockGeneration) {
                if (root.pagSessionState === root.pagPreparedWhileLocked) {
                    root.startPreparedPag(generation)
                    return
                }
                if (root.pagSessionState === root.pagPreloading) {
                    root.pagSessionState = root.pagWaitingAfterUnlock
                    if (pagLoader.item)
                        pagLoader.item.requestPendingFrameUpdate()
                    return
                }
                if (root.pagSessionState === root.pagWaitingAfterUnlock ||
                        root.pagSessionState === root.pagPlaying)
                    return
            }
            root.requestPagSession(generation, false)
        }
        function onUnlockAnimationCancelled(generation) {
            if (generation === root.unlockGeneration) {
                root.lockedPreloadActive = false
                root.cancelUnlockPag()
            }
        }
        function onModeChanged() {
            if (root.lockedPreloadActive)
                root.restartLockedPreload()
        }
    }

    Connections {
        target: platformWindow
        function onIsPrimaryChanged() {
            if (platformWindow.isPrimary)
                windowManager.requestInitialUnlockPreload()
            else
                root.restartLockedPreload()
        }
    }

    onUsePureColorChanged: restartLockedPreload()
    onPagLocalPathChanged: restartLockedPreload()

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
        restartLockedPreload()
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
        restartLockedPreload()
        console.log("onBackgroundOptionChanged", backgroundOption)
        frontImage.fillMode = getFillMode(backgroundOption)
        backImage.fillMode = getFillMode(backgroundOption)
        updateImageAlignment()
        updateImageSourceClipRect()
        backgroundManager.forceUpdated()
    }

    Component.onCompleted: {
        if (platformWindow.isPrimary)
            windowManager.requestInitialUnlockPreload()
    }

}
