import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

//todolist 暂时用焦点体系根据焦点屏蔽启用全局快捷键，后续需要优化不使用全局快捷键 重写keypress事件实现
Item {
    id: shortCutManger

    // 桌面 FocusScope 持有 activeFocus 时，桌面文件快捷键才生效
    readonly property bool desktopShortcutsEnabled: iconContainer.activeFocus

    // 复制
    Shortcut {
        sequences: [StandardKey.Copy]
        enabled: desktopShortcutsEnabled
        onActivated: {
            controller.fileCopy(selectionModel.getSelections())
        }
    }
    // 剪切
    Shortcut {
        sequence:  "Ctrl+X"
        enabled: desktopShortcutsEnabled
        onActivated: controller.fileCut(selectionModel.getSelections())
    }
    // 粘贴
    Shortcut {
        sequences: [StandardKey.Paste]
        enabled: desktopShortcutsEnabled
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
        enabled: desktopShortcutsEnabled
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
        enabled: desktopShortcutsEnabled
        onActivated: {
            console.log("press Shift+Delete")
            controller.remove(selectionModel.getSelections())
        }
    }

    // Ctrl+D
    Shortcut {
        sequences: ["Ctrl+D", "Delete"]
        context: Qt.WindowShortcut
        enabled: desktopShortcutsEnabled
        onActivated: {
            controller.deleteToTrash(selectionModel.getSelections())
        }
    }

    // 多键组合（Alt+Enter/Alt+Return）
    Shortcut {
        sequences: ["Alt+Enter", "Alt+Return"]
        context: Qt.WindowShortcut
        enabled: desktopShortcutsEnabled
        onActivated: controller.showProperties(selectionModel.getSelections())
    }

    // 三键组合（Ctrl+Shift+N）
    Shortcut {
        sequence: "Ctrl+Shift+N"
        context: Qt.WindowShortcut
        enabled: desktopShortcutsEnabled
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
        enabled: desktopShortcutsEnabled || desktopIconView.currentEditIndex !== -1
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
        enabled: desktopShortcutsEnabled
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
        enabled: desktopShortcutsEnabled
        onActivated: selectionModel.invertSelections()
    }

    Shortcut {
        sequences: ["Ctrl+0"]
        onActivated: controller.recoverNormalLevel()
    }

    Connections {
        target: controller
        function onSelectAll(){
            if (desktopShortcutsEnabled) {
                selectionModel.invertSelections();
            }
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
