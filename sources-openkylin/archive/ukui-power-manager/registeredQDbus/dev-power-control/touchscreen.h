/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "devrpminterface.h"
#include <libinput.h>
#include <libudev.h>
// #define KEYBOARD_BACKLIGHT_PATH       "/sys/class/leds/"
// #define KEYBOARD_BACKLIGHT_FILE       "brightness"

class TouchScreenDevice : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit TouchScreenDevice(QObject *parent = nullptr);

private:
    void getDevicePowerInfo();
    QString m_controlPath;
    QString m_deviceId;

public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);
    // libinput 回调
    static int open_restricted(const char *path, int flags, void *user_data);
    static void close_restricted(int fd, void *user_data);
    bool touchScreenPathid(const QString &eventPath);
};

#endif // TOUCHSCREEN_H
