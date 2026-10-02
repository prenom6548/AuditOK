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

#include "joystickmanager.h"
#include "kmrewindow.h"
#include "displaymanager.h"
#include "android_display/displaywidget.h"
#include "joystickgamekeymanager.h"
#include "keyboardgamekeymanager.h"
#include "toast.h"
#include "JoystickCommon.h"
#include "widgets/messagebox.h"
#include "joystickautokey.h"
#include "kmreenv.h"
#include "sessionsettings.h"

#include <QDebug>
#include <QApplication>
#include <sys/syslog.h>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QMessageBox>
#include <typeinfo>

using namespace kmre;

JoystickManager::JoystickManager(KmreWindow* window)
    : mMainWindow(window)
{
    createGameKeysXmlDir();
    initGameKeyTransparency();
    mValidRect = getGameKeyValidRect();
}

JoystickManager::~JoystickManager()
{
    removeJoystickKeys();
    removeKeyMouseKeys();
    if(shadeShell){
        delete shadeShell;
        shadeShell = nullptr;
    }
    if(mIsJoystickSetWidgetInitialed){
        disconnect(timer,SIGNAL(timeout()),this,SLOT(timerCast()));
        disconnect(mMainWindow->getJoystickGameKeyManager(),SIGNAL(joystickChange(bool)),this,SLOT(joystickConnect(bool)));
        disconnect(this,SIGNAL(sigJoystickKeysReady()),mMainWindow->getJoystickGameKeyManager(),SLOT(gameKeyInit()));
        disconnect(this,SIGNAL(sigKeyMouseKeysReady()),mMainWindow->getKeyboardGamekeyManager(),SLOT(gameKeyInit()));
    }
}

//初始化手柄设置界面
void JoystickManager::initJoystickSetWidget(){
    if (mMainWindow->getDisplayManager()->isMultiDisplayEnabled()) {
        KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),
                                     tr("Don't support game key under multi display mode! Please disable multi display mode of this app and restart!"));
        syslog(LOG_DEBUG,"[JoystickManager] initJoystickSetWidget Don't support game key under multi display mode!");
        return;
    }

    timer = new QTimer(this); // 产生一个定时器,这个定时器是暂时的，后边增辉会完善他给出的信号
    connect(timer,SIGNAL(timeout()),this,SLOT(timerCast()));
    connect(mMainWindow->getJoystickGameKeyManager(),SIGNAL(joystickChange(bool)),this,SLOT(joystickConnect(bool)));
    connect(this,SIGNAL(sigJoystickKeysReady()),mMainWindow->getJoystickGameKeyManager(),SLOT(gameKeyInit()));
    connect(this,SIGNAL(sigKeyMouseKeysReady()),mMainWindow->getKeyboardGamekeyManager(),SLOT(gameKeyInit()));
    gameKeyStackWidget = mMainWindow->getDisplayManager()->getGameKeyStackWidget();

    if(mMainWindow->getJoystickGameKeyManager()->getJoystickCount()){   //判断当前是否插着摇杆
        mIsJoystickConnect = true;
        mIsUseJoystickRock = true;
    }else{  //这里要初始化键鼠        
        mIsJoystickConnect = false;
        mIsUseJoystickRock = false;
    }
    setJoystickSetWidgetVisible(true);
    mIsJoystickSetWidgetInitialed = true;
}

void JoystickManager::setJoystickSetWidgetVisible(bool isVisable){
    mIsJoystickSetWidgetVisible = isVisable;
    if(isVisable){
        addShell();      //如果是进入编辑模式，需要显示遮罩
    }else{
        if(shadeShell){
            delete shadeShell;
            shadeShell = nullptr;
        }
    }
    mMainWindow->getDisplayManager()->setJoystickEditMode(isVisable);   //对右侧的编辑栏、标题栏及界面等进行操作
}
//获取遮罩弹窗的坐标及尺寸大小
void JoystickManager::getBenchMarkPos(int &xpos, int &ypos, int &width, int &height){
    QRect mainWidgetSize =  mMainWindow->frameGeometry();
    xpos = mainWidgetSize.x();
    ypos = mainWidgetSize.y()+DEFAULT_TITLEBAR_HEIGHT;
    width = mainWidgetSize.width();
    height = mainWidgetSize.height()-DEFAULT_TITLEBAR_HEIGHT;
}
QRect JoystickManager::getValidOperateArea(){
    return mValidRect;
}
QRect JoystickManager::getGameKeyValidRect(){
    int displayWidth, displayHeight;
    getMainWidgetDisplaySize(displayWidth, displayHeight);

    if (mMainWindow->isFullScreen()) {
        QRect screenSize = mMainWindow->getDisplayManager()->getScreenSize();
        int pos_x = (screenSize.width() - displayWidth) / 2;
        int pos_y = (screenSize.height() - displayHeight) / 2;
        mValidRect = QRect(pos_x, pos_y, displayWidth, displayHeight);
        return mValidRect;
    }
    else {
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            mValidRect = QRect(MOUSE_MARGINS,
                               DEFAULT_TITLEBAR_HEIGHT,
                               displayWidth,
                               displayHeight);
            return mValidRect;
        } else {
            QRect mainWidgetRect = mMainWindow->geometry();
            mValidRect = QRect(mainWidgetRect.x() + MOUSE_MARGINS,
                              mainWidgetRect.y() + DEFAULT_TITLEBAR_HEIGHT,
                              displayWidth,
                              displayHeight);
            return mValidRect;
        }
    }
}

void JoystickManager::addShell(){
    if(!shadeShell){
        shadeShell = new ShadeMask(mMainWindow);
    }
    shadeShell->showShadeWidget(true);
    //shadeShell->lower();
}

void JoystickManager::addJoystickKey(int type){
    int displayWidth, displayHeight, initialWidth, initialHeight;
    getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    /**********单个按键使用*************/
    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);
    int xpos = (mJoystickIndex % 5 +1) * 60/width_scale_ratio ;
    int ypos = (mJoystickIndex / 5 +1) * 60/height_scale_ratio ;
    double x = xpos / static_cast<double>(displayWidth);  //这个操作是为了应对屏幕尺寸变化的
    double y = ypos / static_cast<double>(displayHeight);
    /**********摇杆和方向键使用*************/
    double widthScaleRatio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialWidth);
    double heightScaleRatio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialHeight);

    AddKeyMsgStruct msgStruct = {"",x,y,true,5,widthScaleRatio,heightScaleRatio,mJoystickIndex,"","","",""};
    mJoystickIndex++;

    switch (type) {
    case JoystickBaseKey::SINGLEKEY:
        msgStruct.keyType = JoystickBaseKey::SINGLEKEY;
        _addSingleGameKey(msgStruct,true);
        break;
    case JoystickBaseKey::COMBOKEY:
        msgStruct.keyType = JoystickBaseKey::COMBOKEY;
        _addSingleGameKey(msgStruct,true);
        break;
    case JoystickBaseKey::AUTOMETHOD:
        msgStruct.keyType = JoystickBaseKey::AUTOMETHOD;
        _addSingleGameKey(msgStruct,true);
        break;
    case JoystickBaseKey::KeyType::JOYLEFT:
        _addLeftJoystick(msgStruct);
        break;
    case JoystickBaseKey::KeyType::JOYRIGHT:
        _addRightJoystick(msgStruct);
        break;
    default:
        syslog(LOG_WARNING,"[JoystickManager] addJoystickKey err key type=%d!",type);
        break;
    }
}

void JoystickManager::deleteJoystickKey(int type,int idx){
    switch (type) {
    case JoystickBaseKey::KeyType::SINGLEKEY:
        _deleteJoystickSingleKey(idx);
        break;
    case JoystickBaseKey::KeyType::COMBOKEY:
        _deleteJoystickSingleKey(idx);
        break;
    case JoystickBaseKey::KeyType::AUTOMETHOD:
        _deleteJoystickSingleKey(idx);
        break;
    case JoystickBaseKey::KeyType::JOYLEFT:
        if(mJoystickLeftRock){
            disconnect(mJoystickLeftRock, SIGNAL(deleteLeftJoystickKey()), this, SLOT(deleteLeftJoystickKey()));
            delete mJoystickLeftRock;
            mJoystickLeftRock = nullptr;
        }
        break;
    case JoystickBaseKey::KeyType::JOYRIGHT:
        if(mJoystickRightRock){
            disconnect(mJoystickRightRock, SIGNAL(deleteRightJoystickKey()), this, SLOT(deleteRightJoystickKey()));
            delete mJoystickRightRock;
            mJoystickRightRock = nullptr;
        }
        break;
    default:
        syslog(LOG_WARNING,"[JoystickManager] deleteJoystickKey err key type=%d!",type);
        break;
    }
}

