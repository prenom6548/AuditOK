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

#include "kbubblewidget.h"
#include "themeController.h"
#include <KWindowEffects>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include "kshadowhelper.h"
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include "kysdk-ukuiwindowhelper/ukuiwindowhelper.h"
#include <QStringList>

namespace kdk
{

class KBubbleWidgetPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KBubbleWidget)

public:
    KBubbleWidgetPrivate(KBubbleWidget *parent);

    void updateMargins();

private:
    KBubbleWidget *q_ptr;

    int m_startX;
    int m_startY;
    int m_tailWidth;
    int m_tailHeight;
    TailDirection m_tailDirection;
    TailLocation m_tailLocation;
    int m_topLeftRadius;
    int m_topRightRadius;
    int m_bottomLeftRadius;
    int m_bottomRightRadius;
    bool m_enableBlur;
    bool m_isHighlightBackground;
    qreal m_opacity;
    QGraphicsDropShadowEffect *m_shadowEffect;
    QRegion m_region;
    UkuiWindowHelper *m_ukuiWindowHelper;

    //四个圆角旧值
    int m_lastNonZeroTopLeft = 0;
    int m_lastNonZeroTopRight = 0;
    int m_lastNonZeroBottomRight = 0;
    int m_lastNonZeroBottomLeft = 0;
};

void KBubbleWidget::setTailPosition(TailDirection dirType, TailLocation locType)
{
    Q_D(KBubbleWidget);
    d->m_tailDirection = dirType;
    d->m_tailLocation = locType;
    this->update();
}

TailDirection KBubbleWidget ::tailDirection()
{
    Q_D(KBubbleWidget);
    return d->m_tailDirection;
}

TailLocation KBubbleWidget::tailLocation()
{
    Q_D(KBubbleWidget);
    return d->m_tailLocation;
}

void KBubbleWidget::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KBubbleWidget);
    QString string = d->m_gsetting->get("windowRadius").toString();
    QStringList radiusList = string.split(',');
    int firstValue = radiusList[0].toInt();
    if(firstValue == 0)
    {
        d->m_bottomLeftRadius = 0;
        d->m_topLeftRadius = 0;
        d->m_bottomRightRadius = 0;
        d->m_topRightRadius = 0;

        d->m_lastNonZeroTopLeft = topLeft;
        d->m_lastNonZeroTopRight = topRight;
        d->m_lastNonZeroBottomRight = bottomRight;
        d->m_lastNonZeroBottomLeft = bottomLeft;
    }
    else
    {
        d->m_topLeftRadius = topLeft;
        d->m_bottomLeftRadius = bottomLeft;
        d->m_bottomRightRadius = bottomRight;
        d->m_topRightRadius = topRight;
    }
    d->updateMargins();
}

void KBubbleWidget::setBorderRadius(int radius)
{
    Q_D(KBubbleWidget);
    QString string = d->m_gsetting->get("windowRadius").toString();
    QStringList radiusList = string.split(',');
    int firstValue = radiusList[0].toInt();
    if(firstValue == 0)
    {
        d->m_bottomLeftRadius = 0;
        d->m_topLeftRadius = 0;
        d->m_bottomRightRadius = 0;
        d->m_topRightRadius = 0;

        d->m_lastNonZeroTopLeft = radius;
        d->m_lastNonZeroTopRight = radius;
        d->m_lastNonZeroBottomRight = radius;
        d->m_lastNonZeroBottomLeft = radius;
    }
    else
    {
        d->m_topLeftRadius = radius;
        d->m_bottomLeftRadius = radius;
        d->m_bottomRightRadius = radius;
        d->m_topRightRadius = radius;
    }
}

void KBubbleWidget::setEnableBlur(bool flag)
{
    Q_D(KBubbleWidget);
    d->m_enableBlur = flag;
}

bool KBubbleWidget::enableBlur()
{
    Q_D(KBubbleWidget);
    return d->m_enableBlur;
}

void KBubbleWidget::setOpacity(qreal opacity)
{
    Q_D(KBubbleWidget);
    if (opacity < 0 || opacity > 1)
        return;
    d->m_opacity = opacity;
}

qreal KBubbleWidget::opacity()
{
    Q_D(KBubbleWidget);
    return d->m_opacity;
}

void KBubbleWidget::setHighlightBackground(bool flag)
{
    Q_D(KBubbleWidget);
    d->m_isHighlightBackground = flag;
    update();
}

KBubbleWidget::KBubbleWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KBubbleWidgetPrivate(this))
{
    Q_D(KBubbleWidget);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // effects::KShadowHelper::self()->setWidget(this);
    d->m_shadowEffect = new QGraphicsDropShadowEffect(this);
    d->m_shadowEffect->setBlurRadius(12);
    d->m_shadowEffect->setColor(QColor(0, 0, 0, 102));
    d->m_shadowEffect->setOffset(0, 0);
    setGraphicsEffect(d->m_shadowEffect);

    d->m_ukuiWindowHelper = new UkuiWindowHelper(this);
}

