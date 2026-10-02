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
#include "onehourweather.h"
#include <QDebug>
#include <QThread>
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
onehourweather::onehourweather(QWidget *parent) : QWidget(parent)
{
    m_onehourLayout = new QVBoxLayout(this);

    m_time = new QLabel(this);
    m_time->setAccessibleName("kylin-weather_QLabel_Onehourweather_time");
    m_icon = new QLabel(this);
    m_icon->setAccessibleName("kylin-weather_QLabel_Onehourweather_icon");
    m_temperature = new QLabel(this);
    m_temperature->setAccessibleName("kylin-weather_QLabel_Onehourweather_temperature");

    m_time->setText("N/A");
    m_time->setAlignment(Qt::AlignCenter);

    QPixmap m_iconPixmap(":/res/weather-icon/PNG48/1x/999.png");
    m_icon->setPixmap(m_iconPixmap);
    m_icon->setAlignment(Qt::AlignCenter);

    m_temperature->setText("N/A");
    m_temperature->setAlignment(Qt::AlignCenter);
    initConnect();
    initLayout();
}

void onehourweather::initLayout()
{
    m_onehourLayout->setMargin(0);
    m_onehourLayout->addWidget(m_time);
    m_onehourLayout->addSpacing(-16);
    m_onehourLayout->addWidget(m_icon);
    m_onehourLayout->addSpacing(-16);
    m_onehourLayout->addWidget(m_temperature);
    this->setLayout(m_onehourLayout);
}

void onehourweather::resizeEvent(QResizeEvent *event)
{
    initQss(0.0);
}

void onehourweather::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    initQss(14.0);
}

void onehourweather::lable2SetFontSizeSlot(int size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = 11;                              //魔鬼数字，自行处理
    double nowFontSize = lable2BaseFontSize * double(size) / 11; // 11为系统默认大小，魔鬼数字，自行处理
    initQss(nowFontSize);
}

void onehourweather::initConnect()
{
    connect(ViewVar::getInstance(), &ViewVar::setFontSizeSignal, this, &onehourweather::lable2SetFontSizeSlot);
}

void onehourweather::initQss(double nowFontSize)
{
    QFont m_timeFont;
    m_time->setFont(m_timeFont);
    m_time->setStyleSheet("color: white");
    m_temperature->setStyleSheet("color: white");
//    QPalette timePalette = m_time->palette();
//    timePalette.setColor(QPalette::WindowText, Qt::white);
//    m_time->setPalette(timePalette);

//    QPalette tempPalette = m_temperature->palette();
//    tempPalette.setColor(QPalette::WindowText, Qt::white);
//    m_temperature->setPalette(tempPalette);
    m_temperature->setFont(m_timeFont);
    m_time->setAlignment(Qt::AlignCenter);
    m_temperature->setAlignment(Qt::AlignCenter);
}
