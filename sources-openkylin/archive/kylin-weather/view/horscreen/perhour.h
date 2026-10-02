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
#ifndef PERHOUR_H
#define PERHOUR_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpacerItem>
#include "onehourweather.h"
#include "global/corevar.h"

class perhour : public QWidget
{
    Q_OBJECT
public:
    explicit perhour(QWidget *parent = nullptr);
    void setHourlyInfor(QList<Hourly> tmp_list, int m_isFullScreen);
    void setHourlyIcon(int m_isFullScreen);
    void paintEvent(QPaintEvent *event) override;

private:
    int m_currentPage = 0; // 当前页面索引，从0开始
    int m_totalPages = 0;  // 总页数
    int m_itemsPerPage = 9; // 每页显示9个小时天气
    
    QWidget *m_perhourWid = nullptr;
    QHBoxLayout *m_perhourLayout = nullptr;  // 逐小时布局
    QHBoxLayout *m_Layout = nullptr;         // 主布局
    QPushButton *m_leftBtn = nullptr;        // 左切换按钮
    QPushButton *m_rightBtn = nullptr;       // 右切换按钮
    
    // 用于存储所有小时天气组件的列表
    QList<onehourweather*> m_hourWeatherList;


    onehourweather *m_onehourWeatherFirstFirst;
    onehourweather *m_onehourWeatherFirstSecond;
    onehourweather *m_onehourWeatherFirstThird;
    onehourweather *m_onehourWeatherFirstFourth;
    onehourweather *m_onehourWeatherFirstFifth;
    onehourweather *m_onehourWeatherFirstSixth;
    onehourweather *m_onehourWeatherFirstSeventh;
    onehourweather *m_onehourWeatherFirstEighth;

    onehourweather *m_onehourWeatherSecondFirst;
    onehourweather *m_onehourWeatherSecondSecond;
    onehourweather *m_onehourWeatherSecondThird;
    onehourweather *m_onehourWeatherSecondFourth;
    onehourweather *m_onehourWeatherSecondFifth;
    onehourweather *m_onehourWeatherSecondSixth;
    onehourweather *m_onehourWeatherSecondSeventh;
    onehourweather *m_onehourWeatherSecondEighth;

    onehourweather *m_onehourWeatherThirdFirst;
    onehourweather *m_onehourWeatherThirdSecond;
    onehourweather *m_onehourWeatherThirdThird;
    onehourweather *m_onehourWeatherThirdFourth;
    onehourweather *m_onehourWeatherThirdFifth;
    onehourweather *m_onehourWeatherThirdSixth;
    onehourweather *m_onehourWeatherThirdSeventh;
    onehourweather *m_onehourWeatherThirdEighth;

    QString m_firstFirstIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstSecondIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstThirdIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstFourthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstFifthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstSixthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstSeventhIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_firstEighthIconPath = ":/res/weather-icon/PNG48/1x/999.png";

    QString m_secondFirstIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondSecondIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondThirdIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondFourthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondFifthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondSixthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondSeventhIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_secondEighthIconPath = ":/res/weather-icon/PNG48/1x/999.png";

    QString m_thirdFirstIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdSecondIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdThirdIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdFourthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdFifthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdSixthIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdSeventhIconPath = ":/res/weather-icon/PNG48/1x/999.png";
    QString m_thirdEighthIconPath = ":/res/weather-icon/PNG48/1x/999.png";

    void initWidget();
    void initStyle();
    void initConnect();
    void updatePageDisplay();      // 更新当前页面显示
    void updateButtonVisibility(); // 更新按钮显示状态
    void showPage(int pageIndex);  // 显示指定页面

    QList<Hourly> tmpList;

private slots:
    void onLeftButtonClicked();    // 左按钮点击事件
    void onRightButtonClicked();   // 右按钮点击事件

protected:
    void enterEvent(QEvent *event) override;  // 鼠标进入事件
    void leaveEvent(QEvent *event) override;  // 鼠标离开事件

signals:
    void sendIsFullScreen(int isFullScreen);

public slots:
    void slotChangePadHMode();
    void slotChangePadVMode();
    void slotChangePCMode();

};

#endif // PERHOUR_H
