import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
Item {
    id: shortCutManger

    // 复制
    Shortcut {
        sequences: [StandardKey.Copy]
        onActivated: {
            controller.fileCopy(selectionModel.getSelections())
        }
    }
    // 剪切
    Shortcut {
        sequence:  "Ctrl+X"
        onActivated: controller.fileCut(selectionModel.getSelections())
    }
    // 粘贴
    Shortcut {
        sequences: [StandardKey.Paste]
        onActivated: controller.filePaste(selectionModel.getSelections())
    }

    // 撤销
    Shortcut {
        sequences: [StandardKey.Undo]
        onActivated: controller.undo()
    }

    // 重做
    Shortcut {
        sequences: [StandardKey.Redo]
        onActivated: controller.redo()
    }
    // 放大
    Shortcut {
        sequences: [StandardKey.ZoomIn,"Ctrl+="]
        onActivated: controller.zoomIn()
    }

    // 缩小
    Shortcut {
        sequences: [StandardKey.ZoomOut]
        onActivated: controller.zoomOut()
    }

    // 全选
    Shortcut {
        sequence: StandardKey.SelectAll
        onActivated: {
            if (!controller.enableShortcutKeys()) {
                return
            }
            selectionModel.selectAll();
        }
    }

    // Delete 键（带Shift修饰）
    Shortcut {
        sequence: "Shift+Delete"
        onActivated: {
            console.log("press Shift+Delete")
            controller.remove(selectionModel.getSelections())
        }
    }

    // Ctrl+D
    Shortcut {
        sequences: ["Ctrl+D", "Delete"]
        context: Qt.WindowShortcut
        onActivated: {
            controller.deleteToTrash(selectionModel.getSelections())
        }
    }

    // 多键组合（Alt+Enter/Alt+Return）
    Shortcut {
        sequences: ["Alt+Enter", "Alt+Return"]
        context: Qt.WindowShortcut
        onActivated: controller.showProperties(selectionModel.getSelections())
    }

    // 三键组合（Ctrl+Shift+N）
    Shortcut {
        sequence: "Ctrl+Shift+N"
        context: Qt.WindowShortcut
        onActivated: controller.newFolder()
    }

    // F1
    Shortcut {
        sequence: "F1"
        onActivated: {
            console.log("press F1")
            controller.help()
        }
    }

    // F5
    Shortcut {
        sequences: ["F5", "Ctrl+R"]
        onActivated: controller.refresh()
    }

    // Esc
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (desktopIconView.currentEditIndex !== -1) {
                var editItem = desktopIconView.itemAt(desktopIconView.currentEditIndex)
                if (editItem && editItem.delegateText) {
                    editItem.delegateText.cancelEdit()
                }
            } else {
                controller.changeOpacity(1)
                controller.clearClipboard()
            }
        }
    }

    // 混合组合示例（Alt+E 和 F2）
    Shortcut {
        sequences: ["Alt+E", "F2"]
        onActivated: {
            controller.editUris(selectionModel.getSelections());
        }
    }

    // 混合组合
    Shortcut {
        sequences: ["Ctrl+H"]
        onActivated: controller.setShowHidden()
    }

    Shortcut {
        sequences: ["Ctrl+Shift+L"]
        onActivated: selectionModel.invertSelections()
    }

    Shortcut {
        sequences: ["Ctrl+0"]
        onActivated: controller.recoverNormalLevel()
    }

    Connections {
        target: controller
        function onSelectAll(){
            selectionModel.invertSelections();
        }
    }
    // 信号
    signal customShortcutTriggered(string sequence)
    signal changeOpacity(int opacity)

    // 动态注册快捷键
    function registerShortcut(sequence, callback) {
        Qt.createQmlObject(`
            import QtQuick 2.15
            Shortcut {
                sequence: "${sequence}"
                onActivated: {
                    parent.customShortcutTriggered("${sequence}")
                    if (typeof ${callback} === "function") {
                        ${callback}()
                    }
                }
            }
        `, root)
    }

    // 批量注册快捷键
    function registerShortcuts(shortcutMap) {
        for (var seq in shortcutMap) {
            registerShortcut(seq, shortcutMap[seq])
        }
    }
}
