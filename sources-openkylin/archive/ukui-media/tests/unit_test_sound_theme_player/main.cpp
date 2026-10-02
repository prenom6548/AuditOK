/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../sound-theme-player/sound_theme_player.h"
#include <QTimer>

class soundThemePlayerTest:public testing::Test
{
protected:
    void SetUp() override {
        player = new SoundThemePlayer();
        sleep(1);
    }

    void TearDown() override {
        delete player;
    }

    SoundThemePlayer* player;
};

//  播放音效测试
TEST_F(soundThemePlayerTest, Test_playAlertSound) {
    player->playAlertSound("audio-volume-change");
}

//  播放自定义音效测试
TEST_F(soundThemePlayerTest, Test_playCustomAlertSound) {
    player->playCustomAlertSound("/usr/share/sounds/Light-Seeking/stereo/audio-volume-change.ogg");
}

//  音量转化测试
TEST_F(soundThemePlayerTest, Test_alertVolumeTodB) {
    char *dB_value = player->alertVolumeTodB(100);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
