/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// 控制器基类
class Controller {
    constructor(model, view, manager, acceptedTypeSet) {
        this.model = model;
        this.view = view;
        this.manager = manager;
        this.acceptedTypeSet = acceptedTypeSet;
    }

    // 关闭多任务视图
    closeMultitaskView() {
        this.manager.close();
    }

    // 窗口点击事件处理
    onWindowClicked(windowId) {}

    // 删除预览窗口
    removePreviewWindow(windowId) {}

    // 设置默认态
    setDefaultStatus() {
        this.view.setDefaultStatus();
    }

    // 设置 Hover 态
    setHoverStatus() {
        this.view.setHoverStatus();
    }

    //设置点击状态
    setClickStatus()
    {
        this.view.setClickStatus();
    }

    // 设置拖拽态
    setDragStatus() {}

    // 设置拖拽接收态
    setReceiveStatus() {}

    // 自己进入目标
    onEnterTarget(srcDesktopIndex, destDesktopIndex) {}

    // 自己离开目标
    onExitTarget() {}

    // 来源进入自己
    onSourceEntered(source) {}

    // 来源离开自己
    onSourceExited(source) {}

    // 自己被释放
    onDropSource(source, srcDesktopIndex, destDesktopIndex) {}

    // 接收来源对象
    acceptSource(source, srcDesktopIndex, destDesktopIndex) {}

    // 拒绝来源对象
    rejectSource(source) {
        source.returnOriginal(source.posX,source.posY);
    }

    // 是否合法来源
    validateSource(source) {
        return this.acceptedTypeSet.indexOf(source.windowType) > -1;
    }

    // 来源释放在自己区域
    onSourceDropped(source, srcDesktopIndex, destDesktopIndex) {
        if (this.validateSource(source)) {
            this.acceptSource(source, srcDesktopIndex, destDesktopIndex);
        } else {
            this.rejectSource(source);
        }
    }
}

class AppController extends Controller {
    constructor(model, view, manager, acceptedTypeSet) {
        super(model, view, manager, acceptedTypeSet);
    }

    // 窗口点击事件处理
    onWindowClicked(windowId) {
        this.model.activateAppWindow(windowId);
        this.manager.close();
    }

    // 删除预览窗口
    removePreviewWindow(windowId) {
        if (this.model.hasModalChildWindow(windowId)) {
            this.onWindowClicked(windowId);
        } else {
            this.model.removeAppWindow(windowId);
        }
    }

    // 自己被释放
    onDropSource(source, srcDesktop, destDesktop) {
        this.model.moveAppWindowToDesktop(source.wid, srcDesktop, destDesktop);
    }

    // Up与Down键的走焦目标
    keyUpDownSelectWindow(keyStatus,windowPosInfoModel,currentTabIndex)
    {
        var currentRow = 0
        var currentRowWindowNum = 0
        var windowNum = 0

        //计算当前获取键盘焦点的窗口位于第几行
        for(var i = 0; i < windowPosInfoModel.count; i++) {
            windowNum += windowPosInfoModel.get(i).rowWindowCount
            if (currentTabIndex < windowNum) {
                currentRow = i
                currentRowWindowNum = windowPosInfoModel.get(i).rowWindowCount
                break
            }
        }

        //计算当前获取键盘焦点窗口的leftX rightX
        var currentRowModel = windowPosInfoModel.get(currentRow).model
        var currentIndexLeftX = 0
        var currentIndexRightX = 0
        for (var i = 0; i < currentRowWindowNum; i++) {
            if (currentTabIndex === currentRowModel.get(i).index) {
                currentIndexLeftX = currentRowModel.get(i).leftX
                currentIndexRightX = currentRowModel.get(i).rightX
                break
            }
        }

        //判断应该跳转到上/下一行的哪个窗口
        var lastRowModel
        if (currentRow === 0 && keyStatus) {
            lastRowModel = windowPosInfoModel.get(windowPosInfoModel.count - 1).model
        } else if (currentRow === windowPosInfoModel.count - 1 && !keyStatus) {
            lastRowModel = windowPosInfoModel.get(0).model
        } else {
            lastRowModel = keyStatus ? windowPosInfoModel.get(currentRow - 1).model
                                     : windowPosInfoModel.get(currentRow + 1).model
        }

        var lastIndex = 0
        var resultIndex = 0
        var result = 0
        for (var i = 0; i < lastRowModel.count; i++) {
            var temp = 0
            if (lastRowModel.get(i).rightX < currentIndexLeftX) {
                lastIndex = i
                continue;
            }
            if (lastRowModel.get(i).leftX > currentIndexRightX) {
                break;
            }
            if (lastRowModel.get(i).leftX <= currentIndexLeftX) {
                if (lastRowModel.get(i).rightX <= currentIndexRightX) {
                    temp = lastRowModel.get(i).rightX - currentIndexLeftX
                } else {
                    temp = currentIndexRightX - currentIndexLeftX
                }
            } else  {
                if (lastRowModel.get(i).rightX <= currentIndexRightX) {
                    temp = lastRowModel.get(i).rightX -  lastRowModel.get(i).leftX
                } else {
                    temp = currentIndexRightX - lastRowModel.get(i).leftX
                }
            }

            if (temp > result) {
                result = temp;
                lastIndex = i
            }
        }
        resultIndex = lastRowModel.get(lastIndex).index
        return resultIndex
    }

