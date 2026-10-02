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

#ifndef MOUSECROSSHAIRKEY_H
#define MOUSECROSSHAIRKEY_H

#include "joystickbasekey.h"
#include "sensitivitywidget.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QWidget>

const int DEFAULT_CROSS_HAIR_WIDTH = 148;
const int DEFAULT_CROSS_HAIR_HEIGHT = 148;
const int DEFAULT_CROSS_CORE_SIZE = 125;
const int DEFAULT_CROSS_ICON_SIZE = 66;
const int DEFAULT_CROSS_SENS_WIDTH = 106;
const int DEFAULT_CROSS_SENS_HEIGHT = 30;
const int DEFAULT_CROSS_CHECKBOX_WIDTH = 86;
const int DEFAULT_CROSS_CHECKBOX_HEIGHT = 24;
const int DEFAULT_CROSS_EDIT_SIZE = 30;
const int DEFAULT_CROSS_EDGE_MARGIN = 0;
const int DEFAULT_CROSS_INNER_MARGIN = 2;


class MouseCrossHairKey : public JoystickBaseKey
{
    Q_OBJECT
public:
    explicit MouseCrossHairKey(KmreWindow* window);

    void enableEdit(bool enable) Q_DECL_OVERRIDE;
    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;
    void getOperateHW(double &w,double &h) Q_DECL_OVERRIDE;
    bool isPointReset();
    void setPointReset(bool isReset);
    int getCheckBoxStatus();
    void setCheckBoxStatus(int status);
    void setComboFreq(int freq);
    int getComboFreq(){return mComboFreq;}
    void setKeyString(QString character);
    QString getKeyString() { return mKeyString; }
    QLineEdit* getLineEdit(){return mEdit;}
signals:
    void keyboardPress(QKeyEvent*);
    void keyboardRelease(QKeyEvent*);
public slots:
    void updateComboFreq(int);
protected:
    void getTranslateCenterPos(QPoint pos) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

    QPushButton* mCloseBtn = nullptr;
    QLabel* mBgLabel = nullptr;
    QPushButton* mBgBtn = nullptr;
    QLineEdit* mEdit = nullptr;
    SensitivityWidget* mSensiWidget = nullptr;
    QCheckBox* mCheckBox = nullptr;
    QWidget* mContainWidget = nullptr;
    
    bool mIsPointReset = false;
    int mComboFreq;  //连击按键的频率
private:
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

#endif // MOUSECROSSHAIRKEY_H
