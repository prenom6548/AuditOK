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
#ifndef CHARSMOREWIDGETS_H
#define CHARSMOREWIDGETS_H

#include <QWidget>
#include "kbbutton.h"
#include <QMap>

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QFrame;
class CharsMoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CharsMoreWidget(QWidget *parent = nullptr);
    ~CharsMoreWidget();
    void adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical = false, bool floatStatus = false);

public Q_SLOTS:
    void onBtnClicked(QChar charId);

Q_SIGNALS:
    void clicked(int nKeyId);
    void specialBtnClicked(QString keyName);
    void normalBtnClicked(QChar charId);

private:
    void initUI();

private:
    QMap<QWidget*, QRect> m_mapBtnGeometrys;
    QMap<QWidget*, QRect> m_mapSubWidgetListRects;
    QVBoxLayout *m_vlayoutBtnList = nullptr;
    QScrollArea *m_scrollFrame = nullptr;
    QFrame *listFrame = nullptr;
};

#endif // CHARSMOREWIDGETS_H
