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
#ifndef CITY_H
#define CITY_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFont>
#include "view/search/searchview.h"
#include "global/corevar.h"

class city : public QWidget
{
    Q_OBJECT
public:
    explicit city(QWidget *parent = nullptr);
    QPushButton *m_changeCityBtn = nullptr;  // 切换城市
    QPushButton *m_collectCityBtn = nullptr; // 收藏城市
    QWidget *m_centrlWid = nullptr;
    void getCurrentInfor(CurrentWeather m_currentWeather);
    void isFullScreen(int);
    int m_judgeFullScreen;
    void initWidgetSize();

private:
    QString code;
    QFont m_bigFont;
    QLabel *m_temperatureNum = nullptr; // 12°摄氏度数字（12）
    QLabel *m_degreeNotation = nullptr; // 12°摄氏度符号（°）
    QLabel *m_curCity = nullptr;        // 城市名称
    SearchView *m_searchView = nullptr;
    //天气图标
    QString path;
    QHBoxLayout *m_cityLayout = nullptr;            

    QHBoxLayout *m_cityWidgetLayout = nullptr;      // 城市信息布局（横）
    QVBoxLayout *m_cityDataVLayout = nullptr;       // 城市信息布局（纵）
    QHBoxLayout *m_leftLayout = nullptr;            // 左侧天气摄氏度布局
    QVBoxLayout *m_rightLayout = nullptr;           // 右侧数据布局
    QHBoxLayout *m_cityAndChangeLayout = nullptr;   // 城市名称切换布局
    QHBoxLayout *m_iconAndWeatherLayout = nullptr;  // 天气状态布局
    QHBoxLayout *m_humidityAndWindLayout = nullptr; // 天气湿度和风向布局

    QLabel *m_weatherPicture = nullptr; // 天气状态 - 图标
    QLabel *m_weather = nullptr;        // 天气状态 - 小雨
    QLabel *m_humidity = nullptr;       // 湿度 98％
    QLabel *m_windForce = nullptr;      // 1东南风 一级
    QLabel *m_updateTime = nullptr;     // 更新时间
    QFont m_font;
    void initConnect();

    // void cityMoveItem(int);
    void resizeEvent(QResizeEvent *event);
    void initWidget();
    void initWidgetStyle();
    void initLayout();
    void PCModeWidgetStyle();
    void PadModeWidgetStyle();

    public slots:
    void slotChangePCMode();
    void slotChangePadVMode();
    void slotChangePadHMode();

signals:
};

#endif // CITY_H
