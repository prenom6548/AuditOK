/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

typedef enum : int {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_END
} LogLevel;

extern LogLevel LOG_LEVEL;
extern void setLogLevel(LogLevel logLevel);

#define PRINT_FATAL(fmt, ...)                               \
    do {                                                    \
        if (LOG_LEVEL >= LOG_LEVEL_FATAL) {                 \
            fprintf(stderr, "[FATAL] " fmt, ##__VA_ARGS__); \
        }                                                   \
    } while (0)

#define PRINT_ERROR(fmt, ...)                               \
    do {                                                    \
        if (LOG_LEVEL >= LOG_LEVEL_ERROR) {                 \
            fprintf(stderr, "[ERROR] " fmt, ##__VA_ARGS__); \
        }                                                   \
    } while (0)

#define PRINT_WARN(fmt, ...)                               \
    do {                                                   \
        if (LOG_LEVEL >= LOG_LEVEL_WARN) {                 \
            fprintf(stdout, "[WARN] " fmt, ##__VA_ARGS__); \
            fflush(stdout);                                \
        }                                                  \
    } while (0)

#define PRINT_INFO(fmt, ...)                               \
    do {                                                   \
        if (LOG_LEVEL >= LOG_LEVEL_INFO) {                 \
            fprintf(stdout, "[INFO] " fmt, ##__VA_ARGS__); \
            fflush(stdout);                                \
        }                                                  \
    } while (0)

#define PRINT_DEBUG(fmt, ...)                               \
    do {                                                    \
        if (LOG_LEVEL >= LOG_LEVEL_DEBUG) {                 \
            fprintf(stdout, "[DEBUG] " fmt, ##__VA_ARGS__); \
            fflush(stdout);                                 \
        }                                                   \
    } while (0)
