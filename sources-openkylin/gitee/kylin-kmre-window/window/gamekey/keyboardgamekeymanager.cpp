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

#include "keyboardgamekeymanager.h"
#include "singlekey.h"
#include "displaymanager.h"
#include "android_display/displaywidget.h"
#include "eventdata.h"
#include "joystickmanager.h"
#include "kmrewindow.h"

#include <QList>
#include <QDebug>
#include <QCursor>
#include <QPoint>
#include <QRect>


KeyboardGamekeyManager::KeyboardGamekeyManager(KmreWindow* window)
    : QObject(window)
    , mMainWindow(window)
{
    viewGamekeyInit();
}

KeyboardGamekeyManager::~KeyboardGamekeyManager()
{
    if (mViewTimer) {
        delete mViewTimer;
        mViewTimer = nullptr;
    }
    for (QMap<QString, GamekeyTimer*>::iterator item = mGameKeyMap.begin(); item != mGameKeyMap.end(); ++item)
    {
        delete mGameKeyMap[item.key()];
        mGameKeyMap[item.key()] = nullptr;
    }
}

void KeyboardGamekeyManager::getKeyboardGamekeyPress(QKeyEvent *pKey)
{
    QString gameKey = "";
    gameKey = mapGameKeyModifiers(mapKeyboardGamekey(pKey->nativeScanCode()), pKey->nativeModifiers());
    if(gameKey != ""){
        if(mMainWindow->getJoystickManager()->isKeyMouseInEditing()){
            emit buttonSignal(gameKey);
        } else{
            keyboardGamekeyMethod(gameKey, 1);
        }
    }
}

void KeyboardGamekeyManager::getKeyboardGamekeyRelease(QKeyEvent *pKey)
{
    QString gameKey = "";
    gameKey = mapGameKeyModifiers(mapKeyboardGamekey(pKey->nativeScanCode()), pKey->nativeModifiers());
    if(gameKey != ""){
        if(!(mMainWindow->getJoystickManager()->isKeyMouseInEditing())){
            keyboardGamekeyMethod(gameKey, 0);
        }
    }
}

void KeyboardGamekeyManager::getMouseGamekeyPress(QMouseEvent *pKey)
{
    QString gameKey = "";
    if(pKey->button() == Qt::LeftButton){
        gameKey = MOUSELEFTBUTTON;
    }
    if(pKey->button() == Qt::RightButton){
        gameKey = MOUSERIGHTBUTTON;
    }
    if(gameKey != ""){
        if(mMainWindow->getJoystickManager()->isKeyMouseInEditing()){
            if(gameKey == MOUSERIGHTBUTTON){
                emit buttonSignal(gameKey);
            }
        } else{
            if(gameKey == MOUSERIGHTBUTTON){
                keyboardGamekeyMethod(gameKey, 1);
            } else {
                if(!mIsMouseClick){
                    keyboardGamekeyMethod(gameKey, 1);
                }
            }
        }
    }
}

void KeyboardGamekeyManager::getMouseGamekeyRelease(QMouseEvent *pKey)
{
    QString gameKey = "";
    if(pKey->button() == Qt::LeftButton){
        gameKey = MOUSELEFTBUTTON;
    }
    if(pKey->button() == Qt::RightButton){
        gameKey = MOUSERIGHTBUTTON;
    }
    if(gameKey != ""){
        if(!(mMainWindow->getJoystickManager()->isJoystickInEditing())){
            if(gameKey == MOUSERIGHTBUTTON){
                keyboardGamekeyMethod(gameKey, 0);
            } else {
                if(!mIsMouseClick){
                    keyboardGamekeyMethod(gameKey, 0);
                }
            }
        }
    }
}

