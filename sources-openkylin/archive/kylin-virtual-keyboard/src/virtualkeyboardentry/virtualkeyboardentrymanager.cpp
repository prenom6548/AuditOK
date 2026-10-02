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

#include "virtualkeyboardentry/virtualkeyboardentrymanager.h"

#include <QAction>
#include <QIcon>

#include "geometrymanager/geometrymanager.h"
#include "virtualkeyboardentry/floatbuttonstrategy.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

static const QString floatButtonGroup = "floatButton";
static const QString floatButtonEnabledKey = "floatButtonEnabled";

VirtualKeyboardEntryManager::VirtualKeyboardEntryManager(
    VirtualKeyboardManager &virtualKeyboardManager,
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService)
    : virtualKeyboardManager_(virtualKeyboardManager),
      floatButtonManager_(new FloatButtonManager(
          virtualKeyboardManager, fcitxVirtualKeyboardService,
          floatButtonSettings_, virtualKeyboardManager.themeWatcher())),
      trayIconEntry_(new VirtualKeyboardTrayIcon(virtualKeyboardManager_,
                                                 fcitxVirtualKeyboardService)),
      keyboardServiceProxy_(new KeyboardServiceProxy()) {
    initTrayIconStrategy();

    moveValueFromLocalSettings();

    connectSignals();

    floatButtonManager_->updateFloatButtonEnabled(
        VirtualKeyboardSettings::getInstance().isFloatButtonEnabled());
}

VirtualKeyboardEntryManager::~VirtualKeyboardEntryManager() = default;

void VirtualKeyboardEntryManager::initTrayIconStrategy() {
    alwaysShowStrategy_ = std::make_shared<AlwaysShowStrategy>();
    neverShowStrategy_ = std::make_shared<NeverShowStrategy>();
    keyboardStatusStrategy_ = std::make_shared<KeyboardStatusStrategy>();

    auto showPolicy = VirtualKeyboardSettings::getInstance().trayIconShow();
    KVKBD_INFO("trayIcon showPolicy:{}", showPolicy.toStdString());

    if (showPolicy == "NeverShow") {
        updateStrategy(neverShowStrategy_);
    } else if (showPolicy == "AlwaysShow") {
        updateStrategy(alwaysShowStrategy_);
    } else if (showPolicy == "ShowWhenKeyboardIsConnected") {
        updateStrategy(keyboardStatusStrategy_);
    }
}

void VirtualKeyboardEntryManager::initFloatButtonContextMenuAndAction() {
    if (floatButtonContextMenu_ != nullptr &&
        floatButtonContextMenuAction_ != nullptr) {
        return;
    }
    floatButtonContextMenu_.reset(new QMenu());
    floatButtonContextMenuAction_.reset(new QAction());

    floatButtonContextMenu_->addAction(floatButtonContextMenuAction_.get());

    trayIconEntry_->setContextMenu(floatButtonContextMenu_.get());

    connect(floatButtonContextMenu_.get(), &QMenu::aboutToShow, this,
            [this]() { virtualKeyboardManager_.hide(); });

    connect(floatButtonContextMenuAction_.get(), &QAction::triggered, this,
            [this](bool) {
                if (!actionTriggeredCallback_) {
                    return;
                }

                actionTriggeredCallback_();
            });
}

void VirtualKeyboardEntryManager::connectSignals() {
    connect(floatButtonManager_.get(), &FloatButtonManager::floatButtonEnabled,
            this, [this]() {
                updateFloatButtonContextMenuAction(
                    ":/floatbutton/img/disablefloatbutton.svg",
                    tr("Disable the float button"), []() {
                        VirtualKeyboardSettings::getInstance()
                            .updateFloatButtonAvailability(false);
                    });
            });

    connect(floatButtonManager_.get(), &FloatButtonManager::floatButtonDisabled,
            this, [this]() {
                updateFloatButtonContextMenuAction(
                    ":/floatbutton/img/enablefloatbutton.svg",
                    tr("Enable the float button"), []() {
                        VirtualKeyboardSettings::getInstance()
                            .updateFloatButtonAvailability(true);
                    });
            });

    connect(&virtualKeyboardManager_,
            &VirtualKeyboardManager::virtualKeyboardVisibiltyChanged, this,
            [this](bool visible) {
                if (!visible) {
                    return;
                }

                trayIconEntry_->hideContextMenu();
            });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::requestFloatButtonEnabled, this,
            [this]() { floatButtonManager_->enableFloatButton(); });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::requestFloatButtonDisabled, this,
            [this]() { floatButtonManager_->disableFloatButton(); });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::neverShowTrayIcon, this,
            [this]() { updateStrategy(neverShowStrategy_); });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::alwaysShowTrayIcon, this,
            [this]() { updateStrategy(alwaysShowStrategy_); });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::showTrayIconWhenKeyboardisConnected, this,
            [this]() { updateStrategy(keyboardStatusStrategy_); });

    connect(keyboardServiceProxy_.get(),
            &KeyboardServiceProxy::kbdStatusChanged, this,
            [this]() { updateTrayVisibility(); });
}

