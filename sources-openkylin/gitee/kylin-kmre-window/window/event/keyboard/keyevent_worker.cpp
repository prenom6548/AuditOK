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

#include "keyevent_worker.h"
#include "kmre_keycode.h"

#include <sys/syslog.h>

KeyEventWorker *KeyEventWorker::m_instance = 0;

static void append_linux_key_event(KmreKeyboard *kk, unsigned code, unsigned down)
{
    if (code != 0 && kk->keycode_buffer->keycode_count < MAX_KEYCODE_COUNTS) {
        kk->keycode_buffer->keycodes[(int)kk->keycode_buffer->keycode_count++] = ( (code & 0x3ff) | (down ? 0x400 : 0) );//code包含了down
    }
}

static KmreKeyMod sync_linux_modifier_key(int modifier_key, KmreKeyboard *kk, int keycode, int mod, int down)
{
    static KmreKeyMod tracked = 0;

    KmreKeyMod mask = 0;
    switch (modifier_key) {
    case LINUX_KEY_LEFTALT:
        mask = kKeyModLAlt;
    case LINUX_KEY_LEFTCTRL:
        mask = KeyModLCtrl;
    case LINUX_KEY_LEFTSHIFT:
        mask = KeyModLShift;
    }

    KmreKeyMod old_mod = tracked & mask;
    if (keycode == modifier_key) {
        if (down) {
            tracked |= mask;
        } else {
            tracked &= ~mask;
        }
        return old_mod;
    }

    if (down && old_mod && !(mod & mask)) {
        append_linux_key_event(kk, modifier_key, 0);
        tracked &= ~mask;
    }

    return old_mod;
}

static void manage_linux_modifier_key(KmreKeyboard *kk, KeyEventData *keyevent, int down)
{
    if (keyevent->text.mod & KeyModLCtrl) {
        append_linux_key_event(kk, LINUX_KEY_LEFTCTRL, down);
    }
    if (keyevent->text.mod & KeyModRCtrl) {
        append_linux_key_event(kk, LINUX_KEY_RIGHTCTRL, down);
    }
    if (keyevent->text.mod & KeyModNumLock) {
        append_linux_key_event(kk, LINUX_KEY_NUMLOCK, 1);
        append_linux_key_event(kk, LINUX_KEY_NUMLOCK, 0);
    }
    if (keyevent->text.mod & KeyModCapsLock) {
        append_linux_key_event(kk, LINUX_KEY_CAPSLOCK, 1);
        append_linux_key_event(kk, LINUX_KEY_CAPSLOCK, 0);
    }
    if (keyevent->text.mod & KeyModRShift) {
        append_linux_key_event(kk, LINUX_KEY_RIGHTSHIFT, down);
    }
    if (keyevent->text.mod & KeyModLShift) {
        append_linux_key_event(kk, LINUX_KEY_LEFTSHIFT, down);
    }
    if (keyevent->text.mod & KeyModRAlt) {
        append_linux_key_event(kk, LINUX_KEY_RIGHTALT, down);
    }
    if (keyevent->text.mod & kKeyModLAlt) {
        append_linux_key_event(kk, LINUX_KEY_LEFTALT, down);
    }
}

KmreKeyboard *createKeyboard()
{
    KmreKeyboard *kk = NULL;

    size_t sz = sizeof(*kk);
    if (sz == 0) {
        return NULL;
    }

    kk = calloc(1, sz);
    if (kk != NULL) {
        kk->keycode_buffer->keycode_count = 0;
        return kk;
    }

    syslog(LOG_ERR, "kmre keyboard: Not enough memory for calloc.");
    exit(1);

    return NULL;
}



KeyEventWorker::KeyEventWorker()
{

}

KeyEventWorker::~KeyEventWorker()
{
    if (m_keyEventObj) {
        delete m_keyEventObj;
        m_keyEventObj = nullptr;
    }

    if (m_keyboard) {
        free(m_keyboard);
        m_keyboard = NULL;
    }
}

