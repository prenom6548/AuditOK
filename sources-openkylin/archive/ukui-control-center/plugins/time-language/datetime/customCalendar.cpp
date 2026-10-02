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
#include "customCalendar.h"

#include <QLocale>
#include <QPainter>
#include <QTextCharFormat>
#include <QProxyStyle>
#include <QTableView>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QSvgRenderer>

CustomCalendarWidget::CustomCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
//        setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    this->installEventFilter(this);
    this->setMinimumHeight(230);
    this->setMinimumWidth(350);
    this->setContentsMargins(12,12,12,12);
    this->setDateEditEnabled(false);
    left_button  = this->findChild<QToolButton *>("qt_calendar_prevmonth");
    right_button = this->findChild<QToolButton *>("qt_calendar_nextmonth");
    yearButton   = this->findChild<QToolButton *>("qt_calendar_yearbutton");
    yearEdit     = this->findChild<QSpinBox*>    ("qt_calendar_yearedit");
    monthButton  = this->findChild<QToolButton *>("qt_calendar_monthbutton");
    left_button->setIcon(QIcon::fromTheme("ukui-start-symbolic"));
    right_button->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
}

CustomCalendarWidget::~CustomCalendarWidget()
{

}

void CustomCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    /* 绘制背景 */
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette().color(QPalette::Base));
    painter->drawRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(), 0, 0);
    painter->restore();
    if (date == selectedDate()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(palette().color(QPalette::Highlight));

        painter->drawRoundedRect(rect.x() + 5, rect.y() + 4, rect.width() - 10, rect.height() - 6, 6, 6);
        painter->setPen(QColor(255, 255, 255));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    } else {
        QCalendarWidget::paintCell(painter, rect, date);
    }
}


void CustomCalendarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
//    this->move(0,0);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QColor color = palette().color(QPalette::Dark);
    QColor color2 = palette().color(QPalette::Base);
    QRect rectBoxt = this->rect();
    painter.setBrush(Qt::NoBrush);
    painter.translate(1, 1);

    color2.setAlpha(255);
    painter.setPen(QPen(color2,12));
    painter.drawRoundedRect(rectBoxt.adjusted(+8, +10, -10, -10), 6, 6);

    color.setAlpha(80);
    painter.setPen(QPen(color,1));
    painter.drawRoundedRect(rectBoxt.adjusted(0, +2, -2, -2), 6, 6);

    color.setAlpha(160);
    painter.setPen(color);
    painter.drawRoundedRect(rectBoxt.adjusted(+1, +3, -3, -3), 6, 6);
}

void CustomCalendarWidget::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return;
}

QPixmap CustomCalendarWidget::loadSvg(const QString &path, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap CustomCalendarWidget::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                QColor colorSet = palette().color(QPalette::ButtonText);
                color.setRed(colorSet.red());
                color.setGreen(colorSet.green());
                color.setBlue(colorSet.blue());
                img.setPixelColor(x, y, color);
            }
        }
    }
    return QPixmap::fromImage(img);
}

void CustomCalendarWidget::resetYearEditWidth()
{
    if (yearEdit && right_button && yearButton) {
        yearEdit->setFixedWidth(right_button->x() + right_button->width() - yearButton->x());
    }
    return;
}

bool CustomCalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::Type::Paint) {
        resetYearEditWidth();
    }
    return QCalendarWidget::eventFilter(watched, event);
}
