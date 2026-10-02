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

#ifndef JOYSTICKSINGLEBASEKEY_H
#define JOYSTICKSINGLEBASEKEY_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include "joystickbasekey.h"
#include "sensitivitywidget.h"

const int DEFAULT_SINGLE_KEY_SHORT_WIDTH = 40;
const int DEFAULT_SINGLE_KEY_LONG_WIDTH = 80;
const int DEFAULT_SINGLE_KEY_WIDTH = 56;  //单击按键的宽  20+36
const int DEFAULT_SINGLE_KEY_HEIGHT = 56;  //单击按键的高
const int DEFAULT_COMBO_KEY_WIDTH = 90;   //连击按键的宽
const int DEFAULT_COMBO_KEY_HEIGHT = 90;   //连击按键的高  20+36+30+4    4是下边的margin
const int DEFAULT_MOUSE_KEY_WIDTH = 56;  //鼠标按键的宽  20+36
const int DEFAULT_MOUSE_KEY_HEIGHT = 56;
const int DEFAULT_COMBO_KEY_SENSI_WIDGET_HEIGHT = 30;   //下方灵敏度框的高度
const int DEFAULT_COMBO_KEY_MARGIN = 4;
const int DEFAULT_SINGLE_KEY_EDIT = 36;

class KmreWindow;

class JoystickSingleBaseKey : public JoystickBaseKey
{
    Q_OBJECT
public:
    JoystickSingleBaseKey(KmreWindow* window,int type);
    ~JoystickSingleBaseKey();

    void enableEdit(bool enable) Q_DECL_OVERRIDE;
    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;

    int getIndex(){return mIndex;}
    void setIndex(int index);
    int getComboFreq(){return mComboFreq;}
    void setKeyString(QString character);
    QString getKeyString() { return mKeyString; }
    bool isLineEditHasFocus();
    void setBelongToJoystick(bool isJoy){isJoystick = isJoy;}
    bool isBelongToJoystick(){return isJoystick;}

signals:
    void keyboardPress(QKeyEvent*);
    void keyboardRelease(QKeyEvent*);
    void mouseRightPress(QMouseEvent*);

protected:
    void getTranslateCenterPos(QPoint pos) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *evt) Q_DECL_OVERRIDE;
    void getMouseClickPos(QObject *obj, QPoint pos);

    QLineEdit *mInputEdit = nullptr;
    QPushButton *mCloseBtn = nullptr;
    QPushButton *mBgBtn = nullptr;
    SensitivityWidget* mSensiWidget = nullptr;

    int mIndex = 0;     //按键的索引
    int mComboFreq;  //连击按键的频率
    bool isJoystick = false;   //默认不是摇杆添加

};

#endif // JOYSTICKSINGLEBASEKEY_H
