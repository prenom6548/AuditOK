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

#include "keyevent_handler.h"
#include "keyevent_worker.h"
#include "kmre_keycode.h"

#include <sys/syslog.h>

KeyEventHandler *KeyEventHandler::m_pInstance = nullptr;

KeyEventHandler *KeyEventHandler::getInstance()
{
    if (nullptr == m_pInstance) {
        if(nullptr == m_pInstance) {
            m_pInstance = new KeyEventHandler;
        }
    }

    return m_pInstance;
}

KeyEventHandler::KeyEventHandler()
    : m_altMask(8)
    , m_altGrMask(128)
    , m_numLockMask(16)
{

}

KeyEventHandler::~KeyEventHandler()
{
    if (m_keyEventObj) {
        delete m_keyEventObj;
        m_keyEventObj = nullptr;
    }
}

int KeyEventHandler::distinguishKeyModifier(int keycode, int nativeModifiers, int modifiers)
{
    int result = 0;

    (void)modifiers;

    if (nativeModifiers & 4) {//4 XCB_MOD_MASK_CONTROL Ctrl
        result |= KeyModLCtrl;
    }
    if (nativeModifiers & 1) {//1 XCB_MOD_MASK_SHIFT Shift
        result |= KeyModLShift;
    }
    if (nativeModifiers & 2) {//2  XCB_MOD_MASK_LOCK  capslock
        result |= KeyModCapsLock;
    }
    if (nativeModifiers & m_numLockMask) {//16 XCB_MOD_MASK_2  numlock
        result |= KeyModNumLock;
    }

    if (nativeModifiers & m_altMask) {//m_altMask=8  left alt
        result |= kKeyModLAlt;
    }

    if (nativeModifiers & m_altGrMask) {//m_altGrMask=128  right alt
        result |= KeyModRAlt;
    }

    if ((nativeModifiers & 2) && linux_keycode_is_alphabetical(keycode) &&
        !(nativeModifiers & m_altMask)) {//XCB_MOD_MASK_LOCK=2,  m_altMask=8
        result |= KeyModLShift;
    }

    return result;
}

KeyEventData *KeyEventHandler::handleKeyEvent(KeyEventHandler::KeyEvent keyevent)
{
    //syslog(LOG_DEBUG, "KeyEventHandler::handleKeyEvent scancode %d modifiers 0x%x.", keyevent.scancode, keyevent.nativeModifiers);
    int scancode = keyevent.scancode;
    int nativeModifiers = keyevent.nativeModifiers;
    int modifiers = keyevent.modifiers;
    int linuxKeycode = convert_key_scancode_to_linux_keycode(scancode);

    //比如NumLock按下为77,对应的linuxKeycode为69   (#define LINUX_KEY_NUMLOCK 69)
    if (linuxKeycode == 0 || linux_keycode_is_modifier(linuxKeycode)) {
        //syslog(LOG_DEBUG, "KeyEventHandler::handleKeyEvent linux key code equal 0, or the code is modifier(linuxKeycode:%d)", linuxKeycode);
        return nullptr;
    }

    if (!m_keyEventObj) {
        m_keyEventObj = KeyEventWorker::getInstance()->createKeyEventObj(keyevent.eventType);
    }

    TextInputData &textInputData = m_keyEventObj->text;
    textInputData.keycode = linuxKeycode;
    textInputData.mod = distinguishKeyModifier(textInputData.keycode, nativeModifiers, modifiers);
    //syslog(LOG_DEBUG, "KeyEventHandler::handleKeyEvent linuxKeycode %d modifiers 0x%x.", linuxKeycode, m_keyEventObj->text.mod);

    return m_keyEventObj;
}
