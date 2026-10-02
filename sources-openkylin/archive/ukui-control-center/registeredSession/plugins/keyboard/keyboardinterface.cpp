/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "keyboardinterface.h"

#include <QDebug>

KeyBoardInterface::KeyBoardInterface():UKUI::ServiceObject(nullptr)
{
    keyboardGsettings = UniversalInterface::self()->ukuiKeyBoardGsettings();
    ukccOsdGsettings  = UniversalInterface::self()->ukccOsdGsettings();
    ukuiVirKeyBoardGsettings = UniversalInterface::self()->ukuiVirtualKeyboardGsettings();

    initConnect();
}

KeyBoardInterface::~KeyBoardInterface()
{

}

void KeyBoardInterface::initConnect()
{
    connect(keyboardGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == KEYREPEAT_KEY) {
            Q_EMIT changed("repeat");
        } else if (key == DELAY_KEY) {
            Q_EMIT changed("delay");
        } else if (key == SPEED_KEY) {
            Q_EMIT changed("rate");
        }
    });

    connect(ukccOsdGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == KEYTIPS_KEY) {
            Q_EMIT changed("show-lock-tip");
        }
    });

    connect(ukuiVirKeyBoardGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == FLOAT_KEY) {
            Q_EMIT changed("showOnDesktop");
        } else if (key == TRAY_KEY) {
            Q_EMIT changed("showOnTray");
        } else if (key == SCALE_KEY) {
            Q_EMIT changed("keyboardSize");
        } else if (key == TEXT_SIZE_KEY) {
            Q_EMIT changed("textSize");
        } else if (key == ANIMATION_KEY) {
            Q_EMIT changed("animation");
        } else if (key == SOUND_EFFECT_KEY) {
            Q_EMIT changed("soundEffect");
        }
    });
}

const bool KeyBoardInterface::getKeyRepeat()
{
    if (keyboardGsettings->keys().contains(KEYREPEAT_KEY)) {
        return keyboardGsettings->get(KEYREPEAT_KEY).toBool();
    } else {
        qCritical() << "keyboardGsettings not contains the key: " << KEYREPEAT_KEY;
    }
    return bool();
}

const int KeyBoardInterface::getDelay()
{
    if (keyboardGsettings->keys().contains(DELAY_KEY)) {
        return keyboardGsettings->get(DELAY_KEY).toInt();
    } else {
        qCritical() << "keyboardGsettings not contains the key: " << DELAY_KEY;
    }
    return int();
}

const int KeyBoardInterface::getRate()
{
    if (keyboardGsettings->keys().contains(SPEED_KEY)) {
        return keyboardGsettings->get(SPEED_KEY).toInt();
    } else {
        qCritical() << "keyboardGsettings not contains the key: " << SPEED_KEY;
    }
    return int();
}

const bool KeyBoardInterface::getShowLockTip()
{
    if (ukccOsdGsettings->keys().contains(KEYTIPS_KEY)) {
        return ukccOsdGsettings->get("show-lock-tip").toBool();
    } else {
        qCritical() << "ukccOsdGsettings not contains the key: " << KEYTIPS_KEY;
    }
    return bool();
}

const bool KeyBoardInterface::getShowOnDesktop()
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(FLOAT_KEY)) {
        return ukuiVirKeyBoardGsettings->get("float-button-enabled").toBool();
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << FLOAT_KEY;
    }
    return bool();
}
const bool KeyBoardInterface::getShowOnTray()
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(TRAY_KEY)) {
        return ukuiVirKeyBoardGsettings->get("").toBool();
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << TRAY_KEY;
    }
    return bool();
}

const int KeyBoardInterface::getKeyboardSize()
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(SCALE_KEY)) {
        return ukuiVirKeyBoardGsettings->get("virtual-keyboard-scale-factor").toInt();
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << SCALE_KEY;
    }
    return int();
}

