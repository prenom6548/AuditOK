/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "customstyle.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QLineEdit>
#include <QFileDialog>
#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>

InternalStyle::InternalStyle(const QString &styleName, QObject *parent) : QProxyStyle(styleName)
{

}

void InternalStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch(element)
    {
    case CE_ShapedFrame: {
        //return proxy()->drawPrimitive(PE_Frame, option, painter, widget);
        QStyleOptionFrame frame = *qstyleoption_cast<const QStyleOptionFrame *>(option);
        frame.lineWidth = 0;
        QFrame::Shape shape = frame.frameShape;
        switch (shape) {
        case QFrame::Box: {
            // Draw four rounded corners
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setBrush(option->palette.color(QPalette::Base));

            painter->setPen(Qt::transparent);
            QRect rect = widget->rect();
            painter->drawRoundedRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2, 6, 6);
            painter->restore();
            return;
        }
        case QFrame::HLine: {
            // Draw rounded corners at the bottom left and bottom right
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            QPainterPath path;
            path.addRoundedRect(frame.rect, 6,6);
            path.setFillRule(Qt::WindingFill);

            path.addRect(frame.rect.width() - 6, 0, 6, 6);
            path.addRect(0, 0, 6, 6);

            painter->setPen(Qt::transparent);
            painter->setBrush(option->palette.color(QPalette::Button));

            painter->setClipPath(path);
            painter->drawRect(frame.rect);
            painter->restore();
            return;
        }
        case QFrame::VLine: {
            // Draw rounded corners in the upper left and upper right corners

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            QPainterPath path;
            path.addRoundedRect(frame.rect, 6,6);
            path.setFillRule(Qt::WindingFill);

            path.addRect(0, frame.rect.height() - 6, 6, 6);
            path.addRect(frame.rect.width() - 6, frame.rect.height() - 6, 6, 6);

            painter->setPen(Qt::transparent);
            painter->setBrush(option->palette.color(QPalette::Button));

            painter->setClipPath(path);
            painter->drawRect(frame.rect);
            painter->restore();
            return;
        }
        case QFrame::Panel: {
            // Do not draw corner
            painter->fillRect(frame.rect, option->palette.color(QPalette::Button));
            return;
        }

        case QFrame::StyledPanel: {
            if (widget && qobject_cast<const QComboBox *>(widget->parentWidget())) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(QPen(frame.palette.color(frame.state & State_Enabled ? QPalette::Active : QPalette::Disabled, QPalette::Button), 2));
                painter->setBrush(frame.palette.base());
                painter->drawRoundedRect(frame.rect, 4, 4);
                painter->restore();
            } else if (widget && qobject_cast<const QListWidget *>(widget)){
                return QProxyStyle::drawControl(element, option, painter, widget);
            }

            return;
        }
        default:
            return;
        }
        return;
        break;
    }
    default:
        break;
    }
    return QProxyStyle::drawControl(element, option, painter, widget);
}

void InternalStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ComboBox: {
        break;
    }
    default:
        break;
    }

    return QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void InternalStyle::polish(QPalette &pal)
{
    QProxyStyle::polish(pal);
}

void InternalStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<QDialog *>(widget) && !qobject_cast<QFileDialog *>(widget)) {
        QPalette paltte = widget->palette();
        paltte.setColor(QPalette::Window, paltte.base().color());
        widget->setPalette(paltte);
    }
}
