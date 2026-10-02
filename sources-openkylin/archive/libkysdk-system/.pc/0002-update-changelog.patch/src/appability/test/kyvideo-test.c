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

#include "../libkyvideo.h"
#include "stdio.h"

void test_open_video() 
{
    int result = open_video();
    printf(result == 1 ? "Test passed: open_video() succeeded.\n" : "Test failed: open_video() failed.\n");
}

void test_kdk_video_next() 
{
    int result = kdk_video_next();
    printf(result == 1 ? "Test passed: kdk_video_next() succeeded.\n" : "Test failed: kdk_video_next() failed.\n");
}

void test_kdk_video_add_file() 
{
    const char* test_file = "/home/wjb/影音/llllllll.mp4"; // 替换为实际的测试文件路径
    int result = kdk_video_add_file(test_file);
    printf(result == 1 ? "Test passed: kdk_video_add_file() succeeded.\n" : "Test failed: kdk_video_add_file() failed.\n");
}

void test_kdk_video_current_file() 
{
    char* file = kdk_video_current_file();
    if (file != NULL) 
    {
        printf("Test passed: kdk_video_current_file() returned: %s\n", file);
    } 
    else 
    {
        printf("Test failed: kdk_video_current_file() returned NULL.\n");
    }
}

void test_kdk_video_exit() 
{
    int result = kdk_video_exit();
    printf(result == 1 ? "Test passed: kdk_video_exit() succeeded.\n" : "Test failed: kdk_video_exit() failed.\n");
}

void test_kdk_video_full_screen() 
{
    int result = kdk_video_full_screen();
    printf(result == 1 ? "Test passed: kdk_video_full_screen() succeeded.\n" : "Test failed: kdk_video_full_screen() failed.\n");
}

void test_kdk_video_kv_play_pause() 
{
    int result = kdk_video_kv_play_pause();
    printf(result == 1 ? "Test passed: kdk_video_kv_play_pause() succeeded.\n" : "Test failed: kdk_video_kv_play_pause() failed.\n");
}

void test_kdk_video_loop_mode() 
{
    int result = kdk_video_loop_mode();
    printf(result == 1 ? "Test passed: kdk_video_loop_mode() succeeded.\n" : "Test failed: kdk_video_loop_mode() failed.\n");
}

void test_kdk_video_play_state() 
{
    int state = kdk_video_play_state();
    printf(state != -1 ? "Test passed: kdk_video_play_state() returned: %d\n" : "Test failed: kdk_video_play_state() returned -1.\n", state);
}

void test_kdk_video_previous() 
{
    int result = kdk_video_previous();
    printf(result == 1 ? "Test passed: kdk_video_previous() succeeded.\n" : "Test failed: kdk_video_previous() failed.\n");
}

void test_kdk_video_volume_down() 
{
    int result = kdk_video_volume_down();
    printf(result == 1 ? "Test passed: kdk_video_volume_down() succeeded.\n" : "Test failed: kdk_video_volume_down() failed.\n");
}

void test_kdk_video_volume_up() 
{
    int result = kdk_video_volume_up();
    printf(result == 1 ? "Test passed: kdk_video_volume_up() succeeded.\n" : "Test failed: kdk_video_volume_up() failed.\n");
}

void test_kdk_video_get_play_back_state() 
{
    char* state = kdk_video_get_play_back_state();
    if (state != NULL) 
    {
        printf("Test passed: kdk_video_get_play_back_state() returned: %s\n", state);
    } 
    else 
    {
        printf("Test failed: kdk_video_get_play_back_state() returned NULL.\n");
    }
}

void test_kdk_video_pause() 
{
    int result = kdk_video_pause();
    printf(result == 1 ? "Test passed: kdk_video_pause() succeeded.\n" : "Test failed: kdk_video_pause() failed.\n");
}

void test_kdk_video_play_pause() 
{
    int result = kdk_video_play_pause();
    printf(result == 1 ? "Test passed: kdk_video_play_pause() succeeded.\n" : "Test failed: kdk_video_play_pause() failed.\n");
}

void test_kdk_video_stop() 
{
    int result = kdk_video_stop();
    printf(result == 1 ? "Test passed: kdk_video_stop() succeeded.\n" : "Test failed: kdk_video_stop() failed.\n");
}

void test_kdk_video_play() 
{
    int result = kdk_video_play();
    printf(result == 1 ? "Test passed: kdk_video_play() succeeded.\n" : "Test failed: kdk_video_play() failed.\n");
}

void test_kdk_video_seek() 
{
    int result = kdk_video_seek(10000000); // 假设快进10秒（10000000微秒）
    printf(result == 1 ? "Test passed: kdk_video_seek() succeeded.\n" : "Test failed: kdk_video_seek() failed.\n");
}

void test_kdk_video_set_position() 
{
    int result = kdk_video_set_position("", 5000000); // 设置为5秒（5000000微秒）
    printf(result == 1 ? "Test passed: kdk_video_set_position() succeeded.\n" : "Test failed: kdk_video_set_position() failed.\n");
}

void test_kdk_video_open_uri() 
{
    const char* test_uri = "/home/wjb/影音/llllllll.mp4"; // 替换为实际的测试文件路径
    int result = kdk_video_open_uri(test_uri);
    printf(result == 1 ? "Test passed: kdk_video_open_uri() succeeded.\n" : "Test failed: kdk_video_open_uri() failed.\n");
}

int main() 
{
    // 运行所有测试
    test_open_video();
    // test_kdk_video_next();
    // test_kdk_video_add_file();
    // test_kdk_video_current_file();
    // test_kdk_video_exit();
    // test_kdk_video_full_screen();
    // test_kdk_video_kv_play_pause();
    // test_kdk_video_loop_mode();
    // test_kdk_video_play_state();
    // test_kdk_video_previous();
    // test_kdk_video_volume_down();
    // test_kdk_video_volume_up();
    // test_kdk_video_get_play_back_state();
    // test_kdk_video_pause();
    // test_kdk_video_play_pause();
    // test_kdk_video_stop();
    // test_kdk_video_play();
    // test_kdk_video_seek();
    // test_kdk_video_set_position();
    // test_kdk_video_open_uri();

    return 0;
}