void VirtualKeyboardEntryManager::updateFloatButtonContextMenuAction(
    const QString &icon, const QString &text,
    ActionTriggeredCallback callback) {
    if (floatButtonContextMenuAction_ == nullptr) {
        return;
    }

    floatButtonContextMenuAction_->setIcon(QIcon(icon));
    floatButtonContextMenuAction_->setText(text);

    actionTriggeredCallback_ = std::move(callback);
}

void VirtualKeyboardEntryManager::moveValueFromLocalSettings() {
    if (!floatButtonSettings_.contains(floatButtonEnabledKey)) {
        return;
    }

    const bool value =
        floatButtonSettings_.getValue(floatButtonGroup, floatButtonEnabledKey)
            .value<bool>();

    floatButtonSettings_.remove(floatButtonEnabledKey);

    VirtualKeyboardSettings::getInstance().updateFloatButtonAvailability(value);
}

void VirtualKeyboardEntryManager::updateStrategy(
    std::shared_ptr<TrayIconStrategy> newStrategy) {
    // 更新当前策略
    currentTrayIconStrategy_ = newStrategy;

    // 更新托盘图标可用性
    updateTrayExistence();

    // 更新图标可见性
    updateTrayVisibility();
}

void VirtualKeyboardEntryManager::updateTrayExistence() {
    const bool shouldCreate = currentTrayIconStrategy_->shouldCreateTray();

    if (shouldCreate && !trayIconEntry_->isInit()) {
        trayIconEntry_->initTrayIcon();
        syncTrayIconContextMenuAction();
    } else if (!shouldCreate && trayIconEntry_->isInit()) {
        trayIconEntry_->destroyTrayIcon();
    }
}

void VirtualKeyboardEntryManager::updateTrayVisibility() {
    const auto needMonitor = currentTrayIconStrategy_->needsKeyboardMonitor();
    int kbdCount = 0;
    if (needMonitor) {
        kbdCount = getKeyboardCount(true);
    }
    const auto shouldShow = currentTrayIconStrategy_->shouldShowTray(kbdCount);
    KVKBD_INFO("need monitor keyboard:{}, should show trayIcon:{}", needMonitor,
               shouldShow);
    trayIconEntry_->changeTrayIconVisibility(shouldShow);
}

void VirtualKeyboardEntryManager::syncTrayIconContextMenuAction() {
    initFloatButtonContextMenuAndAction();
    if (floatButtonContextMenu_) {
        trayIconEntry_->setContextMenu(floatButtonContextMenu_.get());
    }

    if (VirtualKeyboardSettings::getInstance().isFloatButtonEnabled()) {
        updateFloatButtonContextMenuAction(
            ":/floatbutton/img/disablefloatbutton.svg",
            tr("Disable the float button"), []() {
                VirtualKeyboardSettings::getInstance()
                    .updateFloatButtonAvailability(false);
            });
    } else {
        updateFloatButtonContextMenuAction(
            ":/floatbutton/img/enablefloatbutton.svg",
            tr("Enable the float button"), []() {
                VirtualKeyboardSettings::getInstance()
                    .updateFloatButtonAvailability(true);
            });
    }
}

int VirtualKeyboardEntryManager::getKeyboardCount(const bool &sync) {
    if (!keyboardServiceProxy_) {
        KVKBD_WARN("keyboardServiceProxy_ is null!");
        return 0;
    }
    int currentKbdCount = 0;
    auto kbdNumcall = keyboardServiceProxy_->GetKbdCount();
    auto kbdNumcallwatcher = new QDBusPendingCallWatcher(kbdNumcall, this);
    QObject::connect(kbdNumcallwatcher, &QDBusPendingCallWatcher::finished,
                     this, [&](QDBusPendingCallWatcher *watcher) {
                         watcher->deleteLater();
                         QDBusPendingReply<int> reply = *watcher;
                         if (!reply.isError()) {
                             if (currentKbdCount != reply) {
                                 currentKbdCount = reply;
                             }
                         } else {
                             KVKBD_WARN("getKeyboardCount(),reply error:{}",
                                        reply.error().message().toStdString());
                         }
                     });
    if (sync) {
        kbdNumcallwatcher->waitForFinished();
    }
    KVKBD_INFO("currentKbdCount:{}", currentKbdCount);
    return currentKbdCount;
}
