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
#include "weekweather.h"
#include <iostream>
#include <QString>
#include <QVector>
#include <QTime>
#include <QPainter>
#include <QStyleOption>
#include <QBitmap>

weekweather::weekweather(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
    this->setWindowFlags(Qt::FramelessWindowHint);   //设置无边框窗口
    this->setFixedSize(856, 216);

    m_weekweatherLayout = new QHBoxLayout();

    m_onedayweatherfirst = new onedayweather(this);
    m_onedayweatherfirst->setAccessibleName("kylin-weather_weekweather_onedayweatherfirst");
    m_splitlinefirst = new splitline(this);
    m_splitlinefirst->setAccessibleName("kylin-weather_weekweather_splitlinefirst");
    m_onedayweathertwo = new onedayweather(this);
    m_onedayweathertwo->setAccessibleName("kylin-weather_weekweather_onedayweathertwo");
    m_splitlinetwo = new splitline(this);
    m_splitlinetwo->setAccessibleName("kylin-weather_weekweather_splitlinetwo");
    m_onedayweatherthree = new onedayweather(this);
    m_onedayweatherthree->setAccessibleName("kylin-weather_weekweather_onedayweatherthree");
    m_splitlinethree = new splitline(this);
    m_splitlinethree->setAccessibleName("kylin-weather_weekweather_splitlinethree");
    m_onedayweatherfour = new onedayweather(this);
    m_onedayweatherfour->setAccessibleName("kylin-weather_weekweather_onedayweatherfour");
    m_splitlinefour = new splitline(this);
    m_splitlinefour->setAccessibleName("kylin-weather_weekweather_splitlinefour");
    m_onedayweatherfive = new onedayweather(this);
    m_onedayweatherfive->setAccessibleName("kylin-weather_weekweather_onedayweatherfive");
    m_splitlinefive = new splitline(this);
    m_splitlinefive->setAccessibleName("kylin-weather_weekweather_splitlinefive");
    m_onedayweathersix = new onedayweather(this);
    m_onedayweathersix->setAccessibleName("kylin-weather_weekweather_onedayweathersix");
    m_splitlinesix = new splitline(this);
    m_splitlinesix->setAccessibleName("kylin-weather_weekweather_splitlinesix");
    m_onedayweatherseven = new onedayweather(this);
    m_onedayweatherseven->setAccessibleName("kylin-weather_weekweather_onedayweatherseven");

    initLayout();
    initConnect();
    initQss();
}


