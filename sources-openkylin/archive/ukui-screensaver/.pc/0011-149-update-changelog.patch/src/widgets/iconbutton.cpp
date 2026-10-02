/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "iconbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QStyleOption>
#include <QPainterPath>
#include <QPixmap>
#include <QDebug>

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}

IconButton::IconButton(int buttonType, QWidget *parent)
    : QPushButton(parent)
    , m_buttonType(buttonType)
{

}

IconButton::~IconButton()
{

}

QColor IconButton::mixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

void IconButton::paintEvent(QPaintEvent *event){
    Q_UNUSED(event)
    if (m_buttonType == EDITBUTTON) {
        QStylePainter p(this);
        QStyleOptionButton option;
        initStyleOption(&option);
        option.state = QStyle::State_Enabled;
        p.drawControl(QStyle::CE_PushButton, option);
    } else if (m_buttonType == NORMALBUTTON) {
        QStyleOptionButton option;
        initStyleOption(&option);

        QPainter p(this);

        p.setRenderHint(QPainter::HighQualityAntialiasing);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        QColor backgroundColor;
        QColor mix = option.palette.brightText().color();
        QColor highlight = option.palette.highlight().color();


        if(option.state.testFlag(QStyle::State_MouseOver))      /*鼠标在按钮上(hover状态)*/
        {
            if(option.state.testFlag(QStyle::State_Sunken)) {     /*按钮被选中（clicked）*/
                backgroundColor = mixColor(highlight,mix,0.2);
            } else {
                backgroundColor = mixColor(highlight,mix,0.1);
            }
        } else {
            backgroundColor = mixColor(highlight,mix,0.05);
        }

        /*绘制背景色和rect*/
        p.save();
        p.setBrush(backgroundColor);
        p.setPen(Qt::NoPen);
        QPoint point;
        QRect ir = option.rect;
        uint tf = Qt::AlignVCenter;

        if (m_buttonType == EDITBUTTON) {
            p.drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), 12, 12);
        } else if (m_buttonType == NORMALBUTTON) {
            p.drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), 6, 6);
        }

        QPixmap pixmap = option.icon.pixmap(option.iconSize, QIcon::Active, QIcon::On);

        point = QPoint(ir.x() + ir.width() / 2 - (pixmap.width() + 1) / 2,
                       ir.y() + ir.height() / 2 - (pixmap.height() + 2)/ 2);

        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
        p.restore();

        if (!this->text().isEmpty() || this->text() != "") {
            //  绘制文字
            if (this->isEnabled())
                p.setPen(QColor(255, 255, 255, 255));
            else
                p.setPen(this->palette().color(QPalette::Disabled, QPalette::ButtonText));
            p.setFont(this->font());
            p.drawText(this->rect(), Qt::AlignCenter, this->text());
            p.restore();
        }
    }
}
