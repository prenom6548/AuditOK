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
#ifndef KBTITLE_H
#define KBTITLE_H

#include <QWidget>
#include <QPushButton>
#include <QMap>

class KBTitle : public QWidget
{
    Q_OBJECT
public:
    KBTitle(QWidget *parent = nullptr);
    virtual ~KBTitle();

public:
    void adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical = false, bool floatStatus = false);

public Q_SLOTS:
    void onBtnClicked();

Q_SIGNALS:
    void btnClicked(QString keyName);

private:
    void initUI();
    void initConnections();

private:
    QPushButton *m_btnFloat = nullptr;
    QPushButton *m_btnClose = nullptr;
    QMap<QPushButton*, QRect> m_mapSubGeometrys;
    bool floatStatus = false;
};

#endif // KBTITLE_H
