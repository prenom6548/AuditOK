import QtQuick 2.0

QtObject {
    property var selectedItems: []
    property var targetView: null

    signal clearAllStatus()

    function getSelectionCount() {
        var count = 0
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndexIndex(i)
            if (item && item.selected) {
                count++
            }
        }
        return count
    }

    function getSelectedItems() {
        getSelections()
        return selectedItems
    }

    function gridPosLesserThan(leftPos, rightPos) {
        if (leftPos.x < rightPos.x)
            return true;

        if (leftPos.x > rightPos.x)
            return false;

        if (leftPos.y < rightPos.y)
            return true;
        return false;
    }

    // 全选函数
    function selectAll() {
        targetView.isMultifileMode = true
        for (var i = 0; i < targetView.count; i++) {
            updateSelection(i, true)
        }
    }

    //　反选函数
    function invertSelections() {
        targetView.isMultifileMode = true
        selectedItems = [];
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i);
            if (!item) return;

            // 更新选中状态
            item.selected = !item.selected;
            if (item.selected) {
                // 如果未包含则添加（避免重复）
                if (selectedItems.indexOf(item) === -1) {
                    selectedItems.push(item);
                }
            }
        }
    }

    function getSelections() {
        var uriList = []
        selectedItems = []
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i)
            if (!item) return

            if (item.selected)　{
                selectedItems.push(item)
                uriList.push(item.uri)
            }
        }
        console.log("get selected uris", uriList)
        return uriList
    }

    function getSelectionsPositions() {
        if (selectedItems.length <= 0)
            return

        var posList = []
        selectedItems = []
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i)
            if (!item) return

            if (item.selected)　{
                console.log("处理选中项:", item.pos)
                selectedItems.push(item)
                posList.push(item.pos)
            }
        }

        console.log("get selected pos", posList)
        return posList
    }

    // 清空选中状态
    function clearSelection() {
        if (selectedItems.length <= 0)
            return
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i);
            if (item) {
                item.selected = false;
            }
        }
        selectedItems = [];
        clearAllStatus();
    }

    // 更新选中状态
    function updateSelection(index, selected) {
        var count = targetView.count;
        var item = targetView.itemAtIndex(index);
        if (!item) {
            return;
        }

        // 更新选中状态
        item.selected = selected;
        // 更新选中项数组
         if (selected) {
             // 如果未包含则添加（避免重复）
             if (selectedItems.indexOf(item) === -1) {
                 selectedItems.push(item);
             }
         } else {
             // ✅ 找到 item 在 selectedItems 中的位置再删除
             const selectedIndex = selectedItems.indexOf(item);
             if (selectedIndex !== -1) {
                 selectedItems.splice(selectedIndex, 1);
             }
         }
    }

    // 仅仅选中当前项
    function selectCurrent(index) {
        if (index < 0 || targetView.count === 0) {
            return
        }
        var item = targetView.itemAtIndex(index);
        if (!item) return;

        clearSelection()
        // 更新选中状态
        item.selected = true;
        selectedItems.push(item);
        console.log("select current uri:", item.uri,item.selected)
    }

    function selectHome() {
        if (targetView.count === 0)
            return
        var homePos = Qt.point(0, 0);
        var homeIndex = -1;
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i);
            if (item) {
                if (!gridPosLesserThan(homePos, item.pos)) {
                    homeIndex = item.itemIndex;
                    homePos = item.pos;
                }
            }
        }
        console.log("select home:", homePos)
        selectCurrent(homeIndex)
    }

    function selectEnd() {
        if (targetView.count === 0)
            return
        var endPos = Qt.point(-1,-1);
        var endIndex = -1;
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i);
            if (item) {
                if (gridPosLesserThan(endPos, item.pos)) {
                    endIndex = item.itemIndex;
                    endPos = item.pos;
                }
            }
        }
        console.log("select end:", endPos)
        selectCurrent(endIndex)
    }

    function selectPrevious() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAtIndex(index);
        if (!item) return;

        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var y = item.pos.y - 1;
            var upIndex = indexByPos(item.pos.x, y)
            selectCurrent(upIndex)
        }
    }

    function selectNext() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAtIndex(index);
        if (!item) return;

        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var y = item.pos.y + 1;
            var nextIndex = indexByPos(item.pos.x, y)
            selectCurrent(nextIndex)
        }
    }

    function selectLeft() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAtIndex(index);
        if (!item) return;

        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var x = item.pos.x - 1;
            var LeftIndex = indexByPos(x, item.pos.y)
            selectCurrent(LeftIndex)
        }
    }

    function selectRight() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAtIndex(index);
        if (!item) return;
        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var x = item.pos.x + 1;
            var rightIndex = indexByPos(x, item.pos.y)
            selectCurrent(rightIndex)
        }
    }

    //选中两个项之间的所有项
    function selectBetween(startIndex, endIndex) {
        var startItem = targetView.itemAtIndex(startIndex);
        var endItem = targetView.itemAtIndex(endIndex);
        if (startItem && endItem) {
            selectedItems = []
            // if (!gridPosLesserThan(startPos, endPos)) {
            //     let tmp = endPos
            //     endPos = startPos
            //     startPos = tmp
            // }
            // var startCol = startPos.x
            // var endCol = endPos.x
            // var startRow = startPos.y
            // var endRow = endPos.y
            var maxindex = startIndex > endIndex ? startIndex : endIndex
            var minindex = startIndex < endIndex ? startIndex : endIndex
            for (var i = minindex; i <=maxindex; i++) {
                var item = targetView.itemAtIndex(i);
                if (item) {
                    if (selectedItems.indexOf(item) === -1) {
                        selectedItems.push(item);
                    }
                    // 更新选中状态
                    item.selected = true;
                }
            }
        }
    }

    function firstIndex() {
        if (selectedItems.length === 0) return -1;

        // 方法1：直接取第一个元素
        const firstItem = selectedItems[0];
        if (firstItem) {
            var viewItem = targetView.itemAtIndex(firstItem.itemIndex)
            if (firstItem.uri === viewItem.uri) {
                console.log("First item index:", firstItem.itemIndex, firstItem.pos);
                return firstItem.itemIndex;
            } else {
                selectedItems = []
            }
        }
        return -1;
    }

    function indexByPos(x, y) {
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAtIndex(i);
            if (item && item.pos.x === x && item.pos.y === y) {
                return i;
            }
        }
        return -1;
    }
}
