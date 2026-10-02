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

#include "gamekeystackwidget.h"
#include "common.h"
#include "JoystickCommon.h"
#include "joysticksetwidget.h"
#include "shadewidget.h"
#include "joystickgamekeymanager.h"
#include "keyboardgamekeymanager.h"
#include "kmrewindow.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QDebug>

GameKeyStackWidget::GameKeyStackWidget(KmreWindow *window) 
    : QWidget(window)
    , mMainWindow(window)
{
    QFont font;
    font.setPixelSize(DEFAULT_GAME_KEY_CHANCE_BUTTON_TEXT_SIZE);
    ktab = new KTabBar(SegmentDark,this);
    //ktab->setFont(font);
    ktab->addTab(tr("K&&M"));
    ktab->addTab(tr("JOYSTICK"));

    //选择键鼠手柄的两个按钮的布局
    QHBoxLayout* mHBLayout_ChangeStack = new QHBoxLayout();
    mHBLayout_ChangeStack->setMargin(7);
    mHBLayout_ChangeStack->setSpacing(0);
    mHBLayout_ChangeStack->addSpacing(10);
    mHBLayout_ChangeStack->addWidget(ktab);
    mHBLayout_ChangeStack->addSpacing(10);
    //存放手柄和键鼠具体设置界面的栈布局
    mStackLayout_ContainWidget = new QStackedLayout();
    keymouseStack = new KeyMouseSetWidget(this);
    joystickStack = new JoystickSetWidget(this);
    noJoystickStack = new NoJoystickWidget(this);
    mStackLayout_ContainWidget->addWidget(keymouseStack);
    mStackLayout_ContainWidget->addWidget(joystickStack);
    mStackLayout_ContainWidget->addWidget(noJoystickStack);

    QVBoxLayout* mVBLayout_RightAll = new QVBoxLayout();
    mVBLayout_RightAll->setMargin(0);
    mVBLayout_RightAll->setSpacing(0);
    mVBLayout_RightAll->addLayout(mHBLayout_ChangeStack);
    mVBLayout_RightAll->addLayout(mStackLayout_ContainWidget);

    this->setLayout(mVBLayout_RightAll);

}
GameKeyStackWidget::~GameKeyStackWidget(){
    if (joystickManager) {
        if(isJoystickRegister){
            isJoystickRegister = false;
            disconnect(joystickStack, SIGNAL(addJoystickKey(int)), joystickManager, SLOT(addJoystickKey(int)));
            disconnect(joystickStack, SIGNAL(clearGameKeys(bool)), joystickManager, SLOT(removeGameKeys(bool)));
            disconnect(joystickStack, SIGNAL(saveGameKeys(bool)), joystickManager, SLOT(saveGameKeys(bool)));
            //disconnect(joystickStack, SIGNAL(updateTransparency(double)), joystickManager, SLOT(updateTransparency(double)));
            disconnect(mMainWindow->getJoystickGameKeyManager(),SIGNAL(buttonSignal(QString)),joystickManager,SLOT(setJoystickKeyStringSlot(QString)));
        }
        if(isKeyMouseRegister){
            isKeyMouseRegister = false;
            disconnect(keymouseStack,SIGNAL(addKeyMouseKey(int)),joystickManager,SLOT(addKeyMouseKey(int)));
            disconnect(keymouseStack, SIGNAL(clearGameKeys(bool)), joystickManager, SLOT(removeGameKeys(bool)));
            disconnect(keymouseStack, SIGNAL(saveGameKeys(bool)), joystickManager, SLOT(saveGameKeys(bool)));
            disconnect(mMainWindow->getKeyboardGamekeyManager(),SIGNAL(buttonSignal(QString)),joystickManager,SLOT(setKeyMouseStringSlot(QString)));
        }
    }
}

void GameKeyStackWidget::readyGameKeyStackWidget(){
    ktab->readyKTabBar();
    connect(ktab,SIGNAL(currentChanged(int)),this,SLOT(currentWidgetTab(int)));
}

void GameKeyStackWidget::setDefaultStackIndex(){
    if (joystickManager) {
        if(joystickManager->isJoystickConnect() && joystickManager->isUseJoystickRock()){
            ktab->setCurrentIndex(1);
            setCurrentStackIndex(JOYSTICK);
        }else{
            ktab->setCurrentIndex(0);
            setCurrentStackIndex(KEYMOUSE);
        }
    }
}

void GameKeyStackWidget::setSpecifiedStackIndex(int index){
    ktab->setCurrentIndex(index);
    setCurrentStackIndex(index);
}

void GameKeyStackWidget::currentWidgetTab(int index){
    setCurrentStackIndex(index);
}

void GameKeyStackWidget::setCurrentStackIndex(int index){
    if(index == 0){
        mStackLayout_ContainWidget->setCurrentIndex(KEYMOUSE);
        toKeyMouseSettings();
    }else if(index == 1){
        if(joystickManager && joystickManager->isJoystickConnect()){  //添加手柄是否插入
            mStackLayout_ContainWidget->setCurrentIndex(JOYSTICK);
            toJoystickSettings();
        }else{
            mStackLayout_ContainWidget->setCurrentIndex(NOJOYSTICK);
            toNoJoystick();
        }
    }
}

