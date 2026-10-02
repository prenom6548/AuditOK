/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef HOTCITYBTN_H
#define HOTCITYBTN_H

#include <QObject>
#include <QPushButton>
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include "global/viewvar.h"
class HotcityBtn : public QPushButton
{
    Q_OBJECT
    friend class TestHotcityBtn;
public:
    HotcityBtn(QWidget *parent, QString name, QString code);
    //    void onThemeChanged(QString str);
signals:
    void hotCityClick(QString);

public slots:
    void onHotCityClick();

private:
    QString cityCode = "";
    QString cityName = "";
};

#endif // HOTCITYBTN_H
