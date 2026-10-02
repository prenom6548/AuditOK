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
#ifndef ONEDAYWEATHER_H
#define ONEDAYWEATHER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QGSettings/QGSettings>
#include "global/viewvar.h"
class onedayweather : public QWidget
{
    Q_OBJECT
public:
    explicit onedayweather(QWidget *parent = nullptr);
    void isFullScreen(int);
    int m_judgeFullScreen;
    QLabel *m_dayOfWeek;
    QLabel *m_date;
    QLabel *m_weatherIcon;
    QLabel *m_temp;
    QLabel *m_weather;
    QLabel *m_wind;

private:
    QVBoxLayout *m_onedayLayout;

    QFont m_font;
    void initLayout();
    void initQss(double, double);
    void initConnect();
    void lable2SetFontSizeSlot(int size);
    QGSettings *m_fontSize = nullptr;

signals:
};

#endif // ONEDAYWEATHER_H
