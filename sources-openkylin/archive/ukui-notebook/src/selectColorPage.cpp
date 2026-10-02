/*
* Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QPainterPath>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

#include "selectColorPage.h"


QString SelectColor::KY_BLUE = "#3790FA";
QString SelectColor::KY_PURPLE = "#722ED1";
QString SelectColor::KY_PINK = "#EB3096";
QString SelectColor::KY_RED = "#F3222D";
QString SelectColor:: KY_ORANGE = "#F68C27";
QString SelectColor::KY_YELLOW = "#F9C53D";
QString SelectColor::KY_GREEN = "#52C429";
QString SelectColor::KY_GREY = "#8C8C8C";

SelectColor::SelectColor(QWidget *parent) :
      CustomColorPanel(parent),
      m_parentWindow(parent)
{
    initSetup();
}

SelectColor::~SelectColor()
{

}

void SelectColor::initSetup()
{
//    setFixedSize(48, 272);
    setWindowFlags(Qt::Popup);
    addColor(KY_BLUE, true);
    addColor(KY_RED);
    addColor(KY_GREEN);
    addColor(KY_ORANGE);
    addColor(KY_PURPLE);
    addColor(KY_YELLOW);
    addColor(KY_GREY);
    addColor(KY_PINK);

}

void SelectColor::showEvent(QShowEvent *event)
{
    CustomColorPanel::showEvent(event);
}
