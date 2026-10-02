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
#include "onedayweather.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
onedayweather::onedayweather(QWidget *parent) : QWidget(parent)
{
    m_onedayLayout = new QVBoxLayout(this);

    m_dayOfWeek = new QLabel(this);
    m_dayOfWeek->setAccessibleName("kylin-weather_QLabel_Onedayweather_dayOfWeek");
    QString m_strDayOfWeek = "N/A";
    m_dayOfWeek->setText(m_strDayOfWeek);
    m_dayOfWeek->setStyleSheet("color: white;");
    m_dayOfWeek->setAlignment(Qt::AlignCenter);
    m_date = new QLabel(this);
    m_date->setAccessibleName("kylin-weather_QLabel_Onedayweather_date");
    QString m_strDate = "N/A";
    m_date->setText(m_strDate);
    m_date->setAlignment(Qt::AlignCenter);
    m_weatherIcon = new QLabel(this);
    m_weatherIcon->setAccessibleName("kylin-weather_QLabel_Onedayweather_weatherIcon");
    QPixmap pixmap(":/res/weather-icon/PNG48/1x/999.png");
    m_weatherIcon->setPixmap(pixmap);
    m_weatherIcon->setAlignment(Qt::AlignCenter);
    m_temp = new QLabel(this);
    m_temp->setAccessibleName("kylin-weather_QLabel_Onedayweather_temp");
    QString m_strTemp = "N/A";
    m_temp->setText(m_strTemp);
    m_temp->setAlignment(Qt::AlignCenter);
    m_weather = new QLabel(this);
    m_weather->setAccessibleName("kylin-weather_QLabel_Onedayweather_weather");
    QString m_strWeather = "N/A";
    m_weather->setText(m_strWeather);
    m_weather->setAlignment(Qt::AlignCenter);
    m_wind = new QLabel(this);
    m_wind->setAccessibleName("kylin-weather_QLabel_Onedayweather_wind");
    QString m_strWind = "N/A";
    m_wind->setText(m_strWind);
    m_wind->setAlignment(Qt::AlignCenter);
    initConnect();
    initLayout();
    //    initQss();
}

void onedayweather::initLayout()
{
    m_onedayLayout->addWidget(m_dayOfWeek);
    m_onedayLayout->addWidget(m_date);
    m_onedayLayout->addWidget(m_weatherIcon);
    m_onedayLayout->addWidget(m_temp);
    m_onedayLayout->addWidget(m_weather);
    m_onedayLayout->addWidget(m_wind);
    m_onedayLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_onedayLayout);
}

void onedayweather::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    initQss(14.0, 1.0);
}

void onedayweather::lable2SetFontSizeSlot(int size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = 11;                              //魔鬼数字，自行处理
    double nowFontSize = lable2BaseFontSize * double(size) / 11; // 11为系统默认大小，魔鬼数字，自行处理
    double times = double(size) / 11;
    //    QFont font;
    //    font.setPointSizeF(nowFontSize);
    //    ui->label2->setFont(font);
    initQss(nowFontSize, times);
}

void onedayweather::initConnect()
{
    connect(ViewVar::getInstance(), &ViewVar::setFontSizeSignal, this, &onedayweather::lable2SetFontSizeSlot);
}

void onedayweather::initQss(double nowFontSize, double times)
{

    m_font.setPointSize(int(11 * times));
    m_dayOfWeek->setFont(m_font);
    //    设置居中
    m_dayOfWeek->setAlignment(Qt::AlignCenter);
    //    设置显示白色字体
    QPalette dayOfWeekPalette = m_dayOfWeek->palette();
    dayOfWeekPalette.setColor(QPalette::WindowText, Qt::white);
    m_dayOfWeek->setPalette(dayOfWeekPalette);

    //    设置字体白色
    m_date->setStyleSheet("color: white;");
    //    设置字体透明度
    QGraphicsOpacityEffect *dateOpacityEffect = new QGraphicsOpacityEffect;
    dateOpacityEffect->setOpacity(0.7);
    m_date->setGraphicsEffect(dateOpacityEffect);
    //    设置居中
    m_date->setAlignment(Qt::AlignCenter);
    QFont m_dateAndTemp;
    m_date->setFont(m_dateAndTemp);

    m_weatherIcon->setAlignment(Qt::AlignCenter);

    m_temp->setStyleSheet("color: white;");
    m_temp->setAlignment(Qt::AlignCenter);
    m_temp->setFont(m_dateAndTemp);

    m_weather->setStyleSheet("color: white;");
    m_font.setPointSize(int(9 * times));
    m_weather->setFont(m_font);
    m_weather->setAlignment(Qt::AlignCenter);
    //    设置字体透明度
    QGraphicsOpacityEffect *weatherOpacityEffect = new QGraphicsOpacityEffect;
    weatherOpacityEffect->setOpacity(0.7);
    m_weather->setGraphicsEffect(weatherOpacityEffect);

    m_wind->setStyleSheet("color: white;");
    m_font.setPointSize(int(9 * times));
    m_wind->setFont(m_font);
    m_wind->setAlignment(Qt::AlignCenter);
    //设置字体透明度
    QGraphicsOpacityEffect *windOpacityEffect = new QGraphicsOpacityEffect;
    windOpacityEffect->setOpacity(0.7);
    m_wind->setGraphicsEffect(windOpacityEffect);
}
