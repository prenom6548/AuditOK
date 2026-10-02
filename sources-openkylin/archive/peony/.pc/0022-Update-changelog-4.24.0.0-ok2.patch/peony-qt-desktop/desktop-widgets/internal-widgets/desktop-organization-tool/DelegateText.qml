import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import org.ukui.peony.desktop 1.0
Item {
    id: root
    width: islistview ? maxwidth :parent.width
    height: editing && editLoader.active && editLoader.item ? Math.min(editLoader.item.textEdit.implicitHeight, maxVisibleHeight) : textItem.height
    z:parent.z + 1
    // 公共属性
    property var selectedUris: []
    property string display: ""  // 显示的文本内容
    property bool editing: false  // 是否处于编辑状态
    property bool selected: false  // 是否处于选中状态
    property bool isDir: false // 是否处于目录
    property bool isSoftLink: false // 是否是软连接
    property bool showFullText: false
    property string lastValidText: root.display
    property int maxBytes: 255
    property int maxVisibleHeight: 0
    property bool islistview: false
    property int maxwidth : width
    signal editFinish(var selectedUris, string newText)

    Label {
        id: textItem
        text: root.display
        color: /*!selected ? "black":*/"white"
        //anchors.horizontalCenter: parent.horizontalCenter
        //anchors.top: parent.top
        anchors.topMargin:2
        width: islistview ?maxwidth :parent.width - 8
        wrapMode: Text.WrapAnywhere
        elide: Text.ElideRight
        maximumLineCount:1
        horizontalAlignment: islistview ? Qt.AlignLeft :Qt.AlignHCenter
        verticalAlignment: islistview ? Text.AlignVCenter: Text.AlignTop
        anchors.horizontalCenter: parent.horizontalCenter
        renderType: Text.NativeRendering
        opacity: 0.8
        font.pixelSize: 11
        Text {
            id: labelText
            visible: false
            width:textItem.width
            font: textItem.font
            text: textItem.text
            wrapMode: textItem.wrapMode
            maximumLineCount: 1
            //verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: islistview ? Qt.AlignLeft :Qt.AlignHCenter
            verticalAlignment: islistview ?Text.AlignVCente: Text.AlignTop
        }
        height: Math.min(labelText.paintedHeight, root.maxVisibleHeight > 0 ? root.maxVisibleHeight : labelText.paintedHeight)

        visible: !root.editing

        onTruncatedChanged: {
            if (model && model.uri) {
                Controller.updateFileElideStatus(model.uri, truncated)
            }
        }
    }

    DropShadow {
        anchors.fill: textItem
        source: textItem
        horizontalOffset: 1
        verticalOffset: 2
        radius: 3
        samples: radius*2
        color: "#ff000000"
        visible: false//!root.editing
    }

    // 编辑状态
    Loader {
        id: editLoader
        active: false  // 初始不加载
        anchors.fill: parent
        z:parent.z + 1
        sourceComponent:ScrollView {
            z:12
            id: scrollView
            width: islistview ? maxwidth :parent.width
            height: Math.min(
                textEdit.implicitHeight,
                maxVisibleHeight > 0 ? maxVisibleHeight :  textEdit.implicitHeight // 限制最大高度
            )

            visible: root.editing

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            property alias textEdit: textEdit // 暴露 TextArea 到外部
            TextArea {
                id: textEdit
                text: root.display
                width: islistview ? maxwidth :root.width
                topPadding: 3
                bottomPadding: 3
                leftPadding: 5
                rightPadding: 5
                wrapMode: Text.WrapAnywhere
                height: implicitHeight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                property var transparent: false
                font.pixelSize: 11
                // 只在编辑状态显示
                visible: root.editing
                focus: root.editing

                property int lastCursor: 0
                property bool isPaste: false
                Keys.onPressed: (event) => {
                    if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V) {
                        Qt.callLater(() => {
                            isPaste = true
                        })
                    }
                }

                onTextChanged: {
                    const currentBytes = getUtf8Length(text)
                    if (currentBytes > maxBytes) {
                        const cursor = cursorPosition
                        let prevText = lastValidText
                        if (isPaste && maxBytes !== getUtf8Length(prevText)) {
                            const newText = truncateUtf8WithExtension(text, maxBytes)
                            let inputLength = text.length - newText.length
                            text = newText
                            inputLength += text.length - prevText.length
                            Qt.callLater(() => {
                                textEdit.cursorPosition = Math.max(0, Math.min(cursor - inputLength, text.length))
                            })
                            lastValidText = newText
                            isPaste = false
                        } else {
                            let inputLength = text.length - prevText.length
                            text = prevText
                            Qt.callLater(() => {
                                textEdit.cursorPosition = Math.max(0, Math.min(cursor - inputLength, text.length))
                            })
                        }
                    } else {
                        lastValidText = text
                    }
                }

                // 按回车完成编辑
                Keys.onReturnPressed: {
                    updateFileName()
                }

                // 点击外部完成编辑
                onActiveFocusChanged: {
                    if (!activeFocus && root.editing) {
                        updateFileName()
                    }
                }

                // UTF-8 字节计算函数
                function getUtf8Length(str) {
                    return unescape(encodeURIComponent(str)).length
                }

                // 按需截断，保留扩展名
                function truncateUtf8WithExtension(str, maxBytes) {
                    let name = str
                    let ext = ""

                    const doubleExts = [".tar.gz", ".tar.xz", ".tar.bz2", ".tar.zst", ".tar.lz"]
                    for (let i = 0; i < doubleExts.length; i++) {
                        if (str.endsWith(doubleExts[i])) {
                            ext = doubleExts[i]
                            name = str.slice(0, -ext.length)
                            break
                        }
                    }

                    if (ext === "") {
                        const lastDot = str.lastIndexOf(".")
                        if (lastDot > 0 && !str.startsWith(".")) {
                            ext = str.slice(lastDot)
                            name = str.slice(0, lastDot)
                        }
                    }

                    const extUtf8Len = getUtf8Length(ext)
                    const nameMaxBytes = maxBytes - extUtf8Len
                    if (nameMaxBytes <= 0) return ext.slice(0, maxBytes)

                    const utf8 = unescape(encodeURIComponent(name))
                    let truncated = utf8.slice(0, nameMaxBytes)
                    let safeName = ""
                    while (truncated.length > 0) {
                        try {
                            safeName = decodeURIComponent(escape(truncated))
                            break
                        } catch (e) {
                            truncated = truncated.slice(0, -1)
                        }
                    }

                    return safeName + ext
                }

                function getSuffixStartIndex(str) {
                    // 处理 .tar.gz 这类情况优先
                    const doubleExts = [".tar.gz", ".tar.xz", ".tar.bz2", ".tar.zst", ".tar.lz"]
                    for (let ext of doubleExts) {
                        if (str.endsWith(ext)) {
                            return str.length - ext.length
                        }
                    }

                    const lastDot = str.lastIndexOf(".")
                    if (lastDot > 0 && !str.startsWith(".")) {
                        return lastDot
                    }

                    return str.length // 无后缀情况
                }

                function selectText() {
                    var chars = Array.from(text); // 正确处理多字节字符
                    var dotIndices = [];

                    var isDesktopFile = text.endsWith(".desktop");
                    if (!isDesktopFile && !isSoftLink && !isDir && text.includes(".") && !text.startsWith(".")) {
                        // 收集所有点号位置（从后向前）
                        for (var i = chars.length - 1; i >= 0; i--) {
                            if (chars[i] === '.') {
                                dotIndices.push(i);
                                if (dotIndices.length >= 2) break; // 只需要最后两个点号
                            }
                        }
                    }
                    // 判断是否为双重扩展名（如.tar.gz）
                    var isDoubleExtension = false;
                    if (dotIndices.length >= 2) {
                        var middleExt = text.substring(dotIndices[1], dotIndices[0]);
                        isDoubleExtension = middleExt === ".tar" || middleExt === ".zip";
                    }

                    // 计算选中结束位置
                    var selectEnd = dotIndices.length > 0
                        ? (isDoubleExtension && dotIndices.length >= 2
                            ? dotIndices[1]  // 对于.tar.gz选中到第一个点号前
                            : dotIndices[0]) // 普通扩展名选中到最后一个点号前
                        : text.length;   // 无扩展名选中全部

                    // 转换为基于代码单元的位置（兼容QML的select方法）
                    var prefix = chars.slice(0, selectEnd).join('');
                    var codeUnitEnd = prefix.length;

                    // 执行选中并定位光标
                    select(0, codeUnitEnd);
                }

                Component.onCompleted: {
                    textEdit.text = root.display
                    forceActiveFocus()
                    selectText()
                    DesktopOrganizationManager.cancelEditRequest.connect(
                                 function () {
                                    updateFileName()
                                 })
                }
            }
        }
    }
    // 当编辑状态改变时，动态加载/卸载编辑控件
    onEditingChanged: {
        editLoader.active = root.editing
    }

    function forceActiveFocus() {
        console.log("edit ", root.display)
        root.editing = true
    }
    // 外部可通过此方法触发焦点
    function updateFileName() {
        if (root.editing && editLoader.active && editLoader.item) {
            let edit = editLoader.item.textEdit
            if (edit) {
                const newName = edit.text
                if (newName === "." || newName === ".." || newName === "") {
                    edit.text = root.display
                    return
                }
                if (newName !== root.display) {
                    editFinish(selectedUris, newName)
                }
                console.log("new file name", newName)
            }
        }
        root.editing = false
    }
}
