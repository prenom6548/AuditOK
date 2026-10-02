/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
*/

#include "captureshow.h"
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QDebug>
#include <QStandardPaths>
#include <QGraphicsDropShadowEffect>
#include "ukuiwindowhelper/ukuiwindowhelper.h"

CaptureShow::CaptureShow(bool fullScreen, QWidget *parent) :
    QWidget(parent)
{
#ifndef UNUI
    if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus |Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground); //透明
        setAttribute(Qt::WA_TransparentForMouseEvents, true); //鼠标穿透
        setWindowFlags( Qt::X11BypassWindowManagerHint);
    }
    else
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTransparentForInput);//
        setAttribute(Qt::WA_TranslucentBackground); //透明

        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
        windowHelper->setSkipTaskBar(true);
        windowHelper->setSkipSwitcher(true);
        UkuiWindowHelper::States states;
        states |= UkuiWindowHelper::State::KeepAbove;
        windowHelper->setWindowState(states);
        windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::OnScreenDisplay);
    }

    QRect screenGeometry = qApp->primaryScreen()->virtualGeometry();
    resize(screenGeometry.width(), screenGeometry.height());
#endif

    // 设置阴影边框
//    auto shadowEffect = new QGraphicsDropShadowEffect(this);
//    shadowEffect->setOffset(0, 2);
//    shadowEffect->setColor(QColor(0, 0, 0, 70));
//    shadowEffect->setBlurRadius(20);
//    this->setGraphicsEffect(shadowEffect);
//    QPalette p = palette();
//    m_backColor = p.color(QPalette::Window);
//    m_backColor.setAlpha(50);
}

CaptureShow::~CaptureShow()
{
}

void CaptureShow::paintEvent(QPaintEvent *event)
{
#ifndef UNUI
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    painter.fillPath(path, m_backColor);

    QWidget::paintEvent(event);
#endif
}