void KBubbleWidget::setTailSize(const QSize &size)
{
    Q_D(KBubbleWidget);
    d->m_tailWidth = size.width();
    d->m_tailHeight = size.height();
    d->updateMargins();
}

QSize KBubbleWidget::tailSize()
{
    Q_D(KBubbleWidget);
    QSize size;
    size.setWidth(d->m_tailWidth);
    size.setHeight(d->m_tailHeight);
    return size;
}

void KBubbleWidget ::paintEvent(QPaintEvent *)
{
    Q_D(KBubbleWidget);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundRect(rect(),0,0);
    QPolygon trianglePolygon;
    QPainterPath path;
    QRect targetRect = this->rect()/*.adjusted(8,8,-8,-8)*/;

    switch (d->m_tailDirection) {
    case TopDirection:
        targetRect = targetRect.adjusted(8, d->m_tailHeight, -8, -8);

        if (d->m_tailLocation == LeftLocation)
            d->m_startX = 18;
        else if (d->m_tailLocation == MiddleLocation)
            d->m_startX = (targetRect.width() - d->m_tailWidth * 2) / 2;
        else if (d->m_tailLocation == RightLocation)
            d->m_startX = targetRect.width() - d->m_tailHeight * 2 - d->m_tailWidth - 19;
        d->m_startY = 0;
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_tailHeight);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth / 2 + d->m_tailHeight, 0);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth + d->m_tailHeight, d->m_tailHeight);
        break;

    case LeftDirection:
        targetRect = targetRect.adjusted(d->m_tailHeight, 8, -8, -8);

        if (d->m_tailLocation == LeftLocation)
            d->m_startY = d->m_tailHeight + 18;
        else if (d->m_tailLocation == MiddleLocation)
            d->m_startY = targetRect.height() / 2 - d->m_tailWidth / 2;
        else if (d->m_tailLocation == RightLocation)
            d->m_startY = d->m_tailHeight + static_cast<int>((targetRect.height() - d->m_tailHeight * 2 - d->m_tailWidth)) - 19;
        d->m_startX = 0;
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_startY);
        trianglePolygon << QPoint(d->m_startX, d->m_startY + d->m_tailWidth / 2);
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_startY + d->m_tailWidth);
        break;

    case RightDirection:
        targetRect = targetRect.adjusted(8, 8, -d->m_tailHeight, -8);

        if (d->m_tailLocation == LeftLocation)
            d->m_startY = d->m_tailHeight + 18;
        else if (d->m_tailLocation == MiddleLocation)
            d->m_startY = targetRect.height() / 2 - d->m_tailWidth / 2;
        else if (d->m_tailLocation == RightLocation)
            d->m_startY = d->m_tailHeight + static_cast<int>((targetRect.height() - d->m_tailHeight * 2 - d->m_tailWidth)) - 19;
        d->m_startX = targetRect.width();
        trianglePolygon << QPoint(d->m_startX - 1, d->m_startY);
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight - 1, d->m_startY + d->m_tailWidth / 2);
        trianglePolygon << QPoint(d->m_startX - 1, d->m_startY + d->m_tailWidth);
        break;
    case BottomDirection:
        targetRect = targetRect.adjusted(8, 8, -8, -d->m_tailHeight);

        if (d->m_tailLocation == LeftLocation)
            d->m_startX = 18;
        else if (d->m_tailLocation == MiddleLocation)
            d->m_startX = (this->rect().width() - d->m_tailWidth * 2) / 2;
        else if (d->m_tailLocation == RightLocation)
            d->m_startX = this->rect().width() - d->m_tailHeight * 2 - d->m_tailWidth - 19;
        d->m_startY = this->rect().height();
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_startY - d->m_tailHeight - 1);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth / 2 + d->m_tailHeight, d->m_startY - 1);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth + d->m_tailHeight, d->m_startY - d->m_tailHeight - 1);
        break;
    case None:
        break;
    }

    path.moveTo(targetRect.topRight() - QPoint(d->m_topRightRadius, 0)); // 右上
    path.lineTo(targetRect.topLeft() + QPointF(d->m_topLeftRadius, 0)); // 上方线
    path.quadTo(targetRect.topLeft(), targetRect.topLeft() + QPointF(0, d->m_topLeftRadius)); // 圆角
    path.lineTo(targetRect.bottomLeft() + QPointF(0, -d->m_bottomLeftRadius)); // 左方线
    path.quadTo(targetRect.bottomLeft(), targetRect.bottomLeft() + QPointF(d->m_bottomLeftRadius, 0)); // 圆角
    path.lineTo(targetRect.bottomRight() - QPointF(d->m_bottomRightRadius, 0)); // 下方线
    path.quadTo(targetRect.bottomRight(), targetRect.bottomRight() + QPointF(0, -d->m_bottomRightRadius)); // 圆角
    path.lineTo(targetRect.topRight() + QPointF(0, d->m_topRightRadius)); // 右方线
    path.quadTo(targetRect.topRight(), targetRect.topRight() - QPointF(d->m_topRightRadius, 0)); // 圆角

    path.addPolygon(trianglePolygon);
    path = path.simplified(); // 合并subpath,取消交集部分的线


    painter.setRenderHints(QPainter::Antialiasing);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    if (d->m_enableBlur && !d->m_isHighlightBackground && isActiveWindow()) {
        // 开启毛玻璃时
        d->m_region = QRegion(path.toFillPolygon().toPolygon());
        #ifdef USE_UKUI_SHELL_PLUGIN
                d->m_ukuiWindowHelper->setBlurEffect(d->m_region, 0, true);
        #else
                KWindowEffects::enableBlurBehind(this->winId(), true, d->m_region);
        #endif
        this->setMask(rect());
        painter.setOpacity(d->m_opacity);
        painter.setBrush(Qt::NoBrush);
    } else {
        // 未开启毛玻璃时
        painter.setOpacity(1);
        #ifdef USE_UKUI_SHELL_PLUGIN
            d->m_ukuiWindowHelper->setBlurEffect(d->m_region, 0, false);
        #else
            KWindowEffects::enableBlurBehind(this->winId(), false, d->m_region);
        #endif
        if (d->m_isHighlightBackground)
            painter.setBrush(ThemeController::getPaletteColorFromDT("highlight-active"));
        else
            painter.setBrush(ThemeController::getCustomColorFromDT("kmenu"));//ThemeController::getCustomColorFromDT("base-active"));
    }
    QPen pen;
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setWidthF(1);
    pen.setColor(Qt::transparent);
    painter.translate(0.5, 0.5);
    painter.setPen(pen);

    painter.drawPath(path);

}

