/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "../../kysdk-soundeffects/src/ksoundeffects.h"

#include <QtTest>

// add necessary includes here

using namespace kdk;

class SoundEffectsTest : public QObject
{
    Q_OBJECT

private slots:
    void soundeffectsTest();

};

void SoundEffectsTest::soundeffectsTest()
{
    KSoundEffects effects;

    effects.playSound(SoundType::AUDIO_VOLUME_CHANGE);
    effects.playSound(SoundType::BATTERY_LOW);
    effects.playSound(SoundType::COMPLETE);
    effects.playSound(SoundType::DEVICE_ADDED_FAILED);
    effects.playSound(SoundType::DEVICE_REMOVED);
    effects.playSound(SoundType::DEVICE_ADDED);
    effects.playSound(SoundType::DIALOG_ERROR);
    effects.playSound(SoundType::DIALOG_WARNING);
    effects.playSound(SoundType::NOTIFICATION_GENERAL);
    effects.playSound(SoundType::NOTIFICATION_SPECIAL);
    effects.playSound(SoundType::OPERATION_FILE);
    effects.playSound(SoundType::OPERATION_UNSUPPORTED);
    effects.playSound(SoundType::POWER_PLUG);
    effects.playSound(SoundType::SCREEN_CAPTURE);
    effects.playSound(SoundType::SCREEN_RECORD);
    effects.playSound(SoundType::START_UP);
    effects.playSound(SoundType::TRASH_EMPTY);
//    effects.playSound(SoundType::TRASH_EMPTY);
}

QTEST_APPLESS_MAIN(SoundEffectsTest)

#include "soundeffectstest.moc"
