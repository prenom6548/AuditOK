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
#include "myqradiobutton.h"

#include <QKeyEvent>
#include <QFocusEvent>
#include <algorithm>

MyQRadioButton::MyQRadioButton(QWidget *parent):
    QRadioButton(parent)
{
    // 互斥由同组点击/空格处理；关闭后方向键移焦才不会连带选中
    setAutoExclusive(false);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QAbstractButton::toggled, this, [this](bool) {
        refreshGroupKeyboardNav();
    });
}

void MyQRadioButton::leaveEvent(QEvent *event)
{
    Q_EMIT leaveWidget();

    QRadioButton::leaveEvent(event);
}

void MyQRadioButton::enterEvent(QEvent *event)
{
     Q_EMIT enterWidget();

    QRadioButton::enterEvent(event);
}

void MyQRadioButton::refreshGroupKeyboardNav()
{
    QWidget *p = parentWidget();
    if (!p) {
        setFocusPolicy(Qt::StrongFocus);
        return;
    }

    QList<MyQRadioButton *> peers;
    for (QObject *child : p->children()) {
        auto *rb = qobject_cast<MyQRadioButton *>(child);
        if (rb) {
            peers.append(rb);
        }
    }
    if (peers.isEmpty()) {
        return;
    }

    MyQRadioButton *checked = nullptr;
    for (MyQRadioButton *rb : peers) {
        if (rb->isChecked()) {
            checked = rb;
            break;
        }
    }
    for (MyQRadioButton *rb : peers) {
        rb->setAutoExclusive(false);
        const bool tabStop = checked ? (rb == checked) : (rb == peers.first());
        rb->setFocusPolicy(tabStop ? Qt::StrongFocus : Qt::ClickFocus);
    }
}

void MyQRadioButton::moveFocusInGroup(int key)
{
    QWidget *p = parentWidget();
    if (!p) {
        return;
    }

    QList<MyQRadioButton *> peers;
    for (QObject *child : p->children()) {
        auto *rb = qobject_cast<MyQRadioButton *>(child);
        if (rb && rb->isVisible() && rb->isEnabled()) {
            peers.append(rb);
        }
    }
    if (peers.size() < 2) {
        return;
    }

    std::sort(peers.begin(), peers.end(), [](MyQRadioButton *a, MyQRadioButton *b) {
        const QPoint pa = a->mapToGlobal(QPoint(0, 0));
        const QPoint pb = b->mapToGlobal(QPoint(0, 0));
        if (pa.y() != pb.y()) {
            return pa.y() < pb.y();
        }
        return pa.x() < pb.x();
    });

    const int idx = peers.indexOf(this);
    if (idx < 0) {
        return;
    }

    int next = idx;
    if (key == Qt::Key_Left || key == Qt::Key_Up) {
        next = (idx - 1 + peers.size()) % peers.size();
    } else if (key == Qt::Key_Right || key == Qt::Key_Down) {
        next = (idx + 1) % peers.size();
    } else {
        return;
    }

    peers.at(next)->setFocus(Qt::TabFocusReason);
}

void MyQRadioButton::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        moveFocusInGroup(event->key());
        event->accept();
        return;
    default:
        break;
    }
    QRadioButton::keyPressEvent(event);
}

void MyQRadioButton::focusInEvent(QFocusEvent *event)
{
    QRadioButton::focusInEvent(event);
    update();
}

void MyQRadioButton::focusOutEvent(QFocusEvent *event)
{
    QRadioButton::focusOutEvent(event);
    update();
}