//删除单个按键的方法
void JoystickManager::_deleteJoystickSingleKey(int idx){
    for(auto game_key : mGameKeys) {
        if (game_key->getIndex() == idx) {
            disconnect(game_key, SIGNAL(deleteGameKey(int)), this, SLOT(deleteKey(int)));
            delete game_key;
            mGameKeys.removeAll(game_key);
            break;
        }
    }
}
bool JoystickManager::_addSingleGameKey(AddKeyMsgStruct msgStruct,bool isJoystick){
    if ((msgStruct.x <= 0) || (msgStruct.y <= 0)) {
        syslog(LOG_ERR, "[JoystickManager] _addKey(%d) failed!: x = %f, y = %f", mJoystickIndex,msgStruct.x, msgStruct.y);
        return false;
    }
    if(isJoystick){
        if(mGameKeys.size()>DEFAULT_GAME_KEY_MAX_SIZE){
            KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),tr("Can't add more game key!"));
            return false;
        }
    }else{
        if(mKeyMouseKeysList.size()>DEFAULT_GAME_KEY_MAX_SIZE){
            KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),tr("Can't add more game key!"));
            return false;
        }
    }
    JoystickSingleBaseKey* gameKey;
    switch (msgStruct.keyType) {
    case JoystickBaseKey::SINGLEKEY:
        gameKey = new JoystickSingleKey(mMainWindow);
        break;
    case JoystickBaseKey::COMBOKEY:
        gameKey = new JoystickComboKey(mMainWindow);
        static_cast<JoystickComboKey*>(gameKey)->setComboFreq(msgStruct.comboFreq);
        break;
    case JoystickBaseKey::AUTOMETHOD:
        gameKey = new JoystickAutoKey(mMainWindow);
        break;
    default:
        syslog(LOG_ERR, "[JoystickManager] _addKey(%d) failed!: type is err",msgStruct.keyType);
        return false;
    }
    gameKey->setGeometryPos(msgStruct.x,msgStruct.y);
    gameKey->setKeyString(msgStruct.key);
    gameKey->updateSize();
    gameKey->updatePos();
    gameKey->setIndex(msgStruct.index);
    if(isJoystick){
        connect(gameKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteJoystickKey(int,int)));
        gameKey->showKey(msgStruct.show);
        mGameKeys.push_back(gameKey);
        return true;
    }else{
        connect(gameKey,SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        connect(gameKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        connect(gameKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        connect(gameKey,SIGNAL(mouseRightPress(QMouseEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getMouseGamekeyPress(QMouseEvent*)));
        gameKey->showKey(msgStruct.show);
        mKeyMouseKeysList.push_back(gameKey);
        return true;
    }
}

bool JoystickManager::_addLeftJoystick(AddKeyMsgStruct msgStruct){
    if (!mJoystickLeftRock) {
        if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
            mJoystickLeftRock = new JoystickLeftRock(mMainWindow);
            mJoystickLeftRock->setGeometryPos(msgStruct.x, msgStruct.y);
            mJoystickLeftRock->setGeometryHWRatio(msgStruct.widthRatio,msgStruct.heigthRatio);
            mJoystickLeftRock->updateSize();
            mJoystickLeftRock->updatePos();
        }else {
            syslog(LOG_ERR, "[JoystickManager] addSteelingWheel failed! x = %f, y = %f", msgStruct.x,msgStruct.y);
            return false;
        }
        connect(mJoystickLeftRock, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteJoystickKey(int,int)));
    }
    mJoystickLeftRock->showKey(msgStruct.show);
    return true;
}

bool JoystickManager::_addRightJoystick(AddKeyMsgStruct msgStruct){
    if (!mJoystickRightRock) {
        if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
            mJoystickRightRock = new JoystickRightRock(mMainWindow);
            mJoystickRightRock->setGeometryPos(msgStruct.x,msgStruct.y);
            mJoystickRightRock->updateSize();
            mJoystickRightRock->updatePos();
        }else {
            syslog(LOG_ERR, "[JoystickManager] addSteelingWheel failed! x = %f, y = %f", msgStruct.x, msgStruct.y);
            return false;
        }
        connect(mJoystickRightRock, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteJoystickKey(int,int)));
    }
    mJoystickRightRock->showKey(msgStruct.show);
    return true;
}

