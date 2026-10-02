/* Copyright (C) 2007-2008 The Android Open Source Project
** Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#ifndef _KEY_EVENT_HANDLER_H_
#define _KEY_EVENT_HANDLER_H_

#include "eventdata.h"
#include "keycodes_common.h"

class KeyEventHandler
{
public:
    KeyEventHandler();
    ~KeyEventHandler();

    static KeyEventHandler *getInstance();

    struct KeyEvent {
        int scancode;
        int nativeModifiers;
        int modifiers;
        int eventType;
    };

    KeyEventData *handleKeyEvent(KeyEventHandler::KeyEvent event);
    int distinguishKeyModifier(int keycode, int nativeModifiers, int modifiers);

private:
    static KeyEventHandler *m_pInstance;
    unsigned int m_altMask;
    unsigned int m_altGrMask;
    unsigned int m_numLockMask;
    KeyEventData *m_keyEventObj = nullptr;
};

#endif // _KEY_EVENT_HANDLER_H_
