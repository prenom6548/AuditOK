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
#include "splitline.h"

splitline::splitline(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(1, 180);
    m_splitline = new QLabel(this);
    m_splitline->setAccessibleName("kylin-weather_QLabel_splitline");
    m_splitline->setFixedSize(1, 180);
    //    m_splitline->move(this->width()/2,(this->height()-m_splitline->height())/2);

    initLayout();
    initQss();
}

void splitline::initLayout() {}

void splitline::initQss()
{
    lineLinearGradientQss =
        "border:none;background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, "
        "0), stop:0.5 rgba(255, 255, 255, 0.7), stop:1 rgba(255, 255, 255, 0));";
    m_splitline->setStyleSheet(lineLinearGradientQss);
    //    m_splitline->setAlignment(Qt::AlignCenter);
}
