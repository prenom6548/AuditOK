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

#include "joystickgamekeymanager.h"
#include "displaymanager.h"
#include "android_display/displaywidget.h"
#include "joystickmanager.h"
#include "eventdata.h"
#include "qjoysticks.h"
#include "kmrewindow.h"

#include <QDebug>
#include <QMap>
#include <QList>

JoystickGamekeyManager::JoystickGamekeyManager(KmreWindow* window)
    : QObject(window)
    , mMainWindow(window)
{
    axisTimerInit();
}

JoystickGamekeyManager::~JoystickGamekeyManager()
{
    if (mLATimer) {
        delete mLATimer;
        mLATimer = nullptr;
    }
    if (mRATimer) {
        delete mLATimer;
        mRATimer = nullptr;
    }
    for (QMap<QString, GamekeyTimer*>::iterator item = mGameKeyMap.begin(); item != mGameKeyMap.end(); ++item)
    {
        delete mGameKeyMap[item.key()];
        mGameKeyMap[item.key()] = nullptr;
    }
}

void JoystickGamekeyManager::sdlJoysticksPOV(int value)
{
    if(mMainWindow->getJoystickManager()->isUseJoystickRock()){
        switch (value)
        {
            case POV_UP:{
                settingGameKey("UP", POV_STATUS);
                handlePovGameKeyPress(value, "BUTTON_UP");
                break;
            }
            case POV_RIGHT:{
                settingGameKey("RIGHT", POV_STATUS);
                handlePovGameKeyPress(value, "BUTTON_RIGHT");
                break;
            }
            case POV_DOWN:{
                settingGameKey("DOWN", POV_STATUS);
                handlePovGameKeyPress(value, "BUTTON_DOWN");
                break;
            }
            case POV_LEFT:{
                settingGameKey("LEFT", POV_STATUS);
                handlePovGameKeyPress(value, "BUTTON_LEFT");
                break;
            }
            case POV_RELEASE:{
                povGamekeyRelease();
                break;
            }
            default:
                break;
        }
    }
}

void JoystickGamekeyManager::sdlJoysticksButton(int value, int btn_status)
{
    if(mMainWindow->getJoystickManager()->isUseJoystickRock()){
        switch (value)
        {
            case BUTTON_A:{
                settingGameKey("A", btn_status);
                handleButtonGameKey(value, "BUTTON_A", btn_status);
                break;
            }
            case BUTTON_B:{
                settingGameKey("B", btn_status);
                handleButtonGameKey(value, "BUTTON_B", btn_status);
                break;
            }
            case BUTTON_X:{
                settingGameKey("X", btn_status);
                handleButtonGameKey(value, "BUTTON_X", btn_status);
                break;
            }
            case BUTTON_Y:{
                settingGameKey("Y", btn_status);
                handleButtonGameKey(value, "BUTTON_Y", btn_status);
                break;
            }
            case BUTTON_LB:{
                settingGameKey("LB", btn_status);
                handleButtonGameKey(value, "BUTTON_LB", btn_status);
                break;
            }
            case BUTTON_RB:{
                settingGameKey("RB", btn_status);
                handleButtonGameKey(value, "BUTTON_RB", btn_status);
                break;
            }
            case BUTTON_BACK:{
                settingGameKey("BACK", btn_status);
                handleButtonGameKey(value, "BUTTON_BACK", btn_status);
                break;
            }
            case BUTTON_START:{
                settingGameKey("START", btn_status);
                handleButtonGameKey(value, "BUTTON_START", btn_status);
                break;
            }
            case BUTTON_HOME:{
                settingGameKey("HOME", btn_status);
                handleButtonGameKey(value, "BUTTON_HOME", btn_status);
                break;
            }
            case BUTTON_LA:{
                settingGameKey("LA", btn_status);
                handleButtonGameKey(value, "BUTTON_LA", btn_status);
                break;
            }
            case BUTTON_RA:{
                settingGameKey("RA", btn_status);
                handleButtonGameKey(value, "BUTTON_RA", btn_status);
                break;
            }
            default:
                break;
         }
    }
}

