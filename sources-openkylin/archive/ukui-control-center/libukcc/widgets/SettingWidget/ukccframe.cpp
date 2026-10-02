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
#include "ukccframe.h"
#include <QPainter>
#include <QDBusReply>
#include <QDebug>
#include <QStyleOption>

UkccFrame::UkccFrame(QWidget *parent, BorderRadiusStyle style, bool heightAdaptive) :
    QFrame(parent),
    mRadiusType(style),
    mHeightAdaptive(heightAdaptive),
    mIscontainer(false)
{
    this->setFrameShape(QFrame::Box);
    mStatusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                             "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(), this);
    if (mStatusSessionDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = mStatusSessionDbus->call("get_current_tabletmode");
        mode_change_signal_slots(is_tabletmode.isValid() ? is_tabletmode.value() : false);
        connect(mStatusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mode_change_signal_slots(bool)));
    } else {
        mode_change_signal_slots(false);
//        qWarning() << "Create com.kylin.statusmanager.interface Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }

}

UkccFrame::~UkccFrame()
{

}

void UkccFrame::mode_change_signal_slots(bool b)
{
    if (mHeightAdaptive) {
        if (b) {
            this->setMinimumSize(550, TABLET_HEIGHT);
            this->setMaximumSize(16777215, TABLET_HEIGHT);
        } else {
            this->setMinimumSize(550, PC_HEIGHT);
            this->setMaximumSize(16777215, PC_HEIGHT);
        }
    }
    mIsTabletMode = b;
    Q_EMIT tabletModeChanged(b);
}


void UkccFrame::setRadiusType(BorderRadiusStyle style)
{
    mRadiusType = style;
    update();
}

void UkccFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    if (this->frameShape() != QFrame::NoFrame) {
        QStyleOption opt;
        opt.initFrom(this);
        const QPalette::ColorGroup cg = !(opt.state & QStyle::State_Enabled)
                ? QPalette::Disabled
                : (opt.state & QStyle::State_Active ? QPalette::Active : QPalette::Inactive);
        painter.setBrush(palette().brush(cg, QPalette::Base));
    }
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();

    int normalRadius = qApp->property("kradius-normal").toInt();
    if (mRadiusType == UkccFrame::None) {
        painter.drawRoundedRect(rect, 0, 0);
    } else {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, normalRadius, normalRadius);

        painterPath.setFillRule(Qt::WindingFill);
        if (mRadiusType == UkccFrame::Bottom) {
            painterPath.addRect(rect.width() - normalRadius, 0, normalRadius, normalRadius);
            painterPath.addRect(0, 0, normalRadius, normalRadius);
        } else if (mRadiusType == UkccFrame::Top) {
            painterPath.addRect(0, rect.height() - normalRadius, normalRadius, normalRadius);
            painterPath.addRect(rect.width() - normalRadius, rect.height() - normalRadius, normalRadius, normalRadius);
        }

        painter.drawPath(painterPath);
    }
    return QFrame::paintEvent(e);
}
