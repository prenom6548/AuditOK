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

#pragma once

#include <string.h>

#include "keycodes_common.h"

class KeyEventWorker
{
public:
    static KeyEventWorker *getInstance() {
        if (!m_instance) {
           m_instance = new KeyEventWorker;
        }

        return m_instance;
    }

    KeyEventWorker();
    ~KeyEventWorker();

    KeyEventData *createKeyEventObj(KeyEventType type);
    KmreKeyboard *manageKeyboardEvent(KeyEventData *event);

private:
    static KeyEventWorker *m_instance;
    KeyEventData *m_keyEventObj = nullptr;
    KmreKeyboard *m_keyboard = nullptr;
};
