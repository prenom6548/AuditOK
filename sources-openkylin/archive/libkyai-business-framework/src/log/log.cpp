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

#include "log.h"

LogLevel LOG_LEVEL = LOG_LEVEL_INFO;

void setLogLevel(LogLevel logLevel) {
    if (logLevel < LOG_LEVEL_FATAL || logLevel >= LOG_LEVEL_END) {
        PRINT_WARN("set log level out of range. %d\n", logLevel);
        return;
    }
    LOG_LEVEL = logLevel;
    PRINT_DEBUG("set log level: %d\n", LOG_LEVEL);
}