void weekweather::getForcastInfor(ForecastWeather m_forecastWeather, int m_isFullScreen)
{
    forecastWeather = m_forecastWeather;
    // 第 1 天
    QString firstWeather;
    QTime current_time = QTime::currentTime();
    int hour = current_time.hour();
    if (hour >= 6 && hour <= 18) {
        firstWeather = m_forecastWeather.first.cond_txt_d;
        firstIconPath = ViewVar::icon_path + m_forecastWeather.first.cond_code_d + ".png";
    } else {
        firstWeather = m_forecastWeather.first.cond_txt_n;
        firstIconPath = ViewVar::icon_path + m_forecastWeather.first.cond_code_n + ".png";
    }
    m_onedayweatherfirst->m_weatherIcon->setPixmap(QPixmap(firstIconPath));
    m_onedayweatherfirst->m_temp->setText(m_forecastWeather.first.tmp_min + "°~" + m_forecastWeather.first.tmp_max
                                          + "°");
    m_onedayweatherfirst->m_weather->setText(firstWeather);
    m_onedayweatherfirst->m_wind->setText(m_forecastWeather.first.wind_dir);
    QStringList dateAndWeekTextListFirst = dateAndWeekText(m_forecastWeather.first.date);
    m_onedayweatherfirst->m_date->setText(dateAndWeekTextListFirst.at(0));
    m_onedayweatherfirst->m_dayOfWeek->setText(tr("Today"));

    // 第 2 天
    QString twoWeather;
    if (hour >= 6 && hour <= 18) { // 白天
        twoWeather = m_forecastWeather.second.cond_txt_d;
        twoIconPath = ViewVar::icon_path + m_forecastWeather.second.cond_code_d + ".png";
    } else { // 晚上
        twoWeather = m_forecastWeather.second.cond_txt_n;
        twoIconPath = ViewVar::icon_path + m_forecastWeather.second.cond_code_n + ".png";
    }
    m_onedayweathertwo->m_weatherIcon->setPixmap(QPixmap(twoIconPath));
    m_onedayweathertwo->m_temp->setText(m_forecastWeather.second.tmp_min + "°~" + m_forecastWeather.second.tmp_max
                                        + "°");
    m_onedayweathertwo->m_weather->setText(twoWeather);
    m_onedayweathertwo->m_wind->setText(m_forecastWeather.second.wind_dir);

    QStringList dateAndWeekTextListSecond = dateAndWeekText(m_forecastWeather.second.date);
    m_onedayweathertwo->m_date->setText(dateAndWeekTextListSecond.at(0));
    m_onedayweathertwo->m_dayOfWeek->setText(dateAndWeekInt(m_forecastWeather.second.date));
    // 第 3 天
    QString threeWeather;
    if (hour >= 6 && hour <= 18) {
        threeWeather = m_forecastWeather.third.cond_txt_d;
        threeIconPath = ViewVar::icon_path + m_forecastWeather.third.cond_code_d + ".png";
    } else {
        threeWeather = m_forecastWeather.third.cond_txt_n;
        threeIconPath = ViewVar::icon_path + m_forecastWeather.third.cond_code_n + ".png";
    }
    m_onedayweatherthree->m_weatherIcon->setPixmap(QPixmap(threeIconPath));
    m_onedayweatherthree->m_temp->setText(m_forecastWeather.third.tmp_min + "°~" + m_forecastWeather.third.tmp_max
                                          + "°");
    m_onedayweatherthree->m_weather->setText(threeWeather);
    m_onedayweatherthree->m_wind->setText(m_forecastWeather.third.wind_dir);

    QStringList dateAndWeekTextListThree = dateAndWeekText(m_forecastWeather.third.date);
    m_onedayweatherthree->m_date->setText(dateAndWeekTextListThree.at(0));
    m_onedayweatherthree->m_dayOfWeek->setText(dateAndWeekInt(m_forecastWeather.third.date));
    // 第 4 天
    QString fourWeather;
    if (hour >= 6 && hour <= 18) {
        fourWeather = m_forecastWeather.fourth.cond_txt_d;
        fourIconPath = ViewVar::icon_path + m_forecastWeather.fourth.cond_code_d + ".png";
    } else {
        fourWeather = m_forecastWeather.fourth.cond_txt_n;
        fourIconPath = ViewVar::icon_path + m_forecastWeather.fourth.cond_code_n + ".png";
    }
    m_onedayweatherfour->m_weatherIcon->setPixmap(QPixmap(fourIconPath));
    m_onedayweatherfour->m_temp->setText(m_forecastWeather.fourth.tmp_min + "°~" + m_forecastWeather.fourth.tmp_max
                                         + "°");
    m_onedayweatherfour->m_weather->setText(fourWeather);
    m_onedayweatherfour->m_wind->setText(m_forecastWeather.fourth.wind_dir);

    QStringList dateAndWeekTextListFour = dateAndWeekText(m_forecastWeather.fourth.date);
    m_onedayweatherfour->m_date->setText(dateAndWeekTextListFour.at(0));
    m_onedayweatherfour->m_dayOfWeek->setText(dateAndWeekInt(m_forecastWeather.fourth.date));


    // 第 5 天
    QString fiveWeather;
    if (hour >= 6 && hour <= 18) {
        fiveWeather = m_forecastWeather.fifth.cond_txt_d;
        fiveIconPath = ViewVar::icon_path + m_forecastWeather.fifth.cond_code_d + ".png";
    } else {
        fiveWeather = m_forecastWeather.fifth.cond_txt_n;

        fiveIconPath = ViewVar::icon_path + m_forecastWeather.fifth.cond_code_n + ".png";
    }
    m_onedayweatherfive->m_weatherIcon->setPixmap(QPixmap(fiveIconPath));
    m_onedayweatherfive->m_temp->setText(m_forecastWeather.fifth.tmp_min + "°~" + m_forecastWeather.fifth.tmp_max
                                         + "°");
    m_onedayweatherfive->m_weather->setText(fiveWeather);
    m_onedayweatherfive->m_wind->setText(m_forecastWeather.fifth.wind_dir);
    QStringList dateAndWeekTextListFive = dateAndWeekText(m_forecastWeather.fifth.date);
    m_onedayweatherfive->m_date->setText(dateAndWeekTextListFive.at(0));
    m_onedayweatherfive->m_dayOfWeek->setText(dateAndWeekInt(m_forecastWeather.fifth.date));

    // 第 6 天
    QString sixWeather;
    if (hour >= 6 && hour <= 18) {
        sixWeather = m_forecastWeather.sixth.cond_txt_d;
        sixIconPath = ViewVar::icon_path + m_forecastWeather.sixth.cond_code_d + ".png";
    } else {
        sixWeather = m_forecastWeather.sixth.cond_txt_n;
        sixIconPath = ViewVar::icon_path + m_forecastWeather.sixth.cond_code_n + ".png";
    }
    m_onedayweathersix->m_weatherIcon->setPixmap(QPixmap(sixIconPath));
    m_onedayweathersix->m_temp->setText(m_forecastWeather.sixth.tmp_min + "°~" + m_forecastWeather.sixth.tmp_max + "°");
    m_onedayweathersix->m_weather->setText(sixWeather);
    m_onedayweathersix->m_wind->setText(m_forecastWeather.sixth.wind_dir);
    QStringList dateAndWeekTextListSix = dateAndWeekText(m_forecastWeather.sixth.date);
    m_onedayweathersix->m_date->setText(dateAndWeekTextListSix.at(0));
    m_onedayweathersix->m_dayOfWeek->setText(dateAndWeekInt(m_forecastWeather.sixth.date));

    // 第 7 天
    QString sevenWeather;
    if (hour >= 6 && hour <= 18) {
        sevenWeather = m_forecastWeather.seventh.cond_txt_d;
        sevenIconPath = ViewVar::icon_path + m_forecastWeather.seventh.cond_code_d + ".png";
    } else {
        sevenWeather = m_forecastWeather.seventh.cond_txt_n;
        sevenIconPath = ViewVar::icon_path + m_forecastWeather.seventh.cond_code_n + ".png";
    }
    m_onedayweatherseven->m_weatherIcon->setPixmap(QPixmap(sevenIconPath));
    m_onedayweatherseven->m_temp->setText(m_forecastWeather.seventh.tmp_min + "°~" + m_forecastWeather.seventh.tmp_max
                                          + "°");
    m_onedayweatherseven->m_weather->setText(sevenWeather);
    m_onedayweatherseven->m_wind->setText(m_forecastWeather.seventh.wind_dir);
    QStringList dateAndWeekTextListSeven = dateAndWeekText(m_forecastWeather.seventh.date);
    m_onedayweatherseven->m_date->setText(dateAndWeekTextListSeven.at(0));
    m_onedayweatherseven->m_dayOfWeek->setText(dateAndWeekInt(m_forecastWeather.seventh.date));
}

