/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STEELING_WHEEL_H
#define STEELING_WHEEL_H

#include "basekey.h"

#include <QLabel>
#include <QPushButton>

#define DEFAULT_STEELING_WHEEL_SIZE 116

class SteelingWheel : public BaseKey
{
    Q_OBJECT
public:
    explicit SteelingWheel(KmreWindow* window);
    ~SteelingWheel();

    void enableEdit(bool enable) Q_DECL_OVERRIDE;
    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;
    void updatePos() Q_DECL_OVERRIDE;
    
    void setCoordinateData(double x, double y, double width, double height);
    double getEventX() { return event_coordinate_x; }
    double getEventY() { return event_coordinate_y; }
    double getWidthRatio(){return m_width_scale_ratio;}
    double getHeightRatio(){return m_height_scale_ratio;}

signals:
    void deleteGameSteelingWheel();

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel *m_labelA = nullptr;
    QLabel *m_labelW = nullptr;
    QLabel *m_labelD = nullptr;
    QLabel *m_labelS = nullptr;
    QPushButton *m_bgBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    OverlayButton *m_overlayBtn = nullptr;

    double m_width_scale_ratio;
    double m_height_scale_ratio;
    double event_coordinate_x;// 左上角
    double event_coordinate_y;// 左上角

    bool m_pressedLeft;               //鼠标按下左侧
    bool m_pressedRight;              //鼠标按下右侧
    bool m_pressedTop;                //鼠标按下上侧
    bool m_pressedBottom;             //鼠标按下下侧
    bool m_pressedLeftTop;            //鼠标按下左上侧
    bool m_pressedRightTop;           //鼠标按下右上侧
    bool m_pressedLeftBottom;         //鼠标按下左下侧
    bool m_pressedRightBottom;        //鼠标按下右下侧

    int rectX, rectY, rectW, rectH; //窗体坐标+宽高

    QRect m_rectLeft;                 //左侧区域
    QRect m_rectRight;                //右侧区域
    QRect m_rectTop;                  //上侧区域
    QRect m_rectBottom;               //下侧区域
    QRect m_rectLeftTop;              //左上侧区域
    QRect m_rectRightTop;             //右上侧区域
    QRect m_rectLeftBottom;           //左下侧区域
    QRect m_rectRightBottom;          //右下侧区域

private:
    void resetValues();
    void calculateTracingArea();
    void setMouseStyle(const QPoint &point);
    void getPressedAreaPosition(const QPoint &point);
    void resetSizeAndPos(int offsetX, int offsetY);

};

#endif // STEELING_WHEEL_H