const int KeyBoardInterface::getTextSize()
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(TEXT_SIZE_KEY)) {
        return ukuiVirKeyBoardGsettings->get("").toInt();
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << TEXT_SIZE_KEY;
    }
    return int();
}

const bool KeyBoardInterface::getAnimation()
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(ANIMATION_KEY)) {
        return ukuiVirKeyBoardGsettings->get("animation-enabled").toBool();
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << ANIMATION_KEY;
    }
    return bool();
}

const bool KeyBoardInterface::getSoundEffect()
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(SOUND_EFFECT_KEY)) {
        return ukccOsdGsettings->get("").toBool();
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << SOUND_EFFECT_KEY;
    }
    return bool();
}

void KeyBoardInterface::setKeyRepeat(const bool checked)
{
    if (keyboardGsettings->keys().contains(KEYREPEAT_KEY)) {
        keyboardGsettings->set(KEYREPEAT_KEY, checked);
    } else {
        qCritical() << "keyboardGsettings not contains the key: " << KEYREPEAT_KEY;
    }
}

void KeyBoardInterface::setDelay(const int value)
{
    if (keyboardGsettings->keys().contains(DELAY_KEY)) {
        keyboardGsettings->set(DELAY_KEY, value);
    } else {
        qCritical() << "keyboardGsettings not contains the key: " << DELAY_KEY;
    }
}

void KeyBoardInterface::setSpeed(const int value)
{
    if (keyboardGsettings->keys().contains(SPEED_KEY)) {
        keyboardGsettings->set(SPEED_KEY, value);
    } else {
        qCritical() << "keyboardGsettings not contains the key: " << SPEED_KEY;
    }
}

void KeyBoardInterface::setKeyTips(const bool checked)
{
    if (ukccOsdGsettings->keys().contains(KEYTIPS_KEY)) {
        ukccOsdGsettings->set("show-lock-tip", checked);
    } else {
        qCritical() << "ukccOsdGsettings not contains the key: " << KEYTIPS_KEY;
    }
}

void KeyBoardInterface::setIconOnDesktop(const bool checked)
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(FLOAT_KEY)) {
        ukuiVirKeyBoardGsettings->set("float-button-enabled", checked);
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << FLOAT_KEY;
    }
}

void KeyBoardInterface::setIconOnTray(const bool checked)
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(TRAY_KEY)) {
        ukuiVirKeyBoardGsettings->set("", checked);
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << TRAY_KEY;
    }
}

void KeyBoardInterface::setKeyboardSize(const int value)
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(SCALE_KEY)) {
        ukuiVirKeyBoardGsettings->set("virtual-keyboard-scale-factor", value);
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << SCALE_KEY;
    }
}

void KeyBoardInterface::setTextSize(const int value)
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(TEXT_SIZE_KEY)) {
        ukuiVirKeyBoardGsettings->set("", value);
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << TEXT_SIZE_KEY;
    }
}

void KeyBoardInterface::setAnimation(const bool checked)
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(ANIMATION_KEY)) {
        ukuiVirKeyBoardGsettings->set("animation-enabled", checked);
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << ANIMATION_KEY;
    }
}

void KeyBoardInterface::setSoundEffect(const bool checked)
{
    if (ukuiVirKeyBoardGsettings != nullptr && ukuiVirKeyBoardGsettings->keys().contains(SOUND_EFFECT_KEY)) {
        ukuiVirKeyBoardGsettings->set("", checked);
    } else {
        qCritical() << "ukuiVirKeyBoardGsettings not contains the key: " << SOUND_EFFECT_KEY;
    }
}

void KeyBoardInterface::resetKey(QString key)
{
    if (ukuiVirKeyBoardGsettings != nullptr) {
        if (key == SCALE_KEY) {
            ukuiVirKeyBoardGsettings->reset(SCALE_KEY);
        }/* else if (key == TEXT_SIZE_KEY) {
            ukuiVirKeyBoardGsettings->reset(TEXT_SIZE_KEY);
        }*/
    }
}
