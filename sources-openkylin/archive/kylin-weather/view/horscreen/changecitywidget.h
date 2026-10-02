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
#ifndef CHANGCITYWIDGET_H
#define CHANGCITYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFont>
#include "ktoolbutton.h"
#include "global/corevar.h"

class ChangeCityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChangeCityWidget(QWidget *parent = nullptr);
    void getCurrentInfor(CurrentWeather m_currentWeather);
    void isFullScreen(int);
    int m_judgeFullScreen;
    kdk::KToolButton *getChangeCityButton();
    kdk::KToolButton *getCollectCityButton();
    void changeCollectBtnIcon();

private:
    QLabel *m_curCity = nullptr; // 城市名称
    kdk::KToolButton *m_changeCityBtn = nullptr;  // 切换城市
    kdk::KToolButton *m_collectCityBtn = nullptr; // 收藏城市

    QHBoxLayout *m_hLayout = nullptr;      // 城市信息布局（横）

    QFont m_font;
    void initWidget();
    void initConnect();
    void initQss(int);
    void cityMoveItem(int);
    void resizeEvent(QResizeEvent *event);
    void slotClickCollectBtn();

    bool m_isCollect = false;


signals:
};


#endif 
