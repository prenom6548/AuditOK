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

#ifndef KEYBOARDCOMMON_H
#define KEYBOARDCOMMON_H

#define CTRLPRESSED         20
#define ALTPRESSED          24
#define SHIFTPRESSED        17

#define CLICKBUTTON         1
#define COMBOBUTTON         2
#define BRAINBUTTON         3

#define WHEELBUTTON         6
#define FIREBUTTON          7
#define VIEWLOCK            8

#define VIEWWHELL           9
#define GRAVITYBUTTON       10

#define MOUSELEFTBUTTON     "MOUSELEFT"
#define MOUSERIGHTBUTTON    "MOUSERIGHT"

#define DIRBUTTON           "DIR"
#define VIEWBUTTON          "VIEW"
#define MOUSEMOVE           "MOVE"

#define UPBUTTON            "UP"
#define DOWNBUTTON          "DOWN"
#define LEFTBUTTON          "LEFT"
#define RIGHTBUTTON         "RIGHT"

#define UPVALUE             1
#define DOWNVALUE           -1
#define LEFTVALUE           3
#define RIGHTVALUE          -3
#define LEFTUPVALUE         4
#define RIGHTUPVALUE        -2
#define LEFTDOWNVALUE       2
#define RIGHTDOWNVALUE      -4

#define BRAINWIDTH          300
#define BRAINHEIGHT         280

struct DirValue
{
    int mUpValue = 1;
    int mDownValue = -1;
    int mLeftValue = 3;
    int mRightValue = -3;
};

struct ViewPos
{
    int mViewPosX = 0;
    int mViewPosY = 0;
    int mViewPosXPre = 0;
    int mViewPosYPre = 0;
};

struct WheelInfo
{
    int mWheelPosX = 0;
    int mWheelPosY = 0;
};

struct BrainInfo
{
    int mBrainPosX = 0;
    int mBrainPosY = 0;
    int mBrainPosXPre = 0;
    int mBrainPosYPre = 0;
};

struct ViewLockInfo
{
    int mPosX = 0;
    int mPosY = 0;
    int mPosXPre = 0;
    int mPosYPre = 0;
    int mWheelX = 0;
    int mWheelY = 0;
    int mMouseSensitivity =0;
    int mDisplayX = 0;
    int mDisplayY = 0;
    int mDisplayXPre = 0;
    int mDisplayYPre = 0;
    bool mIsRecover = true;
};

#endif // KEYBOARDCOMMON_H
