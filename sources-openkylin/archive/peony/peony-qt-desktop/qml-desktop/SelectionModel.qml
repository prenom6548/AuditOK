import QtQuick 2.0

QtObject {
    property var selectedItems: []
    property var targetView: null
    // DesktopView 注入；避免本文件 import C++ 单例导致 QML 加载失败。
    property var a11yNotifier: null
    // Tab/Shift+Tab 线性导航方向："row"=行主序（向右再向下），"column"=列主序（向下再向右）
    property string linearNavDirection: "row"

    signal clearAllStatus()

    function notifyA11yIconState(item) {
        if (!item || !a11yNotifier) {
            return
        }
        try {
            Qt.callLater(function() {
                if (item) {
                    a11yNotifier(item)
                }
            })
        } catch (e) {
            console.warn("SelectionModel: a11y notify failed:", e)
        }
    }

    function getSelectionCount() {
        var count = 0
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAt(i)
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

    // 行主序比较：先 y 后 x（从左到右、从上到下）
    function rowMajorLessThan(leftPos, rightPos) {
        if (leftPos.y < rightPos.y)
            return true;

        if (leftPos.y > rightPos.y)
            return false;

        if (leftPos.x < rightPos.x)
            return true;
        return false;
    }

    // 根据 linearNavDirection 返回统一的严格小于比较
    function linearLessThan(leftPos, rightPos) {
        if (linearNavDirection === "column")
            return gridPosLesserThan(leftPos, rightPos)
        return rowMajorLessThan(leftPos, rightPos)
    }

    // 根据 linearNavDirection 判断 pos 是否为 cur 的“下一个”候选位置
    function isNextCandidate(pos, cur) {
        if (linearNavDirection === "column")
            return (pos.x === cur.x && pos.y > cur.y) || (pos.x > cur.x)
        return (pos.y === cur.y && pos.x > cur.x) || (pos.y > cur.y)
    }

    // 根据 linearNavDirection 判断 pos 是否为 cur 的“上一个”候选位置
    function isPrevCandidate(pos, cur) {
        if (linearNavDirection === "column")
            return (pos.x === cur.x && pos.y < cur.y) || (pos.x < cur.x)
        return (pos.y === cur.y && pos.x < cur.x) || (pos.y < cur.y)
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
            var item = targetView.itemAt(i);
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
            var item = targetView.itemAt(i)
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
            var item = targetView.itemAt(i)
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
            var item = targetView.itemAt(i);
            if (item) {
                if (item.selected) {
                    item.selected = false;
                    notifyA11yIconState(item);
                }
            }
        }
        selectedItems = [];
        clearAllStatus();
    }

    function setSelection(uris) {
        if (uris.length <= 0)
            return

        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAt(i);
            if (item && uris.includes(item.uri)) {
                item.selected = true;
                selectedItems.push(item)
                notifyA11yIconState(item);
            }
        }
    }

    // 更新选中状态
    function updateSelection(index, selected) {
        var item = targetView.itemAt(index);
        if (!item) return;

        // 更新选中状态
        item.selected = selected;
        notifyA11yIconState(item);
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
        var item = targetView.itemAt(index);
        if (!item) return;

        clearSelection()
        // 更新选中状态
        item.selected = true;
        selectedItems.push(item);
        notifyA11yIconState(item);
        console.log("select current uri:", item.uri,item.selected)
    }

    function selectHome() {
        if (targetView.count === 0)
            return
        var homePos = Qt.point(0, 0);
        var homeIndex = -1;
        for (var i = 0; i < targetView.count; i++) {
            var item = targetView.itemAt(i);
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
            var item = targetView.itemAt(i);
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
        var item = targetView.itemAt(index);
        if (!item) return;

        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var idx = findUpNeighbor(item.pos)
            if (idx >= 0) selectCurrent(idx)
        }
    }

    function selectNext() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAt(index);
        if (!item) return;

        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var idx = findDownNeighbor(item.pos)
            if (idx >= 0) selectCurrent(idx)
        }
    }

    // Tab 线性前进：按 linearNavDirection 找到下一个图标，
    // 行末/列末则换到下一行/列，末尾则循环回首个
    function selectNextLinear() {
        if (targetView.count === 0)
            return
        var index = selectionModel.firstIndex()
        if (index < 0) {
            selectHome()
            return
        }
        var curItem = targetView.itemAt(index)
        if (!curItem) return

        var next = null, nextIdx = -1
        var wrap = null, wrapIdx = -1
        for (var i = 0; i < targetView.count; i++) {
            var it = targetView.itemAt(i)
            if (!it) continue
            // 循环回卷：取当前方向下最小的位置
            if (wrap === null || linearLessThan(it.pos, wrap)) {
                wrap = it.pos
                wrapIdx = i
            }
            if (isNextCandidate(it.pos, curItem.pos)) {
                if (next === null || linearLessThan(it.pos, next)) {
                    next = it.pos
                    nextIdx = i
                }
            }
        }
        if (nextIdx >= 0) {
            selectCurrent(nextIdx)
        } else if (wrapIdx >= 0) {
            selectCurrent(wrapIdx)
        }
    }

    // Shift+Tab 线性后退：按 linearNavDirection 找到上一个图标，
    // 行首/列首则换到上一行/列，首个则循环回末尾
    function selectPreviousLinear() {
        if (targetView.count === 0)
            return
        var index = selectionModel.firstIndex()
        if (index < 0) {
            selectEnd()
            return
        }
        var curItem = targetView.itemAt(index)
        if (!curItem) return

        var prev = null, prevIdx = -1
        var wrap = null, wrapIdx = -1
        for (var i = 0; i < targetView.count; i++) {
            var it = targetView.itemAt(i)
            if (!it) continue
            // 循环回卷：取当前方向下最大的位置
            if (wrap === null || !linearLessThan(it.pos, wrap)) {
                wrap = it.pos
                wrapIdx = i
            }
            if (isPrevCandidate(it.pos, curItem.pos)) {
                if (prev === null || !linearLessThan(it.pos, prev)) {
                    prev = it.pos
                    prevIdx = i
                }
            }
        }
        if (prevIdx >= 0) {
            selectCurrent(prevIdx)
        } else if (wrapIdx >= 0) {
            selectCurrent(wrapIdx)
        }
    }

    function selectLeft() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAt(index);
        if (!item) return;

        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var idx = findLeftNeighbor(item.pos)
            if (idx >= 0) selectCurrent(idx)
        }
    }

    function selectRight() {
        var index = selectionModel.firstIndex();
        if (index < 0) {
            selectCurrent(0);
            return
        }
        var item = targetView.itemAt(index);
        if (!item) return;
        if (selectedItems.length <= 0) {
            selectCurrent(0);
        } else {
            var idx = findRightNeighbor(item.pos)
            if (idx >= 0) selectCurrent(idx)
        }
    }

    //选中两个项之间的所有项
    function selectBetween(startIndex, endIndex) {
        var startItem = targetView.itemAt(startIndex);
        var endItem = targetView.itemAt(endIndex);
        if (startItem && endItem) {
            selectedItems = [startItem]
            var startPos = startItem.pos
            var endPos = endItem.pos
            if (!gridPosLesserThan(startPos, endPos)) {
                let tmp = endPos
                endPos = startPos
                startPos = tmp
            }
            var startCol = startPos.x
            var endCol = endPos.x
            var startRow = startPos.y
            var endRow = endPos.y

            console.log("start pos and end pos when shift is pressed :", startCol, endCol, startRow, endRow);
            for (var i = 0; i < targetView.count; i++) {
                var item = targetView.itemAt(i);
                if (item) {
                    var isSelected = (item.pos.x > startCol && item.pos.x < endCol) ||
                            (endCol === startCol && item.pos.x === startCol && item.pos.y >= startRow && item.pos.y <= endRow) ||
                            ((startCol !== endCol) &&
                            ((item.pos.x === startCol && item.pos.y >= startRow) ||
                            (item.pos.x === endCol && item.pos.y <= endRow)))

                    if (isSelected) {
                         // 如果未包含则添加（避免重复）
                        if (selectedItems.indexOf(item) === -1) {
                            selectedItems.push(item);
                        }
                    }
                    // 更新选中状态
                    item.selected = isSelected;
                    notifyA11yIconState(item);
                }
            }
        }
    }

    function firstIndex() {
        if (selectedItems.length === 0) return -1;

        // 方法1：直接取第一个元素
        const firstItem = selectedItems[0];
        if (firstItem) {
            var viewItem = targetView.itemAt(firstItem.itemIndex)
            // viewItem 可能为 null（图标被删除导致 itemIndex 越界、或视图未就绪），
            // 此时访问 viewItem.uri 会抛 TypeError，且本函数被 Tab/方向键路径无 try/catch 调用，
            // 会导致键盘导航崩溃。越界时按"选中态失效"处理：清空并回退到 -1（调用方会走 selectHome 兜底）。
            if (viewItem && firstItem.uri === viewItem.uri) {
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
            var item = targetView.itemAt(i);
            if (item && item.pos.x === x && item.pos.y === y) {
                return i;
            }
        }
        return -1;
    }

    // 向左：同一行(y 相同)中 x < cur.x 且 x 最大的图标
    function findLeftNeighbor(curPos) {
        var best = null, bestIdx = -1
        for (var i = 0; i < targetView.count; i++) {
            var it = targetView.itemAt(i)
            if (!it) continue
            if (it.pos.y === curPos.y && it.pos.x < curPos.x) {
                if (best === null || it.pos.x > best) {
                    best = it.pos.x
                    bestIdx = i
                }
            }
        }
        return bestIdx
    }

    // 向右：同一行中 x > cur.x 且 x 最小的图标
    function findRightNeighbor(curPos) {
        var best = null, bestIdx = -1
        for (var i = 0; i < targetView.count; i++) {
            var it = targetView.itemAt(i)
            if (!it) continue
            if (it.pos.y === curPos.y && it.pos.x > curPos.x) {
                if (best === null || it.pos.x < best) {
                    best = it.pos.x
                    bestIdx = i
                }
            }
        }
        return bestIdx
    }

    // 向上：同一列(x 相同)中 y < cur.y 且 y 最大的图标
    function findUpNeighbor(curPos) {
        var best = null, bestIdx = -1
        for (var i = 0; i < targetView.count; i++) {
            var it = targetView.itemAt(i)
            if (!it) continue
            if (it.pos.x === curPos.x && it.pos.y < curPos.y) {
                if (best === null || it.pos.y > best) {
                    best = it.pos.y
                    bestIdx = i
                }
            }
        }
        return bestIdx
    }

    // 向下：同一列中 y > cur.y 且 y 最小的图标
    function findDownNeighbor(curPos) {
        var best = null, bestIdx = -1
        for (var i = 0; i < targetView.count; i++) {
            var it = targetView.itemAt(i)
            if (!it) continue
            if (it.pos.x === curPos.x && it.pos.y > curPos.y) {
                if (best === null || it.pos.y < best) {
                    best = it.pos.y
                    bestIdx = i
                }
            }
        }
        return bestIdx
    }
}