void JoystickManager::addKeyMouseKey(int type){
    int displayWidth, displayHeight, initialWidth, initialHeight;
    getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    double xpos = (mKeyMouseIndex % 5 +1) * 60 / static_cast<double>(displayWidth);
    double ypos = (mKeyMouseIndex / 5 +1) * 60 / static_cast<double>(displayHeight);
    double widthScaleRatio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialWidth);
    double heightScaleRatio = DEFAULT_JOYSTICK_ROCKER_SIZE / static_cast<double>(initialHeight);

    AddKeyMsgStruct msgStruct = {"",xpos,ypos,true,5,widthScaleRatio,heightScaleRatio,mKeyMouseIndex,"","","",""};
    mKeyMouseIndex++;

    switch (type) {
    case JoystickBaseKey::SINGLEKEY:
        msgStruct.keyType = JoystickBaseKey::SINGLEKEY;
        _addSingleGameKey(msgStruct,false);
        break;
    case JoystickBaseKey::COMBOKEY:
        msgStruct.keyType = JoystickBaseKey::COMBOKEY;
        _addSingleGameKey(msgStruct,false);
        break;
    case JoystickBaseKey::AUTOMETHOD:
        msgStruct.keyType = JoystickBaseKey::AUTOMETHOD;
        _addSingleGameKey(msgStruct,false);
        break;
    case JoystickBaseKey::DIRECTION:
        _addDirectionKey(msgStruct);
        break;
    case JoystickBaseKey::FIRE:
        _addKeyMouseFireKey(msgStruct);
        break;
    case JoystickBaseKey::CROSSHAIR:
        _addKeyMouseCrossHairKey(msgStruct,Qt::Checked);
        break;
    case JoystickBaseKey::VIEW:
        _addViewKey(msgStruct);
        break;
    case JoystickBaseKey::ACCELATE:
        _addAcceleKey(msgStruct);
        break;
    default:
        syslog(LOG_WARNING,"[JoystickManager] addKeyMouseKey err key type=%d!",type);
        break;
    }
}
bool JoystickManager::_addAcceleKey(AddKeyMsgStruct msgStruct){
    if (!mAccelateKey) {
        if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
            mAccelateKey = new AccelateKey(mMainWindow);
            mAccelateKey->setDefaultKeyString(msgStruct.topStr,msgStruct.rightStr,msgStruct.bottomStr,msgStruct.leftStr);
            mAccelateKey->setGeometryPos(msgStruct.x, msgStruct.y);
            mAccelateKey->setGeometryHWRatio(msgStruct.widthRatio,msgStruct.heigthRatio);
            mAccelateKey->updateSize();
            mAccelateKey->updatePos();
        }else {
            syslog(LOG_ERR, "[JoystickManager] _addAcceleKey failed! x = %f, y = %f",  msgStruct.x,msgStruct.y);
            return false;
        }
        connect(mAccelateKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        connect(mAccelateKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        connect(mAccelateKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
    }
    mAccelateKey->showKey(msgStruct.show);
    return true;
}

bool JoystickManager::_addDirectionKey(AddKeyMsgStruct msgStruct){
    if (!mJoystickDirection) {
        if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
            mJoystickDirection = new JoystickDirectKey(mMainWindow);
            mJoystickDirection->setDefaultKeyString(msgStruct.topStr,msgStruct.rightStr,msgStruct.bottomStr,msgStruct.leftStr);
            mJoystickDirection->setGeometryPos(msgStruct.x, msgStruct.y);
            mJoystickDirection->setGeometryHWRatio(msgStruct.widthRatio,msgStruct.heigthRatio);
            mJoystickDirection->updateSize();
            mJoystickDirection->updatePos();
        }else {
            syslog(LOG_ERR, "[JoystickManager] _addDirectionKey failed! x = %f, y = %f",  msgStruct.x,msgStruct.y);
            return false;
        }
        connect(mJoystickDirection, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        connect(mJoystickDirection,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        connect(mJoystickDirection,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
    }
    mJoystickDirection->showKey(msgStruct.show);
    return true;
}
bool JoystickManager::_addKeyMouseFireKey(AddKeyMsgStruct msgStruct){
    if(!mMouseFireKey){
       if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
           mMouseFireKey = new MouseFireKey(mMainWindow);
           mMouseFireKey->setGeometryPos(msgStruct.x, msgStruct.y);
           mMouseFireKey->setKeyString("MOUSELEFT");
           mMouseFireKey->updateSize();
           mMouseFireKey->updatePos();
           mMouseFireKey->setIndex(msgStruct.index);
       }else{
           syslog(LOG_ERR, "[JoystickManager] _addKeyMouseFireKey failed! x = %f, y = %f",  msgStruct.x,msgStruct.y);
           return false;
       }
       connect(mMouseFireKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
       connect(mMouseFireKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
       connect(mMouseFireKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));

       mKeyMouseKeysList.push_back(mMouseFireKey);

    }
    mMouseFireKey->showKey(msgStruct.show);
    return true;
}
bool JoystickManager::_addKeyMouseCrossHairKey(AddKeyMsgStruct msgStruct,int checkStatus){
    if(!mMouseCrossHairKey){
        if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
            mMouseCrossHairKey = new MouseCrossHairKey(mMainWindow);
            mMouseCrossHairKey->setKeyString(msgStruct.key);
            mMouseCrossHairKey->setGeometryPos(msgStruct.x, msgStruct.y);
            mMouseCrossHairKey->updateSize();
            mMouseCrossHairKey->updatePos();
            mMouseCrossHairKey->setCheckBoxStatus(checkStatus);
            mMouseCrossHairKey->setComboFreq(msgStruct.comboFreq);
        }else{
            syslog(LOG_ERR, "[JoystickManager] _addKeyMouseCrossHairKey failed! x = %f, y = %f",  msgStruct.x,msgStruct.y);
            return false;
        }
        connect(mMouseCrossHairKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        connect(mMouseCrossHairKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        connect(mMouseCrossHairKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
    }
    mMouseCrossHairKey->showKey(msgStruct.show);
    return true;
}
bool JoystickManager::_addViewKey(AddKeyMsgStruct msgStruct){
    if(!mMouseViewKey){
        if ((msgStruct.x >= 0) && (msgStruct.y >= 0)){
            mMouseViewKey = new MouseViewKey(mMainWindow);
            mMouseViewKey->setDefaultKeyString(msgStruct.topStr,msgStruct.rightStr,msgStruct.bottomStr,msgStruct.leftStr);
            mMouseViewKey->setGeometryPos(msgStruct.x, msgStruct.y);
            mMouseViewKey->setGeometryHWRatio(msgStruct.widthRatio,msgStruct.heigthRatio);
            mMouseViewKey->updateSize();
            mMouseViewKey->updatePos();
        }else{
            syslog(LOG_ERR, "[JoystickManager] _addViewKey failed! x = %f, y = %f",  msgStruct.x,msgStruct.y);
            return false;
        }
        connect(mMouseViewKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        connect(mMouseViewKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        connect(mMouseViewKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
    }
    mMouseViewKey->showKey(msgStruct.show);
    return true;
}


void JoystickManager::deleteKeyMouseKey(int type,int idx){
    switch (type) {
    case JoystickBaseKey::SINGLEKEY:
        _deleteKeyMouseSingleKey(idx);
        break;
    case JoystickBaseKey::COMBOKEY:
        _deleteKeyMouseSingleKey(idx);
        break;
    case JoystickBaseKey::DIRECTION:
        if(mJoystickDirection){
            disconnect(mJoystickDirection, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
            disconnect(mMouseCrossHairKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
            disconnect(mMouseCrossHairKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
            delete mJoystickDirection;
            mJoystickDirection = nullptr;
        }
        break;
    case JoystickBaseKey::FIRE:
        if(mMouseFireKey){
            _deleteKeyMouseSingleKey(idx);
            mMouseFireKey = nullptr;
        }
        break;
    case JoystickBaseKey::CROSSHAIR:
        if(mMouseCrossHairKey){
            disconnect(mMouseCrossHairKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
            disconnect(mMouseCrossHairKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
            disconnect(mMouseCrossHairKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
            delete mMouseCrossHairKey;
            mMouseCrossHairKey = nullptr;
        }
        break;
    case JoystickBaseKey::VIEW:
        if(mMouseViewKey){
            disconnect(mMouseViewKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
            disconnect(mMouseViewKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
            disconnect(mMouseViewKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
            delete mMouseViewKey;
            mMouseViewKey = nullptr;
        }
        break;
    case JoystickBaseKey::ACCELATE:
        if(mAccelateKey){
            disconnect(mAccelateKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
            disconnect(mAccelateKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
            disconnect(mAccelateKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
            delete mAccelateKey;
            mAccelateKey = nullptr;
        }
        break;
    default:
        syslog(LOG_WARNING,"[JoystickManager] deleteKeyMouseKey err key type=%d!",type);
        break;
    }
}

void JoystickManager::_deleteKeyMouseSingleKey(int idx){
    for(auto key : mKeyMouseKeysList) {
        if (key->getIndex() == idx) {
            disconnect(key, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
            disconnect(key,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
            disconnect(key,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
            disconnect(key,SIGNAL(mouseRightPress(QMouseEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getMouseGamekeyPress(QMouseEvent*)));
            delete key;
            mKeyMouseKeysList.removeAll(key);
            break;
        }
    }
}
//意味着隐藏按键，但是仍然使用外设，后期要做成checkbox，标记为是否显示当前的按键
void JoystickManager::hideGameKeys(){

}
//这里暂时不使用隐藏的逻辑，直接删除
void JoystickManager::hideGameKeysWhileRotation(){
    if(!mIsJoystickSetWidgetInitialed){
       return;
    }
    if(mGameKeyStatus == Default ||mGameKeyStatus == OnlyInit){
        return;
    }else if(mGameKeyStatus == JoystickEditing ||mGameKeyStatus == KeyMouseEditing ||mGameKeyStatus == NoJostick){
        removeKeyMouseKeys();
        removeJoystickKeys();
        setJoystickSetWidgetVisible(false);
        setGameKeyStatus(OnlyInit);
    }else if(mGameKeyStatus == JoystickUse){
        removeGameKeys(true);
        clearGameKeysConfigFile(true);
    }else if(mGameKeyStatus == KeyMouseUse){
        removeGameKeys(false);
        clearGameKeysConfigFile(false);
    }else{
        syslog(LOG_WARNING, "[JoystickManager] hideGameKeysWhileRotation undefine GameKeyStatus %d",mGameKeyStatus);
    }
}
//保存设置，意味着保存当前的所有按键设置，并且退出设置界面，不销毁资源
void JoystickManager::saveGameKeys(bool isJoystick){
    if(isJoystick){
        _saveJoystickKeys();
        //刷新一次界面
        //mMainWindow->getDisplayManager()->onDisplayForceRedraw();
    }else{
        _saveKeyMouseKeys(); 
    }
}

void JoystickManager::_saveJoystickKeys(){
    removeKeyMouseKeys();
    saveGameKeysConfig();
    writeGameKeysConfigToFile();
    enableJoystickKeyEdit(false);       //编辑功能一定是保存完之后才关闭，因为要记录控件的信息
    setJoystickSetWidgetVisible(false);
    setGameKeyStatus(JoystickUse);
    mIsUseJoystickRock = true;
}

void JoystickManager::_saveKeyMouseKeys(){
    if(!saveKeyMouseKeysConfig()){
        return;
    }
    removeJoystickKeys();
    writeKeyMouseKeysConfigToFile();
    enableKeyMouseEdit(false);
    setJoystickSetWidgetVisible(false);
    setGameKeyStatus(KeyMouseUse);
    mIsUseJoystickRock = false;
}

void JoystickManager::exitJoystickSetting(){
    if(isHadEditNotBeSave()){
        bool result = KylinUI::MessageBox::question(mMainWindow,tr("Warning"),
                                     tr("Do you want to save the modification?"));
        if(result){
            if(mGameKeyStatus == JoystickEditing){
                saveGameKeys(true);
                emit sigJoystickKeysReady();
            }else{
                saveGameKeys(false);
                emit sigKeyMouseKeysReady();
            }
            return;
        }else{
            removeKeyMouseKeys();
            removeJoystickKeys();
            if(mGameKeyStatus == JoystickEditing){
                initJoystickKeysFromFile();
                showJoystickKeys(true);
                enableJoystickKeyEdit(false);
                setGameKeyStatus(JoystickUse);
                mIsUseJoystickRock = true;
                emit sigJoystickKeysReady();
            }else if(mGameKeyStatus == KeyMouseEditing){
                initKeyMouseKeysFromFile();
                showKeyMouseKeys(true);
                enableKeyMouseEdit(false);
                setGameKeyStatus(KeyMouseUse);
                mIsUseJoystickRock = false;
                emit sigKeyMouseKeysReady();
            }else if(mGameKeyStatus == NoJostick){
                setGameKeyStatus(OnlyInit);
                mIsUseJoystickRock = false;
            }
            setJoystickSetWidgetVisible(false);
        }
    }else{
        if(mGameKeyStatus == JoystickEditing){
            removeKeyMouseKeys();
            enableJoystickKeyEdit(false);
            setGameKeyStatus(JoystickUse);
            emit sigJoystickKeysReady();
        }else if(mGameKeyStatus == KeyMouseEditing){
            removeJoystickKeys();
            enableKeyMouseEdit(false);
            setGameKeyStatus(KeyMouseUse);
            emit sigKeyMouseKeysReady();
        }else if(mGameKeyStatus == NoJostick){
            removeKeyMouseKeys();
            removeJoystickKeys();
            setGameKeyStatus(OnlyInit);
        }
        setJoystickSetWidgetVisible(false);
    }

}
//移除所有的按键
void JoystickManager::removeGameKeys(bool isJoystick){
    if(isJoystick){
        removeJoystickKeys();
    }else{
        removeKeyMouseKeys();
    }
}
void JoystickManager::removeKeyMouseKeys(){
    for(auto key : mKeyMouseKeysList){
        disconnect(key, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        disconnect(key,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        disconnect(key,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        disconnect(key,SIGNAL(mouseRightPress(QMouseEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getMouseGamekeyPress(QMouseEvent*)));
        delete key;
    }
    mMouseFireKey = nullptr;
    mKeyMouseKeysList.clear();
    mKeyMouseIndex = 0;
    if(mMouseCrossHairKey){
        disconnect(mMouseCrossHairKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        disconnect(mMouseCrossHairKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        disconnect(mMouseCrossHairKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        delete mMouseCrossHairKey;
        mMouseCrossHairKey = nullptr;
    }
    if(mJoystickDirection){
        disconnect(mJoystickDirection, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        disconnect(mJoystickDirection,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        disconnect(mJoystickDirection,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        delete mJoystickDirection;
        mJoystickDirection = nullptr;
    }
    if(mMouseViewKey){
        disconnect(mMouseViewKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        disconnect(mMouseViewKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        disconnect(mMouseViewKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        delete mMouseViewKey;
        mMouseViewKey = nullptr;
    }
    if(mAccelateKey){
        disconnect(mAccelateKey, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteKeyMouseKey(int,int)));
        disconnect(mAccelateKey,SIGNAL(keyboardPress(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        disconnect(mAccelateKey,SIGNAL(keyboardRelease(QKeyEvent*)),mMainWindow->getKeyboardGamekeyManager(), SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        delete mAccelateKey;
        mAccelateKey = nullptr;
    }
    mIsKeyMouseConfigFileInit = false;
}
void JoystickManager::removeJoystickKeys(){
    while(mGameKeys.size()){
        delete mGameKeys.back();  //真正的删除
        mGameKeys.pop_back();   //并没有真的把原来数组内存里面的数值给删掉，而是仅仅取消了数组最后一个元素的地址映射
    }
    mJoystickIndex = 0;
    if(mJoystickLeftRock){
        disconnect(mJoystickLeftRock, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteJoystickKey(int,int)));
        delete mJoystickLeftRock;
        mJoystickLeftRock = nullptr;
    }
    if(mJoystickRightRock){
        disconnect(mJoystickRightRock, SIGNAL(deleteGameKey(int,int)), this, SLOT(deleteJoystickKey(int,int)));
        delete mJoystickRightRock;
        mJoystickRightRock = nullptr;
    }
    mIsJoystickConfigFileInit = false;
}

void JoystickManager::showJoystickKeys(bool visible){
    for(auto key:mGameKeys){
        key->showKey(visible);
        key->setOpacity(mTransparency);
    }
    if(mJoystickLeftRock){
        mJoystickLeftRock->showKey(visible);
        mJoystickLeftRock->setOpacity(mTransparency);
    }
    if(mJoystickRightRock){
        mJoystickRightRock->showKey(visible);
        mJoystickRightRock->setOpacity(mTransparency);
    }
}
void JoystickManager::showKeyMouseKeys(bool visible){
    for(auto key:mKeyMouseKeysList){
        key->showKey(visible);
        key->setOpacity(mTransparency);
    }
    if(mJoystickDirection){
        mJoystickDirection->showKey(visible);
        mJoystickDirection->setOpacity(mTransparency);
    }
    if(mMouseViewKey){
        mMouseViewKey->showKey(visible);
        mMouseViewKey->setOpacity(mTransparency);
    }
    if(mAccelateKey){
        mAccelateKey->showKey(visible);
        mAccelateKey->setOpacity(mTransparency);
    }
    if(mMouseCrossHairKey){  //这个按键很特殊，只更新位置，使用时不能真的显示出来
        if(mGameKeyStatus == KeyMouseUse){
            return;
        }
        mMouseCrossHairKey->showKey(visible);
        mMouseCrossHairKey->setOpacity(mTransparency);
    }
}

void JoystickManager::updateGameKeySizeAndPos(){
    if(mGameKeyStatus == Default || mGameKeyStatus == OnlyInit){
        return;
    }
    updateGameKeySize();
    updateGameKeyPos();
}
void JoystickManager::updateGameKeySize(){
    if(mGameKeyStatus == JoystickUse){
        _updateJoystickSize();
    }else if(mGameKeyStatus == KeyMouseUse){
        _updateKeyMouseSize();
    }else if(mGameKeyStatus == JoystickEditing ||mGameKeyStatus == KeyMouseEditing){
        _updateJoystickSize();
        _updateKeyMouseSize();
    }
}
void JoystickManager::_updateJoystickSize(){
    for(auto key:mGameKeys){
        key->updateSize();
    }
    if(mJoystickLeftRock){
        mJoystickLeftRock->updateSize();
    }
    if(mJoystickRightRock){
        mJoystickRightRock->updateSize();
    }
}
void JoystickManager::_updateKeyMouseSize(){
    for(auto key : mKeyMouseKeysList){
        key->updateSize();
    }
    if(mMouseCrossHairKey){
        mMouseCrossHairKey->updateSize();
    }
    if(mJoystickDirection){
        mJoystickDirection->updateSize();
    }
    if(mMouseViewKey){
        mMouseViewKey->updateSize();
    }
    if(mAccelateKey){
        mAccelateKey->updateSize();
    }
}
void JoystickManager::updateGameKeyPos(){
    if(mGameKeyStatus == JoystickUse){
        _updateJoystickPos();
    }else if(mGameKeyStatus == KeyMouseUse){
        _updateKeyMousePos();
    }else if(mGameKeyStatus == JoystickEditing || mGameKeyStatus == KeyMouseEditing){
        _updateJoystickPos();
        _updateKeyMousePos();
    }
}
void JoystickManager::_updateJoystickPos(){
    for(auto key : mGameKeys){
        key->updatePos();
    }
    if(mJoystickLeftRock){
        mJoystickLeftRock->updatePos();
    }
    if(mJoystickRightRock){
        mJoystickRightRock->updatePos();
    }
}
void JoystickManager::_updateKeyMousePos(){
    for(auto key : mKeyMouseKeysList){
        key->updatePos();
    }
    if(mMouseCrossHairKey){
        mMouseCrossHairKey->updatePos();
    }
    if(mJoystickDirection){
        mJoystickDirection->updatePos();
    }
    if(mMouseViewKey){
        mMouseViewKey->updatePos();
    }
    if(mAccelateKey){
        mAccelateKey->updatePos();
    }
}

void JoystickManager::enableJoystickKeyEdit(bool enable){
    for(auto game_key : mGameKeys) {
        game_key->enableEdit(enable);
    }
    if(mJoystickLeftRock){
        mJoystickLeftRock->enableEdit(enable);
    }
    if(mJoystickRightRock){
        mJoystickRightRock->enableEdit(enable);
    }
}

void JoystickManager::enableKeyMouseEdit(bool enable){
    for(auto game_key : mKeyMouseKeysList) {
        game_key->enableEdit(enable);
    }
    if(mJoystickDirection){
        mJoystickDirection->enableEdit(enable);
    }
    if(mMouseCrossHairKey){
        mMouseCrossHairKey->enableEdit(enable);
    }
    if(mMouseFireKey){
        mMouseFireKey->enableEdit(enable);
    }
    if(mMouseViewKey){
        mMouseViewKey->enableEdit(enable);
    }
    if(mAccelateKey){
        mAccelateKey->enableEdit(enable);
    }
}

void JoystickManager::hideGameKeysWhileMove(){
    if(mGameKeyStatus == Default || mGameKeyStatus == OnlyInit){
        return;
    }

    if(mGameKeyStatus == JoystickEditing){
        showJoystickKeys(false);
        shadeShell->showShadeWidget(false);
    }else if(mGameKeyStatus == JoystickUse){
        showJoystickKeys(false);
    }else if(mGameKeyStatus == KeyMouseEditing){
        showKeyMouseKeys(false);
        shadeShell->showShadeWidget(false);
    }else if(mGameKeyStatus == KeyMouseUse){
        showKeyMouseKeys(false);
    }else if(mGameKeyStatus == NoJostick){
        shadeShell->showShadeWidget(false);
    }
}
void JoystickManager::showGameKeysAfterMove(){
    if(mMainWindow->getShakeStatus()) {
        return;
    }

    updateGameKeyPos();
    if(mGameKeyStatus == JoystickEditing){
        shadeShell->updatePos();
        shadeShell->showShadeWidget(true);
        showJoystickKeys(true);
    }else if(mGameKeyStatus == JoystickUse){
        showJoystickKeys(true);
    }else if(mGameKeyStatus == KeyMouseEditing){
        shadeShell->updatePos();
        shadeShell->showShadeWidget(true);
        showKeyMouseKeys(true);
    }else if(mGameKeyStatus == KeyMouseUse){
        showKeyMouseKeys(true);
    }else if(mGameKeyStatus == NoJostick){
        shadeShell->updatePos();
        shadeShell->showShadeWidget(true);
    }
}
void JoystickManager::setJoystickKeyStringSlot(QString string){
    if(mGameKeyStatus!=JoystickEditing){
        return;
    }
    if(isJoystickKeysExist(string)){
        KylinUI::MessageBox::warning(mMainWindow, tr("Warning"), tr("The Key is existing, please input another key!"));
        return;
    }
    foreach (JoystickSingleKey* singleKey, mGameKeys) {
        if(singleKey->isLineEditHasFocus()){
            singleKey->setKeyString(string);
            return;
        }
    }
}

void JoystickManager::setKeyMouseStringSlot(QString string){
    if(mGameKeyStatus!=KeyMouseEditing){
        return;
    }
    if(isKeyMouseValueExist(string)){
        KylinUI::MessageBox::warning(mMainWindow, tr("Warning"), tr("The Key is existing, please input another key!"));
        return;
    }
    foreach (JoystickSingleKey* singleKey, mKeyMouseKeysList) {
        if(singleKey->isLineEditHasFocus()){
            singleKey->setKeyString(string);
            return;
        }
    }
    if(mMouseCrossHairKey){
        if(mMouseCrossHairKey->getLineEdit()->hasFocus()){
            mMouseCrossHairKey->setKeyString(string);
            return;
        }

    }
    if(mJoystickDirection){
        if(mJoystickDirection->isHavingFocus()){
            mJoystickDirection->setKeyString(string);
            return;
        }
    }
    if(mMouseViewKey){
        if(mMouseViewKey->isHavingFocus()){
            mMouseViewKey->setKeyString(string);
            return;
        }
    }
    if(mAccelateKey){
        if(mAccelateKey->isHavingFocus()){
            mAccelateKey->setKeyString(string);
            return;
        }
    }
}
bool JoystickManager::isKeyMouseValueExist(const QString str){
    foreach (JoystickSingleBaseKey* key, mKeyMouseKeysList) {
        if(str == key->getKeyString()){
            return true;
        }
    }
    if(mJoystickDirection){
        QString strTop,strRight,strBottom,strLeft;
        mJoystickDirection->getDirectKeyString(strTop,strRight,strBottom,strLeft);
        if(str==strTop || str==strRight || str==strBottom|| str==strLeft){
           return true;
        }
    }
    if(mMouseViewKey){
        QString strTop,strRight,strBottom,strLeft;
        mMouseViewKey->getDirectKeyString(strTop,strRight,strBottom,strLeft);
        if(str==strTop || str==strRight || str==strBottom|| str==strLeft){
           return true;
        }
    }
    if(mAccelateKey){
        QString strTop,strRight,strBottom,strLeft;
        mAccelateKey->getDirectKeyString(strTop,strRight,strBottom,strLeft);
        if(str==strTop || str==strRight || str==strBottom|| str==strLeft){
           return true;
        }
    }
    if(mMouseCrossHairKey){
        if(str == mMouseCrossHairKey->getKeyString()){
            return true;
        }
    }
    return false;
}

bool JoystickManager::isJoystickKeysExist(const QString str){
    //判断下界面上是否有重复的按键
    foreach (JoystickSingleBaseKey* key, mGameKeys) {
        if(str == key->getKeyString()){
            return true;
        }
    }
    return false;
}

void JoystickManager::joystickConnect(bool isconnect){
    if(!mIsJoystickSetWidgetInitialed){   //按键没有被初始化过，不启用检测
        return;
    }
    mIsJoystickConnect = isconnect;
    syslog(LOG_DEBUG, "[JoystickManager] joystickConnect() isconnect: %d", isconnect);
    if(timer){
        if(timer->isActive()){
            syslog(LOG_DEBUG, "[JoystickManager] joystickConnect() timer停止");
            timer->stop();
        }
        int id = timer->timerId();
        timer->start(2000);
        syslog(LOG_DEBUG, "[JoystickManager] joystickConnect() timer重新启动");

    }
}
void JoystickManager::timerCast(){
    syslog(LOG_DEBUG, "[JoystickManager] timerCast() current GameKeyStatus=%d,mIsJoystickConnect=%d",mGameKeyStatus,mIsJoystickConnect);
    if(timer->isActive()){
        syslog(LOG_DEBUG, "[JoystickManager] timerCast() timer->stop()");
        timer->stop();
    }
    if(mIsJoystickConnect){
        Toast::getToast()->setParentAttr(mMainWindow);
        Toast::getToast()->setMessage(tr("Handle connected!"));
        if(mGameKeyStatus == NoJostick){
            if(mIsUseJoystickRock){//这种情况只有可能是之前在使用手柄，但是手柄断开了，现在又连上
                initJoystickKeysFromFile();
                showJoystickKeys(true);
                enableJoystickKeyEdit(false);
            }else{
                gameKeyStackWidget->setSpecifiedStackIndex(GameKeyStackWidget::JOYSTICK);
            }

        }else if(mGameKeyStatus == KeyMouseEditing){
            gameKeyStackWidget->setSpecifiedStackIndex(GameKeyStackWidget::JOYSTICK);
        }else{
            return;
        }
    }else{   //摇杆断开了
        Toast::getToast()->setParentAttr(mMainWindow);
        Toast::getToast()->setMessage(tr("Handle disconnected!"));
        if(mGameKeyStatus == JoystickEditing){
            gameKeyStackWidget->setSpecifiedStackIndex(GameKeyStackWidget::KEYMOUSE);
        }else if(mGameKeyStatus == JoystickUse){
            removeGameKeys(true);
            setGameKeyStatus(NoJostick);
        }else{
           return;
        }
    }
}

void JoystickManager::getMainWidgetDisplaySize(int &displayWidth, int &displayHeight){
    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    displayWidth = displayManager->getDisplayWidth();
    displayHeight = displayManager->getDisplayHeight();
}

void JoystickManager::getMainWidgetInitialSize(int &initialWidth, int &initialHeight){
    auto size = mMainWindow->getDisplayManager()->getMainWidgetInitialSize();
    auto initOrientation = mMainWindow->getDisplayManager()->getInitialOrientation();

    initialWidth = (initOrientation == Qt::PortraitOrientation) ? size.width() : size.height();
    initialHeight = (initOrientation == Qt::PortraitOrientation) ? size.height() : size.width();

    //auto isLandscape = mMainWindow->getDisplayManager()->isCurrentLandscapeOrientation();
    //syslog(LOG_DEBUG, "isLandscape: %d, initOrientation = %d", isLandscape, initOrientation);
    //syslog(LOG_DEBUG, "initialWidth: %d, initialHeight: %d", initialWidth, initialHeight);
}

void JoystickManager::getMainWidgetSize(int &displayWidth, int &displayHeight, int &initialWidth, int &initialHeight){
    getMainWidgetDisplaySize(displayWidth, displayHeight);   //linux桌面上实际显示的尺寸
    getMainWidgetInitialSize(initialWidth, initialHeight);   //Android应用本来的分辨率
}

QSize JoystickManager::getEmptySpace(){
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        if (mMainWindow->isFullScreen()) {
            int displayWidth, displayHeight;
            getMainWidgetDisplaySize(displayWidth, displayHeight);
            QRect screenSize = mMainWindow->getDisplayManager()->getScreenSize();
            int empty_space_w = (screenSize.width() - displayWidth) / 2;
            int empty_space_h = (screenSize.height() - displayHeight) / 2;
            return QSize(empty_space_w, empty_space_h);
        }
        else {
            return QSize(0, 0);
        }
    } else {
        return QSize(0, 0);
    }
}

QPoint JoystickManager::getGlobalPos(QPoint pos){
    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    _getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);  //获取显示的相关信息，相对整个显示屏幕
    //syslog(LOG_ERR, "[GameKeyManager] getGlobalPos, main widget: x = %d, y = %d", mainWidget_x, mainWidget_y);
    QSize emptySpace = getEmptySpace();

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        return QPoint(pos.x() + margin_width + emptySpace.width(), pos.y() + titlebar_height + emptySpace.height());
    } else {
        return QPoint(pos.x() + mainWidget_x + margin_width + emptySpace.width(), pos.y() + mainWidget_y + titlebar_height + emptySpace.height());
    }
}

void JoystickManager::_getMainWidgetInfo(int &pos_x, int &pos_y, int &margin_width, int &titlebar_height){
    if (mMainWindow->isFullScreen()) {
        QRect screenSize = mMainWindow->getDisplayManager()->getScreenSize();
        int displayWidth, displayHeight;
        getMainWidgetDisplaySize(displayWidth, displayHeight);
        pos_x = (screenSize.width() - displayWidth) / 2;
        pos_y = (screenSize.height() - displayHeight) / 2;
        margin_width = 0;
        titlebar_height = 0;
    }
    else {
        QRect mainWidgetRect = mMainWindow->geometry();   //获取当前主 widget的信息
        pos_x = mainWidgetRect.x();
        pos_y = mainWidgetRect.y();
        margin_width = MOUSE_MARGINS;
        titlebar_height = DEFAULT_TITLEBAR_HEIGHT;
    }
}

QPoint JoystickManager::_getGameKeyPos(JoystickBaseKey *key){
    //前边两个参数是PC显示器显示安卓窗口的宽高    后边的是初始化时安卓应用分辨率
    int displayWidth, displayHeight, initialWidth, initialHeight;
    getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);

    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    if (!displayManager->isCurrentLandscapeOrientation()) {   //显示器是竖着的
        width_scale_ratio *= displayManager->getOriginalInitialWidth() / static_cast<double>(initialWidth);
        height_scale_ratio *= displayManager->getOriginalInitialHeight() / static_cast<double>(initialHeight);
    }

    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    _getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);


    double x,y;
    key->getRealEventPos(x,y);

    int pos_x = (x - mainWidget_x - margin_width ) * width_scale_ratio;
    int pos_y = (y - mainWidget_y - titlebar_height)  * height_scale_ratio;
    //syslog(LOG_DEBUG, "[GameKeyManager][%s] pos_x = %d, pos_y = %d", __func__, pos_x, pos_y);
    return QPoint(pos_x, pos_y);
}

QPoint JoystickManager::_getGameKeyPos(JoystickBaseKey *key,int &offset_h, int &offset_v){

    int displayWidth, displayHeight, initialWidth, initialHeight;
    getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);

    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    if (!displayManager->isCurrentLandscapeOrientation()) {   //显示器是竖着的
        width_scale_ratio *= displayManager->getOriginalInitialWidth() / static_cast<double>(initialWidth);
        height_scale_ratio *= displayManager->getOriginalInitialHeight() / static_cast<double>(initialHeight);
    }

    double h,v;
    key->getOperateHW(h,v);
    offset_h = h/2*width_scale_ratio;
    offset_v = v/2*height_scale_ratio;

    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    _getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
    double x, y;
    key->getRealEventPos(x, y);

    int pos_x =(x - mainWidget_x - margin_width) * width_scale_ratio;
    int pos_y =(y - mainWidget_y - titlebar_height) * height_scale_ratio;
    //syslog(LOG_DEBUG, "[GameKeyManager][%s] pos_x = %d, pos_y = %d", __func__, pos_x, pos_y);
    return QPoint(pos_x, pos_y);
}

QMap<int,QList<QString>> JoystickManager::getAllKindOfKey(){
    QMap<int,QList<QString>> keyMap;
    QList<QString> comboList;   //连击
    QList<QString> smartList;   //智能施法
    if(mGameKeyStatus == JoystickUse){
        for(auto key:mGameKeys){
            if(key->getKeyType() == JoystickComboKey::COMBOKEY){
                QString name = key->getKeyString();
                comboList.push_back(name);
            }else if(key->getKeyType() == JoystickComboKey::AUTOMETHOD){
                QString name = key->getKeyString();
                smartList.push_back(name);
            }
        }

    }else if(mGameKeyStatus == KeyMouseUse){
        for(auto key:mKeyMouseKeysList){
            if(key->getKeyType() == JoystickComboKey::COMBOKEY){
                QString name = key->getKeyString();
                comboList.push_back(name);
            }else if(key->getKeyType() == JoystickComboKey::AUTOMETHOD){
                QString name = key->getKeyString();
                smartList.push_back(name);
            }
        }
    }
    keyMap.insert(JoystickComboKey::COMBOKEY,comboList);
    keyMap.insert(JoystickComboKey::AUTOMETHOD,smartList);
    return keyMap;
}

JoystickManager::KeyMsgStruct JoystickManager::getSingleGameKeyPos(QString key){
    KeyMsgStruct keyStruct;
    if(key.length()>7){
        key.remove("BUTTON_");
    }else{
        syslog(LOG_DEBUG, "[JoystickManager] getSingleGameKeyPos key.length is err!");
    }
    foreach (JoystickSingleKey* singleKey, mGameKeys) {
        if(singleKey->getKeyString() == key){
            QPoint point = _getGameKeyPos(singleKey);
            if(singleKey->getKeyType() == JoystickComboKey::COMBOKEY){
                keyStruct.comboNumber = singleKey->getComboFreq();
            }
            keyStruct.isSet = true;
            keyStruct.type = singleKey->getKeyType();
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            return keyStruct;
        }
    }
    return keyStruct;
}
JoystickManager::KeyMsgStruct JoystickManager::getKeyMouseKeyPos(QString key){
    KeyMsgStruct keyStruct;
    if(key == QString::fromLocal8Bit("MOUSELEFT")){
        if(mMouseFireKey){
            keyStruct.isSet = true;
            QPoint point = _getGameKeyPos(mMouseFireKey);
            keyStruct.type = mMouseFireKey->getKeyType();
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            return keyStruct;
        }
    }
    if(mJoystickDirection){
        QString top,right,bottom,left;
        mJoystickDirection->getDirectKeyString(top,right,bottom,left);
        if(key == top || key == right ||key == bottom ||key == left){
            keyStruct.isSet = true;
            keyStruct.type = mJoystickDirection->getKeyType();
            int offset_x,offset_y;
            QPoint point = _getGameKeyPos(mJoystickDirection,offset_x,offset_y);
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            keyStruct.wheelbase_x = offset_x;
            QString flag;
            if(key == top){
                flag = "UP";
            }else if(key == right){
                flag = "RIGHT";
            }else if(key == bottom){
                flag = "DOWN";
            }else{
                flag = "LEFT";
            }
            keyStruct.directionFlag = flag;
            return keyStruct;
        }
    }
    if(mAccelateKey){
        QString top,right,bottom,left;
        mAccelateKey->getDirectKeyString(top,right,bottom,left);
        if(key == top || key == right ||key == bottom ||key == left){
            keyStruct.isSet = true;
            keyStruct.type = mAccelateKey->getKeyType();
            int offset_x,offset_y;
            QPoint point = _getGameKeyPos(mAccelateKey,offset_x,offset_y);
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            keyStruct.wheelbase_x = offset_x;
            QString flag;
            if(key == top){
                flag = "UP";
            }else if(key == right){
                flag = "RIGHT";
            }else if(key == bottom){
                flag = "DOWN";
            }else{
                flag = "LEFT";
            }
            keyStruct.directionFlag = flag;
            return keyStruct;
        }
    }
    if(mMouseViewKey){
        QString top,right,bottom,left;
        mMouseViewKey->getDirectKeyString(top,right,bottom,left);
        if(key == top || key == right ||key == bottom ||key == left){
            keyStruct.isSet = true;
            keyStruct.type = mMouseViewKey->getKeyType();
            int offset_x,offset_y;
            QPoint point = _getGameKeyPos(mMouseViewKey,offset_x,offset_y);
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            keyStruct.wheelbase_x = offset_x;
            QString flag;
            if(key == top){
                flag = "UP";
            }else if(key == right){
                flag = "RIGHT";
            }else if(key == bottom){
                flag = "DOWN";
            }else{
                flag = "LEFT";
            }
            keyStruct.directionFlag = flag;
            return keyStruct;
        }
    }
    if(mMouseCrossHairKey){
        if(key == mMouseCrossHairKey->getKeyString()){
            keyStruct.isSet = true;
            keyStruct.type = mMouseCrossHairKey->getKeyType();
            int offset_x,offset_y;
            QPoint point = _getGameKeyPos(mMouseCrossHairKey,offset_x,offset_y);
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            keyStruct.wheelbase_x = offset_x;
            keyStruct.wheelbase_y = offset_y;
            keyStruct.sensitivity = mMouseCrossHairKey->getComboFreq();
            keyStruct.isChecked = mMouseCrossHairKey->getCheckBoxStatus() == Qt::Checked?true:false;
            return keyStruct;
        }
    }
    foreach (JoystickSingleKey* singleKey,mKeyMouseKeysList) {
        if(singleKey->getKeyString() == key){
            QPoint point = _getGameKeyPos(singleKey);
            if(singleKey->getKeyType() == JoystickComboKey::COMBOKEY){
                keyStruct.comboNumber = singleKey->getComboFreq();
            }
            keyStruct.isSet = true;
            keyStruct.type = singleKey->getKeyType();
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            return keyStruct;
        }
    }
    return keyStruct;
}

JoystickManager::KeyMsgStruct JoystickManager::getJoystickPos(QString key){
    KeyMsgStruct keyStruct;
    if(key == QString::fromLocal8Bit("AXIS_LA")){   //左摇杆
        if(mJoystickLeftRock){
            int offset_x,offset_y;
            QPoint point = _getGameKeyPos(mJoystickLeftRock,offset_x,offset_y);
            keyStruct.isSet = true;
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            keyStruct.wheelbase_x = offset_x;
            keyStruct.wheelbase_y = offset_y;
            return keyStruct;
        }
    }else if(key == QString::fromLocal8Bit("AXIS_RA")){  //右摇杆
        if(mJoystickRightRock){
            int offset_x,offset_y;
            QPoint point = _getGameKeyPos(mJoystickRightRock,offset_x,offset_y);
            keyStruct.isSet = true;
            keyStruct.x = point.x();
            keyStruct.y = point.y();
            keyStruct.wheelbase_x = offset_x;
            keyStruct.wheelbase_y = offset_y;
            return keyStruct;
        }
    }else{
        syslog(LOG_DEBUG, "[JoystickManager] getJoystickPos parms string is err!");
        return keyStruct;
    }
    return keyStruct;
}
void JoystickManager::saveGameKeysConfig(){

    int displayWidth, displayHeight;
    getMainWidgetDisplaySize(displayWidth, displayHeight);   //获取显示的宽高

    foreach (auto key, mGameKeys)  {
        if(key->getKeyString().isEmpty()){
            deleteJoystickKey(key->getKeyType(),key->getIndex());
            continue;
        }
        _saveKeyConfig(key);
    }

    if (mJoystickLeftRock) {
        _saveKeyConfig(mJoystickLeftRock);
        double iw = (mJoystickLeftRock->width()-DEFAULT_JOYSTICK_ROCKER_MARGIN)/ static_cast<double>(displayWidth);
        double ih = (mJoystickLeftRock->height()-DEFAULT_JOYSTICK_ROCKER_MARGIN) / static_cast<double>(displayHeight);
        mJoystickLeftRock->setOperateHWRatio(iw,ih);
    }

    if (mJoystickRightRock) {
        _saveKeyConfig(mJoystickRightRock);
    }

}
bool JoystickManager::saveKeyMouseKeysConfig(){
    if (mJoystickDirection) {
        if(mJoystickDirection->isHadLineEditNotSetValue()){
            KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),
                                                 tr("Please complete the key value of the direction key！"));
            return false;
        }
        _saveKeyConfig(mJoystickDirection);
    }
    if(mMouseViewKey){
        if(mMouseViewKey->isHadLineEditNotSetValue()){
            KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),
                                                 tr("Please complete the key value of the view key！"));
            return false;
        }
        _saveKeyConfig(mMouseViewKey);
    }
    if(mAccelateKey){
        if(mAccelateKey->isHadLineEditNotSetValue()){
            KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),
                                                 tr("Please complete the key value of the Accelate key！"));
            return false;
        }
        _saveKeyConfig(mAccelateKey);
    }
    foreach (auto key, mKeyMouseKeysList)  {
        if(key->getKeyString().isEmpty()){
            deleteKeyMouseKey(key->getKeyType(),key->getIndex());
            continue;
        }
        _saveKeyConfig(key);
    }

    if(mMouseCrossHairKey){
        if(mMouseCrossHairKey->getKeyString().isEmpty()){
            KylinUI::MessageBox::warning(mMainWindow,tr("Warning"),
                                                 tr("Please complete the key value of the CrossHair key！"));
            return false;
        }
        _saveKeyConfig(mMouseCrossHairKey);
    }
//    if(mMouseFireKey){
//        _saveKeyConfig(mMouseFireKey);
//    }
    return true;

}
void JoystickManager::_saveKeyConfig(JoystickBaseKey* key){
    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    //获取当前主显示界面的坐标，边距和标题栏高度
    _getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
    QSize emptySpace = getEmptySpace();     //这个竟然是获取主界面内除掉安卓界面剩下的空间

    int displayWidth, displayHeight;
    getMainWidgetDisplaySize(displayWidth, displayHeight);   //获取显示的宽高

    QRect rect = key->geometry();
    int deltaX;
    int deltaY;
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        deltaX = rect.x() - margin_width - emptySpace.width();
        deltaY = rect.y() - titlebar_height - emptySpace.height();
    } else {
        deltaX = rect.x() - mainWidget_x - margin_width - emptySpace.width();
        deltaY = rect.y() - mainWidget_y - titlebar_height - emptySpace.height();
    }
    if (deltaX < 0) {
        deltaX = 0;
    }
    else if (deltaX + rect.width() > displayWidth) {
        deltaX = displayWidth - rect.width();
    }
    if (deltaY < 0) {
        deltaY = 0;
    }
    else if (deltaY + rect.height() > displayHeight) {
        deltaY = displayHeight - rect.height();
    }
    double x = deltaX / static_cast<double>(displayWidth);
    double y = deltaY / static_cast<double>(displayHeight);

    double w = key->width() / static_cast<double>(displayWidth);
    double h = key->height() / static_cast<double>(displayHeight);

    key->setGeometryPos(x, y);
    key->setGeometryHWRatio(w,h);
    key->updatePos();
    key->setIsBeEdti(false);
}
bool JoystickManager::isHadEditNotBeSave(){
    if(mGameKeyStatus == JoystickEditing){
        foreach (auto key, mGameKeys)  {
            if(key->getIsBeEdit()){
                return true;
            }
        }

        if (mJoystickLeftRock && mJoystickLeftRock->getIsBeEdit()) {
           return true;
        }

        if (mJoystickRightRock && mJoystickRightRock->getIsBeEdit()) {
            return true;
        }
    }else if(mGameKeyStatus == KeyMouseEditing){
        foreach (auto key, mKeyMouseKeysList)  {
            if(key->getIsBeEdit()){
                return true;
            }
        }
        if (mJoystickDirection && mJoystickDirection->getIsBeEdit() ) {
            return true;
        }
        if(mMouseCrossHairKey && mMouseCrossHairKey->getIsBeEdit()){
            return true;
        }
        if(mMouseFireKey && mMouseFireKey->getIsBeEdit()){
            return true;
        }
        if(mMouseViewKey && mMouseViewKey->getIsBeEdit()){
            return true;
        }
    }
    return false;
}
void JoystickManager::clearGameKeysConfigFile(bool isJoystick)
{
    QString jsonFile;
    if(isJoystick){
        jsonFile= QString("%1/%2.json").arg(mJoystickJsonDir).arg(mMainWindow->getPackageName());
    }else{
        jsonFile= QString("%1/%2.json").arg(mKeyMouseJsonDir).arg(mMainWindow->getPackageName());
    }
    QFile file(jsonFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.resize(0);
        file.close();
    }
}

void JoystickManager::createGameKeysXmlDir()
{
    mJoystickJsonDir = KmreEnv::GetConfigPath() + "/joystick";
    mKeyMouseJsonDir = KmreEnv::GetConfigPath() + "/keymouse";
    QDir dir;
    if (!dir.exists(mJoystickJsonDir)) {
        if (!dir.mkpath(mJoystickJsonDir)) {
            syslog(LOG_ERR, "[JoystickManager]Fail to create directory %s", mJoystickJsonDir.toUtf8().data());
        }
    }
    if(!dir.exists(mKeyMouseJsonDir)){
        if (!dir.mkpath(mKeyMouseJsonDir)){
            syslog(LOG_ERR, "[JoystickManager]Fail to create mKeyMouseJsonDir %s", mJoystickJsonDir.toUtf8().data());
        }
    }
}

//写入到配置文件
void JoystickManager::writeGameKeysConfigToFile(){

    QString jsonFile = QString("%1/%2.json").arg(mJoystickJsonDir).arg(mMainWindow->getPackageName());
    QFile file(jsonFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.resize(0);

        //=======================object部分的操作===============================
        QJsonObject transparency;
        transparency.insert("value", mTransparency);

        QJsonObject orientation;
        int mKeyOrientation = mMainWindow->getDisplayManager()->getCurrentDisplayOrientation();
        orientation.insert("orientation",mKeyOrientation);

        QJsonObject leftJoystick;
        if (mJoystickLeftRock) {
            leftJoystick.insert("valid", true);
            double x,y;
            mJoystickLeftRock->getGeometryPos(x,y);
            leftJoystick.insert("x", x);
            leftJoystick.insert("y", y);
            double w,h;
            mJoystickLeftRock->getGeometruHWRatio(w,h);
            leftJoystick.insert("width", w);
            leftJoystick.insert("height", h);
        }else {
            leftJoystick.insert("valid", false);
        }

        QJsonObject rightJoystick;
        if (mJoystickRightRock) {
            rightJoystick.insert("valid", true);
            double x,y;
            mJoystickRightRock->getGeometryPos(x,y);
            rightJoystick.insert("x", x);
            rightJoystick.insert("y", y);
            double w,h;
            mJoystickRightRock->getGeometruHWRatio(w,h);
            rightJoystick.insert("width", w);
            rightJoystick.insert("height", h);
        }else {
            rightJoystick.insert("valid", false);
        }

        QJsonArray keyArray;
        foreach (JoystickSingleBaseKey* key, mGameKeys) {
            if(key->getKeyString().isEmpty()){
                continue;
            }
            QJsonObject keyObj;
            keyObj.insert("key", key->getKeyString());
            keyObj.insert("keyType",key->getKeyType());
            keyObj.insert("freq",key->getComboFreq());
            double x, y;
            key->getGeometryPos(x,y);
            keyObj.insert("x", x);
            keyObj.insert("y", y);
            keyArray.append(keyObj);
        }

        QJsonObject rootObject;
        rootObject.insert("Orientation",orientation);
        rootObject.insert("Transparency", transparency);
        rootObject.insert("LeftJoystick", leftJoystick);
        rootObject.insert("RightJoystick", rightJoystick);
        if (!keyArray.isEmpty()) {
            rootObject.insert("Keys", keyArray);
        }

        QJsonDocument jsonDoc;
        jsonDoc.setObject(rootObject);
        file.write(jsonDoc.toJson());
        file.close();
    }
}
void JoystickManager::writeKeyMouseKeysConfigToFile(){
    QString jsonFile = QString("%1/%2.json").arg(mKeyMouseJsonDir).arg(mMainWindow->getPackageName());
    QFile file(jsonFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.resize(0);

        //=======================object部分的操作===============================
        QJsonObject transparency;
        transparency.insert("value", mTransparency);

        QJsonObject orientation;
        int mKeyOrientation = mMainWindow->getDisplayManager()->getCurrentDisplayOrientation();
        orientation.insert("orientation",mKeyOrientation);

        QJsonObject direction;
        if (mJoystickDirection) {
            direction.insert("valid", true);
            double x,y;
            mJoystickDirection->getGeometryPos(x,y);
            direction.insert("x", x);
            direction.insert("y", y);
            double w,h;
            mJoystickDirection->getGeometruHWRatio(w,h);
            direction.insert("width", w);
            direction.insert("height", h);
            QString top,right,bottom,left;
            mJoystickDirection->getDirectKeyString(top,right,bottom,left);
            direction.insert("topStr",top);
            direction.insert("rightStr",right);
            direction.insert("bottomStr",bottom);
            direction.insert("leftStr",left);
        }else {
            direction.insert("valid", false);
        }
        QJsonObject crosshair;
        if (mMouseCrossHairKey) {
            crosshair.insert("valid", true);
            crosshair.insert("key",mMouseCrossHairKey->getKeyString());
            double x,y;
            mMouseCrossHairKey->getGeometryPos(x,y);
            crosshair.insert("x", x);
            crosshair.insert("y", y);
            double w,h;
            mMouseCrossHairKey->getGeometruHWRatio(w,h);
            crosshair.insert("width", w);
            crosshair.insert("height", h);
            crosshair.insert("freq",mMouseCrossHairKey->getComboFreq());
            crosshair.insert("checkstatus",mMouseCrossHairKey->getCheckBoxStatus());
        }else {
            crosshair.insert("valid", false);
        }
        QJsonObject fire;
        if (mMouseFireKey) {
            fire.insert("valid", true);
            double x,y;
            mMouseFireKey->getGeometryPos(x,y);
            fire.insert("x", x);
            fire.insert("y", y);
        }else {
            fire.insert("valid", false);
        }
        QJsonObject view;
        if (mMouseViewKey) {
            view.insert("valid", true);
            double x,y;
            mMouseViewKey->getGeometryPos(x,y);
            view.insert("x", x);
            view.insert("y", y);
            double w,h;
            mMouseViewKey->getGeometruHWRatio(w,h);
            view.insert("width", w);
            view.insert("height", h);
            QString top,right,bottom,left;
            mMouseViewKey->getDirectKeyString(top,right,bottom,left);
            view.insert("topStr",top);
            view.insert("rightStr",right);
            view.insert("bottomStr",bottom);
            view.insert("leftStr",left);
        }else {
            view.insert("valid", false);
        }

        QJsonObject accelate;
        if (mAccelateKey) {
            accelate.insert("valid", true);
            double x,y;
            mAccelateKey->getGeometryPos(x,y);
            accelate.insert("x", x);
            accelate.insert("y", y);
            double w,h;
            mAccelateKey->getGeometruHWRatio(w,h);
            accelate.insert("width", w);
            accelate.insert("height", h);
            QString top,right,bottom,left;
            mAccelateKey->getDirectKeyString(top,right,bottom,left);
            accelate.insert("topStr",top);
            accelate.insert("rightStr",right);
            accelate.insert("bottomStr",bottom);
            accelate.insert("leftStr",left);
        }else {
            accelate.insert("valid", false);
        }
        QJsonArray keyArray;
        foreach (JoystickSingleBaseKey* key, mKeyMouseKeysList) {
            if(key->getKeyString().isEmpty()){
                continue;
            }
            if(key->getKeyType() == JoystickBaseKey::FIRE){
                continue;
            }
            QJsonObject keyObj;
            keyObj.insert("key", key->getKeyString());
            keyObj.insert("keyType",key->getKeyType());
            keyObj.insert("freq",key->getComboFreq());
            double x, y;
            key->getGeometryPos(x,y);
            keyObj.insert("x", x);
            keyObj.insert("y", y);
            keyArray.append(keyObj);
        }

        QJsonObject rootObject;
        rootObject.insert("Orientation",orientation);
        rootObject.insert("Transparency", transparency);
        rootObject.insert("Direction", direction);
        rootObject.insert("Crosshair",crosshair);
        rootObject.insert("Fire",fire);
        rootObject.insert("View",view);
        rootObject.insert("Accelate",accelate);
        if (!keyArray.isEmpty()) {
            rootObject.insert("Keys", keyArray);
        }

        QJsonDocument jsonDoc;
        jsonDoc.setObject(rootObject);
        file.write(jsonDoc.toJson());
        file.close();
    }
}

void JoystickManager::initJoystickKeysFromFile(){
    if(mIsJoystickConfigFileInit){
        return;
    }
    QString jsonFile = QString("%1/%2.json").arg(mJoystickJsonDir).arg(mMainWindow->getPackageName());
    if (utils::isFileExist(jsonFile)) {
        QFile file(jsonFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ba = file.readAll();
            file.close();
            QJsonParseError err;
            QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
            if (err.error != QJsonParseError::NoError) {
                return;
            }
            if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                return;
            }
            QJsonObject jsonObj = jsonDocment.object();
            if (jsonObj.isEmpty() || jsonObj.size() == 0) {
                return;
            }
            if(jsonObj.contains("Orientation")){
                QJsonObject orientation = jsonObj.value("Orientation").toObject();
                if(!orientation.isEmpty() || orientation.size() > 0){
                    if(orientation.contains("orientation")){
                        int fileOri = orientation.value("orientation").toInt();
                        int mKeyOrientation = mMainWindow->getDisplayManager()->getCurrentDisplayOrientation();
                        if(fileOri!=mKeyOrientation){
                            clearGameKeysConfigFile(true);
                            Toast::getToast()->setParentAttr(mMainWindow);
                            Toast::getToast()->setMessage(tr("The environment change，please reset button!"));
                            return;
                        }
                    }
                }
            }
            if (jsonObj.contains("RightJoystick")) {
                QJsonObject rightObj = jsonObj.value("RightJoystick").toObject();
                if (!rightObj.isEmpty() || rightObj.size() > 0) {
                    if (rightObj.contains("valid") && rightObj.contains("x") && rightObj.contains("y") && rightObj.contains("width") && rightObj.contains("height")) {
                        if (rightObj.value("valid").toBool()) {
                            AddKeyMsgStruct msgStruct;
                            msgStruct.x = rightObj.value("x").toDouble();
                            msgStruct.y = rightObj.value("y").toDouble();
                            msgStruct.show = false;
                            msgStruct.isBeEdit = false;
                            _addRightJoystick(msgStruct);
                        }
                    }
                }
            }
            if (jsonObj.contains("LeftJoystick")) {
                QJsonObject leftObj = jsonObj.value("LeftJoystick").toObject();
                if (!leftObj.isEmpty() || leftObj.size() > 0) {
                    if (leftObj.contains("valid") && leftObj.contains("x") && leftObj.contains("y") && leftObj.contains("width") && leftObj.contains("height")) {
                        if (leftObj.value("valid").toBool()) {
                            AddKeyMsgStruct msgStruct;
                            msgStruct.x = leftObj.value("x").toDouble();
                            msgStruct.y = leftObj.value("y").toDouble();
                            msgStruct.widthRatio = leftObj.value("width").toDouble();
                            msgStruct.heigthRatio = leftObj.value("height").toDouble();
                            msgStruct.show = false;
                            msgStruct.isBeEdit = false;
                            _addLeftJoystick(msgStruct);
                        }
                    }
                }
            }

            if (jsonObj.contains("Keys")) {
                QJsonValue arrayVaule = jsonObj.value("Keys");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        if (child.contains("key") && child.contains("x") && child.contains("y")) {
                            AddKeyMsgStruct msgStruct;
                            msgStruct.key = child.value("key").toString();
                            msgStruct.x = child.value("x").toDouble();
                            msgStruct.y = child.value("y").toDouble();
                            msgStruct.show = false;
                            if(child.value("keyType").toInt() == JoystickComboKey::COMBOKEY){
                                msgStruct.comboFreq = child.value("freq").toInt();
                            }
                            msgStruct.index = mJoystickIndex++;
                            msgStruct.isBeEdit = false;
                            msgStruct.keyType = child.value("keyType").toInt();
                            _addSingleGameKey(msgStruct,true);
                        }
                    }
                }
            }
        }
    }else{  //配置文件不存在
        syslog(LOG_DEBUG, "[JoystickManager] initJoystickKeysFromFile no Json ini file");
    }
    mIsJoystickConfigFileInit = true;
}
void JoystickManager::initKeyMouseKeysFromFile(){
    if(mIsKeyMouseConfigFileInit){
        return;
    }
    QString jsonFile = QString("%1/%2.json").arg(mKeyMouseJsonDir).arg(mMainWindow->getPackageName());
    if (utils::isFileExist(jsonFile)) {
        QFile file(jsonFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ba = file.readAll();
            file.close();
            QJsonParseError err;
            QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
            if (err.error != QJsonParseError::NoError) {
                return;
            }
            if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                return;
            }
            QJsonObject jsonObj = jsonDocment.object();
            if (jsonObj.isEmpty() || jsonObj.size() == 0) {
                return;
            }
            if(jsonObj.contains("Orientation")){
                QJsonObject orientation = jsonObj.value("Orientation").toObject();
                if(!orientation.isEmpty() || orientation.size() > 0){
                    if(orientation.contains("orientation")){
                        int fileOri = orientation.value("orientation").toInt();
                        int mKeyOrientation = mMainWindow->getDisplayManager()->getCurrentDisplayOrientation();
                        if(fileOri!=mKeyOrientation){
                            clearGameKeysConfigFile(false);
                            Toast::getToast()->setParentAttr(mMainWindow);
                            Toast::getToast()->setMessage(tr("The environment change，please reset button!"));
                            return;
                        }
                    }
                }
            }
            if (jsonObj.contains("Direction")) {
                QJsonObject rightObj = jsonObj.value("Direction").toObject();
                if (!rightObj.isEmpty() || rightObj.size() > 0) {
                    if (rightObj.contains("valid") && rightObj.contains("x") && rightObj.contains("y") && rightObj.contains("width") && rightObj.contains("height")) {
                        if (rightObj.value("valid").toBool()) {
                           AddKeyMsgStruct msgStruct;
                           msgStruct.show = false;
                           msgStruct.x = rightObj.value("x").toDouble();
                           msgStruct.y = rightObj.value("y").toDouble();
                           msgStruct.widthRatio = rightObj.value("width").toDouble();
                           msgStruct.heigthRatio = rightObj.value("height").toDouble();
                           msgStruct.topStr = rightObj.value("topStr").toString();
                           msgStruct.rightStr = rightObj.value("rightStr").toString();
                           msgStruct.bottomStr = rightObj.value("bottomStr").toString();
                           msgStruct.leftStr = rightObj.value("leftStr").toString();
                           msgStruct.isBeEdit = false;
                           _addDirectionKey(msgStruct);
                        }
                    }
                }
            }
            if (jsonObj.contains("Crosshair")) {
                QJsonObject crossObj = jsonObj.value("Crosshair").toObject();
                if (!crossObj.isEmpty() || crossObj.size() > 0) {
                    if (crossObj.contains("valid") && crossObj.contains("x") && crossObj.contains("y") && crossObj.contains("width") && crossObj.contains("height")) {
                        if (crossObj.value("valid").toBool()) {
                           AddKeyMsgStruct msgStruct;
                           msgStruct.key = crossObj.value("key").toString();
                           msgStruct.x = crossObj.value("x").toDouble();
                           msgStruct.y = crossObj.value("y").toDouble();
                           msgStruct.show = false;
                           msgStruct.widthRatio = crossObj.value("width").toDouble();
                           msgStruct.heigthRatio = crossObj.value("height").toDouble();
                           msgStruct.comboFreq = crossObj.value("freq").toInt();
                           msgStruct.isBeEdit = false;
                           _addKeyMouseCrossHairKey(msgStruct,crossObj.value("checkstatus").toInt());
                        }
                    }
                }
            }
            if (jsonObj.contains("Fire")) {
                QJsonObject fireObj = jsonObj.value("Fire").toObject();
                if (!fireObj.isEmpty() || fireObj.size() > 0) {
                    if (fireObj.contains("valid") && fireObj.contains("x") && fireObj.contains("y")) {
                        if (fireObj.value("valid").toBool()) {
                           AddKeyMsgStruct msgStruct;
                           msgStruct.x = fireObj.value("x").toDouble();
                           msgStruct.y = fireObj.value("y").toDouble();
                           msgStruct.index = mKeyMouseIndex++;
                           msgStruct.show = false;
                           msgStruct.isBeEdit = false;
                           _addKeyMouseFireKey(msgStruct);
                        }
                    }
                }
            }
            if (jsonObj.contains("View")) {
                QJsonObject viewObj = jsonObj.value("View").toObject();
                if (!viewObj.isEmpty() || viewObj.size() > 0) {
                    if (viewObj.contains("valid") && viewObj.contains("x") && viewObj.contains("y")) {
                        if (viewObj.value("valid").toBool()) {
                           AddKeyMsgStruct msgStruct;
                           msgStruct.x = viewObj.value("x").toDouble();
                           msgStruct.y = viewObj.value("y").toDouble();
                           msgStruct.widthRatio = viewObj.value("width").toDouble();
                           msgStruct.heigthRatio = viewObj.value("height").toDouble();
                           msgStruct.show = false;
                           msgStruct.topStr = viewObj.value("topStr").toString();
                           msgStruct.rightStr = viewObj.value("rightStr").toString();
                           msgStruct.bottomStr = viewObj.value("bottomStr").toString();
                           msgStruct.leftStr = viewObj.value("leftStr").toString();
                           msgStruct.isBeEdit = false;
                           _addViewKey(msgStruct);
                        }
                    }
                }
            }

            if (jsonObj.contains("Accelate")) {
                QJsonObject accelateObj = jsonObj.value("Accelate").toObject();
                if (!accelateObj.isEmpty() || accelateObj.size() > 0) {
                    if (accelateObj.contains("valid") && accelateObj.contains("x") && accelateObj.contains("y")) {
                        if (accelateObj.value("valid").toBool()) {
                            AddKeyMsgStruct msgStruct;
                            msgStruct.x = accelateObj.value("x").toDouble();
                            msgStruct.y = accelateObj.value("y").toDouble();
                            msgStruct.widthRatio = accelateObj.value("width").toDouble();
                            msgStruct.heigthRatio = accelateObj.value("height").toDouble();
                            msgStruct.show = false;
                            msgStruct.topStr = accelateObj.value("topStr").toString();
                            msgStruct.rightStr = accelateObj.value("rightStr").toString();
                            msgStruct.bottomStr = accelateObj.value("bottomStr").toString();
                            msgStruct.leftStr = accelateObj.value("leftStr").toString();
                            msgStruct.isBeEdit = false;
                            _addAcceleKey(msgStruct);
                        }
                    }
                }
            }
            if (jsonObj.contains("Keys")) {
                QJsonValue arrayVaule = jsonObj.value("Keys");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        if (child.contains("key") && child.contains("x") && child.contains("y")) {
                            AddKeyMsgStruct msgStruct;
                            msgStruct.key = child.value("key").toString();
                            msgStruct.x = child.value("x").toDouble();
                            msgStruct.y = child.value("y").toDouble();
                            msgStruct.show = false;
                            if(child.value("keyType").toInt() == JoystickComboKey::COMBOKEY){
                                msgStruct.comboFreq = child.value("freq").toInt();
                            }
                            msgStruct.index = mKeyMouseIndex++;
                            msgStruct.isBeEdit = false;
                            msgStruct.keyType = child.value("keyType").toInt();
                            _addSingleGameKey(msgStruct,false);
                        }
                    }
                }
            }
        }
    }else{  //配置文件不存在
        syslog(LOG_DEBUG, "[JoystickManager] initKeyMouseKeysFromFile no Json ini file");
    }
    mIsKeyMouseConfigFileInit = true;
}

void JoystickManager::initGameKeyTransparency(){
    double transparency = mTransparency;
    QString jsonFile = QString("%1/%2.json").arg(mJoystickJsonDir).arg(mMainWindow->getPackageName());
    if (utils::isFileExist(jsonFile)) {
        QFile file(jsonFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ba = file.readAll();
            file.close();
            QJsonParseError err;
            QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
            if (err.error != QJsonParseError::NoError) {
                return;
            }
            if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                return;
            }
            QJsonObject jsonObj = jsonDocment.object();
            if (jsonObj.isEmpty() || jsonObj.size() == 0) {
                return;
            }
            if (jsonObj.contains("Transparency")) {
                QJsonObject wheelObj = jsonObj.value("Transparency").toObject();
                if (!wheelObj.isEmpty() || wheelObj.size() > 0) {
                    if (wheelObj.contains("value")) {
                        transparency = wheelObj.value("value").toDouble();
                    }
                }
            }
        }
    }else{
        syslog(LOG_DEBUG, "[JoystickManager] initGameKeyTransparency no json file!");
    }

    mTransparency = transparency;
}

QPoint JoystickManager::getMousePos(QPoint mPos, int type){
    int displayWidth, displayHeight, initialWidth, initialHeight;
    getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);

    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    if (!displayManager->isCurrentLandscapeOrientation()) {
        width_scale_ratio *= displayManager->getOriginalInitialWidth() / static_cast<double>(initialWidth);
        height_scale_ratio *= displayManager->getOriginalInitialHeight() / static_cast<double>(initialHeight);
    }
    int pos_x = 0;
    int pos_y = 0;
    if(type == 1){
        pos_x = mPos.x() * width_scale_ratio;
        pos_y = mPos.y() * height_scale_ratio;
    }
    else{
        if(width_scale_ratio != 0 && height_scale_ratio != 0){
            pos_x = mPos.x() / width_scale_ratio;
            pos_y = mPos.y() / height_scale_ratio;
        }
    }
    return QPoint(pos_x, pos_y);
}
void JoystickManager::sendEventToMainDisplay(QEvent *event){
    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    QApplication::sendEvent(displayManager->getMainDisplayWidget(), event);
}
