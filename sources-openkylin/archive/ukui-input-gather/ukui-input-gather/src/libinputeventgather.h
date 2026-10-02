/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
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
 *
 */

#ifndef LIBINPUTEVENTGATHER_H
#define LIBINPUTEVENTGATHER_H

#include "inputeventgather.h"

class LibinputEventGather : public InputEventGather
{
    Q_OBJECT
public:
    LibinputEventGather(InputEventGather* parent = nullptr);
    ~LibinputEventGather();

    void run() override;

private:
    void init();
    void handleEvent(struct libinput_event *event);
    /*!
     *  \brief 事件转换
     */
    void handleSwipeEvent(struct libinput_event *e, Event &event);
    void handlePinchEvent(struct libinput_event *e, Event &event);
    void handleSwitchEvent(struct libinput_event *e, Event &event);
    void handleRawTapEvent(struct libinput_event *e, Event &event);
    void handleDeviceAdded(struct libinput_event *e, Event &event);

    void initTapToClickState(struct libinput_device *device);

    /**
     * udev context.
     */
    struct udev *udevContext = nullptr;

    /**
     * libinput context.
     */
    struct libinput *libinputContext = nullptr;
    /**
     * libinput structure with pointers to the open/close callbacks.
     */
    struct libinput_interface libinputInterface {
            LibinputEventGather::openRestricted,
            LibinputEventGather::closeRestricted
    };
    static int openRestricted(const char *path, int flags, void *userData);
    static void closeRestricted(int fd, void *userData);

    QString m_project;
};

#endif // LIBINPUTEVENTGATHER_H
