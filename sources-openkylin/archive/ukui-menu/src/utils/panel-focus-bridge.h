/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef PANEL_FOCUS_BRIDGE_H
#define PANEL_FOCUS_BRIDGE_H

#include <QObject>
#include <QString>

namespace UkuiMenu {

// 与 ukui-panel 的“外部窗口 Esc 回焦”协议对接的桥接对象。
//
// 职责仅限于：在开始菜单窗口隐藏时，按 Esc/非 Esc 路径决定
// 调用 panel 的 restoreFocusByExternalKey 还是 clearFocusReturnByExternalKey。
// 锚点的注册由 ukui-panel 侧的开始菜单插件（starter）在打开本窗口前完成，
// 本类不持有任何 panel 侧状态，只负责“关闭时通知”。
class PanelFocusBridge : public QObject
{
    Q_OBJECT
public:
    static PanelFocusBridge *instance();
    PanelFocusBridge(const PanelFocusBridge &) = delete;
    PanelFocusBridge &operator=(const PanelFocusBridge &) = delete;

    // 标记“下一次窗口隐藏是由 Esc 触发的”，隐藏时走 restore 而不是 clear。
    Q_INVOKABLE void scheduleRestorePanelFocusOnHide();

public Q_SLOTS:
    // 窗口 visibleChanged 且变为隐藏时由 UkuiMenuApplication 调用。
    // 这里只做关闭路径分流：Esc 关闭尝试回焦，其他关闭清理 panel 侧锚点。
    void handleMenuHidden();

private:
    explicit PanelFocusBridge(QObject *parent = nullptr);
    // 异步请求 panel 执行 restoreFocusByExternalKey。失败时回退为 clear，
    // 以避免 panel 侧遗留旧锚点。
    void requestPanelFocusRestore();
    // 非 Esc 路径只需要异步清理 panel 侧锚点，不等待返回。
    void clearPanelFocusReturn();

    // 记录“下一次隐藏是否来自 Esc 关闭”。
    bool m_restorePanelFocusOnNextHide = false;
};

} // namespace UkuiMenu

#endif // PANEL_FOCUS_BRIDGE_H
