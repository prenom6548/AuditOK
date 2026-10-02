/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "ringsprogressbar.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>

#include "../../common/mydefine.h"
#include "../gsettingswrapper.h"

RingsProgressbar::RingsProgressbar(QWidget *parent) :
    QWidget(parent)
{}

RingsProgressbar::~RingsProgressbar()
{}

void RingsProgressbar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.save();

    m_rotateAngle = 360*m_persent/100;

    int side = qMin(width(), height());
    QRectF outRect(0, 0, side, side);
    QRectF inRect(10, 10, side - 20, side - 20);
    QString valueStr = QString("%1%").arg(QString::number(m_persent));

    //画外圆
    QPen pen = p.pen();
    p.setPen(Qt::NoPen);
    if (g_GSettingWrapper.isDarkTheme())
        p.setBrush(QBrush(palette().button().color()));
    else
        p.setBrush(QBrush(QColor(COLOR_LIGHT_BLUE)));
    p.drawEllipse(outRect);

    //画遮罩
    p.setPen(Qt::NoPen);
    if (g_GSettingWrapper.isDarkTheme())
        p.setBrush(QBrush(QColor(COLOR_BLUE_DARK)));
    else
        p.setBrush(palette().window().color());
    p.drawEllipse(inRect);

    //画圆弧
    pen.setCapStyle(Qt::RoundCap);
    // pen.setColor(QColor(COLOR_BLUE));
    pen.setColor(this->palette().highlight().color());
    qreal penWidth = 5;
    pen.setWidthF(penWidth);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(5 + penWidth/2 , 5 + penWidth/2, side - 10 - penWidth, side - 10 - penWidth), (90-m_rotateAngle)*16, m_rotateAngle*16);
    // p.drawPie(QRectF(5 , 5, side - 10, side - 10), (90-m_rotateAngle)*16, m_rotateAngle*16);

    // 画进度圆点（白圈套篮圈，白圈有个篮圈模糊边）
    p.save();
    QPainterPath path;
    path.moveTo(inRect.center());
    path.arcTo(QRectF(5 + penWidth/2 , 5 + penWidth/2, side - 10 - penWidth, side - 10 - penWidth), 90, -1 * m_rotateAngle);
    QPointF currentPosition = path.currentPosition();
    QColor color(this->palette().highlight().color());
    color.setAlphaF(0.5);
    p.setPen(color);
    p.setBrush(QBrush(QColor(Qt::white)));
    p.drawEllipse(currentPosition, 6, 6);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(this->palette().highlight().color()));
    p.drawEllipse(currentPosition, 3.5, 3.5);
    p.restore();

    //画文字
    QFont f = this->font();
    f.setBold(true);
    f.setPixelSize(25);
    p.setFont(f);
    p.setPen(this->palette().highlight().color());
    p.drawText(inRect, Qt::AlignCenter, valueStr);

    p.restore();
}

void RingsProgressbar::setPersent(int persent)
{
    m_persent = persent;
    repaint();
}
