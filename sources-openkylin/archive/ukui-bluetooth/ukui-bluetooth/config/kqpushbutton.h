/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef KQPUSHBUTTON_H
#define KQPUSHBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QEvent>

class KQPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KQPushButton(const QString &text, QWidget *parent = nullptr);
    explicit KQPushButton(QWidget *parent = nullptr);

protected:
    bool event(QEvent *e);
signals:
    void hovered(bool);
};

#endif // KQPUSHBUTTON_H