void GameKeyStackWidget::toJoystickSettings(){//进入到手柄设置页面
    //注册业务，是无论如何都要执行的
    if(!isJoystickRegister){
        registerJoystickFunc();
    }

    if (joystickManager) {
        //根据当前的状态，确定执行动作
        if(joystickManager->getGameKeyStatus() == JoystickManager::Default||joystickManager->getGameKeyStatus() == JoystickManager::OnlyInit){
            joystickManager->setGameKeyStatus(JoystickManager::JoystickEditing);
            //默认值，第一次启动直接进入，初始化配置文件并显示出来即可
            joystickManager->initJoystickKeysFromFile();
            joystickManager->showJoystickKeys(true);
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::JoystickUse){
            joystickManager->setGameKeyStatus(JoystickManager::JoystickEditing);
            //当前正在使用摇杆，直接进入编辑状态即可
            joystickManager->enableJoystickKeyEdit(true);
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::KeyMouseEditing){
            joystickManager->setGameKeyStatus(JoystickManager::JoystickEditing);
            //当前处在键鼠的编辑状态,首先隐藏键鼠的控件
            joystickManager->showKeyMouseKeys(false);
            //然后初始化手柄的配置文件（不会重复初始化）
            joystickManager->initJoystickKeysFromFile();
            joystickManager->showJoystickKeys(true);
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::NoJostick){
            joystickManager->setGameKeyStatus(JoystickManager::JoystickEditing);
            joystickManager->initJoystickKeysFromFile();
            joystickManager->showJoystickKeys(true);
        }
    }
}
void GameKeyStackWidget::toKeyMouseSettings(){ //切换到键鼠设置页面
    if(!isKeyMouseRegister){
        registerKeyMouseFunc();
    }

    if (joystickManager) {
        if(joystickManager->getGameKeyStatus() == JoystickManager::Default ||joystickManager->getGameKeyStatus() == JoystickManager::OnlyInit){
            //默认值，第一次启动直接进入，初始化配置文件并显示出来即可
            joystickManager->initKeyMouseKeysFromFile();
            joystickManager->showKeyMouseKeys(true);
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::KeyMouseUse){
            //当前正在使用摇杆，直接进入编辑状态即可
            joystickManager->enableKeyMouseEdit(true);
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::JoystickEditing){
            //当前处在手柄的编辑状态,首先隐藏
            joystickManager->showJoystickKeys(false);
            //然后初始化键鼠的配置文件（不会重复初始化）
            joystickManager->initKeyMouseKeysFromFile();
            joystickManager->showKeyMouseKeys(true);
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::NoJostick){
            joystickManager->initKeyMouseKeysFromFile();
            joystickManager->showKeyMouseKeys(true);
        }
        joystickManager->setGameKeyStatus(JoystickManager::KeyMouseEditing);
    }
}

void GameKeyStackWidget::toNoJoystick(){
    if (joystickManager) {
        if(joystickManager->getGameKeyStatus() == JoystickManager::JoystickEditing){
            //需要释放当前所有的手柄控件资源
            joystickManager->removeJoystickKeys();
        }else if(joystickManager->getGameKeyStatus() == JoystickManager::KeyMouseEditing){
            //需要释放当前所有的键鼠控件资源
            joystickManager->removeKeyMouseKeys();
        }
        joystickManager->setGameKeyStatus(JoystickManager::NoJostick);
    }
}

void GameKeyStackWidget::registerJoystickFunc(){//注册手柄相关的业务
    if (joystickManager) {
        isJoystickRegister = true;

        connect(joystickStack, SIGNAL(addJoystickKey(int)), joystickManager, SLOT(addJoystickKey(int)));
        connect(joystickStack, SIGNAL(clearGameKeys(bool)), joystickManager, SLOT(removeGameKeys(bool)));
        connect(joystickStack, SIGNAL(saveGameKeys(bool)), joystickManager, SLOT(saveGameKeys(bool)));
        //connect(joystickStack, SIGNAL(updateTransparency(double)), joystickManager, SLOT(updateTransparency(double)));
        connect(mMainWindow->getJoystickGameKeyManager(),SIGNAL(buttonSignal(QString)),joystickManager,SLOT(setJoystickKeyStringSlot(QString)));
    }
}
void GameKeyStackWidget::registerKeyMouseFunc(){ //注册键鼠相关的业务
    if (joystickManager) {
        isKeyMouseRegister = true;
        connect(keymouseStack,SIGNAL(addKeyMouseKey(int)),joystickManager,SLOT(addKeyMouseKey(int)));
        connect(keymouseStack, SIGNAL(clearGameKeys(bool)), joystickManager, SLOT(removeGameKeys(bool)));
        connect(keymouseStack, SIGNAL(saveGameKeys(bool)), joystickManager, SLOT(saveGameKeys(bool)));
        connect(mMainWindow->getKeyboardGamekeyManager(),SIGNAL(buttonSignal(QString)),joystickManager,SLOT(setKeyMouseStringSlot(QString)));
    }
}

void GameKeyStackWidget::enableAddSteelingWheelBtn(bool enable){

}
void GameKeyStackWidget::enableLeftJoystickBtn(bool enable){
    joystickStack->enableLeftJoystickBtn(enable);
}
void GameKeyStackWidget::enableRightJoystickBtn(bool enable){
    joystickStack->enableRightJoystickBtn(enable);
}

