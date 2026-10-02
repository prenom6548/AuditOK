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
#ifndef INFORMATION_H
#define INFORMATION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "perhour.h"
#include "weekweather.h"
#include "lifestyle.h"
#include "global/corevar.h"
#include <QDebug>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QMouseEvent>

class information : public QWidget
{
    Q_OBJECT
public:
    explicit information(QWidget *parent = nullptr);


private:
    static const int TOP_SPACING = -5;
    static const int BELOW_SPACING = -9; //设置小标题间距上高下低
    QVBoxLayout *m_informationLayout;
    perhour *m_perhour = nullptr;
    weekweather *m_weekweather = nullptr;
    lifestyle *m_lifestyle = nullptr;
    void initLayout();
    void initQss();
    void initConnect();
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event) override; // 事件过滤器
    QLabel *m_hoursTipLabel = nullptr;
    QLabel *m_dayTipLabel = nullptr;
    QLabel *m_lifeTipLabel = nullptr;
    QHBoxLayout *m_dayTipLayout = nullptr;  // 逐日天气标题行的水平布局
    QLabel *m_40DayLinkLabel = nullptr;     // 40日天气预报链接
    QLabel *m_dataSource = nullptr;         //数据来源标签

private slots:
    void on40DayLinkClicked(); // 处理40日天气预报链接点击事件

signals:
    void getLifeStyleInfor(LifeStyle m_lifeStyle, int m_isFullScreen);
    void sendIsFullScreen(int isFullScreen);
    void sendForcastInfor(ForecastWeather m_forecastWeather, int m_isFullScreen);
    void sendHourlyInfor(QList<Hourly> tmp_list, int m_isFullScreen); // 逐小时信息
    void weekWeatherIcon(int isFullScreen);
};

#endif // INFORMATION_H
