/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#ifndef DIRECTTYPEBASEKEY_H
#define DIRECTTYPEBASEKEY_H

#include "joystickbasekey.h"
#include <QLineEdit>
#include <QPushButton>

const int DEFAULT_MARGIN = 8;      //方向键使用
const int DEFAULT_LABEL_AWDS_SIZE = 24;
const int MIN_LABEL_AWDS_TEXT_SIZE = 8;
const int MAX_LABEL_AWDS_TEXT_SIZE = 20;
const int DEFAULT_DIRECTION_SIZE = 100;

class DirectTypeBaseKey : public JoystickBaseKey
{
    Q_OBJECT
public:
    DirectTypeBaseKey(KmreWindow* window, int type);

    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;
    void getOperateHW(double &w,double &h) Q_DECL_OVERRIDE;
    void setKeyString(QString str);
    bool isHavingFocus();
    void setDefaultKeyString(QString strTop,QString strRight,QString strBottom,QString strLeft);
    void getDirectKeyString(QString &strTop,QString &strRight,QString &strBottom,QString &strLeft);
    bool isHadLineEditNotSetValue();
signals:
    void keyboardPress(QKeyEvent*);
    void keyboardRelease(QKeyEvent*);
protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void getTranslateCenterPos(QPoint pos) Q_DECL_OVERRIDE;
    void getMouseClickPos(QObject *obj, QPoint pos);

    QLineEdit* mEditTop = nullptr;
    QLineEdit* mEditLeft = nullptr;
    QLineEdit* mEditRight = nullptr;
    QLineEdit* mEditBottom = nullptr;
    QPushButton* mCloseBtn = nullptr;
    QPushButton* mBgBtn = nullptr;

    void resetValues();
    void calculateTracingArea();
    void setMouseStyle(const QPoint &point);
    void getPressedAreaPosition(const QPoint &point);
    void resetSizeAndPos(int offsetX, int offsetY);
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


};

#endif // DIRECTTYPEBASEKEY_H
