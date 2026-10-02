/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "popuptip.h"
#include "utils.h"

#include <QApplication>
#include <QLabel>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QTimer>
#include <sys/syslog.h>

using namespace kmre;


PopupTip::PopupTip() : QFrame(nullptr)
  , m_label(new QLabel(this))
{
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    setContentsMargins(10, 10, 10, 10);
    setFixedSize(340, 80);


    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_label->adjustSize();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(this->m_label);
    setLayout(layout);
}

void PopupTip::popup(const QPoint &pos, int showTime)
{
    const QSize size = this->size();
    move(pos.x() - size.width()/2, pos.y() - size.height());
    raise();
    show();

    if (showTime > 0) {
        QTimer::singleShot(showTime, [=] {
            hide();
        });
    }
}

void PopupTip::setTipMessage(const QString &content)
{
    m_label->setText(content);
}

void PopupTip::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        this->hide();
    }

    QWidget::keyPressEvent(event);
}

void PopupTip::mousePressEvent(QMouseEvent *event)
{
    this->hide();
    QWidget::mousePressEvent(event);
}

void PopupTip::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(Qt::gray));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 6, 6);

    QWidget::paintEvent(event);
}
