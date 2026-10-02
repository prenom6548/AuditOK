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
 * Authors: JinBao Wang <wangjinbao@kylinos.cn>
 *
 */

#ifndef LIBKYVIDEO_H
#define LIBKYVIDEO_H

/**
 * @file        libkyvideo.h
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
 * @brief 打开视频播放器
 *
 * @return int 成功返回 1，失败返回 -1
 */
extern int open_video();

/**
 * @brief 播放下一个视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_next();

/**
 * @brief 添加视频文件
 *
 * @param trackid 添加的视频文件路径
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_add_file(const char* trackid);

/**
 * @brief 获取当前播放的视频文件路径
 *
 * @return char* 成功返回视频文件路径，失败返回NULL.
 */
extern char* kdk_video_current_file();

/**
 * @brief 退出视频播放器
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_exit();

/**
 * @brief 全屏视频播放器
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_full_screen();

/**
 * @brief 暂停/播放视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_kv_play_pause();

/**
 * @brief 启用视频循环模式
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_loop_mode();

/**
 * @brief 获取视频播放状态
 *
 * @return int 返回播放状态为1，返回暂停状态为0，Dbus服务未正常运行返回-1
 */
extern int kdk_video_play_state();

/**
 * @brief 播放前一个视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_previous();

/**
 * @brief 降低音量
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_volume_down();

/**
 * @brief 提高音量
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_volume_up();

/**
 * @brief 获取当前视频播放状态
 *
 * @return 成功返回播放状态，失败返回NULL
 */
extern char* kdk_video_get_play_back_state();

/**
 * @brief 暂停视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_pause();

/**
 * @brief 暂停/播放视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_play_pause();

/**
 * @brief 停止播放视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_stop();

/**
 * @brief 播放视频
 *
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_play();

/**
 * @brief 快进/快退
 *
 * @param offset 微秒数
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_seek(int offset);

/**
 * @brief 将视频的进度条设置在指定位置
 *
 * @param trackid 轨道标识符。当此参数为空时，表示当前播放的曲目
 * @param position 微秒数
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_set_position(const char* trackid, int position);

/**
 * @brief 打开指定路径下的mp4文件
 *
 * @param uri 指定的mp4文件路径
 * @return int 成功返回1，只能说明SDK接口调用成功，不能保证应用都能有动作响应，失败返回-1.
 */
extern int kdk_video_open_uri(const char* uri);

#ifdef __cplusplus
}
#endif

#endif // LIBKYSENSOR_H


