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

#ifndef JOYSTICKGAMEKEYMANAGER_H
#define JOYSTICKGAMEKEYMANAGER_H

#include <QObject>
#include <QMap>

#include "joystickscommon.h"
#include "common.h"
#include "gamekeytimer.h"

class KmreWindow;

class JoystickGamekeyManager : public QObject
{
    Q_OBJECT
public:
    JoystickGamekeyManager(KmreWindow* window);
    ~JoystickGamekeyManager();

    bool getJoystickCount();

public slots:
    void sdlJoysticksPOV(int value);
    void sdlJoysticksButton(int value, int btn_status);
    void sdlJoysticksAxis(qreal value, int axis_id);
    void gameKeyInit();
    void getJoystickChange(int value);
    void focusOutMethod();

private:
    KmreWindow* mMainWindow = nullptr;
    QString mPackName;
    AxisValue mAxisValue;
    AxisPos mAxisPos;
    AxisFrequency mAxisFrequency;
    PovStatus mPovStatus;
    AxisStatus mAxisAtatus;
    int mButtonClickOrder[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};//多点触控
    QMap<QString, GamekeyTimer*> mGameKeyMap;
    QTimer *mLATimer = nullptr;
    QTimer *mRATimer = nullptr;
    int mJoystickCount = 0;
    bool isBrainPress = false;
    QString mBrainPressGameKey = "";
    JoystickBrainInfo mBrainInfo;

private:
    void settingGameKey(QString gameKey, int status);
    void handleButtonGameKey(int value, QString gameKey, int status);
    void handlePovGameKeyPress(int value, QString gameKey);
    void handlePovGameKeyRelease();
    void handleAxisGameKey(int value, QString gameKey, int status);
    void gameKeyPress(int pos_x , int pos_y, int order);
    void gameKeyMove(int pos_x , int pos_y, int order);
    void gameKeyRelease(int pos_x , int pos_y, int order);
    int getGameKeyOrderPress(int value);
    int getGameKeyOrderRelease(int value);
    int getGameKeyOrderMove(int value);
    void initGameKeyTimers();
    void addGameKeyMap(QString gamekey);
    void povGamekeyRelease();
    void updatePovStatus(int value, bool status);
    void axisTimerInit();
    bool isGamekeySetting(QString gamekey);
    void setAxisUpDownGamekeyFrequency(qreal value);
    void setAxisRightLeftGamekeyFrequency(qreal value);
    void axisRAMove();

private slots:
    void handleAxisLAGamekey();
    void handleAxisRAGamekey();

signals:
    void buttonSignal(QString gameKey);
    void joystickChange(bool count);
};

#endif // JOYSTICKGAMEKEYMANAGER_H
