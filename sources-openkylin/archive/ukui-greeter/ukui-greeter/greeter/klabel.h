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
#ifndef KLABEL_H
#define KLABEL_H

#include <QLabel>

class Configuration;

class KLabel : public QLabel
{
    Q_OBJECT
public:
    KLabel(QWidget *parent = nullptr);
    void setFontSize(int fontSize);

private:
    Configuration *configuration;
    QFont sysFont;
    double m_ptToPx = 1.0;
};

#endif // KLABEL_H