void KeyboardGamekeyManager::getMouseGamekeyMove(QMouseEvent *pKey)
{
    if (!mIsMouseClick){
        if(pKey->pos().x() != mViewLockInfo.mDisplayX || pKey->pos().y() != mViewLockInfo.mDisplayY){
            QPoint mousePos =mapMousePos(QPoint((pKey->pos().x() - mViewLockInfo.mDisplayX), (pKey->pos().y() - mViewLockInfo.mDisplayY)));

            mViewLockInfo.mPosXPre = mViewLockInfo.mPosXPre + mousePos.x();
            mViewLockInfo.mPosYPre = mViewLockInfo.mPosYPre + mousePos.y();

            if(mButtonClickOrder[8] == ""){
                gameKeyPress(mViewLockInfo.mPosX, mViewLockInfo.mPosY, MOUSEMOVE);
            }

            if(mViewLockInfo.mPosXPre > mViewLockInfo.mPosX + mViewLockInfo.mWheelX ||
                    mViewLockInfo.mPosYPre > mViewLockInfo.mPosY + mViewLockInfo.mWheelY ||
                    mViewLockInfo.mPosXPre < mViewLockInfo.mPosX - mViewLockInfo.mWheelX ||
                    mViewLockInfo.mPosYPre < mViewLockInfo.mPosY - mViewLockInfo.mWheelY){
                recoverMousePos();
            } else{
                gameKeyMove(mViewLockInfo.mPosXPre, mViewLockInfo.mPosYPre, MOUSEMOVE);
            }

            mViewLockInfo.mDisplayXPre = pKey->pos().x();
            mViewLockInfo.mDisplayYPre = pKey->pos().y();
        }
    }
}

