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
#ifndef DATEEDIT_H
#define DATEEDIT_H
#include <QDateEdit>
#include <QStyle>
#include <QDBusInterface>

class DateEdit : public QDateEdit {
    Q_OBJECT
public :
    DateEdit(QWidget *parent = nullptr);
    ~DateEdit();
    QPixmap loadSvg(const QString &path, int size);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
private Q_SLOTS:
    void mode_change_signal_slots(bool b);
protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    bool hoverFlag = false;
    bool focusFlag = false;
    QDBusInterface *m_statusSessionDbus = nullptr;
Q_SIGNALS:
    void changeDate();
};

#endif // DATEEDIT_H
