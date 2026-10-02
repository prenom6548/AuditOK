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

#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <QObject>
#include <QList>
#include <mutex>
#include <memory>
#include <QMap>
#include <QString>

#include "common.h"
#include "joysticksetwidget.h"
#include "joysticksinglekey.h"
#include "joystickcombokey.h"
#include "joystickdirectkey.h"
#include "joystickleftrock.h"
#include "joystickrightrock.h"
#include "shadewidget.h"
#include "mousefirekey.h"
#include "mousecrosshairkey.h"
#include "mouseviewkey.h"
#include "accelatekey.h"
#include "shademask.h"


class KmreWindow;    //这个只能用前置声明，否则会造成循环引用报错
class GameKeyStackWidget;

class JoystickManager : public QObject
{
    Q_OBJECT
public:
    explicit JoystickManager(KmreWindow* window);
    ~JoystickManager();
    /*default定义为默认的初始化的状态
      OnlyInit定义为1、进入初始化界面，却没有进行设置就离开，2、从NoJostick保存 3、编辑状态下屏幕发生转动，会删除当前所有的数据*/
    enum GameKeyStatus {
        Default,
        OnlyInit,
        JoystickEditing,
        JoystickUse,
        KeyMouseEditing,
        KeyMouseUse,
        NoJostick
    };
    struct KeyMsgStruct{
        bool isSet = false;   //这个按键是否被设置了
        int type = 0;      //
        int comboNumber = 5;   //每秒的连击次数，只有连击按键才有
        int x = 0;    //x
        int y = 0;   //y
        int sensitivity = 5;   //灵敏度,取值范围为1-10，只有摇杆才有
        int wheelbase_x = 0;   //x方向的轴距，默认的，现在四个方向都使用这个值
        int wheelbase_y = 0;   //y方向的轴距，暂时没用
        bool isChecked = false;
        QString directionFlag = "";  //这个是只有方向按键才有的
    };
    struct AddKeyMsgStruct{  //添加按钮时使用
        QString key = "";
        double x = 0;
        double y = 0;
        bool show = false;
        int comboFreq = 1;
        double widthRatio = 0;
        double heigthRatio = 0;
        int index = 0;
        QString topStr = "";
        QString leftStr = "";
        QString rightStr = "";
        QString bottomStr = "";
        bool isBeEdit = true;
        int keyType = 0;
    };
    void initJoystickSetWidget();
    void setJoystickSetWidgetVisible(bool isVisable);
    void getBenchMarkPos(int &xpos,int &ypos,int &width,int &height);  //获取弹出窗口的基准点
    void setGameKeyStatus(GameKeyStatus status){mGameKeyStatus = status;}
    int getGameKeyStatus(){return mGameKeyStatus;}
    void getMainWidgetDisplaySize(int &displayWidth, int &displayHeight);
    void getMainWidgetInitialSize(int &initialWidth, int &initialHeight);
    void getMainWidgetSize(int &displayWidth, int &displayHeight, int &initialWidth, int &initialHeight);
    QRect getGameKeyValidRect();
    QRect getValidOperateArea();
    QSize getEmptySpace();
    QPoint getGlobalPos(QPoint pos);
    void enableJoystickKeyEdit(bool enable);
    void enableKeyMouseEdit(bool enable);
    void _getMainWidgetInfo(int &pos_x, int &pos_y, int &margin_width, int &titlebar_height);
    QPoint _getGameKeyPos(JoystickBaseKey *key);
    QPoint _getGameKeyPos(JoystickBaseKey* key,int &offset_h, int &offset_v);
    KeyMsgStruct getSingleGameKeyPos(QString key);
    KeyMsgStruct getKeyMouseKeyPos(QString key);
    KeyMsgStruct getJoystickPos(QString key);
    bool isJoystickSetWidgetInitialed(){return mIsJoystickSetWidgetInitialed;}
    bool isJoystickSetWidgetVisible(){return mIsJoystickSetWidgetVisible;}
    bool isJoystickInEditing(){
        return mGameKeyStatus==JoystickEditing ? true : false;
    }
    bool isKeyMouseInEditing(){
        return mGameKeyStatus==KeyMouseEditing ? true : false;
    }
    bool isUseJoystickRock(){return mIsUseJoystickRock;}
    void setIsUseJoystickRock(bool isUse){mIsUseJoystickRock = isUse;}
    bool isJoystickConnect(){return mIsJoystickConnect;}
    QMap<int,QList<QString>> getAllKindOfKey();
    void updateGameKeySizeAndPos();
    void hideGameKeysWhileMove();
    void showGameKeysAfterMove();
    void hideGameKeysWhileRotation();   //发生转屏时的隐藏按键
    void addShell();
    void showJoystickKeys(bool visible);
    void showKeyMouseKeys(bool visible);
    void initJoystickKeysFromFile();
    void initKeyMouseKeysFromFile();
    QPoint getMousePos(QPoint mPos, int type);
    void removeKeyMouseKeys();
    void removeJoystickKeys();
    bool isKeyMouseValueExist(const QString str);
    void sendEventToMainDisplay(QEvent *event);

signals:
    void sigJoystickKeysReady();    //这个信号表示手柄按键设置完成
    void sigKeyMouseKeysReady();   //这个信号表示键鼠设置完成

private:
    KmreWindow* mMainWindow = nullptr;
    QList<JoystickSingleBaseKey*> mGameKeys;
    QList<JoystickSingleBaseKey*> mKeyMouseKeysList;
    JoystickLeftRock* mJoystickLeftRock = nullptr;
    JoystickRightRock* mJoystickRightRock = nullptr;
    JoystickDirectKey* mJoystickDirection = nullptr;
    MouseFireKey* mMouseFireKey = nullptr;
    MouseCrossHairKey* mMouseCrossHairKey = nullptr;
    MouseViewKey* mMouseViewKey = nullptr;
    AccelateKey* mAccelateKey = nullptr;
    //ShadeWidget* shadeShell = nullptr;
    ShadeMask* shadeShell = nullptr;
    GameKeyStackWidget* gameKeyStackWidget;
    QTimer *timer = nullptr;
    GameKeyStatus mGameKeyStatus = Default;

