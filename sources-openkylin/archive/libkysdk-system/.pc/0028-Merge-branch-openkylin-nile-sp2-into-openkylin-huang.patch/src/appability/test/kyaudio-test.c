/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "../libkyaudio.h"
#include "stdio.h"

// 测试函数示例
void test_open_audio() 
{
    int result = open_audio();
    printf(result == 1 ? "Test passed: open_audio() succeeded.\n" : "Test failed: open_audio() failed.\n");
}

void test_kdk_audio_exit() 
{
    int result = kdk_audio_exit();
    printf(result == 1 ? "Test passed: kdk_audio_exit() succeeded.\n" : "Test failed: kdk_audio_exit() failed.\n");
}

void test_kdk_audio_full_screen() 
{
    int result = kdk_audio_full_screen();
    printf(result == 1 ? "Test passed: kdk_audio_full_screen() succeeded.\n" : "Test failed: kdk_audio_full_screen() failed.\n");
}

void test_kdk_audio_next() 
{
    int result = kdk_audio_next();
    printf(result == 1 ? "Test passed: kdk_audio_next() succeeded.\n" : "Test failed: kdk_audio_next() failed.\n");
}

void test_kdk_audio_open_uri() 
{
    const char* test_uri = "/home/zm/音乐/大气.mp3";  // 假设的测试文件路径
    int result = kdk_audio_open_uri(test_uri);
    printf(result == 1 ? "Test passed: kdk_audio_open_uri() succeeded.\n" : "Test failed: kdk_audio_open_uri() failed.\n");
}

void test_kdk_audio_pause() 
{
    int result = kdk_audio_pause();
    printf(result == 1 ? "Test passed: kdk_audio_pause() succeeded.\n" : "Test failed: kdk_audio_pause() failed.\n");
}

void test_kdk_audio_play() 
{
    int result = kdk_audio_play();
    printf(result == 1 ? "Test passed: kdk_audio_play() succeeded.\n" : "Test failed: kdk_audio_play() failed.\n");
}

void test_kdk_audio_play_pause() 
{
    int result = kdk_audio_play_pause();
    printf(result == 1 ? "Test passed: kdk_audio_play_pause() succeeded.\n" : "Test failed: kdk_audio_play_pause() failed.\n");
}

void test_kdk_audio_previous() 
{
    int result = kdk_audio_previous();
    printf(result == 1 ? "Test passed: kdk_audio_previous() succeeded.\n" : "Test failed: kdk_audio_previous() failed.\n");
}

void test_kdk_audio_seek() 
{
    int offset = -10000000; // 假设的偏移量，单位为微秒
    int result = kdk_audio_seek(offset);
    printf(result == 1 ? "Test passed: kdk_audio_seek() succeeded.\n" : "Test failed: kdk_audio_seek() failed.\n");
}

void test_kdk_audio_set_position() 
{
    const char* trackid = ""; // 假设的轨道标识符
    // const char* trackid = "给我一个吻"; // 假设的轨道标识符
    int position = 60000000; // 假设的指定位置，单位为微秒
    int result = kdk_audio_set_position(trackid, position);
    printf(result == 1 ? "Test passed: kdk_audio_set_position() succeeded.\n" : "Test failed: kdk_audio_set_position() failed.\n");
}

void test_kdk_audio_stop() 
{
    int result = kdk_audio_stop();
    printf(result == 1 ? "Test passed: kdk_audio_stop() succeeded.\n" : "Test failed: kdk_audio_stop() failed.\n");
}

void test_kdk_audio_volume_down() 
{
    int result = kdk_audio_volume_down();
    printf(result == 1 ? "Test passed: kdk_audio_volume_down() succeeded.\n" : "Test failed: kdk_audio_volume_down() failed.\n");
}

void test_kdk_audio_volume_up() 
{
    int result = kdk_audio_volume_up();
    printf(result == 1 ? "Test passed: kdk_audio_volume_up() succeeded.\n" : "Test failed: kdk_audio_volume_up() failed.\n");
}

void test_kdk_audio_get_state() 
{
    char* state = kdk_audio_get_state();
    if (state) 
    {
        printf("Test passed: kdk_audio_get_state() succeeded. Current state: %s\n", state);
    } 
    else 
    {
        printf("Test failed: kdk_audio_get_state() failed.\n");
    }
}

int main() 
{
    // 执行所有测试函数
    test_open_audio();
    // test_kdk_audio_exit();
    // test_kdk_audio_full_screen();
    // test_kdk_audio_next();
    // test_kdk_audio_open_uri();
    // test_kdk_audio_pause();
    // test_kdk_audio_play();
    // test_kdk_audio_play_pause();
    // test_kdk_audio_previous();
    // test_kdk_audio_seek();
    // test_kdk_audio_set_position();
    // test_kdk_audio_stop();
    // test_kdk_audio_volume_down();
    // test_kdk_audio_volume_up();
    // test_kdk_audio_get_state();

    return 0;
}