void JoystickGamekeyManager::sdlJoysticksAxis(qreal value, int axis_id)
{
    if(mMainWindow->getJoystickManager()->isUseJoystickRock()){
        switch (axis_id)
        {
            case AXIS_LA_RIGHTLEFT:{
                if(isGamekeySetting("AXIS_LA") && (mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse)){
                    mAxisValue.mAxisRightLeftValueLA = value;
                    if(!mLATimer->isActive()){
                        handleAxisLAGamekey();
                        mLATimer->start(LA_TIMER);
                    }
                }
                break;
            }
            case AXIS_LA_UPDOWN:{
                if(isGamekeySetting("AXIS_LA") && (mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse)){
                    mAxisValue.mAxisUpDownValueLA = value;
                    if(!mLATimer->isActive()){
                        handleAxisLAGamekey();
                        mLATimer->start(LA_TIMER);
                    }
                }
                break;
            }
            case AXIS_RA_RIGHTLEFT:{
                if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse){
                    mAxisValue.mAxisRightLeftValueRA = value;
                    if(isBrainPress){
                        axisRAMove();
                    } else{
                        if(isGamekeySetting("AXIS_RA")){
                            setAxisRightLeftGamekeyFrequency(abs(value));
                            if(!mRATimer->isActive()){
                                handleAxisRAGamekey();
                                mRATimer->start(RA_TIMER);
                            }
                        }
                    }
                }
                break;
            }
            case AXIS_RA_UPDOWN:{
                if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse){
                    mAxisValue.mAxisUpDownValueRA = value;
                    if(isBrainPress){
                        axisRAMove();
                    } else{
                        if(isGamekeySetting("AXIS_RA")){
                            setAxisUpDownGamekeyFrequency(abs(value));
                            if(!mRATimer->isActive()){
                                handleAxisRAGamekey();
                                mRATimer->start(RA_TIMER);
                            }
                        }
                    }
                }
                break;
            }
            case AXIS_LT:{
                if(value == 0){
                    mAxisAtatus.mAxisLTStatus = false;
                    handleButtonGameKey(BUTTON_LT, "BUTTON_LT", 0);
                }
                else {
                    if(mAxisAtatus.mAxisLTStatus == false){
                        settingGameKey("LT", 1);
                        handleButtonGameKey(BUTTON_LT, "BUTTON_LT", 1);
                        mAxisAtatus.mAxisLTStatus = true;
                    }
                }
                break;
            }
            case AXIS_RT:{
                if(value == 0){
                    mAxisAtatus.mAxisRTStatus = false;
                    handleButtonGameKey(BUTTON_RT, "BUTTON_RT", 0);
                }
                else {
                    if(mAxisAtatus.mAxisRTStatus == false){
                        settingGameKey("RT", 1);
                        handleButtonGameKey(BUTTON_RT, "BUTTON_RT", 1);
                        mAxisAtatus.mAxisRTStatus = true;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

void JoystickGamekeyManager::handleAxisLAGamekey(){
    JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getJoystickPos("AXIS_LA");
    if(mAxisValue.mAxisRightLeftValueLA == 0 && mAxisValue.mAxisUpDownValueLA == 0){
        if(mLATimer->isActive()){
            mLATimer->stop();
            gameKeyMove(gamekeymsg.x, gamekeymsg.y, AXIS_LA + AXIS_ORDER);
            gameKeyRelease(gamekeymsg.x, gamekeymsg.y, AXIS_LA + AXIS_ORDER);
            mAxisPos.mAxisLAPosX = gamekeymsg.x;
            mAxisPos.mAxisLAPosY = gamekeymsg.y;
        }
    }
    else{
        if(mAxisValue.mAxisRightLeftValueLAPre == 0 && mAxisValue.mAxisUpDownValueLAPre == 0){
            gameKeyPress(gamekeymsg.x, gamekeymsg.y, AXIS_LA + AXIS_ORDER);
        }
        else{
            if(mAxisValue.mAxisRightLeftValueLAPre != mAxisValue.mAxisRightLeftValueLA || mAxisValue.mAxisUpDownValueLAPre != mAxisValue.mAxisUpDownValueLA){
                int movePosX = gamekeymsg.x + mAxisValue.mAxisRightLeftValueLA * gamekeymsg.wheelbase_y;
                int movePosY = gamekeymsg.y + mAxisValue.mAxisUpDownValueLA * gamekeymsg.wheelbase_y;
                gameKeyMove(movePosX, movePosY, AXIS_LA + AXIS_ORDER);
                mAxisPos.mAxisLAPosX = movePosX;
                mAxisPos.mAxisLAPosY = movePosY;
            }
        }
    }
    mAxisValue.mAxisRightLeftValueLAPre = mAxisValue.mAxisRightLeftValueLA;
    mAxisValue.mAxisUpDownValueLAPre = mAxisValue.mAxisUpDownValueLA;
}

void JoystickGamekeyManager::handleAxisRAGamekey(){
    JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getJoystickPos("AXIS_RA");
    if(mAxisValue.mAxisRightLeftValueRA == 0 && mAxisValue.mAxisUpDownValueRA == 0){
        mRATimer->stop();
        gameKeyRelease(mAxisPos.mAxisRAPosX, mAxisPos.mAxisRAPosY, AXIS_RA + AXIS_ORDER);
    }
    else{
        if(mAxisValue.mAxisRightLeftValueRAPre == 0 && mAxisValue.mAxisUpDownValueRAPre == 0){
            gameKeyPress(gamekeymsg.x, gamekeymsg.y, AXIS_RA + AXIS_ORDER);
            mAxisPos.mAxisRAPosX = gamekeymsg.x;
            mAxisPos.mAxisRAPosY = gamekeymsg.y;
        }
        else{
            int movePosX = mAxisPos.mAxisRAPosX;
            int movePosY = mAxisPos.mAxisRAPosY;
            int frequencyX = mAxisFrequency.mAxisRARightLeft;
            int frequencyY = mAxisFrequency.mAxisRAUpDown;

            if(mAxisValue.mAxisRightLeftValueRA >= 0 && mAxisValue.mAxisUpDownValueRA > 0){
                mAxisPos.mAxisRAPosX = movePosX + frequencyX;
                mAxisPos.mAxisRAPosY = movePosY + frequencyY;
                if(mAxisPos.mAxisRAPosX > (gamekeymsg.x + gamekeymsg.wheelbase_x) || mAxisPos.mAxisRAPosY > (gamekeymsg.y + gamekeymsg.wheelbase_y)){
                    gameKeyRelease(movePosX, movePosX, AXIS_RA + AXIS_ORDER);
                    gameKeyPress(gamekeymsg.x, gamekeymsg.y, AXIS_RA + AXIS_ORDER);
                    mAxisPos.mAxisRAPosX = gamekeymsg.x + frequencyX;
                    mAxisPos.mAxisRAPosY = gamekeymsg.y + frequencyY;
                }
            }
            if(mAxisValue.mAxisRightLeftValueRA > 0 && mAxisValue.mAxisUpDownValueRA <= 0){
                mAxisPos.mAxisRAPosX = movePosX + frequencyX;
                mAxisPos.mAxisRAPosY = movePosY - frequencyY;
                if(mAxisPos.mAxisRAPosX > (gamekeymsg.x + gamekeymsg.wheelbase_x) || mAxisPos.mAxisRAPosY < (gamekeymsg.y - gamekeymsg.wheelbase_y)){
                    gameKeyRelease(movePosX, movePosX, AXIS_RA + AXIS_ORDER);
                    gameKeyPress(gamekeymsg.x, gamekeymsg.y, AXIS_RA + AXIS_ORDER);
                    mAxisPos.mAxisRAPosX = gamekeymsg.x + frequencyX;
                    mAxisPos.mAxisRAPosY = gamekeymsg.y - frequencyY;
                }
            }
            if(mAxisValue.mAxisRightLeftValueRA < 0 && mAxisValue.mAxisUpDownValueRA >= 0){
                mAxisPos.mAxisRAPosX = movePosX - frequencyX;
                mAxisPos.mAxisRAPosY = movePosY + frequencyY;
                if((mAxisPos.mAxisRAPosX < (gamekeymsg.x - gamekeymsg.wheelbase_x)) || (mAxisPos.mAxisRAPosY > (gamekeymsg.y + gamekeymsg.wheelbase_y))){
                    gameKeyRelease(movePosX, movePosX, AXIS_RA + AXIS_ORDER);
                    gameKeyPress(gamekeymsg.x, gamekeymsg.y, AXIS_RA + AXIS_ORDER);
                    mAxisPos.mAxisRAPosX = gamekeymsg.x - frequencyX;
                    mAxisPos.mAxisRAPosY = gamekeymsg.y + frequencyY;
                }
            }
            if(mAxisValue.mAxisRightLeftValueRA <= 0 && mAxisValue.mAxisUpDownValueRA < 0){
                mAxisPos.mAxisRAPosX = movePosX - frequencyX;
                mAxisPos.mAxisRAPosY = movePosY - frequencyY;
                if(mAxisPos.mAxisRAPosX < (gamekeymsg.x - gamekeymsg.wheelbase_x) || mAxisPos.mAxisRAPosY < (gamekeymsg.y - gamekeymsg.wheelbase_y)){
                    gameKeyRelease(movePosX, movePosX, AXIS_RA + AXIS_ORDER);
                    gameKeyPress(gamekeymsg.x, gamekeymsg.y, AXIS_RA + AXIS_ORDER);
                    mAxisPos.mAxisRAPosX = gamekeymsg.x - frequencyX;
                    mAxisPos.mAxisRAPosY = gamekeymsg.y - frequencyY;
                }
            }
            gameKeyMove(mAxisPos.mAxisRAPosX, mAxisPos.mAxisRAPosY, AXIS_RA + AXIS_ORDER);
        }
    }
    mAxisValue.mAxisRightLeftValueRAPre = mAxisValue.mAxisRightLeftValueRA;
    mAxisValue.mAxisUpDownValueRAPre = mAxisValue.mAxisUpDownValueRA;
}

void JoystickGamekeyManager::settingGameKey(QString gameKey, int status){
    if(mMainWindow->getJoystickManager()->isJoystickInEditing() && status == 1){
        emit buttonSignal(gameKey);
    }
}

void JoystickGamekeyManager::handleButtonGameKey(int value, QString gameKey, int status){
    if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse){
        JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getSingleGameKeyPos(gameKey);
        if(gamekeymsg.isSet){
            if(gamekeymsg.type == 1){
                if(status){
                    gameKeyPress(gamekeymsg.x, gamekeymsg.y, value + BUTTON_ORDER);
                }
                else{
                    gameKeyRelease(gamekeymsg.x, gamekeymsg.y, value + BUTTON_ORDER);
                }
            }
            else if(gamekeymsg.type == 2){
                if(status){
                    QString mkey = gameKey;
                    addGameKeyMap(mkey.remove("BUTTON_"));
                    mGameKeyMap[gameKey]->setGameKeyMassage(gamekeymsg.x, gamekeymsg.y, gamekeymsg.comboNumber);
                    mGameKeyMap[gameKey]->startGamekeyTimer(getGameKeyOrderPress(value + BUTTON_ORDER));
                } else{
                    mGameKeyMap[gameKey]->stopGamekeyTimer(getGameKeyOrderRelease(value + BUTTON_ORDER));
                }
            } else if(gamekeymsg.type == 3){//智能按钮
                if(status){
                    if(!isBrainPress){
                        isBrainPress = true;
                        mBrainPressGameKey = gameKey;
                        if(mRATimer->isActive()){
                            mRATimer->stop();
                        }
                        gameKeyPress(gamekeymsg.x, gamekeymsg.y, value + BUTTON_ORDER);
                        mBrainInfo.mBrainPosX = gamekeymsg.x;
                        mBrainInfo.mBrainPosY = gamekeymsg.y;
                        mBrainInfo.mBrainPosXPre = gamekeymsg.x;
                        mBrainInfo.mBrainPosYPre = gamekeymsg.y;
                        mBrainInfo.mBrainOrder = value + BUTTON_ORDER;
                        axisRAMove();
                    }
                } else{
                    if(isBrainPress && mBrainPressGameKey == gameKey){
                        mAxisValue.mAxisRightLeftValueRAPre = 0;
                        mAxisValue.mAxisUpDownValueRAPre = 0;
                        isBrainPress = false;
                        mBrainPressGameKey = "";
                        gameKeyRelease(mBrainInfo.mBrainPosX, mBrainInfo.mBrainPosX, value + BUTTON_ORDER);
                    }
                }
            }
        }
    }
}

void JoystickGamekeyManager::handlePovGameKeyPress(int value, QString gameKey){
    if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse){
        povGamekeyRelease();
        JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getSingleGameKeyPos(gameKey);
        if(gamekeymsg.isSet){
            if(gamekeymsg.type == 1){
                gameKeyPress(gamekeymsg.x, gamekeymsg.y, value + POV_ORDER);
                updatePovStatus(value, true);
            }
            else if(gamekeymsg.type == 2){
                QString mkey = gameKey;
                addGameKeyMap(mkey.remove("BUTTON_"));
                mGameKeyMap[gameKey]->setGameKeyMassage(gamekeymsg.x, gamekeymsg.y, gamekeymsg.comboNumber);
                mGameKeyMap[gameKey]->startGamekeyTimer(getGameKeyOrderPress(value + POV_ORDER));
                updatePovStatus(value, true);
            } else if(gamekeymsg.type == 3){//智能按钮

            }
        }
    }
}

void JoystickGamekeyManager::gameKeyPress(int pos_x , int pos_y, int order){
    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display) {
        int gamekeyorder = getGameKeyOrderPress(order);
        display->sendMouseEvent(MouseEventInfo{Button1_Press, gamekeyorder, pos_x, pos_y});
        //qDebug() << "Press::order = " << gamekeyorder << " pos_x = "  << pos_x  << " pos_y = "  << pos_y;
    }
}

void JoystickGamekeyManager::gameKeyMove(int pos_x , int pos_y, int order){
    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display) {
        int gamekeyorder = getGameKeyOrderMove(order);
        display->sendMouseEvent(MouseEventInfo{Mouse_Motion, gamekeyorder, pos_x, pos_y});
        //qDebug() << "Move::order = " << gamekeyorder << " pos_x = "  << pos_x  << " pos_y = "  << pos_y;
    }
}