    // PageUp与PageDown键的走焦目标
    keyPageUpDownSelectWindow(keyStatus,windowPosInfoModel,currentTabIndex)
    {
        var currentRow = 0
        var lastRowWindowNum = 0
        var currentRowWindowNum = 0
        var windowNum = 0

        for(var i = 0; i < windowPosInfoModel.count; i++)
        {
            windowNum += windowPosInfoModel.get(i).rowWindowCount
            if (currentTabIndex < windowNum)
            {
                if (i === 0 && keyStatus)
                {
                    return currentTabIndex
                }
                else if (i === windowPosInfoModel.count - 1 && !keyStatus)
                {
                    return currentTabIndex
                }
                currentRow = i
                currentRowWindowNum = windowPosInfoModel.get(i).rowWindowCount
                lastRowWindowNum = keyStatus ? windowPosInfoModel.get(i - 1).rowWindowCount
                                             : windowPosInfoModel.get(i + 1).rowWindowCount
                break
            }
        }

        //判断应该跳转到上一行的哪个窗口
        if (keyStatus) {
            var lastRowModel = windowPosInfoModel.get(currentRow - 1).model
            var resultIndex = lastRowModel.get(0).index
            return resultIndex
        } else{
            var lastRowModel = windowPosInfoModel.get(currentRow + 1).model
            var resultIndex = lastRowModel.get(lastRowWindowNum - 1).index
            return resultIndex
        }

    }
}

class DesktopController extends Controller {
    constructor(model, view, manager, acceptedTypeSet) {
        super(model, view, manager, acceptedTypeSet);
    }

    // 窗口点击事件处理
    onWindowClicked(desktopIndex) {
        this.manager.close();
    }

    // 删除预览窗口
    removePreviewWindow(desktopIndex) {
        this.model.removeDesktop(desktopIndex);
    }

    // 设置 Hover 态
    setHoverStatus() {
        this.view.setHoverStatus();
        this.model.setCurrentDesktop(this.view.wid);
    }

    setCurrentDesktop(wid) {
        this.model.setCurrentDesktop(wid);
    }

    // 自己进入目标
    onEnterTarget(srcDesktop, destDesktopIndex) {
        // 进入动画靠 虚拟桌面预览区域的 ListView 自带提供，不需调用
        this.model.moveDesktop(srcDesktop, destDesktopIndex);
    }

    // 来源进入自己
    onSourceEntered(source, srcDesktopIndex, destDesktopIndex) {
        source.controller.onEnterTarget(srcDesktopIndex, destDesktopIndex);
    }

    // 来源离开自己
    onSourceExited(source) {
        source.controller.onExitTarget();
        this.setDefaultStatus();
    }

    // 接收来源对象
    acceptSource(source, srcDesktop, destDesktop) {
        this.model.setCurrentDesktop(destDesktop);
        source.controller.onDropSource(source, srcDesktop, destDesktop);
    }
}

class CloseButtonController extends Controller {
    constructor(model, view, manager, acceptedTypeSet) {
        super(model, view, manager, acceptedTypeSet);
    }
}

class CreationController extends Controller {
    constructor(model, view, manager, acceptedTypeSet) {
        super(model, view, manager, acceptedTypeSet);
    }

    // 窗口点击事件处理
    onWindowClicked(/*unused*/) {
        this.model.appendDesktop();
    }

    // 来源进入自己
    onSourceEntered(source) {
        if (!validateSource(source)) {
            return;
        }
        source.controller.onEnterTarget(srcDesktopIndex, destDesktopIndex);
    }

    // 来源离开自己
    onSourceExited(source) {
        if (!validateSource(source)) {
            return;
        }
        source.controller.onExitTarget();
    }

    // 接收来源对象
    acceptSource(source, srcDesktop, destDesktop) {
        source.controller.onDropSource(source, srcDesktop, destDesktop);
        // 设置新建的虚拟桌面为当前虚拟桌面。
    }
}
