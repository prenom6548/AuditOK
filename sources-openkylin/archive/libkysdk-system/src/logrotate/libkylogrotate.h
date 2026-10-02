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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

/**
 * @file libkylogrotate.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 日志转储
 * @version 0.1
 * @date 2025-07-04
 * 
 * @copyright Copyright (c) 2021
 * @defgroup libkysdk-logrotate
 * @{
 * @since 3.0.1.0
 */

#include <stdbool.h>

#define LOGROTATE_OK                 0      // 成功
#define LOGROTATE_INVALID_PATH      -1      // 路径不存在/不可读
#define LOGROTATE_PERM_DENIED       -2      // 无权限写入/etc/logrotate.d
#define LOGROTATE_INVALID_CONFIG    -3      // 如rotate_count为负数
#define LOGROTATE_WRITE_FAIL        -4      // 文件写入失败

// 日志轮转频率枚举
typedef enum _KDK_LOGROTATE_FREQUENCY
{
    KDK_DAILY = 0x01,   // 天
    KDK_WEEKLY = 0x02,  // 周
    KDK_MONTHLY = 0x04, // 月
    KDK_YEARLY = 0x08,  // 年
    KDK_NONE = 0x00,
} KdkLogrotateFrequency;

// 日志轮转大小单位枚举
typedef enum _KDK_LOGROTATE_UNIT
{
    KDK_KB,
    KDK_MB,
    KDK_GB
} KdkLogrotateUnit;

// 压缩模式枚举
typedef enum _KDK_COMPRESS_MODE
{
    KDK_COMPRESS_BZIP2,
    KDK_COMPRESS_XZ
} KdkCompressMode;

typedef struct
{
    // 日志轮转
    int frequency;                   // 日志切割的频率，取值KdkLogrotateFrequency
    int rotate;                      // 保留的日志数量，设为0时表示不保留日志

    bool by_size;                    // 根据大小切割
    int max_size;                    // 触发切割的大小
    KdkLogrotateUnit unit;           // 触发切割的大小的单位

    char extension[16];              // 压缩后文件后缀

    // 压缩与归档
    bool compress;                  // 是否压缩
    bool delaycompress;             // 延迟压缩，仅压缩前一次轮转的日志（需与 compress 配合使用）

    KdkCompressMode compress_ext;   // 压缩格式
    char compresscmd[32];           // 压缩命令，确保需要符合compress_ext且命令无误

    // 日期后缀
    bool dateext;                   // 为旧日志添加日期后缀
    char dateformat[32];            // 自定义日期格式（如 %Y%m%d-%s 表示日期+时间戳）

    // 转存模式
    bool copytruncate;              // 开启后旧日志复制到转存文件，并清空旧日志。与creat互斥

    // 文件处理
    bool notifempty;                // 空文件不切割
    bool missingok;                 // 日志不存在时忽略，关闭后会报错

    // 创建文件， 以下成员copytruncate为false时有效
    // 将就日志重命名为转存文件，并创建空日志文件。
    // 需要通过信号等方式通知对应用重新打开日志文件，否则对应用将无法写入到日志。
    bool nocreate;                  // 不创建新日志文件，如应用程序会自动创建日志文件可设置为true
    char create_mode[8];            // 新日志权限，如 "0644"
    char create_owner[32];          // 属主，如 "root:root"

    // 高级设置
    char postrotate[256];           // 切割后执行的命令
    bool sharedscripts;             // 当处理多个日志时，postrotate 脚本仅执行一次（默认每个日志触发一次）
} KdkLogrotateConfig;

/**
 * @brief 初始化logrotate配置，将配置初始化为默认值,例:
 * /var/log/nginx/*.log {
 *    daily
 *    rotate 7
 *    compress
 *    missingok
 *    notifempty
 *    copytruncate
 *}
 *
 * @param config 申请的KdkLogrotateConfig
 */
void kdk_logrotate_config_init(KdkLogrotateConfig *config);

/**
 * @brief 为目标文件生成logrotate配置文件，并添加到logrotate.d目录下
 * 
 * @param config_path 日志文件路径列表，以NULL为结尾的字符串数组
 * @param config logroate配置
 * @return int  0: 成功，其他: 错误码
 */
int kdk_logrotate_config_create(const char **config_path, const KdkLogrotateConfig *config);
