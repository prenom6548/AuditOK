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

#ifndef SDL_JOYSTICKS_H
#define SDL_JOYSTICKS_H

#include <QObject>
#include <QMap>
#include <SDL2/SDL.h>

#include "joystickscommon.h"

class SDL_Joysticks : public QObject
{
    Q_OBJECT
signals:
   void countChanged();
   void joystickPOVEvent(int value, int pov_status);
   void joystickButtonEvent(int value, int btn_status);
   void joystickAxisEvent(qreal value, int axis_id);

public:
    SDL_Joysticks(QObject *parent = nullptr);
    ~SDL_Joysticks();
    QMap<int, QJoystickDevice *> joysticks();
public slots:
   void rumble(const QJoystickRumble &request);

private slots:
   void update();
   void configureJoystick(const SDL_Event *event);

private:
   QJoystickDevice *getJoystick(int id);
   QJoystickPOVEvent getPOVEvent(const SDL_Event *sdl_event);
   QJoystickAxisEvent getAxisEvent(const SDL_Event *sdl_event);
   QJoystickButtonEvent getButtonEvent(const SDL_Event *sdl_event);
   QMap<int, QJoystickDevice *> m_joysticks;
};

#endif // SDL_JOYSTICKS_H