void JoystickGamekeyManager::gameKeyRelease(int pos_x , int pos_y, int order){
    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display) {
        int gamekeyorder = getGameKeyOrderRelease(order);
        display->sendMouseEvent(MouseEventInfo{Button1_Release, gamekeyorder, pos_x, pos_y});
        //qDebug() << "Release::order = " << gamekeyorder << " pos_x = "  << pos_x  << " pos_y = "  << pos_y;
    }
}

int JoystickGamekeyManager::getGameKeyOrderPress(int value){
    int orderSize = sizeof(mButtonClickOrder)/sizeof(unsigned int);
    int gameKeyOrder = 0;
    if(value >= 10 && value < 20){
        if(value == 16){
            return LA_ORDER;
        }
        if(value == 17){
            return RA_ORDER;
        }
    }
    else{
        for (int i = 0; i < orderSize - 2; i++) {
            if (mButtonClickOrder[i] == 0) {
                gameKeyOrder = i + 1;
                mButtonClickOrder[i] = value;
                return gameKeyOrder;
            }
        }
    }
    return -1;
}

int JoystickGamekeyManager::getGameKeyOrderRelease(int value){
    int orderSize = sizeof(mButtonClickOrder)/sizeof(unsigned int);
    int gameKeyOrder = 0;
    if(value >= 10 && value < 20){
        if(value == AXIS_ORDER + AXIS_LA){
            return LA_ORDER;
        }
        if(value == AXIS_ORDER + AXIS_RA){
            return RA_ORDER;
        }
    }
    else{
        for (int i = 0; i < orderSize - 2; i++) {
            if (mButtonClickOrder[i] == value) {
                gameKeyOrder = i + 1;
                mButtonClickOrder[i] = 0;
                return gameKeyOrder;
            }
        }
    }
    return -1;
}

