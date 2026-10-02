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
 */

#include "keyboardmanager.h"

KeyboardManager::KeyboardManager(QObject *parent) : QObject(parent)
{

}

void KeyboardManager::setKeyboardAvailable(bool keyBoardAvailable)
{
    m_keyboardAvailable = keyBoardAvailable;
}

bool KeyboardManager::isKeyboardAvailable() const
{
    return m_keyboardAvailable;
}

void KeyboardManager::setKeyboardEnabled()
{
    if (m_keyboardAvailable) {
        return;
    }
    m_keyboardAvailable = true;
    Q_EMIT keyboardAvailableChanged(m_keyboardAvailable);
}

void KeyboardManager::setKeyboardDisabled()
{
    if (!m_keyboardAvailable) {
        return;
    }
    m_keyboardAvailable = false;
    Q_EMIT keyboardAvailableChanged(m_keyboardAvailable);
}
