/*
 * Copyright 2022 KylinSoft Co., Ltd.
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

#ifndef VIRTUALKEYBOARDENTRYMANAGER_H
#define VIRTUALKEYBOARDENTRYMANAGER_H

#include <functional>
#include <memory>

#include <QMenu>
#include <QObject>

#include "ipc/fcitxvirtualkeyboardserviceproxy.h"
#include "ipc/keyboardserviceproxy.h"
#include "localsettings/viewlocalsettings.h"
#include "trayiconstrategy.h"
#include "virtualkeyboard/virtualkeyboardmanager.h"
#include "virtualkeyboardentry/floatbuttonmanager.h"
#include "virtualkeyboardentry/virtualkeyboardtrayicon.h"

/**
 * Used to manage the entrance section of the Kylin Virtual Keyboard.
 * 1. the entry of tray icon.
 * 2. the entry of float button.
 */

class VirtualKeyboardEntryManager : public QObject {
    Q_OBJECT

public:
    VirtualKeyboardEntryManager(
        VirtualKeyboardManager &virtualKeyboardManager,
        const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService);
    ~VirtualKeyboardEntryManager() override;

private:
    using ActionTriggeredCallback = std::function<void()>;

private:
    void connectSignals();

    void moveValueFromLocalSettings();

    void initFloatButtonContextMenuAndAction();
    void updateFloatButtonContextMenuAction(const QString &icon,
                                            const QString &text,
                                            ActionTriggeredCallback callback);

    void initTrayIconStrategy();
    void toggleVirtualKeyboard();
    void updateStrategy(std::shared_ptr<TrayIconStrategy> newStrategy);
    void updateTrayExistence();
    void updateTrayVisibility();
    void syncTrayIconContextMenuAction();
    int getKeyboardCount(const bool &sync);

private:
    VirtualKeyboardManager &virtualKeyboardManager_;

    ViewLocalSettings floatButtonSettings_{"kylinsoft", "kylin float button"};
    std::unique_ptr<FloatButtonManager> floatButtonManager_ = nullptr;

    std::unique_ptr<VirtualKeyboardTrayIcon> trayIconEntry_ = nullptr;

    std::unique_ptr<QMenu> floatButtonContextMenu_ = nullptr;
    std::unique_ptr<QAction> floatButtonContextMenuAction_ = nullptr;
    ActionTriggeredCallback actionTriggeredCallback_;

    // 三种策略实现托盘图标可用性和可见性
    std::shared_ptr<AlwaysShowStrategy> alwaysShowStrategy_ = nullptr;
    std::shared_ptr<NeverShowStrategy> neverShowStrategy_ = nullptr;
    std::shared_ptr<KeyboardStatusStrategy> keyboardStatusStrategy_ = nullptr;
    // 当前策略
    std::shared_ptr<TrayIconStrategy> currentTrayIconStrategy_ = nullptr;
    // 键盘服务代理
    std::unique_ptr<KeyboardServiceProxy> keyboardServiceProxy_ = nullptr;
};

#endif // VIRTUALKEYBOARDENTRYMANAGER_H
