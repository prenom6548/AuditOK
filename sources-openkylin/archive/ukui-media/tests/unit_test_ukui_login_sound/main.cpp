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
#include "../../ukui-login-sound/ukui_login_sound.h"

class soundThemePlayerTest:public testing::Test
{
protected:
    void SetUp() override {
        player = new UkuiLoginSound();
        sleep(1);
    }

    void TearDown() override {
        delete player;
    }

    UkuiLoginSound* player;
};

//  播放音效测试
TEST_F(soundThemePlayerTest, playStartMusicTest) {
    QString defaultOutputDev = "default";
    player->checkPcm(defaultOutputDev.toLatin1().data());
    player->aplayMain(PLAY_STARTUP_WAV, defaultOutputDev.toLatin1().data());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