KmreKeyboard *KeyEventWorker::manageKeyboardEvent(KeyEventData *event)
{
    KeyEventType keyEventType = event->type;
    if (keyEventType == EventTextInput) {
        if (!m_keyboard) {
            m_keyboard = createKeyboard();
        }

        if (event->type == EventTextInput) {
            KmreKeyMod mod = 0;
            mod = sync_linux_modifier_key(LINUX_KEY_LEFTSHIFT, m_keyboard, 0, 0, 1);

            /*
            // NumLock test code
            if ((event->text.mod & KeyModNumLock) != 0) {
                syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent numlock on.");
            }
            else {
                syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent numlock off.");
            }

            //小键盘数字转换到大键盘数字上
            // /usr/include/linux/input-event-codes.h   (example: code=79,   LINUX_KEY_KP1   vs   KEY_KP1)
            if ((event->text.mod & KeyModNumLock) != 0) {//bug???? 发现按下NumLock键与否，event->text.mod都为0
                switch ((int)event->text.keycode) {
                    case LINUX_KEY_KP0:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP0 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_0;
                        break;
                    case LINUX_KEY_KP1:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP1 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_1;
                        break;
                    case LINUX_KEY_KP2:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP2 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_2;
                        break;
                    case LINUX_KEY_KP3:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP3 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_3;
                        break;
                    case LINUX_KEY_KP4:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP4 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_4;
                        break;
                    case LINUX_KEY_KP5:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP5 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_5;
                        break;
                    case LINUX_KEY_KP6:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP6 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_6;
                        break;
                    case LINUX_KEY_KP7:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP7 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_7;
                        break;
                    case LINUX_KEY_KP8:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP8 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_8;
                        break;
                    case LINUX_KEY_KP9:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP9 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_9;
                        break;
                    case LINUX_KEY_KPPLUS:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPPLUS keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_PLUS;
                        break;
                    case LINUX_KEY_KPMINUS://小键盘的 - 符号
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPMINUS keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_MINUS;
                        break;
                    case LINUX_KEY_KPASTERISK://小键盘的 * 符号
                        //syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPASTERISK keycode=%d.", event->text.keycode);
                        break;
                    case LINUX_KEY_KPSLASH:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPSLASH keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_SLASH;
                        break;
                    case LINUX_KEY_KPEQUAL:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPEQUAL keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_EQUAL;
                        break;
                    case LINUX_KEY_KPDOT://小键盘的 . 符号
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPDOT keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_DOT;
                        break;
                    case LINUX_KEY_KPENTER:
                        syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPENTER keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_ENTER;
                        break;
                    default:
                        ;
                }
            }
            else {
                switch ((int)event->text.keycode) {
                    case LINUX_KEY_KP0:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP0 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_0;
                        break;
                    case LINUX_KEY_KP1:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP1 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_1;
                        break;
                    case LINUX_KEY_KP2:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP2 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_2;
                        break;
                    case LINUX_KEY_KP3:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP3 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_3;
                        break;
                    case LINUX_KEY_KP4:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP4 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_4;
                        break;
                    case LINUX_KEY_KP5:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP5 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_5;
                        break;
                    case LINUX_KEY_KP6:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP6 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_6;
                        break;
                    case LINUX_KEY_KP7:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP7 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_7;
                        break;
                    case LINUX_KEY_KP8:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP8 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_8;
                        break;
                    case LINUX_KEY_KP9:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KP9 keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_9;
                        break;
                    case LINUX_KEY_KPPLUS:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPPLUS keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_PLUS;
                        break;
                    case LINUX_KEY_KPMINUS://小键盘的 - 符号
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPMINUS keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_MINUS;
                        break;
                    case LINUX_KEY_KPASTERISK://小键盘的 * 符号
                        //syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPASTERISK keycode=%d.", event->text.keycode);
                        break;
                    case LINUX_KEY_KPSLASH:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPSLASH keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_SLASH;
                        break;
                    case LINUX_KEY_KPEQUAL:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPEQUAL keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_EQUAL;
                        break;
                    case LINUX_KEY_KPDOT://小键盘的 . 符号
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPDOT keycode=%d.", event->text.keycode);
                        event->text.keycode = LINUX_KEY_PAUSE;
                        break;
                    case LINUX_KEY_KPENTER:
                        syslog(LOG_DEBUG, "OFF $$$$$KeyEventWorker::manageKeyboardEvent LINUX_KEY_KPENTER keycode=%d.", event->text.keycode);
                        //event->text.keycode = LINUX_KEY_ENTER;
                        break;
                    default:
                        ;
                }
            }
            */

            manage_linux_modifier_key(m_keyboard, event, 1);
            //syslog(LOG_DEBUG, "KeyEventWorker::manageKeyboardEvent keycode=%d mod=0x%x.", event->text.keycode, event->text.mod);
            append_linux_key_event(m_keyboard, event->text.keycode, 1);
            //printf("EventTextInput: keycodes[0]:%d, ", m_keyboard->keycode_buffer->keycodes[0]);//EventTextInput: keycodes[0]:1053,  keycodes[1]:1054
            append_linux_key_event(m_keyboard, event->text.keycode, 0);
            manage_linux_modifier_key(m_keyboard, event, 0);

            if (mod) {
                sync_linux_modifier_key(LINUX_KEY_LEFTSHIFT, m_keyboard, LINUX_KEY_LEFTSHIFT, 0, 1);
                append_linux_key_event(m_keyboard, LINUX_KEY_LEFTSHIFT, 1);
            }
        }

        return m_keyboard;
    }

    return nullptr;
}

KeyEventData* KeyEventWorker::createKeyEventObj(KeyEventType type)
{
    if (m_keyEventObj) {
        // 清空text中存放的数据，否则还是第一次键盘输入的数据
        memset(&m_keyEventObj->text, 0, sizeof(TextInputData));
        return m_keyEventObj;
    }

    m_keyEventObj = new KeyEventData();
    m_keyEventObj->type = type;

    return m_keyEventObj;
}
