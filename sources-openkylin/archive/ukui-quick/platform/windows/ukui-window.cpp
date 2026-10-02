/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "ukui-window.h"

#include <QMoveEvent>
#include <QResizeEvent>
#include <QGuiApplication>

namespace UkuiQuick {

class UKUIWindow::Private
{
public:
    bool m_needSetupWhenExposed {true};
    bool m_removeTitleBar {false};
    bool m_skipTaskBar {false};
    bool m_skipSwitcher {false};
    bool m_enableBlurEffect {false};

    WindowType::Type m_type {WindowType::Normal};

    WindowProxy *windowProxy {nullptr};
    Region *m_blurRegion {nullptr};
    Region *m_maskRegion {nullptr};
};

UKUIWindow::UKUIWindow(QWindow *parent) : QQuickWindow(parent), d(new Private)
{
    qRegisterMetaType<UkuiQuick::WindowType::Type>();
    d->windowProxy = new WindowProxy(this, WindowProxy::Operations());
    d->m_blurRegion = new Region(this);
    d->m_maskRegion = new Region(this);
    connect(d->m_blurRegion, &Region::regionChanged, this, &UKUIWindow::blurRegionChanged);
    connect(d->m_maskRegion, &Region::regionChanged, this, &UKUIWindow::maskRegionChanged);
}

UKUIWindow::~UKUIWindow()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QPoint UKUIWindow::windowPosition() const
{
    return d->windowProxy->position();
}

void UKUIWindow::setWindowPosition(const QPoint &pos)
{
    d->windowProxy->setPosition(pos);
}

bool UKUIWindow::enableBlurEffect() const
{
    return d->m_enableBlurEffect;
}

void UKUIWindow::setEnableBlurEffect(bool enable)
{
    if (d->m_enableBlurEffect == enable) {
        return;
    }

    d->m_enableBlurEffect = enable;
    d->windowProxy->setBlurRegion(enable, d->m_blurRegion->region());
    emit enableBlurEffectChanged();
}

Region* UKUIWindow::blurRegion() const
{
    return d->m_blurRegion;
}

Region* UKUIWindow::maskRegion() const
{
    return d->m_maskRegion;
}

WindowType::Type UKUIWindow::windowType() const
{
    return d->m_type;
}

void UKUIWindow::setWindowType(WindowType::Type type)
{
    if (type == d->m_type) {
        return;
    }

    d->m_type = type;
    d->windowProxy->setWindowType(d->m_type);
    emit windowTypeChanged();
}

bool UKUIWindow::skipTaskBar() const
{
    return d->m_skipTaskBar;
}

bool UKUIWindow::skipSwitcher() const
{
    return d->m_skipSwitcher;
}

bool UKUIWindow::removeTitleBar() const
{
    return d->m_removeTitleBar;
}

void UKUIWindow::setRemoveTitleBar(bool remove)
{
    if (d->m_removeTitleBar == remove) {
        return;
    }

    d->m_removeTitleBar = remove;
    d->windowProxy->setRemoveTitleBar(remove);

    emit removeTitleBarChanged();
}

void UKUIWindow::setSkipTaskBar(bool skip)
{
    if (d->m_skipTaskBar == skip) {
        return;
    }

    d->m_skipTaskBar = skip;
    d->windowProxy->setSkipTaskBar(d->m_skipTaskBar);

    emit skipTaskBarChanged();
}

void UKUIWindow::setSkipSwitcher(bool skip)
{
    if (d->m_skipSwitcher == skip) {
        return;
    }

    d->m_skipSwitcher = skip;
    d->windowProxy->setSkipSwitcher(d->m_skipSwitcher);

    emit skipSwitcherChanged();
}

void UKUIWindow::moveEvent(QMoveEvent *event)
{
    QWindow::moveEvent(event);
}

void UKUIWindow::resizeEvent(QResizeEvent *event)
{
    QQuickWindow::resizeEvent(event);
}

void UKUIWindow::blurRegionChanged()
{
    if (d->m_enableBlurEffect) {
        d->windowProxy->setBlurRegion(d->m_enableBlurEffect, d->m_blurRegion->region());
    }
}

void UKUIWindow::maskRegionChanged()
{
    setMask(d->m_maskRegion->region());
}

// ====== UKUIWindowExtension ====== //
UKUIWindowExtension::UKUIWindowExtension(QObject *parent) : QObject(parent), m_window(qobject_cast<UKUIWindow*>(parent))
{

}

int UKUIWindowExtension::x() const
{
    return m_window->windowPosition().x();
}

int UKUIWindowExtension::y() const
{
    return m_window->windowPosition().y();
}

void UKUIWindowExtension::setX(int x)
{
    QPoint wPos = m_window->windowPosition();
    if (x == wPos.x()) {
        return;
    }

    wPos.setX(x);
    m_window->setWindowPosition(wPos);

    emit xChanged(x);
}

void UKUIWindowExtension::setY(int y)
{
    QPoint wPos = m_window->windowPosition();
    if (y == wPos.y()) {
        return;
    }

    wPos.setY(y);
    m_window->setWindowPosition(wPos);

    emit yChanged(y);
}

} // UkuiQuick
