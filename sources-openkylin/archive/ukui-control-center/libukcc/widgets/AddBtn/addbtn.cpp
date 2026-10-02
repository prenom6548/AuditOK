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

#include "addbtn.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QPainter>
#include <QGSettings/QGSettings>

#define TOP_LEFT_CORNER 1
#define TOP_RIGHT_CORNER 2
#define BOTTOM_RIGHT_CORNER 4
#define BOTTOM_LEFT_CORNER 8

AddBtn::AddBtn(QWidget *parent):
    QPushButton(parent)
{
    this->setObjectName("AddBtn");
    this->setMinimumSize(QSize(580, 60));
    this->setMaximumSize(QSize(16777215, 60));
    this->setBtnStyle(Box);
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel *iconLabel = new QLabel();
    QLabel *textLabel = new QLabel(tr("Add"));

    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    iconLabel->setPixmap(mAddIcon.pixmap(mAddIcon.actualSize(QSize(16, 16))));
    iconLabel->setProperty("iconHighlightEffectMode", 1);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode){
        iconLabel->setProperty("useIconHighlightEffect", true);
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", true);
            } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", false);
            }
        }
    });

    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    this->setLayout(addLyt);
    this->setAccessibleName(tr("Add"));
}

AddBtn::~AddBtn()
{

}

void AddBtn::enterEvent(QEvent *event){
    Q_EMIT enterWidget();

    QPushButton::enterEvent(event);
}

void AddBtn::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setClipping(true);
    p.setPen(Qt::transparent);
    QPalette pal;
    p.setBrush(pal.base());
    QPainterPath picPath = makeRoundedRect(this->rect(), 6, mCorner);
    p.setClipPath(picPath);
    p.drawPath(picPath);

    QPushButton::paintEvent(event);
}

void AddBtn::setBtnStyle(AddBtn::Shape type)
{
    switch (type) {
    case None:
        mCorner = 0;
        break;
    case Top:
        mCorner = TOP_LEFT_CORNER | TOP_RIGHT_CORNER;
        break;
    case Bottom:
        mCorner = BOTTOM_RIGHT_CORNER | BOTTOM_LEFT_CORNER;
        break;
    case Box:
        mCorner = TOP_LEFT_CORNER | TOP_RIGHT_CORNER | BOTTOM_RIGHT_CORNER | BOTTOM_LEFT_CORNER;
        break;
    }
    repaint();
}

QPainterPath AddBtn::makeRoundedRect(const QRect &rect, int radius, int corner)
{
    QPainterPath path;
    if(corner & TOP_LEFT_CORNER) {
        path.moveTo(rect.x() + radius,rect.y());
        path.arcTo(rect.x(), rect.y(), radius*2, radius*2, 90.f, 90.f);//画左上圆角
    } else {
        path.moveTo(rect.x(), rect.y());
    }

    if(corner & BOTTOM_LEFT_CORNER) {
        path.lineTo(rect.x(), rect.y() + rect.height() -radius);
        path.arcTo(rect.x(), rect.y() + rect.height() - radius *2, radius*2, radius*2, 180.0f, 90.f);
    } else {
        path.lineTo(rect.x(), rect.y() + rect.height());
    }

    if(corner & BOTTOM_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width() - radius,rect.y() + rect.height());
        path.arcTo(rect.x() + rect.width() - 2*radius, rect.y() + rect.height() - 2*radius,radius*2, radius*2, 270.0f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y() + rect.height());
    }

    if(corner & TOP_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width(), rect.y() + radius);
        path.arcTo(rect.x() + rect.width() - 2*radius, rect.y(), radius*2,radius*2, 0.f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y());
    }
    return path;
}

void AddBtn::leaveEvent(QEvent *event){
    Q_EMIT leaveWidget();

    QPushButton::leaveEvent(event);
}