int JoystickGamekeyManager::getGameKeyOrderMove(int value){
    int orderSize = sizeof(mButtonClickOrder)/sizeof(unsigned int);
    int gameKeyOrder = 0;
    if(value == AXIS_ORDER + AXIS_LA){
        return LA_ORDER;
    } else if(value == AXIS_ORDER + AXIS_RA){
        return RA_ORDER;
    } else {
        for (int i = 0; i < orderSize - 2; i++) {
            if (mButtonClickOrder[i] == value) {
                gameKeyOrder = i + 1;
                return gameKeyOrder;
            }
        }
    }

    return -1;
}

void JoystickGamekeyManager::axisTimerInit(){
    mLATimer = new QTimer();
    connect(mLATimer, SIGNAL(timeout()), this, SLOT(handleAxisLAGamekey()));

    mRATimer = new QTimer();
    connect(mRATimer, SIGNAL(timeout()), this, SLOT(handleAxisRAGamekey()));
}

void JoystickGamekeyManager::gameKeyInit(){
    mGameKeyMap.clear();
    QMap<int,QList<QString>> pGamekeyMap = mMainWindow->getJoystickManager()->getAllKindOfKey();
    if(!pGamekeyMap.isEmpty()){
        if(pGamekeyMap.contains(JoystickComboKey::COMBOKEY)){
            QList<QString> pList = pGamekeyMap[JoystickComboKey::COMBOKEY];
            for (int i = 0; i < pList.size(); ++i)
            {
                addGameKeyMap(pList.at(i));
            }
        }
    }
}

