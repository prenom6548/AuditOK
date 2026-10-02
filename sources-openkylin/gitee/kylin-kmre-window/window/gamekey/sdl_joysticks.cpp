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

#include "sdl_joysticks.h"

#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QApplication>

static QString GENERIC_MAPPINGS;

#ifdef SDL_SUPPORTED
#   define GENERIC_MAPPINGS_PATH ":/QJoysticks/SDL/GenericMappings/Linux.txt"
#endif

SDL_Joysticks::SDL_Joysticks(QObject *parent)
    : QObject(parent)
{
#ifdef SDL_SUPPORTED
    if (SDL_Init(SDL_INIT_HAPTIC | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER))
    {
        qDebug() << "Cannot initialize SDL:" << SDL_GetError();
        qApp->quit();
    }

    QFile database(":/QJoysticks/SDL/Database.txt");
    if (database.open(QFile::ReadOnly))
    {
        while (!database.atEnd())
        {
            QString line = QString::fromUtf8(database.readLine());
            SDL_GameControllerAddMapping(line.toStdString().c_str());
        }

        database.close();
    }

    QFile genericMappings(GENERIC_MAPPINGS_PATH);
    if (genericMappings.open(QFile::ReadOnly))
    {
        GENERIC_MAPPINGS = QString::fromUtf8(genericMappings.readAll());
        genericMappings.close();
    }

    QTimer::singleShot(100, Qt::PreciseTimer, this, SLOT(update()));
#endif
}
SDL_Joysticks::~SDL_Joysticks()
{
    for (QMap<int, QJoystickDevice *>::iterator i = m_joysticks.begin(); i != m_joysticks.end(); ++i)
    {
        delete i.value();
    }

#ifdef SDL_SUPPORTED
    SDL_Quit();
#endif
}

QMap<int, QJoystickDevice *> SDL_Joysticks::joysticks()
{
#ifdef SDL_SUPPORTED
    int index = 0;
    QMap<int, QJoystickDevice *> joysticks;
    for (QMap<int, QJoystickDevice *>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    {
        it.value()->id = index;
        joysticks[index++] = it.value();
    }
    qDebug() << "joysticks:: " << joysticks;
    return joysticks;
#endif
    return QMap<int, QJoystickDevice *>();
}

void SDL_Joysticks::rumble(const QJoystickRumble &request)
{
#ifdef SDL_SUPPORTED
    SDL_Haptic *haptic = SDL_HapticOpen(request.joystick->id);

    if (haptic)
    {
        SDL_HapticRumbleInit(haptic);
        SDL_HapticRumblePlay(haptic, request.strength, request.length);
    }
#else
    Q_UNUSED(request);
#endif
}

void SDL_Joysticks::update()
{
#ifdef SDL_SUPPORTED
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_JOYDEVICEADDED: {
            configureJoystick(&event);
        }
            break;
        case SDL_JOYDEVICEREMOVED: {
            SDL_Joystick *js = SDL_JoystickFromInstanceID(event.jdevice.which);
            if (js)
            {
                SDL_JoystickClose(js);
            }

            SDL_GameController *gc = SDL_GameControllerFromInstanceID(event.cdevice.which);
            if (gc)
            {
                SDL_GameControllerClose(gc);
            }
        }

            delete m_joysticks[event.jdevice.which];
            m_joysticks.remove(event.jdevice.which);

            emit countChanged();
            break;
        case SDL_JOYAXISMOTION: {
            if (!SDL_IsGameController(event.cdevice.which))
            {
                getAxisEvent(&event);
            }
        }
            break;
        case SDL_CONTROLLERAXISMOTION: {
            if (SDL_IsGameController(event.cdevice.which))
            {
                getAxisEvent(&event);
            }
        }
            break;
        case SDL_JOYBUTTONUP: {
            getButtonEvent(&event);
        }
            break;
        case SDL_JOYBUTTONDOWN: {
            getButtonEvent(&event);
        }
            break;
        case SDL_JOYHATMOTION: {
            getPOVEvent(&event);
        }
            break;
        }
    }

    QTimer::singleShot(1000, Qt::PreciseTimer, this, SLOT(update()));
