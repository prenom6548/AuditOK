/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "logger.h"

#include <sys/time.h>

#include <sstream>

void Logger::printLn() { std::cout << '\n'; }

void Logger::printErrorLn() { std::cerr << '\n'; }

std::string Logger::location(const char* fileName, int line, const char* funcName) {
    std::ostringstream oss;
    oss << '[' << fileName << ':' << line << ':' << funcName << ']';
    return oss.str();
}

std::string Logger::currentTime() {
    struct timeval tv{};
    struct tm* tm = nullptr;
    gettimeofday(&tv, nullptr);
    time_t tt = tv.tv_sec;
    tm = localtime(&tt);
    char buffer[32]{};
#ifdef RUN_IN_DEBUG
    sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d.%03ld]", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 1000);
#else
    sprintf(buffer, "[%ld]", tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
    return buffer;
}

Logger::Logger() {
#ifdef RUN_IN_DEBUG
    level_ = LOG_LEVEL_DEBUG;
#endif
}
