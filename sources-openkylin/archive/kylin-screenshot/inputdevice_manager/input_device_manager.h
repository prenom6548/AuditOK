/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This file is part of Kylin-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef INPUTDEVICEMANAGER_H
#define INPUTDEVICEMANAGER_H

#include <libudev.h>
#include <libinput.h>
#include <QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QPoint>

struct udev;
struct libinput;
struct libinput_event;

class QSocketNotifier;

class InputDeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit InputDeviceManager(QObject *parent = nullptr);
    virtual ~InputDeviceManager();
    void onReadyRead();

signals:
    void onMouseWheel(QPoint angleDelta);
private:
    udev *m_udev;
    libinput *m_li;
    int m_liFd;
    QPoint m_pos;
    QScopedPointer<QSocketNotifier> m_notifier;

    void processEvent(libinput_event *ev);
    void processAxis(libinput_event_pointer *e);
};

#endif // INPUTDEVICEMANAGER_H
