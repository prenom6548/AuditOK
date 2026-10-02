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

#include "virtualkeyboard/virtualkeyboardview.h"
#include <QQmlContext>
#include <QQuickItem>
#include "log.h"
#include "screenwatcher.h"
#include "shadowborderitem.h"
#include "themewatcher.h"
#include "ukuiwaylandhelper/ukuiwaylandproperties.h"
#include "utils.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

VirtualKeyboardView::VirtualKeyboardView(
    QObject &manager, QObject &model,
    std::unique_ptr<PlacementModeManager> placementModeManager,
    std::unique_ptr<ExpansionGeometryManager> expansionGeometryManager,
    std::unique_ptr<FloatGeometryManager> floatGeometryManager,
    ThemeWatcher &themeWatcher)
    : manager_(manager), model_(model), themeWatcher_(themeWatcher),
      placementModeManager_(std::move(placementModeManager)),
      expansionGeometryManager_(std::move(expansionGeometryManager)),
      floatGeometryManager_(std::move(floatGeometryManager)) {

    connect(placementModeManager_.get(),
            &PlacementModeManager::isFloatModeChanged, this,
            &VirtualKeyboardView::isFloatModeChanged);
    connect(floatGeometryManager_.get(), &FloatGeometryManager::viewMoved, this,
            &VirtualKeyboardView::move);
    connect(floatGeometryManager_.get(), &FloatGeometryManager::viewResized,
            this, &VirtualKeyboardView::resize);
    connect(
        &VirtualKeyboardSettings::getInstance(),
        &VirtualKeyboardSettings::animationAvailabilityChanged, this, [this]() {
            if (!VirtualKeyboardSettings::getInstance().isAnimationEnabled() &&
                view_) {
                view_->setOpacity(1.0f);
            }
        });
    initState();
    if (VirtualKeyboardSettings::getInstance().isPreloadViewEnabled()) {
        KVKBD_INFO("will preload quickview");
        initView();
    }
}

VirtualKeyboardView::~VirtualKeyboardView() {
    destroyView();
    if (view_ != nullptr) {
        view_.release()->deleteLater();
    }

    currentState_.reset();
    visibleState_.reset();
    hidingState_.reset();
    showingState_.reset();
    invisibleState_.reset();
    flippingState_.reset();
}

void VirtualKeyboardView::moveBy(int offsetX, int offsetY) {
    if (!isFloatMode()) {
        return;
    }

    floatGeometryManager_->moveBy(offsetX, offsetY);
}

void VirtualKeyboardView::endDrag() {
    if (!isFloatMode()) {
        return;
    }

    floatGeometryManager_->endDrag(view_->position());
}

QRect VirtualKeyboardView::geometry() const {
    return getCurrentGeometryManager().geometry();
}

QRect VirtualKeyboardView::screenGeometry() const {
    return getCurrentGeometryManager().screenGeometry();
}

void VirtualKeyboardView::updateGeometry() {
    if (!isVisible()) {
        return;
    }

    QRect geo = geometry();
    view_->setGeometry(geo);
    emit positionChanged(geo.topLeft());
    emit sizeChanged();
    emitContentGeometrySignals();
}

void VirtualKeyboardView::updateMarginRatio() {
    floatGeometryManager_->updateViewMarginRatio();
}

void VirtualKeyboardView::updateExpansionFlippingStartGeometry() {
    auto geometry = floatGeometryManager_->geometry();
    view_->setGeometry(geometry.x(), view_->y(), geometry.width(),
                       geometry.height());

    emitContentGeometrySignals();
}

void VirtualKeyboardView::move(int x, int y) {
    if (!view_) {
        KVKBD_WARN("view_ is null!");
        return;
    }
    KVKBD_DEBUG("position:{},{}", x, y);
    view_->setX(x);
    view_->setY(y);
    emit positionChanged(QPoint(x, y));
}

void VirtualKeyboardView::resize(int width, int height) {
    if (!view_) {
        KVKBD_WARN("view_ is null!");
        return;
    }

    view_->resize(width, height);
    emitContentGeometrySignals();
    emit sizeChanged();
}