void JoystickGamekeyManager::addGameKeyMap(QString gamekey){
    QString addGameKey = "BUTTON_" + gamekey;
    if(!mGameKeyMap.contains(addGameKey)){
        GamekeyTimer *gamekeytimer = new GamekeyTimer(mMainWindow->getDisplayManager());
        mGameKeyMap.insert(addGameKey, gamekeytimer);
    }
}

void JoystickGamekeyManager::povGamekeyRelease(){
    if(mPovStatus.mPovUpStatus){
        JoystickManager::KeyMsgStruct upmsg = mMainWindow->getJoystickManager()->getSingleGameKeyPos("BUTTON_UP");
        gameKeyRelease(upmsg.x, upmsg.y, POV_UP + POV_ORDER);
        if(mGameKeyMap.contains("BUTTON_UP")){
            mGameKeyMap["BUTTON_UP"]->stopGamekeyTimer(getGameKeyOrderRelease(POV_UP + POV_ORDER));
        }
        updatePovStatus(POV_UP, false);
    }
    if(mPovStatus.mPovDownStatus){
        JoystickManager::KeyMsgStruct downmsg = mMainWindow->getJoystickManager()->getSingleGameKeyPos("BUTTON_DOWN");
        gameKeyRelease(downmsg.x, downmsg.y, POV_DOWN + POV_ORDER);
        if(mGameKeyMap.contains("BUTTON_DOWN")){
            mGameKeyMap["BUTTON_DOWN"]->stopGamekeyTimer(getGameKeyOrderRelease(POV_DOWN + POV_ORDER));
        }
        updatePovStatus(POV_DOWN, false);
    }
    if(mPovStatus.mPovLeftStatus){
        JoystickManager::KeyMsgStruct leftmsg = mMainWindow->getJoystickManager()->getSingleGameKeyPos("BUTTON_LEFT");
        gameKeyRelease(leftmsg.x, leftmsg.y, POV_LEFT + POV_ORDER);
        if(mGameKeyMap.contains("BUTTON_LEFT")){

            mGameKeyMap["BUTTON_LEFT"]->stopGamekeyTimer(getGameKeyOrderRelease(POV_LEFT + POV_ORDER));
        }
        updatePovStatus(POV_LEFT, false);
    }
    if(mPovStatus.mPovRightStatus){
        JoystickManager::KeyMsgStruct rightmsg = mMainWindow->getJoystickManager()->getSingleGameKeyPos("BUTTON_RIGHT");
        gameKeyRelease(rightmsg.x, rightmsg.y, POV_RIGHT + POV_ORDER);
        if(mGameKeyMap.contains("BUTTON_RIGHT")){
            mGameKeyMap["BUTTON_RIGHT"]->stopGamekeyTimer(getGameKeyOrderRelease(POV_RIGHT + POV_ORDER));
        }
        updatePovStatus(POV_RIGHT, false);
    }
}

