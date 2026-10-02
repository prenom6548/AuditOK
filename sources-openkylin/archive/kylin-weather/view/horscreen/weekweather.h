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
#ifndef WEEKWEATHER_H
#define WEEKWEATHER_H

#include <QWidget>
#include <QHBoxLayout>
#include "onedayweather.h"
#include "splitline.h"
#include "global/corevar.h"
#include "global/viewvar.h"
#include <QDebug>
class weekweather : public QWidget
{
    Q_OBJECT
public:
    explicit weekweather(QWidget *parent = nullptr);
    void getForcastInfor(ForecastWeather m_forecastWeather, int m_isFullScreen);
    void setWeatherIcon(int m_isFullScreen);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QHBoxLayout *m_weekweatherLayout = nullptr;
    onedayweather *m_onedayweatherfirst = nullptr;
    splitline *m_splitlinefirst = nullptr;
    onedayweather *m_onedayweathertwo = nullptr;
    splitline *m_splitlinetwo = nullptr;
    onedayweather *m_onedayweatherthree = nullptr;
    splitline *m_splitlinethree = nullptr;
    onedayweather *m_onedayweatherfour = nullptr;
    splitline *m_splitlinefour = nullptr;
    onedayweather *m_onedayweatherfive = nullptr;
    splitline *m_splitlinefive = nullptr;
    onedayweather *m_onedayweathersix = nullptr;
    splitline *m_splitlinesix = nullptr;
    onedayweather *m_onedayweatherseven = nullptr;

    QString firstIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString twoIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString threeIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString fourIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString fiveIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString sixIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString sevenIconPath = ":/res/weather-icon/PNG48/1x/999.png";

    ForecastWeather forecastWeather;

    void initLayout();
    void initQss();
    void initConnect();
    //字符串切分函数
    //    void splitStr(QString m_forecastWeather);
    int splitStr(QString m_forecastWeather, QString pattern);
    void resizeEvent(QResizeEvent *event);
    QStringList dateAndWeekText(QString data);//返回日期和星期
    QString dateAndWeekInt(QString data);//返回日期和星期

signals:
    void sendIsFullScreen(int isFullScreen);
    //    void sendForcastInfor(QString m_forecastWeather);
};

#endif // WEEKWEATHER_H
