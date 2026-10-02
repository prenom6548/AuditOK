/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _TRAYICONSTRATEGY_H_
#define _TRAYICONSTRATEGY_H_

class TrayIconStrategy {
public:
    virtual ~TrayIconStrategy() = default;

    // 控制托盘图标的可用性（是否创建）
    virtual bool shouldCreateTray() const = 0;

    // 控制托盘图标的可见性（是否显示）
    virtual bool shouldShowTray(int keyboardCount) const = 0;

    // 是否需要监听键盘状态
    virtual bool needsKeyboardMonitor() const = 0;
};

// 具体策略：始终显示
class AlwaysShowStrategy : public TrayIconStrategy {
public:
    bool shouldCreateTray() const override { return true; }
    bool shouldShowTray(int /*keyboardCount*/) const override { return true; }
    bool needsKeyboardMonitor() const override { return false; }
};

// 具体策略：始终不显示（不创建）
class NeverShowStrategy : public TrayIconStrategy {
public:
    bool shouldCreateTray() const override { return false; }
    bool shouldShowTray(int /*keyboardCount*/) const override { return false; }
    bool needsKeyboardMonitor() const override { return false; }
};

// 具体策略：根据键盘状态来显示
class KeyboardStatusStrategy : public TrayIconStrategy {
public:
    bool shouldCreateTray() const override { return true; }
    bool shouldShowTray(int keyboardCount) const override {
        return keyboardCount == 0;
    }
    bool needsKeyboardMonitor() const override { return true; }
};
#endif