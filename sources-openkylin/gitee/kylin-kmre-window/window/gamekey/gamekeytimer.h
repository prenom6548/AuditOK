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

#ifndef GAMEKEYTIMER_H
#define GAMEKEYTIMER_H

#include <QObject>
#include <QTimer>

class DisplayManager;

class GamekeyTimer : public QObject
{
    Q_OBJECT
public:
    GamekeyTimer(DisplayManager* displayManager);
    ~GamekeyTimer();
    void startGamekeyTimer(int gamekeyorder);

    void stopGamekeyTimer(int gamekeyorder);

    void setGameKeyMassage(int posx,int posy,int combonumber);

    void deleteGamekeyTimer();

public:
    QTimer *mTimer = nullptr;

private:
    int mPosX = 0;
    int mPosY = 0;
    int mComboNumber = 1;
    int mGamekeyOrder = 0;

    DisplayManager* mDisplayManager = nullptr;

private slots:
    void gameKeySolt();
};

#endif // GAMEKEYTIMER_H
