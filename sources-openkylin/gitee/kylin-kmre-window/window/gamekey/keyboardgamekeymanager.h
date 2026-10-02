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

#ifndef KEYBOARDGAMEKEYMANAGER_H
#define KEYBOARDGAMEKEYMANAGER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMap>
#include <QTimer>

#include "gamekeytimer.h"
#include "keyboardcommon.h"

class KmreWindow;

class KeyboardGamekeyManager : public QObject
{
    Q_OBJECT
public:
    KeyboardGamekeyManager(KmreWindow* window);
    ~KeyboardGamekeyManager();
    bool isMouseClick(){
        return mIsMouseClick;
    }
    void recoverMousePos();
    QPoint getViewLockPos();
    bool isBrainGamekeyPress(){
        return isBrainPress;
    }

public slots:
    void getKeyboardGamekeyPress(QKeyEvent *pKey);
    void getKeyboardGamekeyRelease(QKeyEvent *pKey);
    void getMouseGamekeyPress(QMouseEvent *pKey);
    void getMouseGamekeyRelease(QMouseEvent *pKey);
    void getMouseGamekeyMove(QMouseEvent *pKey);
    void gameKeyInit();
    void viewGamekeyMethod();
    void focusOutMethod();
    void resetMousePos();

private:
    void gameKeyPress(int pos_x , int pos_y, QString gameKey);
    void gameKeyMove(int pos_x , int pos_y, QString gameKey);
    void gameKeyRelease(int pos_x , int pos_y, QString gameKey);
    int getGameKeyOrderPress(QString value);
    int getGameKeyOrderRelease(QString value);
    int getGameKeyOrderMove(QString value);
    QString mapKeyboardGamekey(quint32 keyCode);
    void mouseViewLock();
    QString mapGameKeyModifiers(QString gameKey, quint32 Modifiers);
    void keyboardGamekeyMethod(QString gameKey, int status);
    void addGameKeyMap(QString gamekey);
    void wheelGamekey(QString gameKey, int x, int y, int wheel, int status);
    void directionGamekeyValue(QString gameKey, int status, int type);
    void viewGamekeyValue(QString gameKey, int status, int type);
    void viewGamekeyInit();
    void viewPosMethod(int posX, int posY);
    QPoint mapMousePos(QPoint pos);
    void brainGamekeyMethod(QPoint mousePos, QPoint widgetPos);

private:
    KmreWindow* mMainWindow = nullptr;
    bool mIsMouseClick = true;
    QString mButtonClickOrder[9] = {"", "", "", "", "", "", "", "", ""};//多点触控
    QMap<QString, GamekeyTimer*> mGameKeyMap;
    QTimer *mViewTimer = nullptr;
    DirValue mDirValue;
    DirValue mAngleValue;
    int mPovValue = 0;
    int mViewValue = 0;
    QList<DirValue> mValueList;
    QList<int> mTipList;
    ViewPos mViewPos;
    ViewPos mMousePos;
    ViewLockInfo mViewLockInfo;
    QPoint androidPos = QPoint(0, 0);
    WheelInfo mWheelInfo;
    bool isBrainPress = false;
    bool isViewLockPress = false;
    QString mBrainPressGameKey = "";
    BrainInfo mBrainInfo;

signals:
    void buttonSignal(QString gameKey);
    void isShowMouseSignal(bool status);
    void settingMousePosSignal(QPoint pos);

};

#endif // KEYBOARDGAMEKEYMANAGER_H