void KeyboardGamekeyManager::keyboardGamekeyMethod(QString gameKey, int status){
    if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::KeyMouseUse){
        JoystickManager::KeyMsgStruct gamekeymsg = mMainWindow->getJoystickManager()->getKeyMouseKeyPos(gameKey);
        if(gamekeymsg.isSet){
            switch (gamekeymsg.type) {
                case CLICKBUTTON:{
                    if(status){
                        gameKeyPress(gamekeymsg.x, gamekeymsg.y, gameKey);
                    } else{
                        gameKeyRelease(gamekeymsg.x, gamekeymsg.y, gameKey);
                    }
                    break;
                }
                case COMBOBUTTON:{
                    if(status){
                        addGameKeyMap(gameKey);
                        mGameKeyMap[gameKey]->setGameKeyMassage(gamekeymsg.x, gamekeymsg.y, gamekeymsg.comboNumber);
                        mGameKeyMap[gameKey]->startGamekeyTimer(getGameKeyOrderPress(gameKey));
                    } else{
                        mGameKeyMap[gameKey]->stopGamekeyTimer(getGameKeyOrderRelease(gameKey));
                    }
                    break;
                }
                case WHEELBUTTON:{
                    wheelGamekey(gamekeymsg.directionFlag, gamekeymsg.x, gamekeymsg.y, gamekeymsg.wheelbase_x, status);
                    mWheelInfo.mWheelPosX = gamekeymsg.x;
                    mWheelInfo.mWheelPosY = gamekeymsg.y;
                    break;
                }
                case VIEWLOCK:{
                    if(status){
                        if(mIsMouseClick){
                            QPoint displayPos = mMainWindow->getJoystickManager()->getMousePos(QPoint(gamekeymsg.x, gamekeymsg.y), 2);
                            mViewLockInfo.mMouseSensitivity = gamekeymsg.sensitivity;
                            mViewLockInfo.mPosX = gamekeymsg.x;
                            mViewLockInfo.mPosY = gamekeymsg.y;
                            mViewLockInfo.mWheelX = gamekeymsg.wheelbase_x;
                            mViewLockInfo.mWheelY = gamekeymsg.wheelbase_y;
                            mViewLockInfo.mDisplayX = displayPos.x();
                            mViewLockInfo.mDisplayY = displayPos.y();
                            mViewLockInfo.mDisplayXPre = displayPos.x();
                            mViewLockInfo.mDisplayYPre = displayPos.y();
                            mViewLockInfo.mPosXPre = gamekeymsg.x;
                            mViewLockInfo.mPosYPre = gamekeymsg.y;
                            mViewLockInfo.mIsRecover = gamekeymsg.isChecked;
                        }
                        mouseViewLock();
                    }
                    break;
                }
                case VIEWWHELL:{
                    if(mViewValue == 0){
                        gameKeyPress(gamekeymsg.x, gamekeymsg.y, VIEWBUTTON);
                        QRect widgetRect = mMainWindow->getJoystickManager()->getValidOperateArea();
                        androidPos = mMainWindow->getJoystickManager()->getMousePos(QPoint(widgetRect.width(), widgetRect.height()), 1);
                    }
                    viewGamekeyValue(gamekeymsg.directionFlag, status, 1);
                    if(!(mViewTimer->isActive())){
                        mViewPos.mViewPosX = gamekeymsg.x;
                        mViewPos.mViewPosY = gamekeymsg.y;
                        mViewPos.mViewPosXPre = gamekeymsg.x;
                        mViewPos.mViewPosYPre = gamekeymsg.y;
                        mViewTimer->start(3);
                    }
                    break;
                }
                case GRAVITYBUTTON://重力，需要调芳姐提供的接口
                    break;
                case FIREBUTTON:{
                    if(!mIsMouseClick){
                        if(status){
                            gameKeyPress(gamekeymsg.x, gamekeymsg.y, gameKey);
                        } else{
                            gameKeyRelease(gamekeymsg.x, gamekeymsg.y, gameKey);
                        }
                    }
                    break;
                }
                case BRAINBUTTON:{
                    if(status){
                        if(!isBrainPress){
                            isBrainPress = true;
                            mBrainPressGameKey = gameKey;
                            if(isViewLockPress){
                                mIsMouseClick = true;
                            } else{
                                emit isShowMouseSignal(true);
                            }
                            gameKeyPress(gamekeymsg.x, gamekeymsg.y, gameKey);
                            mBrainInfo.mBrainPosX = gamekeymsg.x;
                            mBrainInfo.mBrainPosY = gamekeymsg.y;
                            mBrainInfo.mBrainPosXPre = gamekeymsg.x;
                            mBrainInfo.mBrainPosYPre = gamekeymsg.y;
                            resetMousePos();
                        }
                    } else{
                        if(isBrainPress && mBrainPressGameKey == gameKey){
                            isBrainPress = false;
                            mBrainPressGameKey = "";
                            if(isViewLockPress){
                                mIsMouseClick = false;
                            } else{
                                emit isShowMouseSignal(false);
                            }
                            gameKeyRelease(mBrainInfo.mBrainPosX, mBrainInfo.mBrainPosX, gameKey);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void KeyboardGamekeyManager::gameKeyPress(int pos_x , int pos_y, QString gameKey){
    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display) {
        int gamekeyorder = getGameKeyOrderPress(gameKey);
        display->sendMouseEvent(MouseEventInfo{Button1_Press, gamekeyorder, pos_x, pos_y});
    }
}

void KeyboardGamekeyManager::gameKeyMove(int pos_x , int pos_y, QString gameKey){
    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display) {
        int gamekeyorder = getGameKeyOrderMove(gameKey);
        display->sendMouseEvent(MouseEventInfo{Mouse_Motion, gamekeyorder, pos_x, pos_y});
    }
}

void KeyboardGamekeyManager::gameKeyRelease(int pos_x , int pos_y, QString gameKey){
    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display) {
        int gamekeyorder = getGameKeyOrderRelease(gameKey);
        display->sendMouseEvent(MouseEventInfo{Button1_Release, gamekeyorder, pos_x, pos_y});
    }
}

int KeyboardGamekeyManager::getGameKeyOrderPress(QString value){
    int orderSize = sizeof(mButtonClickOrder)/(sizeof(unsigned int) * 2);
    int gameKeyOrder = 0;
    if(value == MOUSEMOVE){
        mButtonClickOrder[8] = value;
        return 9;
    }
    if(value == VIEWBUTTON){
        mButtonClickOrder[7] = value;
        return 8;
    }
    if(value == DIRBUTTON){
        mButtonClickOrder[6] = value;
        return 7;
    }
    for (int i = 0; i < orderSize - 3; i++) {
        if (mButtonClickOrder[i] == "") {
            gameKeyOrder = i + 1;
            mButtonClickOrder[i] = value;
            return gameKeyOrder;
        }
    }
    return -1;
}

int KeyboardGamekeyManager::getGameKeyOrderRelease(QString value){
    int orderSize = sizeof(mButtonClickOrder)/(sizeof(unsigned int) * 2);
    int gameKeyOrder = 0;
    if(value == MOUSEMOVE){
        mButtonClickOrder[8] = "";
        return 9;
    }
    if(value == VIEWBUTTON){
        mButtonClickOrder[7] = "";
        return 8;
    }
    if(value == DIRBUTTON){
        mButtonClickOrder[6] = "";
        return 7;
    }
    for (int i = 0; i < orderSize - 3; i++) {
        if (mButtonClickOrder[i] == value) {
            gameKeyOrder = i + 1;
            mButtonClickOrder[i] = "";
            return gameKeyOrder;
        }
    }
    return -1;
}

int KeyboardGamekeyManager::getGameKeyOrderMove(QString value){
    int orderSize = sizeof(mButtonClickOrder)/(sizeof(unsigned int) * 2);
    int gameKeyOrder = 0;
    if(value == MOUSEMOVE){
        return 9;
    }
    if(value == VIEWBUTTON){
        return 8;
    }
    if(value == DIRBUTTON){
        return 7;
    }
    for (int i = 0; i < orderSize - 3; i++) {
        if (mButtonClickOrder[i] == value) {
            gameKeyOrder = i + 1;
            return gameKeyOrder;
        }
    }
    return -1;
}

QString KeyboardGamekeyManager::mapKeyboardGamekey(quint32 keyCode){
    switch (keyCode) {
        case 65:
            return "SP";
        case 24:
            return "Q";
        case 25:
            return "W";
        case 26:
            return "E";
        case 27:
            return "R";
        case 28:
            return "T";
        case 29:
            return "Y";
        case 30:
            return "U";
        case 31:
            return "I";
        case 32:
            return "O";
        case 33:
            return "P";
        case 38:
            return "A";
        case 39:
            return "S";
        case 40:
            return "D";
        case 41:
            return "F";
        case 42:
            return "G";
        case 43:
            return "H";
        case 44:
            return "J";
        case 45:
            return "K";
        case 46:
            return "L";
        case 52:
            return "Z";
        case 53:
            return "X";
        case 54:
            return "C";
        case 55:
            return "V";
        case 56:
            return "B";
        case 57:
            return "N";
        case 58:
            return "M";
        case 10:
            return "1";
        case 11:
            return "2";
        case 12:
            return "3";
        case 13:
            return "4";
        case 14:
            return "5";
        case 15:
            return "6";
        case 16:
            return "7";
        case 17:
            return "8";
        case 18:
            return "9";
        case 19:
            return "0";
        case 87:
            return "N1";
        case 88:
            return "N2";
        case 89:
            return "N3";
        case 83:
            return "N4";
        case 84:
            return "N5";
        case 85:
            return "N6";
        case 79:
            return "N7";
        case 80:
            return "N8";
        case 81:
            return "N9";
        case 90:
            return "N0";
        case 111:
            return "↑";
        case 113:
            return "←";
        case 114:
            return "→";
        case 116:
            return "↓";
        case 50: case 62:
            return "SHIFT";
        case 37: case 105:
            return "CTRL";
        case 64: case 108:
            return "ALT";
        case 49:
            return "~";
        case 23:
            return "Tab";
        case 66:
            return "Caps";
        default:
            break;
    }
    return "";
}

void KeyboardGamekeyManager::wheelGamekey(QString gameKey, int x, int y, int wheel, int status){
    if(mPovValue == 0){
        gameKeyPress(x, y, DIRBUTTON);
    }
    directionGamekeyValue(gameKey, status, 0);
    switch (mPovValue) {
        case UPVALUE:{
            gameKeyMove(x, y - wheel, DIRBUTTON);
            break;
        }
        case DOWNVALUE:{
            gameKeyMove(x, y + wheel, DIRBUTTON);
            break;
        }
        case LEFTVALUE:{
            gameKeyMove(x - wheel, y, DIRBUTTON);
            break;
        }
        case RIGHTVALUE:{
            gameKeyMove(x + wheel, y, DIRBUTTON);
            break;
        }
        case LEFTUPVALUE:{
            gameKeyMove(x- (wheel * 7 / 10), y - (wheel * 7 / 10), DIRBUTTON);
            break;
        }
        case RIGHTUPVALUE:{
            gameKeyMove(x + (wheel * 7 / 10), y - (wheel * 7 / 10), DIRBUTTON);
            break;
        }
        case LEFTDOWNVALUE:{
            gameKeyMove(x - (wheel * 7 / 10), y + (wheel * 7 / 10), DIRBUTTON);
            break;
        }
        case RIGHTDOWNVALUE:{
            gameKeyMove(x + (wheel * 7 / 10), y + (wheel * 7 / 10), DIRBUTTON);
            break;
        }
        default:{
            gameKeyMove(x, y, DIRBUTTON);
            gameKeyRelease(x, y, DIRBUTTON);
            break;
        }
    }
}

void KeyboardGamekeyManager::viewGamekeyValue(QString gameKey, int status, int type){
    if(gameKey == UPBUTTON){
        if(status){
            mViewValue = mViewValue + mValueList.at(type).mUpValue;
        } else{
            mViewValue = mViewValue - mValueList.at(type).mUpValue;
        }
    }
    if(gameKey == DOWNBUTTON){
        if(status){
            mViewValue = mViewValue + mValueList.at(type).mDownValue;
        } else{
            mViewValue = mViewValue - mValueList.at(type).mDownValue;
        }
    }
    if(gameKey == LEFTBUTTON){
        if(status){
            mViewValue = mViewValue + mValueList.at(type).mLeftValue;
        } else{
            mViewValue = mViewValue - mValueList.at(type).mLeftValue;
        }
    }
    if(gameKey == RIGHTBUTTON){
        if(status){
            mViewValue = mViewValue + mValueList.at(type).mRightValue;
        } else{
            mViewValue = mViewValue - mValueList.at(type).mRightValue;
        }
    }
}

void KeyboardGamekeyManager::directionGamekeyValue(QString gameKey, int status, int type){
    if(gameKey == UPBUTTON){
        if(status){
            mPovValue = mPovValue + mValueList.at(type).mUpValue;
        } else{
            mPovValue = mPovValue - mValueList.at(type).mUpValue;
        }
    }
    if(gameKey == DOWNBUTTON){
        if(status){
            mPovValue = mPovValue + mValueList.at(type).mDownValue;
        } else{
            mPovValue = mPovValue - mValueList.at(type).mDownValue;
        }
    }
    if(gameKey == LEFTBUTTON){
        if(status){
            mPovValue = mPovValue + mValueList.at(type).mLeftValue;
        } else{
            mPovValue = mPovValue - mValueList.at(type).mLeftValue;
        }
    }
    if(gameKey == RIGHTBUTTON){
        if(status){
            mPovValue = mPovValue + mValueList.at(type).mRightValue;
        } else{
            mPovValue = mPovValue - mValueList.at(type).mRightValue;
        }
    }
}

void KeyboardGamekeyManager::mouseViewLock(){
    if(mIsMouseClick){
        emit isShowMouseSignal(true);
        emit settingMousePosSignal(getViewLockPos());
        mIsMouseClick = false;
        isViewLockPress = true;
    }
    else{
        emit settingMousePosSignal(getViewLockPos());
        emit isShowMouseSignal(false);
        mIsMouseClick = true;
        isViewLockPress = false;
        gameKeyRelease(mViewLockInfo.mPosXPre, mViewLockInfo.mPosYPre, MOUSEMOVE);
    }
}

void KeyboardGamekeyManager::viewGamekeyInit(){
    mViewTimer = new QTimer();
    connect(mViewTimer, SIGNAL(timeout()), this, SLOT(viewGamekeyMethod()));

    mValueList.append(mDirValue);
    mValueList.append(mAngleValue);
    mTipList.append(mPovValue);
    mTipList.append(mViewValue);
}

void KeyboardGamekeyManager::viewGamekeyMethod(){
    switch (mViewValue) {
        case UPVALUE:{
            viewPosMethod(0, -1);
            break;
        }
        case DOWNVALUE:{
            viewPosMethod(0, 1);
            break;
        }
        case LEFTVALUE:{
            viewPosMethod(-1, 0);
            break;
        }
        case RIGHTVALUE:{
            viewPosMethod(1, 0);
            break;
        }
        case LEFTUPVALUE:{
            viewPosMethod(-1, -1);
            break;
        }
        case RIGHTUPVALUE:{
            viewPosMethod(1, -1);
            break;
        }
        case LEFTDOWNVALUE:{
            viewPosMethod(-1, 1);
            break;
        }
        case RIGHTDOWNVALUE:{
            viewPosMethod(1, 1);
            break;
        }
        default:{
            if(mViewTimer->isActive() && mViewValue == 0){
                mViewTimer->stop();
                gameKeyRelease(mViewPos.mViewPosX, mViewPos.mViewPosY, VIEWBUTTON);
            }
            break;
        }
    }
}

void KeyboardGamekeyManager::viewPosMethod(int posX, int posY){

    if(mViewPos.mViewPosX + posX >= (androidPos.x() - 10) || mViewPos.mViewPosX + posX <= 10
            || mViewPos.mViewPosY + posY >= (androidPos.y() - 10) || mViewPos.mViewPosY + posY <= 10){
        gameKeyRelease(mViewPos.mViewPosX, mViewPos.mViewPosY, VIEWBUTTON);
        mViewPos.mViewPosX = mViewPos.mViewPosXPre;
        mViewPos.mViewPosY = mViewPos.mViewPosYPre;
        gameKeyPress(mViewPos.mViewPosX, mViewPos.mViewPosY, VIEWBUTTON);
    }
    mViewPos.mViewPosX = mViewPos.mViewPosX + posX;
    mViewPos.mViewPosY = mViewPos.mViewPosY + posY;
    gameKeyMove(mViewPos.mViewPosX, mViewPos.mViewPosY, VIEWBUTTON);
}

QString KeyboardGamekeyManager::mapGameKeyModifiers(QString gameKey, quint32 Modifiers){
    if(gameKey != "" && gameKey != "CTRL" && gameKey != "ALT" && gameKey != "SHIFT"){
        switch (Modifiers) {
            case CTRLPRESSED:{
                gameKey = "CTRL+" + gameKey;
                break;
            }
            case ALTPRESSED:{
                gameKey = "ALT+" + gameKey;
                break;
            }
            case SHIFTPRESSED:{
                gameKey = "SHIFT+" + gameKey;
                break;
            }
            default:
                break;
        }
    }
    return gameKey;
}

void KeyboardGamekeyManager::gameKeyInit(){
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

void KeyboardGamekeyManager::addGameKeyMap(QString gamekey){
    if(!mGameKeyMap.contains(gamekey)){
        GamekeyTimer *gamekeytimer = new GamekeyTimer(mMainWindow->getDisplayManager());
        mGameKeyMap.insert(gamekey, gamekeytimer);
    }
}

void KeyboardGamekeyManager::recoverMousePos(){
    gameKeyRelease(mViewLockInfo.mPosXPre, mViewLockInfo.mPosYPre, MOUSEMOVE);
    mViewLockInfo.mPosXPre = mViewLockInfo.mPosX;
    mViewLockInfo.mPosYPre = mViewLockInfo.mPosY;
    gameKeyPress(mViewLockInfo.mPosX, mViewLockInfo.mPosY, MOUSEMOVE);
}

QPoint KeyboardGamekeyManager::mapMousePos(QPoint pos){
    return pos;
}

QPoint KeyboardGamekeyManager::getViewLockPos(){
    return QPoint(mViewLockInfo.mDisplayX, mViewLockInfo.mDisplayY);
}

void KeyboardGamekeyManager::focusOutMethod(){
    if(mMainWindow->getJoystickManager()->getGameKeyStatus() == JoystickManager::KeyMouseUse){
        int orderSize = sizeof(mButtonClickOrder)/(sizeof(unsigned int) * 2);
        for (int i = 0; i < orderSize - 3; i++) {
            if (mButtonClickOrder[i] != "") {
                keyboardGamekeyMethod(mButtonClickOrder[i], 0);
                mButtonClickOrder[i] = "";
            }
        }
        if(mButtonClickOrder[6] != ""){
            mPovValue = 0;
            gameKeyMove(mWheelInfo.mWheelPosX, mWheelInfo.mWheelPosY, DIRBUTTON);
            gameKeyRelease(mWheelInfo.mWheelPosX, mWheelInfo.mWheelPosY, DIRBUTTON);
            mButtonClickOrder[6] = "";
        }
        if(mButtonClickOrder[7] != ""){
            mViewValue = 0;
            mViewTimer->stop();
            gameKeyRelease(mViewPos.mViewPosX, mViewPos.mViewPosY, VIEWBUTTON);
            mButtonClickOrder[7] = "";
        }
        if(mButtonClickOrder[8] != ""){
            mouseViewLock();
            mButtonClickOrder[8] = "";
        }
    }
}

void KeyboardGamekeyManager::resetMousePos(){
    QRect widgetRect = mMainWindow->getJoystickManager()->getValidOperateArea();
    int pos_x = QCursor::pos().x() - widgetRect.x() - ((widgetRect.width() - BRAINWIDTH) / 2);
    int pos_y = QCursor::pos().y() - widgetRect.y() - ((widgetRect.height() - BRAINHEIGHT) / 2);
    if(pos_x <= 0){
        if(pos_y <= 0){
            emit settingMousePosSignal(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, (widgetRect.height() - BRAINHEIGHT + 2) / 2));
            brainGamekeyMethod(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, (widgetRect.height() - BRAINHEIGHT + 2) / 2),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        } else if(pos_y > 0 && pos_y < BRAINHEIGHT){
            emit settingMousePosSignal(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, QCursor::pos().y() - widgetRect.y()));
            brainGamekeyMethod(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, QCursor::pos().y() - widgetRect.y()),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        } else if(pos_y >= BRAINHEIGHT){
            emit settingMousePosSignal(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, (widgetRect.height() + BRAINHEIGHT - 2) / 2));
            brainGamekeyMethod(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, (widgetRect.height() + BRAINHEIGHT - 2) / 2),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        }
    } else if(pos_x > 0 && pos_x < BRAINWIDTH){
        if(pos_y <= 0){
            emit settingMousePosSignal(QPoint(QCursor::pos().x() - widgetRect.x(), (widgetRect.height() - BRAINHEIGHT + 2) / 2));
            brainGamekeyMethod(QPoint(QCursor::pos().x() - widgetRect.x(), (widgetRect.height() - BRAINHEIGHT + 2) / 2),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        } else if(pos_y > 0 && pos_y < BRAINHEIGHT){
            brainGamekeyMethod(QPoint(QCursor::pos().x() - widgetRect.x(), QCursor::pos().y() - widgetRect.y()),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        } else if(pos_y >= BRAINHEIGHT){
            emit settingMousePosSignal(QPoint(QCursor::pos().x() - widgetRect.x(), (widgetRect.height() + BRAINHEIGHT - 2) / 2));
            brainGamekeyMethod(QPoint((widgetRect.width() - BRAINWIDTH + 2) / 2, (widgetRect.height() + BRAINHEIGHT - 2) / 2),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        }
    } else if(pos_x >= BRAINWIDTH){
        if(pos_y <= 0){
            emit settingMousePosSignal(QPoint((widgetRect.width() + BRAINWIDTH - 2) / 2, (widgetRect.height() - BRAINHEIGHT + 2) / 2));
            brainGamekeyMethod(QPoint((widgetRect.width() + BRAINWIDTH - 2) / 2, (widgetRect.height() - BRAINHEIGHT + 2) / 2),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        } else if(pos_y > 0 && pos_y < BRAINHEIGHT){
            emit settingMousePosSignal(QPoint((widgetRect.width() + BRAINWIDTH - 2) / 2, QCursor::pos().y() - widgetRect.y()));
            brainGamekeyMethod(QPoint((widgetRect.width() + BRAINWIDTH - 2) / 2, QCursor::pos().y() - widgetRect.y()),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        } else if(pos_y >= BRAINHEIGHT){
            emit settingMousePosSignal(QPoint((widgetRect.width() + BRAINWIDTH - 2) / 2, (widgetRect.height() + BRAINHEIGHT - 2) / 2));
            brainGamekeyMethod(QPoint((widgetRect.width() + BRAINWIDTH - 2) / 2, (widgetRect.height() + BRAINHEIGHT - 2) / 2),
                               QPoint(widgetRect.width() / 2, widgetRect.height() / 2));
        }
    }
}

void KeyboardGamekeyManager::brainGamekeyMethod(QPoint mousePos, QPoint widgetPos){
    QPoint brainPos = mMainWindow->getJoystickManager()->getMousePos(QPoint(mousePos.x() - widgetPos.x(), mousePos.y() - widgetPos.y()), 1);
    if(mBrainPressGameKey != ""){
        mBrainInfo.mBrainPosX = mBrainInfo.mBrainPosXPre + brainPos.x();
        mBrainInfo.mBrainPosY = mBrainInfo.mBrainPosYPre + brainPos.y();
        gameKeyMove(mBrainInfo.mBrainPosX, mBrainInfo.mBrainPosY, mBrainPressGameKey);
    }
}
