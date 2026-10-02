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

#include <QDebug>
#include "qjoysticks.h"
#include "sdl_joysticks.h"

QJoysticks::QJoysticks(JoystickGamekeyManager* JoystickGamekey, QObject* parent)
    : QObject(parent)
{

    m_sdlJoysticks = new SDL_Joysticks(this);
    connect(sdlJoysticks(), &SDL_Joysticks::countChanged, this, &QJoysticks::updateInterfaces);

    setJoystickGameKeyManager(JoystickGamekey);
}

QJoysticks::~QJoysticks()
{
    delete m_sdlJoysticks;
}

int QJoysticks::count() const
{
    return inputDevices().count();
}

QStringList QJoysticks::deviceNames() const
{
    QStringList names;

    foreach (QJoystickDevice *joystick, inputDevices()){
        names.append(joystick->name);
    }
    return names;
}

SDL_Joysticks *QJoysticks::sdlJoysticks() const
{
    return m_sdlJoysticks;
}

QList<QJoystickDevice *> QJoysticks::inputDevices() const
{
    return m_devices;
}

void QJoysticks::updateInterfaces()
{
    m_devices.clear();
    foreach (QJoystickDevice *joystick, sdlJoysticks()->joysticks())
    {
        addInputDevice(joystick);
    }
    emit setJoysticks(this->count());
}

void QJoysticks::resetJoysticks()
{
    m_devices.clear();
}

void QJoysticks::addInputDevice(QJoystickDevice *device)
{
    Q_ASSERT(device);
    m_devices.append(device);
}

void QJoysticks::setJoystickGameKeyManager(JoystickGamekeyManager* JoystickGamekey)
{
    this->m_joystickgameKeyManager = JoystickGamekey;
    connect(sdlJoysticks(), &SDL_Joysticks::joystickPOVEvent, this->m_joystickgameKeyManager, &JoystickGamekeyManager::sdlJoysticksPOV);
    connect(sdlJoysticks(), &SDL_Joysticks::joystickButtonEvent, this->m_joystickgameKeyManager, &JoystickGamekeyManager::sdlJoysticksButton);
    connect(sdlJoysticks(), &SDL_Joysticks::joystickAxisEvent, this->m_joystickgameKeyManager, &JoystickGamekeyManager::sdlJoysticksAxis);
    connect(this,SIGNAL(setJoysticks(int)),this->m_joystickgameKeyManager,SLOT(getJoystickChange(int)));
}
