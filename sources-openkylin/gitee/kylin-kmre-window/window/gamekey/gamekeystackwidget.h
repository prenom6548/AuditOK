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

#ifndef GAMEKEYSTACKWIDGET_H
#define GAMEKEYSTACKWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QStackedLayout>

#include "joysticksetwidget.h"
#include "keymousesetwidget.h"
#include "nojoystickwidget.h"
#include "ktabbar.h"
#include "joystickmanager.h"

using namespace kdk;

class KmreWindow;

class GameKeyStackWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameKeyStackWidget(KmreWindow* window);
    ~GameKeyStackWidget();

    enum StackIndex{
        KEYMOUSE,
        JOYSTICK,
        NOJOYSTICK,
    };

    void setJoystickManager(JoystickManager* manager) {joystickManager = manager;}
    void enableAddSteelingWheelBtn(bool enable);
    void enableLeftJoystickBtn(bool enable);
    void enableRightJoystickBtn(bool enable);
    void setDefaultStackIndex();
    void setSpecifiedStackIndex(int index);
    void toJoystickSettings();   //键鼠切换到手柄
    void toKeyMouseSettings();   //手柄切换到键鼠
    void toNoJoystick();
    void registerJoystickFunc();    //注册手柄相关的业务
    void registerKeyMouseFunc();        //注册键鼠相关的业务
    void readyGameKeyStackWidget();

    JoystickSetWidget* getJoystickSetWidget(){
        return joystickStack;
    }


private:
    KmreWindow* mMainWindow = nullptr;
    KTabBar* ktab = nullptr;
    QStackedLayout* mStackLayout_ContainWidget = nullptr;
    JoystickSetWidget* joystickStack = nullptr;
    KeyMouseSetWidget* keymouseStack = nullptr;
    NoJoystickWidget* noJoystickStack = nullptr;
    JoystickManager* joystickManager = nullptr;
    bool isJoystickRegister = false;
    bool isKeyMouseRegister = false;
    void setCurrentStackIndex(int index);

signals:
    //void addShadeWidget();

public slots:
    void currentWidgetTab(int index);



};

#endif // GAMEKEYSTACKWIDGET_H