void weekweather::setWeatherIcon(int m_isFullScreen)
{
    {
        firstIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.first.cond_code_d + ".png";
        twoIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.second.cond_code_d + ".png";
        threeIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.third.cond_code_d + ".png";
        fourIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.fourth.cond_code_d + ".png";
        fiveIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.fifth.cond_code_d + ".png";
        sixIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.sixth.cond_code_d + ".png";
        sevenIconPath = ":/res/weather-icon/PNG48/1x/" + forecastWeather.seventh.cond_code_d + ".png";

        m_onedayweatherfirst->m_weatherIcon->resize(72, 72);
        QPixmap firstpixmap(firstIconPath);
        m_onedayweatherfirst->m_weatherIcon->setPixmap(firstpixmap);

        m_onedayweathertwo->m_weatherIcon->resize(72, 72);
        QPixmap twopixmap(twoIconPath);
        m_onedayweathertwo->m_weatherIcon->setPixmap(twopixmap);

        m_onedayweatherthree->m_weatherIcon->resize(72, 72);
        QPixmap threepixmap(threeIconPath);
        m_onedayweatherthree->m_weatherIcon->setPixmap(threepixmap);

        m_onedayweatherfour->m_weatherIcon->resize(72, 72);
        QPixmap fourpixmap(fourIconPath);
        m_onedayweatherfour->m_weatherIcon->setPixmap(fourpixmap);

        m_onedayweatherfive->m_weatherIcon->resize(72, 72);
        QPixmap fivepixmap(fiveIconPath);
        m_onedayweatherfive->m_weatherIcon->setPixmap(fivepixmap);

        m_onedayweathersix->m_weatherIcon->resize(72, 72);
        QPixmap sixpixmap(sixIconPath);
        m_onedayweathersix->m_weatherIcon->setPixmap(sixpixmap);

        m_onedayweatherseven->m_weatherIcon->resize(72, 72);
        QPixmap pixmap(sevenIconPath);
        m_onedayweatherseven->m_weatherIcon->setPixmap(pixmap);
    }
}

