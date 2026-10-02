/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef DATA_TIME_H
#define DATA_TIME_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <chrono>
#include <cerrno>

#ifndef errno_t
typedef int errno_t;
#endif

namespace simple_logger
{
    using Now = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

    time_t GetTime();

    Now GetCurrentTime();

    errno_t GetUtcTm(tm* utcTm);

    errno_t GetLocalTm(tm* localTm);

    std::string GetUtcDateTime();

    std::string GetUtcDate();

    std::string GetUtcTime();

    std::string GetUtcDateTimeWithMilliSecond();

    std::string GetUtcDateTimeWithMilliSecond(const Now& now);

    std::string ToUtcDateTime(const time_t* time);

    std::string ToUtcDate(const time_t* time);

    std::string ToUtcTime(const time_t* time);

    std::string GetLocalDateTime();

    std::string GetLocalDate();

    std::string GetLocalTime();

    std::string GetLocalDateTimeWithMilliSecond();

    std::string GetLocalDateTimeWithMilliSecond(const Now& now);

    std::string GetLocalDateFromUnixTimeStamp(long long timeStamp);

    std::string GetLocalTimeFromUnixTimeStamp(long long timeStamp);

    std::string ToLocalDateTime(const time_t* time);

    std::string ToLocalDate(const time_t* time);

    std::string ToLocalTime(const time_t* time);

    time_t GetTimeFromString(std::string dateTime, std::string fmt = "%04d%02d%02d-%02d:%02d:%02d");
}

#endif // !DATA_TIME_H
