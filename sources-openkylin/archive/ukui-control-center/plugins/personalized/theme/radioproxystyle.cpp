/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "radioproxystyle.h"

#include <QStyleOptionComplex>
#include <QDebug>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>

RadioProxystyle::RadioProxystyle(const QColor &color, QStyle *style) : mColor(color), QProxyStyle(style)
{

}

int RadioProxystyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ExclusiveIndicatorHeight:
        return 24;
    case PM_ExclusiveIndicatorWidth:
        return 24;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void RadioProxystyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_RadioButtonIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_RadioButtonContents, option, widget);
            proxy()->drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    default:
    {

    }break;

    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

void RadioProxystyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_IndicatorRadioButton:
    {
        if (const QStyleOptionButton* radiobutton = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            QRectF rect = radiobutton->rect.adjusted(1, 1, -1, -1);

            bool on = radiobutton->state & State_On;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(mColor);
            painter->setBrush(mColor);
            painter->drawEllipse(rect);

            if (on) {
                QRectF childRect(rect.x(), rect.y(), (rect.width() / 2) - 2, (rect.height() / 2) - 2);
                childRect.moveCenter(rect.center());
                painter->setPen(Qt::NoPen);
                painter->setBrush(radiobutton->palette.brush(QPalette::Active, QPalette::HighlightedText));
                painter->drawEllipse(childRect);
            }

            // 键盘焦点框：与色块同大；颜色跟调色板文字色，深色主题下自动反白
            if (radiobutton->state & State_HasFocus) {
                painter->setBrush(Qt::NoBrush);
                const QColor focusColor = radiobutton->palette.color(QPalette::WindowText);
                painter->setPen(QPen(focusColor, 2));
                painter->drawEllipse(rect);
            }
            painter->restore();
            return;
        }
        break;
    }

    default:
    {

    }break;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QRect RadioProxystyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_RadioButtonIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
        rect.setRect(option->rect.x(), option->rect.y() + ((option->rect.height() - h) / 2),
                  proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }
    case SE_RadioButtonClickRect:
    {
        return proxy()->subElementRect(SE_RadioButtonIndicator, option, widget);
    }

    default:
    {

    }break;
    }

    return QProxyStyle::subElementRect(element, option, widget);
}

QSize RadioProxystyle::sizeFromContents(QStyle::ContentsType element, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize  newSize = size;
    switch (element) {
    case CT_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), 36));
            return newSize;
        }
        break;
    }
    default:
    {

    }break;

    }

    return QProxyStyle::sizeFromContents(element, option, size, widget);
}

RadioProxystyle::~RadioProxystyle()
{

}
