"""
 Copyright (C) 2026, KylinSoft Co., Ltd.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
from typing import Any, Optional

KEY_DIFF_THRESHOLD = 2

class HandInfo:
    def __init__(self, handmarks = None, handedness = None):
        self._handmarks = handmarks
        self._handedness = handedness

    @property
    def handmarks(self):
        return self._handmarks

    @property
    def handedness(self):
        return self._handedness

class Node:
    def __init__(self, key = None, data:Optional[HandInfo] = None):
        self.prev = None
        self.next = None
        self._key = key
        self._data = data

class LRUCache:
    def __init__(self, capacity = 20):
        if capacity <= 0:
            raise ValueError("capacity should not be negative")

        self._capacity = capacity
        self._head = Node(None, None)
        self._tail = Node(None, None)
        self._head.next = self._tail
        self._tail.prev = self._head
        self._map:dict[Any, Node] = {}

    def get(self, key: Any):
        # node = self._map.get(key, None)
        node = self._find_similar_image(key)
        if node is None:
            return None
        else:
            # 将该节点移至头部
            self._move2head(node)
            return node._data

    def put(self, key: Any, handmarks = None, handedness = None):
        if handmarks is None and handedness is None:
            return
        # 如果键已经存在，删除原先的节点
        if key in self._map:
            self._delete(self._map[key])

        handinfo = HandInfo(handmarks, handedness)
        node = Node(key, handinfo)
        self._map[key] = node

        self._move2head(node)

        if(len(self._map) > self._capacity):
            self._pop_tail()

    def _move2head(self, node:Optional[Node] = None):
        if node is None:
            return

        if node.next is not None and node.prev is not None:
            node.next.prev = node.prev
            node.prev.next = node.next

        node.next = self._head.next
        node.prev = self._head
        node.next.prev = node
        node.prev.next = node

    def _pop_tail(self):
        node = self._tail.prev
        self._tail.prev = node.prev
        node.prev.next = self._tail

        del self._map[node._key]
        node.prev = None
        node.next = None

        return node

    def _delete(self, node:Optional[Node] = None):
        if node is None:
            return

        node.next.prev = node.prev
        node.prev.next = node.next

        del self._map[node._key]
        node.prev = None
        node.next = None

    def _find_similar_image(self, key: Any):
        # 1. 设定阈值，沿链表从头部(最新)向尾部(最旧)遍历，
        #    收集汉明距离满足条件的 node，并记录其在 LRU 中的位置(idx 越小优先级越高)
        container = []
        idx = 0
        node = self._head.next
        while node is not self._tail:
            distance = key - node._key  # ImageHash 的减法即汉明距离
            if distance <= KEY_DIFF_THRESHOLD:
                container.append((distance, idx, node))
            node = node.next
            idx += 1

        # 2. 排序：1.key 差值小优先；2.差值相同时选常用（或者是较新）的帧
        container.sort(key=lambda e: (e[0], e[1]))

        return container[0][2] if container else None