    bool mIsJoystickSetWidgetVisible = false;    //标记当前设置的窗口是否可见
    bool mIsJoystickSetWidgetInitialed = false;   //标记当前的设置的窗口是否有被初始化过
    bool mIsUseJoystickRock = false;    //标记是否使用手柄外设,包括编辑状态和使用状态两种
    //bool mIsShowKeysHint = true;     //默认显示按键的提示
    bool mIsJoystickConfigFileInit = false;  //摇杆配置文件是否初始化
    bool mIsKeyMouseConfigFileInit = false;  //键鼠配置文件是否初始化
    bool mIsJoystickConnect = false;
    int mJoystickIndex = 0;
    int mKeyMouseIndex = 0;
    double mTransparency = 0.8;
    QString mJoystickJsonDir="";
    QString mKeyMouseJsonDir = "";
    QRect mValidRect = {0,0,0,0};

    bool _addSingleGameKey(AddKeyMsgStruct msgStruct,bool isJoystick);
    bool _addAcceleKey(AddKeyMsgStruct msgStruct);
    bool _addLeftJoystick(AddKeyMsgStruct msgStruct);
    bool _addRightJoystick(AddKeyMsgStruct msgStruct);
    bool _addDirectionKey(AddKeyMsgStruct msgStruct);
    bool _addKeyMouseFireKey(AddKeyMsgStruct msgStruct);
    bool _addKeyMouseCrossHairKey(AddKeyMsgStruct msgStruct,int checkStatus);
    bool _addViewKey(AddKeyMsgStruct msgStruct);
    void _deleteJoystickSingleKey(int idx);
    void _deleteKeyMouseSingleKey(int idx);
    void saveGameKeysConfig();
    bool saveKeyMouseKeysConfig();
    void _saveKeyConfig(JoystickBaseKey* key);
    void clearGameKeysConfigFile(bool isJoystick);
    void createGameKeysXmlDir();
    void writeGameKeysConfigToFile();
    void writeKeyMouseKeysConfigToFile();
    void initGameKeyTransparency();
    bool isHadEditNotBeSave();

    void updateGameKeySize();
    void _updateJoystickSize();
    void _updateKeyMouseSize();
    void updateGameKeyPos();
    void _updateJoystickPos();
    void _updateKeyMousePos();
    void setShadeWidgetVisible(bool enab);
    void hideGameKeys();
    void _saveJoystickKeys();
    void _saveKeyMouseKeys();
    bool isJoystickKeysExist(const QString str);


public slots:
    void addJoystickKey(int type);
    void addKeyMouseKey(int type);
    void exitJoystickSetting();
    void deleteJoystickKey(int type,int idx);
    void deleteKeyMouseKey(int type,int idx);
    void saveGameKeys(bool isJoystick);
    void removeGameKeys(bool isJoystick);
    void setJoystickKeyStringSlot(QString string);
    void setKeyMouseStringSlot(QString string);
    void joystickConnect(bool isconnect);
    void timerCast();
};

#endif // JOYSTICKMANAGER_H