KBubbleWidgetPrivate::KBubbleWidgetPrivate(KBubbleWidget *parent)
    : q_ptr(parent)
    , m_tailHeight(8)
    , m_tailWidth(16)
    , m_enableBlur(false)
    , m_opacity(0.5)
    , m_isHighlightBackground(false)
    , m_tailLocation(TailLocation::LeftLocation)
    , m_tailDirection(TailDirection::BottomDirection)
{
    Q_Q(KBubbleWidget);
    setParent(parent);
    int radius = ThemeController::getRadiusFromDT("kradius-menu");
    updateMargins();
    if (radius == -1) {
        m_bottomLeftRadius = 8;
        m_topLeftRadius = 8;
        m_bottomRightRadius = 8;
        m_topRightRadius = 8;
    } else {
        m_bottomLeftRadius = radius;
        m_topLeftRadius = radius;
        m_bottomRightRadius = radius;
        m_topRightRadius = radius;
    }
    connect(m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "windowRadius")
        {
            QString string = m_gsetting->get("windowRadius").toString();
            QStringList radiusList = string.split(',');
            int firstValue = radiusList[0].toInt();
            if(firstValue == 0)
            {
                if(m_topLeftRadius != 0)
                    m_lastNonZeroTopLeft = m_topLeftRadius;
                if(m_topLeftRadius != 0)
                    m_lastNonZeroTopRight = m_topRightRadius;
                if(m_topLeftRadius != 0)
                    m_lastNonZeroBottomRight = m_bottomRightRadius;
                if(m_topLeftRadius != 0)
                    m_lastNonZeroBottomLeft = m_bottomLeftRadius;

                m_bottomLeftRadius = 0;
                m_topLeftRadius = 0;
                m_bottomRightRadius = 0;
                m_topRightRadius = 0;
            }
            else
            {
                m_topLeftRadius = m_lastNonZeroTopLeft;
                m_topRightRadius = m_lastNonZeroTopRight;
                m_bottomRightRadius = m_lastNonZeroBottomRight;
                m_bottomLeftRadius = m_lastNonZeroBottomLeft;
            }
            q->update();
        }
    });
}

void KBubbleWidgetPrivate::updateMargins()
{
    Q_Q(KBubbleWidget);
    switch (m_tailDirection) {
    case TopDirection:
        q->setContentsMargins(4, 4 + m_tailHeight, 4, 4);
        break;
    case LeftDirection:
        q->setContentsMargins(4 + m_tailHeight, 4, 4, 4);
        break;
    case RightDirection:
        q->setContentsMargins(4, 4, 4 + m_tailHeight, 4);
        break;
    case BottomDirection:
        q->setContentsMargins(4, 4, 4, 4 + m_tailHeight);
        break;
    case None:
        break;
    }
}

}
#include "kbubblewidget.moc"
#include "moc_kbubblewidget.cpp"
