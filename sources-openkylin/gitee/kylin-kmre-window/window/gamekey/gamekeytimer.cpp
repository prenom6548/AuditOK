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

#include "gamekeytimer.h"
#include "displaymanager.h"
#include "android_display/displaywidget.h"
#include "eventdata.h"
#include <QDebug>

GamekeyTimer::GamekeyTimer(DisplayManager* displayManager)
    : mDisplayManager(displayManager)
{
    mTimer = new QTimer();
    connect(mTimer, SIGNAL(timeout()), this, SLOT(gameKeySolt()));
}

GamekeyTimer::~GamekeyTimer()
{
    deleteGamekeyTimer();
}

void GamekeyTimer::setGameKeyMassage(int posx,int posy,int combonumber)
{
    mPosX = posx;
    mPosY = posy;
    if(combonumber <= 0){
        mComboNumber = 1;
    }
    else{
        mComboNumber = combonumber;
    }

}

void GamekeyTimer::startGamekeyTimer(int gamekeyorder)
{
    mGamekeyOrder = gamekeyorder;
    if(!(mTimer->isActive())){
        if(mComboNumber == 0){
            gameKeySolt();
        }
        else{
            gameKeySolt();
            mTimer->start((1000/mComboNumber));
        }
    }
}

void GamekeyTimer::stopGamekeyTimer(int gamekeyorder)
{
    if(mTimer->isActive()){
        mTimer->stop();
    }
}

void GamekeyTimer::gameKeySolt()
{
    DisplayWidget* display = mDisplayManager->getFocusedDisplay();
    if (display) {
        display->sendMouseEvent(MouseEventInfo{Button1_Press, mGamekeyOrder, mPosX, mPosY});
        //qDebug() << "Press::order = " << mGamekeyOrder << " pos_x = "  << mPosX  << " pos_y = "  << mPosY ;
        display->sendMouseEvent(MouseEventInfo{Button1_Release, mGamekeyOrder, mPosX, mPosY});
        //qDebug() << "Release::order = " << mGamekeyOrder << " pos_x = "  << mPosX  << " pos_y = "  << mPosY ;
    }
}
void GamekeyTimer::deleteGamekeyTimer()
{
    delete mTimer;
    mTimer = nullptr;
}