void VirtualKeyboardView::initView() {
    const auto preloadViewEnabled =
        VirtualKeyboardSettings::getInstance().isPreloadViewEnabled();
    if (view_ != nullptr && preloadViewEnabled) {
        return;
    }
    KVKBD_INFO("preloadViewEnabled:{}", preloadViewEnabled);

    qmlRegisterType<ShadowBorderItem>("VirtualKeyboard.ShadowBorderItem", 1, 0,
                                      "ShadowBorderItem");

    view_.reset(new QQuickView());
    view_->rootContext()->setContextProperty("manager", &manager_);
    view_->rootContext()->setContextProperty("model", &model_);
    view_->rootContext()->setContextProperty("view", this);
    view_->rootContext()->setContextProperty("themeWatcher", &themeWatcher_);

    view_->rootContext()->setContextProperty("QT_VERSION_MAJOR",
                                             QT_VERSION_MAJOR);
    view_->rootContext()->setContextProperty("QT_VERSION_MINOR",
                                             QT_VERSION_MINOR);
    view_->rootContext()->setContextProperty("QT_VERSION_PATCH",
                                             QT_VERSION_PATCH);

    view_->setTitle("kylin-virtual-keyboard");
    view_->setColor(QColor(Qt::transparent));
    view_->setSource(QUrl("qrc:/qml/VirtualKeyboard.qml"));

    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        view_->setProperty(UkuiWaylandProperty::SURFACE_ROLE,
                           UkuiWaylandProperty::Role::INPUT_PANEL);
        UkuiWindowStates defaultState = UkuiWindowState::Movable;
        QPair<uint32_t, uint32_t> moveablePair(UKUI_WINDOW_STATE_MASK_ALL,
                                               defaultState);
        view_->setProperty(UkuiWaylandProperty::SURFACE_STATE,
                           QVariant::fromValue(moveablePair));
        view_->setProperty(UkuiWaylandProperty::SURFACE_NO_TITLEBAR, true);
        QPair<QRegion, int> blurPair(QRegion(), 0);
        view_->setProperty(UkuiWaylandProperty::SURFACE_BLUR,
                           QVariant::fromValue(blurPair));
    } else {
        view_->setFlags(Qt::Window | Qt::WindowDoesNotAcceptFocus |
                        Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                        Qt::BypassWindowManagerHint);
    }

    view_->setGeometry(calculateInitialGeometry());
    emit positionChanged(geometry().topLeft());
    setViewOpacity();

    connectSignals();
}

void VirtualKeyboardView::pressed() {
    floatGeometryManager_->pressed();
    if (view_ == nullptr) {
        return;
    }
    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        KVKBD_DEBUG("moveStart");
        view_->startSystemMove();
    }
}

QRect VirtualKeyboardView::calculateInitialGeometry() {
    auto geo = geometry();
    auto screenHeight = getScreenRelativeHeight();
    auto yOffset = geo.height();
    if (isFloatMode()) {
        yOffset = 40;
    }
    int normalizedY = std::min(screenHeight, geo.y() + yOffset);

    return QRect(geo.x(), normalizedY, geo.width(), geo.height());
}

int VirtualKeyboardView::getScreenRelativeHeight() {
    auto screenRect = screenGeometry();
    return screenRect.y() + screenRect.height();
}

void VirtualKeyboardView::connectSignals() {
    auto *rootObject = view_->rootObject();

    connect(this, SIGNAL(updateCandidateArea(const QVariant &, int)),
            rootObject, SIGNAL(qmlUpdateCandidateList(QVariant, int)));
    connect(this, SIGNAL(imDeactivated()), rootObject,
            SIGNAL(qmlImDeactivated()));
}

void VirtualKeyboardView::destroyView() {
    if (view_ == nullptr) {
        return;
    }

    if (view_->isVisible()) {
        view_->hide();
    }

    if (!VirtualKeyboardSettings::getInstance().isPreloadViewEnabled()) {
        view_.release()->deleteLater();
    }
}

void VirtualKeyboardView::emitContentGeometrySignals() {
    emit contentHeightChanged();
    emit contentWidthChanged();
}

int VirtualKeyboardView::getContentWidth() {
    return getCurrentGeometryManager().getViewContentWidth();
}

int VirtualKeyboardView::getContentHeight() {
    return getCurrentGeometryManager().getViewContentHeight();
}

void VirtualKeyboardView::setViewOpacity() {
    if (VirtualKeyboardSettings::getInstance().isAnimationEnabled()) {
        view_->setOpacity(isFloatMode() ? 0.0f : 1.0f);
    }
}

GeometryManager &VirtualKeyboardView::getCurrentGeometryManager() const {
    if (isFloatMode()) {
        return *floatGeometryManager_;
    } else {
        return *expansionGeometryManager_;
    }
}