#endif
}

void SDL_Joysticks::configureJoystick(const SDL_Event *event)
{
#ifdef SDL_SUPPORTED
    QJoystickDevice *joystick = getJoystick(event->jdevice.which);
    if (!SDL_IsGameController(event->cdevice.which))
    {
        SDL_Joystick *js = SDL_JoystickFromInstanceID(joystick->instanceID);
        if (js)
        {
            char guid[1024];
            SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(js), guid, sizeof(guid));

            QString mapping = QString("%1,%2,%3").arg(guid).arg(SDL_JoystickName(js)).arg(GENERIC_MAPPINGS);

            SDL_GameControllerAddMapping(mapping.toStdString().c_str());
        }
    }

    SDL_GameControllerOpen(event->cdevice.which);

    emit countChanged();
#else
    Q_UNUSED(event);
#endif
}

QJoystickDevice *SDL_Joysticks::getJoystick(int id)
{
#ifdef SDL_SUPPORTED
    QJoystickDevice *joystick = new QJoystickDevice;
    SDL_Joystick *sdl_joystick = SDL_JoystickOpen(id);

    if (sdl_joystick)
    {
        joystick->id = id;
        joystick->instanceID = SDL_JoystickInstanceID(sdl_joystick);
        joystick->name = SDL_JoystickName(sdl_joystick);

        int povs = SDL_JoystickNumHats(sdl_joystick);
        int axes = SDL_JoystickNumAxes(sdl_joystick);
        int buttons = SDL_JoystickNumButtons(sdl_joystick);

        for (int i = 0; i < povs; ++i)
            joystick->povs.append(0);

        for (int i = 0; i < axes; ++i)
            joystick->axes.append(0);

        for (int i = 0; i < buttons; ++i)
            joystick->buttons.append(false);

        m_joysticks[joystick->instanceID] = joystick;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Cannot find joystick with id:" << id;
    }

    return joystick;
#else
    Q_UNUSED(id);
    return NULL;
#endif
}

QJoystickPOVEvent SDL_Joysticks::getPOVEvent(const SDL_Event *sdl_event)
{
    qreal x;
    qreal y;
    QJoystickPOVEvent event;

    if (!m_joysticks.contains(sdl_event->jdevice.which))
    {
        return event;
    }

#ifdef SDL_SUPPORTED
    event.pov = sdl_event->jhat.hat;
    event.joystick = m_joysticks[sdl_event->jdevice.which];
    event.angle = sdl_event->jhat.value;
    emit joystickPOVEvent(event.angle, event.pov);
#else
    Q_UNUSED(sdl_event);
#endif
    return event;
}

QJoystickAxisEvent SDL_Joysticks::getAxisEvent(const SDL_Event *sdl_event)
{
    QJoystickAxisEvent event;

    if (!m_joysticks.contains(sdl_event->cdevice.which))
    {
        return event;
    }

#ifdef SDL_SUPPORTED
    event.axis = sdl_event->caxis.axis;
    if(sdl_event->caxis.value >= 0){
        event.value = static_cast<qreal>(sdl_event->caxis.value) / 32767;
    }
    else {
        event.value = static_cast<qreal>(sdl_event->caxis.value + 1) / 32767;
    }
    event.joystick = m_joysticks[sdl_event->cdevice.which];
    emit joystickAxisEvent(event.value, event.axis);
#else
    Q_UNUSED(sdl_event);
#endif

    return event;
}

QJoystickButtonEvent SDL_Joysticks::getButtonEvent(const SDL_Event *sdl_event)
{
    QJoystickButtonEvent event;

    if (!m_joysticks.contains(sdl_event->jdevice.which))
    {
        return event;
    }

#ifdef SDL_SUPPORTED
    event.button = sdl_event->jbutton.button;
    event.pressed = sdl_event->jbutton.state == SDL_PRESSED;
    event.joystick = m_joysticks[sdl_event->jdevice.which];
    event.joystick->buttons[event.button] = event.pressed;
    emit joystickButtonEvent(event.button,event.pressed);
#else
    Q_UNUSED(sdl_event);
#endif

    return event;
}
