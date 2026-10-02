/*
 * libkysdk-soundeffects's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef KSOUNDEFFECTS_H
#define KSOUNDEFFECTS_H

#include "kysdk-soundeffects_global.h"
#include <QObject>

namespace kdk
{
/**
 * @defgroup NotificationModule
 */

/**
 * @brief 声音类型，不同场景下可以选择不同枚举类型来播放不同音效
 * @since 2.2.1.0
 */

enum SoundType{
    AUDIO_VOLUME_CHANGE,
    BATTERY_LOW,
    COMPLETE,
    DEVICE_ADDED_FAILED,
    DEVICE_REMOVED,
    DEVICE_ADDED,
    DIALOG_ERROR,
    DIALOG_WARNING,
    NOTIFICATION_GENERAL,
    NOTIFICATION_SPECIAL,
    OPERATION_FILE,
    OPERATION_UNSUPPORTED,
    POWER_PLUG,
    SCREEN_CAPTURE,
    SCREEN_RECORD,
    START_UP,
    TRASH_EMPTY
};

/**
 * @brief 音效通知，可以根据输入的类型触发不同的音效提示用户
 */
class KYSDKSOUNDEFFECTS_EXPORT KSoundEffects : public QObject
{
    Q_OBJECT
public:
    KSoundEffects();

    /**
     * @brief 播放系统音效
     * @param soundEvent
     * @return
     */
    static void playSound(SoundType type);

};
}
#endif // KSOUNDEFFECTS_H
