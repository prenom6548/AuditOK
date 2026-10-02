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

#ifndef QJOYSTICKS_H
#define QJOYSTICKS_H

#include <QObject>
#include <QStringList>
#include "joystickscommon.h"
#include "gamekeymanager.h"
#include "joystickgamekeymanager.h"

class SDL_Joysticks;

class QJoysticks : public QObject
{
    Q_OBJECT
public:
   explicit QJoysticks(JoystickGamekeyManager* JoystickGamekey, QObject* parent = nullptr);
   ~QJoysticks();

signals:
    void setJoysticks(int);

public:
    int count() const;
    QStringList deviceNames() const;

    SDL_Joysticks *sdlJoysticks() const;
    QJoystickDevice *getInputDevice(const int index);
    QList<QJoystickDevice *> inputDevices() const;

public slots:
   void updateInterfaces();

private slots:
   void resetJoysticks();
   void addInputDevice(QJoystickDevice *device);
   void setJoystickGameKeyManager(JoystickGamekeyManager* JoystickGamekey);
   
private:
   SDL_Joysticks *m_sdlJoysticks;

   QList<QJoystickDevice *> m_devices;
   GameKeyManager* m_gameKeyManager = nullptr;
   JoystickGamekeyManager* m_joystickgameKeyManager = nullptr;
};

#endif // QJOYSTICKS_H
