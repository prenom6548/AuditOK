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
#ifndef CUSTOMCALENDAR_H
#define CUSTOMCALENDAR_H

#include <QSpinBox>
#include <QCalendarWidget>
#include <QToolButton>
#include <QTimer>

class QPushButton;
class QLabel;
class CalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    CalendarWidget(QWidget *parent = nullptr);
    ~CalendarWidget();
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    QPixmap loadSvg(const QString &path, int size);
    bool eventFilter(QObject *watched, QEvent *event);
    void resetYearEditWidth();

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *event);

private:
    QToolButton *yearButton   = nullptr;
    QToolButton *monthButton  = nullptr;
    QSpinBox    *yearEdit     = nullptr;
    QToolButton *left_button  = nullptr;
    QToolButton *right_button = nullptr;
    QTimer *m_itimer = nullptr;

};




#endif // CUSTOMCALENDAR_H
