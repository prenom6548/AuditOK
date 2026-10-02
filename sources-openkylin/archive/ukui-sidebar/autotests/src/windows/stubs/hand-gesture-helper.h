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
 *
 */

#ifndef AUTOTESTS_WINDOWS_STUB_HAND_GESTURE_HELPER_H
#define AUTOTESTS_WINDOWS_STUB_HAND_GESTURE_HELPER_H

#include <QObject>

namespace Sidebar {

class HandGestureHelper : public QObject
{
    Q_OBJECT
public:
    static HandGestureHelper *getInstance();
    void reset();

public Q_SLOTS:
    void callNotificationCenter(int posY);
    void top2BottomRelease(int posX, int posY);
    void callControlCenter(int posX);
    void right2LeftRelease(int posX, int posY);

Q_SIGNALS:
    void notificationCenterCalled(int posY);
    void top2BottomReleased(int posX, int posY);
    void controlCenterCalled(int posX);
    void right2LeftReleased(int posX, int posY);

private:
    explicit HandGestureHelper(QObject *parent = nullptr);
};

} // namespace Sidebar

#endif
