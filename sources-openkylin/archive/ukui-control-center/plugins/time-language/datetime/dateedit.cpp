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
#include "dateedit.h"
#include <QPainter>
#include <QBrush>
#include <QDebug>
#include <QRect>
#include <QApplication>
#include <QSvgRenderer>
#include <QLineEdit>
#include <QCalendarWidget>
#include <QGraphicsBlurEffect>
#include <QDBusReply>

QGraphicsDropShadowEffect *shadow_effect;
DateEdit::DateEdit(QWidget *parent) : QDateEdit(parent){
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    this->setFixedWidth(150);
    installEventFilter(this);
    this->setStyleSheet("\
                        QCalendarWidget QWidget#qt_calendar_navigationbar { \
                            background-color: palette(base);\
                        } \
                        QCalendarWidget QWidget {alternate-background-color: palette(base);} \
                        QCalendarWidget QTableView { \
                            selection-background-color: palette(highlight); \
                        }\
                        QCalendarWidget QToolButton { \
                            color: palette(text);\
                        } \
                        ");

    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                             "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(), this);
    if (m_statusSessionDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
        mode_change_signal_slots(is_tabletmode.isValid() ? is_tabletmode.value() : false);
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mode_change_signal_slots(bool)));
    } else {
        mode_change_signal_slots(false);
        qWarning() << "Create com.kylin.statusmanager.interface Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }
}

void DateEdit::mode_change_signal_slots(bool b)
{
    if (b) {
        this->setFixedHeight(48);
    } else {
        this->setFixedHeight(36);
    }
}

DateEdit::~DateEdit() {

}

void DateEdit::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QBrush brush = QBrush(palette().color(QPalette::Button));
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRoundedRect(this->rect(),6,6);
    QPixmap pix = loadSvg(":/img/dropArrow/ukui-down-symbolic.svg",24);
    QRect rect = QRect(125,10,16,16);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    style()->drawItemPixmap(&painter, rect, Qt::AlignCenter, pix);

    QRect rectBoxt = this->rect();
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    if (this->calendarWidget() && this->calendarWidget()->isVisible()) {
        focusFlag = true;
    }
    if (focusFlag == true) {
        QPen pen(palette().brush(QPalette::Active, QPalette::Highlight), 2);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.translate(1, 1);
        painter.drawRoundedRect(rectBoxt.adjusted(0, 0, -2, -2), 6, 6);
    } else if (hoverFlag == true) {
        painter.setPen(palette().color(QPalette::Active, QPalette::Highlight));
        painter.setBrush(Qt::NoBrush);
        painter.translate(0.5, 0.5);
        painter.drawRoundedRect(rectBoxt.adjusted(0, 0, -1, -1), 6, 6);
    }
}


QPixmap DateEdit::loadSvg(const QString &path, int size) {
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

QPixmap DateEdit::drawSymbolicColoredPixmap(const QPixmap &source) {
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


bool DateEdit::eventFilter(QObject *obj, QEvent *event) {
    if (QEvent::HoverEnter == event->type()) {
        hoverFlag = true;
        repaint();
    } else if (QEvent::HoverLeave == event->type()){
        hoverFlag = false;
        repaint();
    } else if (QEvent::FocusIn == event->type()) {
        focusFlag = true;
        repaint();
    } else if (QEvent::FocusOut == event->type()) {
        focusFlag = false;
        hoverFlag = false;
        repaint();
        if (this->date() != QDateTime::currentDateTime().date() && !this->calendarWidget()->isVisible()) {
            Q_EMIT changeDate();
        }
    }


    return QObject::eventFilter(obj,event);
}
