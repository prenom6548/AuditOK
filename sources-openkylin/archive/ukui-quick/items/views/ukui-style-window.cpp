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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "ukui-style-window.h"

#include <QDebug>
#include <QEvent>
#include <QRegion>

namespace UkuiQuick {

class UkuiStyleWindowPrivate
{
public:
    // 毛玻璃
    bool enableWindowBlur {false};
    WindowType::Type windowType {WindowType::Normal};
    QRegion windowBlurRegion;
    QRect windowRect {0, 0, 1, 1};
    WindowProxy *windowProxy = nullptr;
};

UkuiStyleWindow::UkuiStyleWindow(QWindow *parent) : QQuickWindow(parent), d(new UkuiStyleWindowPrivate)
{
    qRegisterMetaType<UkuiQuick::WindowType::Type>();
    d->windowProxy = new WindowProxy(this);
}

quint64 UkuiStyleWindow::getWinId()
{
    return winId();
}

UkuiStyleWindow::~UkuiStyleWindow()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool UkuiStyleWindow::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Show: {
//        case QEvent::Expose: {
//            if (isExposed()) {
                updateBlurRegion();
//            }
        }
        default:
            break;
    }

    return QQuickWindow::event(event);
}

bool UkuiStyleWindow::enableWindowBlur() const
{
    return d->enableWindowBlur;
}

void UkuiStyleWindow::setEnableWindowBlur(bool enable)
{
    if (d->enableWindowBlur == enable) {
        return;
    }

    d->enableWindowBlur = enable;
    updateBlurRegion();

    Q_EMIT enableWindowBlurChanged();
}

QRegion UkuiStyleWindow::blurRegion() const
{
    return d->windowBlurRegion;
}

void UkuiStyleWindow::setBlurRegion(const QRegion& region)
{
    if (d->windowBlurRegion == region) {
        return;
    }

    d->windowBlurRegion = region;
    updateBlurRegion();
}

void UkuiStyleWindow::updateBlurRegion()
{
    d->windowProxy->setBlurRegion(d->enableWindowBlur, d->windowBlurRegion);
}

void UkuiStyleWindow::updateGeometry()
{
    QRect rect = windowGeometry();
    setGeometry(rect);
    d->windowProxy->setGeometry(rect);
}

int UkuiStyleWindow::x() const
{
    return d->windowRect.x();
}

void UkuiStyleWindow::setX(int x)
{
    if (d->windowRect.x() == x) {
        return;
    }

    setWindowGeometry(QRect(x, d->windowRect.y(), d->windowRect.width(), d->windowRect.height()));
}

int UkuiStyleWindow::y() const
{
    return d->windowRect.y();
}

void UkuiStyleWindow::setY(int y)
{
    if (d->windowRect.y() == y) {
        return;
    }

    setWindowGeometry(QRect(d->windowRect.x(), y, d->windowRect.width(), d->windowRect.height()));
}

int UkuiStyleWindow::width() const
{
    return d->windowRect.width();
}

void UkuiStyleWindow::setWidth(int width)
{
    if (d->windowRect.width() == width) {
        return;
    }

    setWindowGeometry(QRect(d->windowRect.x(), d->windowRect.y(), width, d->windowRect.height()));
}

int UkuiStyleWindow::height() const
{
    return d->windowRect.height();
}

void UkuiStyleWindow::setHeight(int height)
{
    if (d->windowRect.height() == height) {
        return;
    }

    setWindowGeometry(QRect(d->windowRect.x(), d->windowRect.y(), d->windowRect.width(), height));
}

QRect UkuiStyleWindow::windowGeometry() const
{
    return d->windowRect;
}

void UkuiStyleWindow::setWindowGeometry(const QRect &rect)
{
    if (d->windowRect == rect) {
        return;
    }

    QRect old = d->windowRect;
    d->windowRect = rect;

    updateGeometry();

    if (old.x() != rect.x()) {
        Q_EMIT windowXChanged();
    }
    if (old.y() != rect.y()) {
        Q_EMIT windowYChanged();
    }
    if (old.width() != rect.width()) {
        Q_EMIT windowWidthChanged();
    }
    if (old.height() != rect.height()) {
        Q_EMIT windowHeightChanged();
    }

    Q_EMIT windowGeometryChanged();
}

QString UkuiStyleWindow::graphBackend() const
{
    return QQuickWindow::sceneGraphBackend();
}

WindowType::Type UkuiStyleWindow::windowType() const
{
    return d->windowType;
}

void UkuiStyleWindow::setWindowType(WindowType::Type windowType)
{
    if (d->windowType == windowType) {
        return;
    }

    d->windowType = windowType;
    d->windowProxy->setWindowType(windowType);
    Q_EMIT windowTypeChanged();
}

} // UkuiQuick
