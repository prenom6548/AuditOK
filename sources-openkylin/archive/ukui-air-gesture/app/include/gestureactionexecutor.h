/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#ifndef GESTUREACTIONEXECUTOR_H
#define GESTUREACTIONEXECUTOR_H

#include "mediacontroller.h"
#include <QElapsedTimer>
#include <QHash>
#include <QString>
#include <QGSettings>
#include <cerrno>
#include <cstring>
#include <linux/input-event-codes.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>

class GestureActionExecutor
{
public:
    GestureActionExecutor();
    ~GestureActionExecutor();

    void executeWheel(QString direction);
    void executeHorizontal(const QString &direction);
    bool executePrintScreen();

private:
    bool shouldSkipAction(const QString &action);
    bool scrollVertical(int delta);
    bool pressKey(int keyCode, const char *keyName);
    bool initInputDevice();
    void cleanup();

private:
    MediaController m_mediaController;

    static constexpr qint64 ACTION_COOLDOWN_MS = 400;    // 动作防抖阈值
    QElapsedTimer m_timer;
    QHash<QString, qint64> m_lastTriggerMs;              // 记录每个方向上次触发的时间

    libevdev *m_device = nullptr;
    libevdev_uinput *m_uinput = nullptr;
};

#endif // GESTUREACTIONEXECUTOR_H