void JoystickGamekeyManager::updatePovStatus(int value, bool status){
    switch (value)
    {
        case POV_UP:{
            mPovStatus.mPovUpStatus = status;
            break;
        }
        case POV_RIGHT:{
            mPovStatus.mPovRightStatus = status;
            break;
        }
        case POV_DOWN:{
            mPovStatus.mPovDownStatus = status;
            break;
        }
        case POV_LEFT:{
            mPovStatus.mPovLeftStatus = status;
            break;
        }
        default:
            break;
    }
}

bool JoystickGamekeyManager::isGamekeySetting(QString gamekey){
    JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getJoystickPos(gamekey);
    if(gamekeymsg.isSet){
        return true;
    }
    else{
        return false;
    }
}

void JoystickGamekeyManager::setAxisUpDownGamekeyFrequency(qreal value){
    if(value == 0){
        mAxisFrequency.mAxisRAUpDown = 0;
    } else if(value < 0.2){
        mAxisFrequency.mAxisRAUpDown = 1;
    } else if(value < 0.4){
        mAxisFrequency.mAxisRAUpDown = 4;
    } else if(value < 0.6){
        mAxisFrequency.mAxisRAUpDown = 7;
    } else if(value < 0.8){
        mAxisFrequency.mAxisRAUpDown = 10;
    } else if(value <= 1){
        mAxisFrequency.mAxisRAUpDown = 13;
    }
}