void weekweather::initLayout()
{
    m_weekweatherLayout->setMargin(0);
    m_weekweatherLayout->setSpacing(0);
    m_weekweatherLayout->addWidget(m_onedayweatherfirst, 1);
    m_weekweatherLayout->addWidget(m_splitlinefirst, 1);
    m_weekweatherLayout->addWidget(m_onedayweathertwo, 1);
    m_weekweatherLayout->addWidget(m_splitlinetwo, 1);
    m_weekweatherLayout->addWidget(m_onedayweatherthree, 1);
    m_weekweatherLayout->addWidget(m_splitlinethree, 1);
    m_weekweatherLayout->addWidget(m_onedayweatherfour, 1);
    m_weekweatherLayout->addWidget(m_splitlinefour, 1);
    m_weekweatherLayout->addWidget(m_onedayweatherfive, 1);
    m_weekweatherLayout->addWidget(m_splitlinefive, 1);
    m_weekweatherLayout->addWidget(m_onedayweathersix, 1);
    m_weekweatherLayout->addWidget(m_splitlinesix, 1);
    m_weekweatherLayout->addWidget(m_onedayweatherseven, 1);
    m_weekweatherLayout->setContentsMargins(15, 15, 15, 15);
    this->setLayout(m_weekweatherLayout);
}

void weekweather::initConnect()
{
    connect(this, &weekweather::sendIsFullScreen, m_onedayweatherfirst, &onedayweather::isFullScreen);
    connect(this, &weekweather::sendIsFullScreen, m_onedayweathertwo, &onedayweather::isFullScreen);
    connect(this, &weekweather::sendIsFullScreen, m_onedayweatherthree, &onedayweather::isFullScreen);
    connect(this, &weekweather::sendIsFullScreen, m_onedayweatherfour, &onedayweather::isFullScreen);
    connect(this, &weekweather::sendIsFullScreen, m_onedayweatherfive, &onedayweather::isFullScreen);
    connect(this, &weekweather::sendIsFullScreen, m_onedayweathersix, &onedayweather::isFullScreen);
    connect(this, &weekweather::sendIsFullScreen, m_onedayweatherseven, &onedayweather::isFullScreen);
}

void weekweather::resizeEvent(QResizeEvent *event) {}

QStringList weekweather::dateAndWeekText(QString data)
{
    QStringList dateAndWeekTextList;
    QDateTime timeSecond;
    timeSecond = QDateTime::fromString(data, "yyyy-MM-dd");
    if (timeSecond.toString("MM") == "" || timeSecond.toString("dd") == "") {
        dateAndWeekTextList.append("N/A");
        dateAndWeekTextList.append("N/A");
        return dateAndWeekTextList;
    } else {
        QString monthTwo = timeSecond.toString("MM");
        QString dateTwo = timeSecond.toString("dd");

        if ("0" == monthTwo.mid(0, 1)) {
            monthTwo = monthTwo.mid(1, 1);
        }
        if ("0" == dateTwo.mid(0, 1)) {
            dateTwo = dateTwo.mid(1, 1);
        }
        dateAndWeekTextList.append(monthTwo + tr("-") + dateTwo + tr(" "));
        dateAndWeekTextList.append(timeSecond.toString("dddd"));
        return dateAndWeekTextList;
    }
    dateAndWeekTextList.append("N/A");
    dateAndWeekTextList.append("N/A");
    return dateAndWeekTextList;
}

QString weekweather::dateAndWeekInt(QString data)
{
    int weekNum = 0;
    QString week;
    QDate dateWeek;
    dateWeek = QDate::fromString(data, "yyyy-MM-dd");
    weekNum = dateWeek.dayOfWeek();
    switch (weekNum)
    {
    case 1:
        week = tr("Monday");
        break;
    case 2:
        week = tr("Tuesday");
        break;
    case 3:
        week = tr("Wednesday");
        break;
    case 4:
        week = tr("Thursday");
        break;
    case 5:
        week = tr("Friday");
        break;
    case 6:
        week = tr("Saturday");
        break;
    case 7:
        week = tr("Sunday");
        break;
    default:
        week = tr("N/A");
        break;
    }
    return week;
}

void weekweather::initQss()
{
    //    this->setStyleSheet("QWidget{background:blue;}");
    //    this->setStyleSheet("QWidget{background:transparent;}");
}

void weekweather::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = this->palette().color(QPalette::Dark);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setOpacity(0.08);
    painter.setPen(Qt::transparent);
    int radius = 3 * ViewVar::SAME_ITEMRADIUS;
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, radius, radius);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);

}
