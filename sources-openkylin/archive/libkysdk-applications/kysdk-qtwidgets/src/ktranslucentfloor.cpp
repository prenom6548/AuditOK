/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "ktranslucentfloor.h"
#include "kshadowhelper.h"
#include "themeController.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include <kwindoweffects.h>
#include "kysdk-ukuiwindowhelper/ukuiwindowhelper.h"
#include <QEvent>
#include <QGuiApplication>
#include "ukuistylehelper/ukui-decoration-manager.h"
#include "ukuistylehelper/ukui-shell-manager.h"
#include "xatom-helper.h"

namespace kdk
{

class KTranslucentFloorPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTranslucentFloor)
public:
    KTranslucentFloorPrivate(KTranslucentFloor *parent);

protected:
    bool eventFilter(QObject *target, QEvent *event) override;

private:
    KTranslucentFloor *q_ptr;
    int m_radius;
    bool m_shadowFlag;
    bool m_enableBlur;
    qreal m_opacity;
    QRegion m_region;
    UkuiWindowHelper *m_ukuiWindowHelper;
};

KTranslucentFloor::KTranslucentFloor(QWidget *parent)
    : QFrame(parent)
    , d_ptr(new KTranslucentFloorPrivate(this))
{
    Q_D(KTranslucentFloor);
    // setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    d->m_ukuiWindowHelper = new UkuiWindowHelper(this);

    QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("xcb"), Qt::CaseInsensitive)) {
            MotifWmHints hints;
            hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
            hints.functions = MWM_FUNC_ALL;
            hints.decorations = MWM_DECOR_BORDER;
            XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
        }

}

void KTranslucentFloor::setBorderRadius(int radius)
{
    Q_D(KTranslucentFloor);
    d->m_radius = radius;
    if (shadow()) {
        effects::KShadowHelper::self()->setWidget(this, d->m_radius);
    }
}

int KTranslucentFloor::borderRadius()
{
    Q_D(KTranslucentFloor);
    return d->m_radius;
}

void KTranslucentFloor::setShadow(bool flag)
{
    Q_D(KTranslucentFloor);
    d->m_shadowFlag = flag;
    if (d->m_shadowFlag)
        effects::KShadowHelper::self()->setWidget(this, d->m_radius);
}

bool KTranslucentFloor::shadow()
{
    Q_D(KTranslucentFloor);
    return d->m_shadowFlag;
}

void KTranslucentFloor::setEnableBlur(bool flag)
{
    Q_D(KTranslucentFloor);
    d->m_enableBlur = flag;
}

bool KTranslucentFloor::enableBlur()
{
    Q_D(KTranslucentFloor);
    return d->m_enableBlur;
}

void KTranslucentFloor::setOpacity(qreal opacity)
{
    Q_D(KTranslucentFloor);
    d->m_opacity = opacity;
}

qreal KTranslucentFloor::opacity()
{
    Q_D(KTranslucentFloor);
    return d->m_opacity;
}

void KTranslucentFloor::paintEvent(QPaintEvent *event)
{
    Q_D(KTranslucentFloor);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    QPainterPath path;
    QRect rect = this->rect();
    path.addRoundedRect(rect, d->m_radius, d->m_radius);

    if (d->m_enableBlur && isActiveWindow()) {
        // 开启毛玻璃时
        d->m_region =QRegion(path.toFillPolygon().toPolygon());
        if(d->m_enableBlur)
        {
            #ifdef USE_UKUI_SHELL_PLUGIN
                        d->m_ukuiWindowHelper->setBlurEffect(d->m_region, 0, true);
            #else
                        KWindowEffects::enableBlurBehind(this->winId(), true, d->m_region);
            #endif
        }
        this->setMask(d->m_region);
        painter.setOpacity(d->m_opacity);
        painter.setBrush(Qt::NoBrush);
    } else {
        // 未开启毛玻璃时
        #ifdef USE_UKUI_SHELL_PLUGIN
            d->m_ukuiWindowHelper->setBlurEffect(d->m_region, 0, false);
        #else
            KWindowEffects::enableBlurBehind(this->winId(), false, d->m_region);
        #endif
        painter.setOpacity(1);
        painter.setBrush(this->palette().color(this->backgroundRole()));
    }
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
}

KTranslucentFloorPrivate::KTranslucentFloorPrivate(KTranslucentFloor *parent)
    : q_ptr(parent)
    , m_shadowFlag(true)
    , m_enableBlur(true)
    , m_opacity(0.5)
{
    setParent(parent);
    parent->installEventFilter(this);
    m_radius = ThemeController::getRadiusFromDT("kradius-window");
    if (m_radius == -1)
        m_radius = 12;
}

bool KTranslucentFloorPrivate::eventFilter(QObject *target, QEvent *event)
{
    Q_Q(KTranslucentFloor);
    QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive))
    {
        if ((event->type() == QEvent::PlatformSurface || event->type() == QEvent::Show || event->type() == QEvent::UpdateRequest))
        {
#ifdef USE_UKUI_SHELL_PLUGIN
        m_ukuiWindowHelper->removeTitleBar();
#else
#ifdef USE_UKUI_PROTOCOL
      if (UkuiShellManager::getInstance()->ukuishellReady())
        UkuiShellManager::getInstance()->removeHeaderBar(q->windowHandle());
      else
        UKUIDecorationManager::getInstance()->removeHeaderBar(q->windowHandle());
#else
      UKUIDecorationManager::getInstance()->removeHeaderBar(q->windowHandle());
#endif
#endif
        }
}
    return QObject::eventFilter(target,event);
}

}

#include "ktranslucentfloor.moc"
#include "moc_ktranslucentfloor.cpp"
