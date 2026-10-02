/*
 * Copyright (c) KylinSoft Co., Ltd. 2022.All rights reserved.
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

#include "virtualkeyboardentry/virtualkeyboardtrayicon.h"

#include <QIcon>

VirtualKeyboardTrayIcon::VirtualKeyboardTrayIcon(
    VirtualKeyboardManager &virtualKeyboardManager,
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService)
    : virtualKeyboardManager_(virtualKeyboardManager),
      fcitxVirtualKeyboardService_(fcitxVirtualKeyboardService) {}

void VirtualKeyboardTrayIcon::setContextMenu(QMenu *contextMenu) {
    if (trayIcon_ == nullptr) {
        KVKBD_WARN("trayIcon_ is null!");
        return;
    }

    trayIcon_->setContextMenu(contextMenu);
}

void VirtualKeyboardTrayIcon::hideContextMenu() {
    if (trayIcon_ == nullptr) {
        KVKBD_WARN("trayIcon_ is null!");
        return;
    }

    if (trayIcon_->contextMenu() == nullptr) {
        KVKBD_WARN("contextMenu is null!");
        return;
    }

    trayIcon_->contextMenu()->hide();
}

void VirtualKeyboardTrayIcon::toggleVirtualKeyboard() {
    if (virtualKeyboardManager_.isVirtualKeyboardVisible()) {
        fcitxVirtualKeyboardService_.hideVirtualKeyboard();
    } else {
        fcitxVirtualKeyboardService_.showVirtualKeyboard();
    }
}

void VirtualKeyboardTrayIcon::onTrayIconActivated(
    QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger: {
        toggleVirtualKeyboard();
        break;
    };
    default:
        break;
    }
}

void VirtualKeyboardTrayIcon::initTrayIcon() {
    trayIcon_.reset(new QSystemTrayIcon(this));
    trayIcon_->setIcon(QIcon::fromTheme("ukui-virtual-keyboard-symbolic"));
    trayIcon_->setToolTip(tr("Virtual Keyboard"));
    connect(trayIcon_.get(),
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon_->setVisible(true);
}

void VirtualKeyboardTrayIcon::destroyTrayIcon() {
    if (trayIcon_ == nullptr) {
        return;
    }

    trayIcon_.reset();
}

bool VirtualKeyboardTrayIcon::isInit() const { return trayIcon_ != nullptr; }

void VirtualKeyboardTrayIcon::changeTrayIconVisibility(bool enable) {
    if (trayIcon_ == nullptr) {
        return;
    }

    trayIcon_->setVisible(enable);
}
