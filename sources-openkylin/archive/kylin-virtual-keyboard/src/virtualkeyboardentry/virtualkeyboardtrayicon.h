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

#ifndef VIRTUALKEYBOARDTRAYICON_H
#define VIRTUALKEYBOARDTRAYICON_H

#include <memory>

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

#include "virtualkeyboard/virtualkeyboardmanager.h"
#include "virtualkeyboardentry/fcitxvirtualkeyboardservice.h"

/**
 * Used to manage the entrance section of the Kylin Virtual Keyboard tray icon.
 * 1. click to show/hide kylin virtual keyboard.
 * 2. register tray icon.
 */

class VirtualKeyboardTrayIcon : public QObject {
    Q_OBJECT

public:
    VirtualKeyboardTrayIcon(
        VirtualKeyboardManager &virtualKeyboardManager,
        const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService);

    ~VirtualKeyboardTrayIcon() override = default;

    void setContextMenu(QMenu *contextMenu);
    void hideContextMenu();
    void initTrayIcon();
    void destroyTrayIcon();
    bool isInit() const;
    void changeTrayIconVisibility(bool enable = false);

private:
    void toggleVirtualKeyboard();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    std::unique_ptr<QSystemTrayIcon> trayIcon_ = nullptr;

    VirtualKeyboardManager &virtualKeyboardManager_;
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService_;
};

#endif // VIRTUALKEYBOARDTRAYICON_H
