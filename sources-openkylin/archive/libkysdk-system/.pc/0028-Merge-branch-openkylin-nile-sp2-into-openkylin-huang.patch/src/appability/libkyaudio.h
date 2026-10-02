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

#ifndef LIBKYAUDIO_H
#define LIBKYAUDIO_H

/**
 * @file        libkyaudio.h
 * @brief       应用能力模块
 * @author      wangjinbao (wangjinbao@kylinos.cn)
 * @version     0.1
 * @date        2024-10-28
 * @copyright   Copyright (c) 2025
 * @defgroup    libkysdk-appability
 * @since       3.0.1.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 打开音频播放器
 *
 * @return int 成功返回 1，失败返回 -1
 */
extern int open_audio();

/**
 * @brief 退出音频播放器
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_exit();

/**
 * @brief 全屏音频播放器
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_full_screen();

/**
 * @brief 下一曲
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_next();

/**
 * @brief 打开指定路径下的mp3文件
 *
 * @param uri 指定的mp3文件路径
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_open_uri(const char* uri);

/**
 * @brief 暂停音乐
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_pause();

/**
 * @brief 播放音乐
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_play();

/**
 * @brief 暂停/播放音乐
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_play_pause();

/**
 * @brief 上一曲
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_previous();

/**
 * @brief 快进/快退
 *
 * @param offset 微秒数
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_seek(int offset);

/**
 * @brief 将音乐的进度条设置在指定位置
 *
 * @param trackid 轨道标识符。当此参数为空时，表示当前播放的曲目
 * @param position 微秒数
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_set_position(const char* trackid, int position);

/**
 * @brief 停止播放音乐
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_stop();

/**
 * @brief 降低音量
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_volume_down();

/**
 * @brief 提高音量
 *
 * @return int 成功返回1，失败返回-1
 */
extern int kdk_audio_volume_up();

/**
 * @brief 获取当前歌曲状态
 *
 * @return char 成功返回播放状态，失败返回NULL
 */
extern char* kdk_audio_get_state();

#ifdef __cplusplus
}
#endif

#endif // LIBKYSENSOR_H


