/* Wrap XLIB code in a new file as it defines keywords that conflict with Qt

    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "xtest-sender.h"
#include <X11/extensions/XTest.h>

void sendXTestPressed(Display *display, int button)
{
    XTestFakeButtonEvent(display, button, true, 0);
}

void sendXTestReleased(Display *display, int button)
{
    XTestFakeButtonEvent(display, button, false, 0);
}

void sendXTestMotion(Display *display, int screen, int x, int y)
{
    XTestFakeMotionEvent(display, screen, x, y, 0);
}