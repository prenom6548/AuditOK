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
#ifndef ONEHOURWEATHER_H
#define ONEHOURWEATHER_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGSettings/QGSettings>
#include "global/viewvar.h"
class onehourweather : public QWidget
{
    Q_OBJECT
public:
    explicit onehourweather(QWidget *parent = nullptr);
    void isFullScreen(int);
    int m_judgeFullScreen;
    QLabel *m_time;
    QLabel *m_icon;
    QLabel *m_temperature;

private:
    //    QLabel *m_time;
    //    QLabel *m_icon;
    //    QLabel *m_temperature;
    QWidget *m_onehourWid;
    QVBoxLayout *m_onehourLayout1;
    QVBoxLayout *m_onehourLayout2;
    QVBoxLayout *m_onehourLayout3;
    QVBoxLayout *m_onehourLayout4;
    QVBoxLayout *m_onehourLayout5;
    QVBoxLayout *m_onehourLayout6;
    QVBoxLayout *m_onehourLayout7;
    QVBoxLayout *m_onehourLayout8;
    QVBoxLayout *m_onehourLayout;
    void initLayout();
    void initQss(double);
    void resizeEvent(QResizeEvent *event);
    void initConnect();
    void lable2SetFontSizeSlot(int size);
    QGSettings *m_fontSize = nullptr;


signals:
    void setFontSizeSignal(int);
};

#endif // ONEHOURWEATHER_H
