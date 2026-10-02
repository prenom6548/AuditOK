/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef KTOOLTIP_H
#define KTOOLTIP_H

#include <QLabel>

class QLabel;
class KLabel;

class Tooltip : public QWidget
{
public:
    Tooltip(QWidget *parent = nullptr);
    ~Tooltip();

    void setText(const QString &text, const int fontSize);
    void popup(Tooltip *kToolTip, const QString &text, const int fontSize);
    void tipClose(Tooltip *kToolTip);
protected:
    void paintEvent(QPaintEvent * event);
private:
    KLabel *tipLabel;
};

#endif // KTOOLTIP_H
