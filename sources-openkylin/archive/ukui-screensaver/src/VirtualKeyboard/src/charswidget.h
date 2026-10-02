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
#ifndef CHARSWIDGET_H
#define CHARSWIDGET_H

#include <QWidget>
#include "kbbutton.h"
#include <QMap>

class CharsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CharsWidget(QWidget *parent = nullptr);
    virtual ~CharsWidget();

    void adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical = false, bool floatStatus = false);

public Q_SLOTS:
    void onBtnClicked(QChar charId);

Q_SIGNALS:
    void clicked(int nKeyId);
    void specialBtnClicked(QString keyName);
    void normalBtnClicked(QChar c);

protected:
    void changeEvent(QEvent *event);

private:
    void initUI();
    void refreshTranslate();

private:
    QMap<KBButton*, QRect> m_mapBtnGeometrys;

};

#endif // CHARSWIDGET_H