void JoystickGamekeyManager::setAxisRightLeftGamekeyFrequency(qreal value){
    if(value == 0){
        mAxisFrequency.mAxisRARightLeft = 0;
    } else if(value < 0.2){
        mAxisFrequency.mAxisRARightLeft = 1;
    } else if(value < 0.4){
        mAxisFrequency.mAxisRARightLeft = 4;
    } else if(value < 0.6){
        mAxisFrequency.mAxisRARightLeft = 7;
    } else if(value < 0.8){
        mAxisFrequency.mAxisRARightLeft = 10;
    } else if(value <= 1){
        mAxisFrequency.mAxisRARightLeft = 13;
    }
}

void JoystickGamekeyManager::getJoystickChange(int value){
    mJoystickCount = value;
    emit joystickChange(this->getJoystickCount());
}

bool JoystickGamekeyManager::getJoystickCount(){
    bool status = false;
    if(mJoystickCount == 0){
        status = false;
    }
    else{
        status = true;
    }
    return status;
}

void JoystickGamekeyManager::focusOutMethod(){
    if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::JoystickUse){
        int orderSize = sizeof(mButtonClickOrder)/sizeof(unsigned int);;
        for (int i = 0; i < orderSize - 2; i++) {
            if (mButtonClickOrder[i] != 0) {
                if(mButtonClickOrder[i] < 15){
                    povGamekeyRelease();
                } else{
                    //...
                }
                mButtonClickOrder[i] = 0;
            }
        }
        if(mButtonClickOrder[7] != 0){
            if(mLATimer->isActive()){
                JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getJoystickPos("AXIS_LA");
                mAxisValue.mAxisRightLeftValueLA = 0;
                mAxisValue.mAxisUpDownValueLA = 0;
                mLATimer->stop();

                gameKeyMove(gamekeymsg.x, gamekeymsg.y, AXIS_LA + AXIS_ORDER);
                gameKeyRelease(gamekeymsg.x, gamekeymsg.y, AXIS_LA + AXIS_ORDER);
                mAxisPos.mAxisLAPosX = gamekeymsg.x;
                mAxisPos.mAxisLAPosY = gamekeymsg.y;
                mAxisValue.mAxisRightLeftValueLAPre = mAxisValue.mAxisRightLeftValueLA;
                mAxisValue.mAxisUpDownValueLAPre = mAxisValue.mAxisUpDownValueLA;
            }
            mButtonClickOrder[7] = 0;
        }
        if(mButtonClickOrder[8] != 0){
            if(mRATimer->isActive()){
                mAxisValue.mAxisRightLeftValueRA = 0;
                mAxisValue.mAxisUpDownValueRA = 0;
                mRATimer->stop();
                gameKeyRelease(mAxisPos.mAxisRAPosX, mAxisPos.mAxisRAPosY, AXIS_RA + AXIS_ORDER);
                mAxisValue.mAxisRightLeftValueRAPre = mAxisValue.mAxisRightLeftValueRA;
                mAxisValue.mAxisUpDownValueRAPre = mAxisValue.mAxisUpDownValueRA;
            }
            mButtonClickOrder[8] = 0;
        }
    }
}

void JoystickGamekeyManager::axisRAMove(){
    mBrainInfo.mBrainPosX = mBrainInfo.mBrainPosXPre + (mAxisValue.mAxisRightLeftValueRA * JOYSTICKBRAINWIDTH);
    mBrainInfo.mBrainPosY = mBrainInfo.mBrainPosYPre + (mAxisValue.mAxisUpDownValueRA * JOYSTICKBRAINHEIGHT);
    gameKeyMove(mBrainInfo.mBrainPosX, mBrainInfo.mBrainPosY, mBrainInfo.mBrainOrder);
}
