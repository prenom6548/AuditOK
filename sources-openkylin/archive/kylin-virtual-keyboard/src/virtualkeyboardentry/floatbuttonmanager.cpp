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

#include "virtualkeyboardentry/floatbuttonmanager.h"

#include <QEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QMouseEvent>
#include <QScreen>
#include <QTime>

#include "geometrymanager/floatgeometrymanager.h"
#include "geometrymanager/geometrymanager.h"
#include "screenwatcher.h"
#include "themewatcher.h"
#include "virtualkeyboardentry/floatbuttonstrategy.h"

FloatButtonManager::FloatButtonManager(
    const VirtualKeyboardManager &virtualKeyboardManager,
    const FcitxVirtualKeyboardService &fcitxVirtualKeyboardService,
    LocalSettings &floatButtonSettings, ThemeWatcher &themeWatcher)
    : virtualKeyboardManager_(virtualKeyboardManager),
      fcitxVirtualKeyboardService_(fcitxVirtualKeyboardService),
      floatButtonSettings_(floatButtonSettings), themeWatcher_(themeWatcher) {
    initGeometryManager();

    updateGeometryTimer_.setSingleShot(true);
    updateGeometryTimer_.setInterval(500);
    connect(&updateGeometryTimer_, &QTimer::timeout, this,
            [this]() { geometryManager_->updateGeometry(); });

    initInternalSignalConnections();
    initScreenSignalConnections();
    initGeometryManagerConnections();
}

void FloatButtonManager::enableFloatButton() { setFloatButtonEnabled(true); }

void FloatButtonManager::disableFloatButton() { setFloatButtonEnabled(false); }

void FloatButtonManager::initGeometryManager() {
    geometryManager_.reset(new FloatGeometryManager(
        std::unique_ptr<FloatGeometryManager::Strategy>(
            new FloatButtonStrategy()),
        floatButtonSettings_));
}

void FloatButtonManager::initScreenSignalConnections() {
    ScreenWatcher &screenWatcher = ScreenWatcher::getInstance();

    connect(&screenWatcher, &ScreenWatcher::screensChanged, this,
            &FloatButtonManager::onScreenResolutionChanged);
    connect(&screenWatcher, &ScreenWatcher::screenMarkChanged, this,
            &FloatButtonManager::onMarkedScreenChanged);
}

void FloatButtonManager::initGeometryManagerConnections() {
    connect(geometryManager_.get(), &FloatGeometryManager::viewMoved, this,
            &FloatButtonManager::onViewMoved, Qt::UniqueConnection);
    connect(geometryManager_.get(), &FloatGeometryManager::viewResized, this,
            &FloatButtonManager::onViewResized, Qt::UniqueConnection);
}

void FloatButtonManager::initInternalSignalConnections() {
    connect(this, &FloatButtonManager::floatButtonEnabled, this,
            &FloatButtonManager::initFloatButton);
    connect(this, &FloatButtonManager::floatButtonDisabled, this,
            &FloatButtonManager::destroyFloatButton);
}

void FloatButtonManager::initFloatButton() {
    fcitxVirtualKeyboardService_.hideVirtualKeyboard();

    createFloatButton();

    connectFloatButtonSignals();

    geometryManager_->updateGeometry();
}

void FloatButtonManager::destroyFloatButton() {
    if (floatButton_ == nullptr) {
        return;
    }

    // 销毁之前必须隐藏，否则会导致虚拟键盘进程
    // 直接退出
    floatButton_->hide();

    floatButton_.reset();
}

void FloatButtonManager::onScreenResolutionChanged() {
    if (ScreenWatcher::getInstance().isAnyScreenMarked() == false) {
        KVKBD_DEBUG("marked screen removed, update float button margin ratio");
        geometryManager_->updateViewMarginRatio();
    }
    if (!floatButtonEnabled_) {
        return;
    }
    updateGeometryTimer_.start();
}

void FloatButtonManager::onMarkedScreenChanged() {
    if (floatButton_ != nullptr && floatButton_->isVisible()) {
        return;
    }

    // 悬浮球不可见时，标记屏幕改变是由虚拟键盘视图触发，此时需要更新悬浮球的边缘比例
    KVKBD_DEBUG("marked screen changed, update float button margin ratio");
    geometryManager_->updateViewMarginRatio();
}

void FloatButtonManager::onViewMoved(int x, int y) {
    if (!floatButton_) {
        return;
    }
    floatButton_->move(x, y);
    ScreenWatcher::getInstance().markScreen(QPoint(x, y));
}

void FloatButtonManager::onViewResized(int width, int height) {
    if (!floatButton_) {
        return;
    }
    KVKBD_DEBUG("onViewResized");
    floatButton_->resize(width, height);
    ScreenWatcher::getInstance().notifyScreenMarkChanged();
}

void FloatButtonManager::showFloatButton() {
    if (!floatButtonEnabled_) {
        return;
    }

    floatButton_->show();
    geometryManager_->updateGeometry();
}

void FloatButtonManager::hideFloatButton() {
    if (!floatButtonEnabled_) {
        return;
    }

    floatButton_->hide();
}

void FloatButtonManager::createFloatButton() {
    floatButton_.reset(new FloatButton(
        [this]() { fcitxVirtualKeyboardService_.showVirtualKeyboard(); }));
    floatButton_->updateThemeStyle(themeWatcher_.currentThemeColor());
    floatButton_->show();
}

void FloatButtonManager::connectFloatButtonSignals() {
    connect(&themeWatcher_, &ThemeWatcher::currentThemeColorChanged,
            floatButton_.get(), [this]() {
                floatButton_->updateThemeStyle(
                    themeWatcher_.currentThemeColor());
            });

    connect(floatButton_.get(), &FloatButton::mousePressed,
            geometryManager_.get(), &FloatGeometryManager::pressed,
            Qt::UniqueConnection);
    connect(floatButton_.get(), &FloatButton::mouseMoved,
            geometryManager_.get(), &FloatGeometryManager::moveBy,
            Qt::UniqueConnection);
    connect(floatButton_.get(), &FloatButton::mouseReleased,
            geometryManager_.get(), &FloatGeometryManager::endDrag,
            Qt::UniqueConnection);
    connect(
        &virtualKeyboardManager_,
        &VirtualKeyboardManager::virtualKeyboardVisibiltyChanged,
        floatButton_.get(),
        [this](bool visible) {
            if (visible) {
                hideFloatButton();
            } else {
                showFloatButton();
            }
        },
        Qt::UniqueConnection);
}

void FloatButtonManager::updateFloatButtonEnabled(bool enabled) {
    floatButtonEnabled_ = enabled;

    if (floatButtonEnabled_) {
        emit floatButtonEnabled();
    } else {
        emit floatButtonDisabled();
    }
}

void FloatButtonManager::setFloatButtonEnabled(bool enabled) {
    if (floatButtonEnabled_ == enabled) {
        return;
    }

    updateFloatButtonEnabled(enabled);
}
