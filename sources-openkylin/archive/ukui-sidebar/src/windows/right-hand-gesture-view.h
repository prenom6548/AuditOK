/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Author: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_SIDEBAR_RIGHT_HAND_GESTURE_VIEW_H
#define UKUI_SIDEBAR_RIGHT_HAND_GESTURE_VIEW_H

#include <shared-engine-view.h>

#include "window-helper.h"

namespace Sidebar {

class RightHandGestureView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit RightHandGestureView(QWindow *parent = nullptr);
    void updateGeometry();
    void init();

protected:
    bool event(QEvent *event) override;

private:
    QRect windowGeometry();
    void callControlCenter(int x);
    void callControlCenterEnd(int x, int y);
    QWindow* targetWindow(QTouchEvent* touchEvent);

    bool m_isPressed {false};
    UkuiQuick::WindowProxy* m_windowProxy {nullptr};
};

} // Sidebar

#endif //UKUI_SIDEBAR_RIGHT_HAND_GESTURE_VIEW_H
