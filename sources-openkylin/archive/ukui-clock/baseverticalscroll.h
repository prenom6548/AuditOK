/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef BASEVERTICALSCROLL_H
#define BASEVERTICALSCROLL_H

#include <QWidget>
#include "gsettingsubject.h"
#include "primarymanager.h"
#include <QPropertyAnimation>
#include <QPainter>
#include <QLineEdit>
class BaseVerticalScroll : public QWidget
{
    Q_OBJECT
public:
    enum BaseVerticalScroll_TYPE{
        TEXT_SCROLL=0,
        NUM_SCROLL=1
    };
    explicit BaseVerticalScroll(int currentValue,int minRange,int maxRange,BaseVerticalScroll_TYPE scrollType=NUM_SCROLL,QWidget *parent = nullptr);

    void setWheelSpeed(int wheelSpeed);
    int m_currentValue=0;
    int m_wheelSpeed = 1;
    bool m_isFirstFocus = false;
    int m_minRange=0;      //最小值  // minimum value
    int m_maxRange=0;      //最大值  // Maximum
    void  settingsStyle();
    GsettingSubject * subject;
    PrimaryManager * m_priManager;
    void homing();
    int calculateCurrentValue(int current,int offsetValue);
    void commonCalcValue(int height);
    bool isDragging;     //鼠标是否按下 // Muse down
    int m_deviation;     //偏移量,记录鼠标按下后移动的垂直距离  // Offset, record the vertical distance after mouse is pressed
    int m_mouseSrcPos;
    int m_numSize;
    QPropertyAnimation *homingAni;
    const int interval;  //间隔大小 // Interval size
    const int devide;       //分隔数量 // Number of partitions
    void paintNum(QPainter &painter, int num, int deviation);
    QString change_NUM_to_str(int alarmHour);


signals:
    void currentValueChanged(int value);
    void deviationChange(int deviation);
    void srcollDoubleClicked();
protected:
    void wheelEvent(QWheelEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private:
    BaseVerticalScroll_TYPE m_scrollType;

};

#endif // BASEVERTICALSCROLL_H
