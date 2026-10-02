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
#ifndef DRAGWIDGET_H
#define DRAGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMap>

class DragWidget : public QWidget
{
    Q_OBJECT
public:
    DragWidget(QWidget *parent = nullptr);
    virtual ~DragWidget();

public:
    void adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical = false, bool floatStatus = false);

private:
    void initUI();

private:
    QLabel *m_labelDrag = nullptr;
    QMap<QLabel*, QRect> m_mapSubGeometrys;
};

#endif // DRAGWIDGET_H
