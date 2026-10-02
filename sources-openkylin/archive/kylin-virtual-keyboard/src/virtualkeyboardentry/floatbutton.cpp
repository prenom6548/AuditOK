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

#include "floatbutton.h"

#include <QBitmap>
#include <QPainter>
#include <QVariant>
#include <QWindow>
#include "log.h"
#include "ukuiwaylandhelper/ukuiwaylandproperties.h"
#include "utils.h"

FloatButton::FloatButton(MouseClickedCallback mouseClickedCallback)
    : mouseClickedCallback_(std::move(mouseClickedCallback)) {
    initAttributes();
}

void FloatButton::move(int x, int y) {
    KVKBD_DEBUG("position:{},{}", x, y);
    QPushButton::move(x, y);
}

void FloatButton::resize(int width, int height) {
    setFixedSize(width, height);
    setIconSize(size());
    QPushButton::resize(width, height);
}

void FloatButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        startX_ = event->pos().x();
        startY_ = event->pos().y();
        emit mousePressed();
        startGlobalX_ = windowHandle()->position().x();
        startGlobalY_ = windowHandle()->position().y();

        startClickTimer();

        if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
            getDesktopType() == DesktopType::WAYLAND) {
            windowHandle()->startSystemMove();
        }
    }

    QPushButton::mousePressEvent(event);
}

bool FloatButton::shouldPerformMouseClick() const {
    return !isFloatButtonMoved() && clickTimer_->isActive();
}

void FloatButton::processMouseReleaseEvent() {
    emit mouseReleased(windowHandle()->position());

    manhattonLength = 0;

    startX_ = -1;
    startY_ = -1;
    startGlobalX_ = -1;
    startGlobalY_ = -1;
}

void FloatButton::processMouseClickEvent() {
    if (!mouseClickedCallback_) {
        return;
    }
    mouseClickedCallback_();
}

void FloatButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);

    if (event->button() != Qt::LeftButton) {
        return;
    }
    bool couldPerformMouseClick = shouldPerformMouseClick();
    stopClickTimer();

    if (couldPerformMouseClick) {
        processMouseClickEvent();

        return;
    }

    if (isFloatButtonMoved()) {
        processMouseReleaseEvent();
    }
}

void FloatButton::updateManhattonLength(QMouseEvent *event) {
    int offsetX = event->pos().x() - startX_;
    int offsetY = event->pos().y() - startY_;

    manhattonLength += std::abs(offsetX) + std::abs(offsetY);
}

bool FloatButton::isFloatButtonMoved() const {
    auto offsetX = windowHandle()->position().x() - startGlobalX_;
    auto offsetY = windowHandle()->position().y() - startGlobalY_;
    auto offset = std::abs(offsetX) + std::abs(offsetY);
    return manhattonLength > manhattonLengthThreshold || offset > 0;
}

void FloatButton::processMouseMoveEvent(QMouseEvent *event) {
    if (!isFloatButtonMoved()) {
        updateManhattonLength(event);
    } else {
        emit mouseMoved(event->pos().x() - startX_, event->pos().y() - startY_);
    }
}

void FloatButton::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        processMouseMoveEvent(event);
    }
    QPushButton::mouseMoveEvent(event);
}

void FloatButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(Qt::transparent));
    painter.setPen(Qt::transparent);

    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height());

    painter.drawEllipse(rect);

    QPushButton::paintEvent(event);
}

void FloatButton::updateThemeStyle(const QString &themeColor) {
    const QString themePrefix = QString(":/img/%1/").arg(themeColor);
    const QString defaultIcon = themePrefix + "floatbuttondefault.svg";
    const QString hoveredIcon = themePrefix + "floatbuttonhovered.svg";
    const QString pressedIcon = themePrefix + "floatbuttonpressed.svg";

    setStyleSheet(QString("QPushButton{border-image: url(%1);}"
                          "QPushButton:hover{border-image: url(%2);}"
                          "QPushButton:pressed{border-image: url(%3);}")
                      .arg(defaultIcon, hoveredIcon, pressedIcon));
}

void FloatButton::initAttributes() {
    setWindowTitle("kylin-virtual-keyboard-float-button");
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    setToolTip(tr("Click to show virtual keyboard"));

    setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint |
                   Qt::Tool);

    // 主题框架默认禁用了move消息，因此，QPushButton需要禁用主题框架
    setProperty("useStyleWindowManager", QVariant(false));

    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        setProperty(UkuiWaylandProperty::SURFACE_ROLE,
                    UkuiWaylandProperty::Role::INPUT_PANEL);
        // inputpanel 默认禁止移动，需要设置窗口属性为可移动
        UkuiWindowStates defaultState = UkuiWindowState::Movable;
        QPair<uint32_t, uint32_t> pair(UKUI_WINDOW_STATE_MASK_ALL,
                                       defaultState);
        setProperty(UkuiWaylandProperty::SURFACE_STATE,
                    QVariant::fromValue(pair));
        // TODO: 合成器目前圆角是固定值，而且没有给inputpanel窗口设置圆角
        // 设置阴影和毛玻璃会有圆角问题，暂时屏蔽
        //        setProperty(UkuiWaylandProperty::SURFACE_NO_TITLEBAR, true);
        //                QPair<QRegion, int> pair(QRegion(), 0);
        //                setProperty(UkuiWaylandProperty::SURFACE_BLUR,
        //                            QVariant::fromValue(pair));
    }
}

void FloatButton::startClickTimer() {
    clickTimer_.reset(new QTimer());
    clickTimer_->setSingleShot(true);
    clickTimer_->start(clickTimeThreshold_);
}

void FloatButton::stopClickTimer() {
    if (clickTimer_ == nullptr) {
        clickTimer_.reset();
    }
}
