import QtQuick 2.0

QtObject {
    id: selectionStore

    // 选中状态以 URI 为主键，不保存 GridView/ListView delegate。
    // 因此 delegate 被懒加载回收后，选中状态仍然存在。
    property var selectedUris: []
    property var selectedLookup: ({})
    property var targetView: null
    property var multifileOwner: null

    signal clearAllStatus()

    function modelObject() {
        return targetView ? targetView.model : null
    }

    function uriAt(itemIndex) {
        var model = modelObject()
        return model && itemIndex >= 0 && itemIndex < targetView.count
                ? model.uriAt(itemIndex) : ""
    }

    function indexForUri(uri) {
        var model = modelObject()
        return model ? model.indexOfUri(uri) : -1
    }

    function isSelected(uri) {
        return uri !== "" && selectedLookup["$" + uri] === true
    }

    function replaceSelection(uris) {
        var lookup = {}
        for (var i = 0; i < uris.length; ++i) {
            lookup["$" + uris[i]] = true
        }
        selectedLookup = lookup
        selectedUris = uris
    }

    function getSelectionCount() {
        return selectedUris.length
    }

    function getSelections() {
        return selectedUris.slice(0)
    }

    // 只扫描一次模型，避免“全选后拖拽”对每个 URI 再做一次全表查找。
    function selectedRows() {
        var model = modelObject()
        var rows = []
        if (!model || !targetView || selectedUris.length === 0) {
            return rows
        }

        for (var row = 0; row < targetView.count; ++row) {
            var uri = model.uriAt(row)
            if (selectedLookup["$" + uri] === true) {
                rows.push(row)
            }
        }
        return rows
    }

    // 生成拖拽预览所需的数据 map，不依赖 delegate 是否已实例化。
    function getSelectedItems() {
        var model = modelObject()
        var items = []
        if (!model || !targetView) {
            return items
        }

        var rows = selectedRows()
        for (var i = 0; i < rows.length; ++i) {
            var itemIndex = rows[i]
            var itemData = model.itemDataAt(itemIndex)
            var dragPos = targetView.dragPositionForIndex(itemIndex)
            items.push({
                "x": dragPos.x,
                "y": dragPos.y,
                "displayname": itemData.display,
                "uri": itemData.uri,
                "isfile": !itemData.isDesktop && !itemData.isDir,
                "size": itemData.fileSize,
                "type": itemData.fileType
            })
        }
        return items
    }

    function getSelectionsPositions() {
        var positions = []
        if (!targetView) {
            return positions
        }
        var rows = selectedRows()
        for (var i = 0; i < rows.length; ++i) {
            positions.push(targetView.positionForIndex(rows[i]))
        }
        return positions
    }

    function selectAll() {
        var model = modelObject()
        if (!model) {
            return
        }
        if (multifileOwner) {
            multifileOwner.ismultifilemode = true
        }
        var all = model.allUris()
        var uris = []
        for (var i = 0; i < all.length; ++i) {
            uris.push(all[i])
        }
        replaceSelection(uris)
    }

    function invertSelections() {
        var model = modelObject()
        if (!model) {
            return
        }
        if (multifileOwner) {
            multifileOwner.ismultifilemode = true
        }
        var all = model.allUris()
        var inverted = []
        for (var i = 0; i < all.length; ++i) {
            if (selectedLookup["$" + all[i]] !== true) {
                inverted.push(all[i])
            }
        }
        replaceSelection(inverted)
    }

    function clearSelection() {
        if (selectedUris.length === 0) {
            return
        }
        replaceSelection([])
        clearAllStatus()
    }

    function pruneSelection() {
        var model = modelObject()
        if (!model || selectedUris.length === 0) {
            return
        }
        var all = model.allUris()
        var existing = {}
        for (var i = 0; i < all.length; ++i) {
            existing["$" + all[i]] = true
        }
        var valid = []
        for (var j = 0; j < selectedUris.length; ++j) {
            if (existing["$" + selectedUris[j]] === true) {
                valid.push(selectedUris[j])
            }
        }
        if (valid.length !== selectedUris.length) {
            replaceSelection(valid)
        }
    }

    function updateSelection(itemIndex, selected) {
        var uri = uriAt(itemIndex)
        if (uri === "") {
            return
        }

        var next = selectedUris.slice(0)
        var selectedIndex = next.indexOf(uri)
        if (selected && selectedIndex === -1) {
            next.push(uri)
        } else if (!selected && selectedIndex !== -1) {
            next.splice(selectedIndex, 1)
        } else {
            return
        }
        replaceSelection(next)
    }

    function selectCurrent(itemIndex) {
        var uri = uriAt(itemIndex)
        if (uri === "") {
            return
        }
        replaceSelection([uri])
    }

    function selectHome() {
        if (targetView && targetView.count > 0) {
            selectCurrent(0)
        }
    }

    function selectEnd() {
        if (targetView && targetView.count > 0) {
            selectCurrent(targetView.count - 1)
        }
    }

    function firstIndex() {
        return selectedUris.length > 0 ? indexForUri(selectedUris[0]) : -1
    }

    function indexByPos(row, column) {
        return targetView ? targetView.indexForPosition(row, column) : -1
    }

    function selectPrevious() {
        var itemIndex = firstIndex()
        if (itemIndex < 0) {
            selectCurrent(0)
            return
        }
        var pos = targetView.positionForIndex(itemIndex)
        selectCurrent(indexByPos(pos.x, pos.y - 1))
    }

    function selectNext() {
        var itemIndex = firstIndex()
        if (itemIndex < 0) {
            selectCurrent(0)
            return
        }
        var pos = targetView.positionForIndex(itemIndex)
        selectCurrent(indexByPos(pos.x, pos.y + 1))
    }

    function selectLeft() {
        var itemIndex = firstIndex()
        if (itemIndex < 0) {
            selectCurrent(0)
            return
        }
        var pos = targetView.positionForIndex(itemIndex)
        selectCurrent(indexByPos(pos.x - 1, pos.y))
    }

    function selectRight() {
        var itemIndex = firstIndex()
        if (itemIndex < 0) {
            selectCurrent(0)
            return
        }
        var pos = targetView.positionForIndex(itemIndex)
        selectCurrent(indexByPos(pos.x + 1, pos.y))
    }

    function selectBetween(startIndex, endIndex) {
        if (!targetView || startIndex < 0 || endIndex < 0) {
            return
        }
        var first = Math.min(startIndex, endIndex)
        var last = Math.max(startIndex, endIndex)
        var uris = []
        for (var i = first; i <= last; ++i) {
            var uri = uriAt(i)
            if (uri !== "") {
                uris.push(uri)
            }
        }
        replaceSelection(uris)
    }
}
