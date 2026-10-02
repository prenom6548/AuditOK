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


#include "DateTime.h"

#include <cstdio>
#include <iostream>
#include <chrono>
#include <cstdarg>

namespace simple_logger
{

    errno_t GetTime(const time_t* timer, struct tm* buf)
    {
#if defined(__linux__)
        gmtime_r(timer, buf);
        return 0;
#else
        return gmtime_s(buf, timer);
#endif
    }

    errno_t GetLocalTime(const time_t* timer, struct tm* buf)
    {
#if defined(__linux__)
        localtime_r(timer, buf);
        return 0;
#else
        return localtime_s(buf, timer);
#endif
    }

    int Sprintf(char* const buffer, size_t const bufferCount, char const* const fmt, ...)
    {
        int ret;
        va_list argList;
        va_start(argList, fmt);

#if defined(__linux__)
        ret = vsprintf(buffer, fmt, argList);
#else
        ret = _vsprintf_s_l(buffer, bufferCount, fmt, nullptr, argList);
#endif
        va_end(argList);

        return ret;
    }

    int StrScanf(char const* const buffer, char const* const fmt, ...)
    {
        int ret;
        va_list argList;
        va_start(argList, fmt);

#if _MSC_VER
    ret = vsscanf_s(buffer, fmt, argList);
#else // defined(__linux__) or defined(__MINGW32__) or defined(__MINGW64__)
    ret = vsscanf(buffer, fmt, argList);
#endif
        va_end(argList);

        return ret;
    }

    time_t GetTime()
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        return std::chrono::system_clock::to_time_t(now);
    }

    errno_t GetUtcTm(tm* t)
    {
        time_t time = GetTime();
        return GetTime(&time, t);
    }

    errno_t GetLocalTm(tm* localTm)
    {
        time_t t = GetTime();
        return GetLocalTime(&t, localTm);
    }

    std::string GetUtcDateTime()
    {
        auto time = GetTime();
        tm localTm = { 0 };
        auto ret = GetTime(&time, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string GetUtcDate()
    {
        auto t = GetTime();
        tm localTm = { 0 };
        auto ret = GetTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        strftime(buff, sizeof(buff), "%Y-%m-%d", &localTm);
        return std::string(buff);
    }

    std::string GetUtcTime()
    {
        auto t = GetTime();
        tm localTm = { 0 };
        auto ret = GetTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        strftime(buff, sizeof(buff), "%H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string GetUtcDateTimeWithMilliSecond()
    {
        return GetUtcDateTimeWithMilliSecond(GetCurrentTime());
    }

    std::string GetUtcDateTimeWithMilliSecond(const Now& now)
    {
        time_t t = std::chrono::system_clock::to_time_t(now);
        int milliSecond = now.time_since_epoch().count() % 1000;

        tm localTm = { 0 };
        auto ret = GetTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        int len = (int)strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &localTm);
        Sprintf(buff + len, sizeof(buff) - len, ".%03u", milliSecond);
        return std::string(buff);
    }

    std::string ToUtcDateTime(const time_t* time)
    {
        char buff[32] = { 0 };
        tm localTm = { 0 };
        auto ret = GetTime(time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string ToUtcDate(const time_t* time)
    {
        char buff[32] = { 0 };
        tm localTm = { 0 };
        auto ret = GetTime(time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%Y-%m-%d", &localTm);
        return std::string(buff);
    }

    std::string ToUtcTime(const time_t* time)
    {
        char buff[32] = { 0 };
        tm localTm = { 0 };
        auto ret = GetTime(time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string GetLocalDateTime()
    {
        auto t = GetTime();
        tm localTm = { 0 };
        auto ret = GetLocalTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string GetLocalDate()
    {
        auto t = GetTime();
        tm localTm = { 0 };
        auto ret = GetLocalTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        strftime(buff, sizeof(buff), "%Y-%m-%d", &localTm);
        return std::string(buff);
    }

    std::string GetLocalTime()
    {
        auto t = GetTime();
        tm localTm = { 0 };
        auto ret = GetLocalTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        strftime(buff, sizeof(buff), "%H:%M:%S", &localTm);
        return std::string(buff);
    }

    Now GetCurrentTime()
    {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    }

    std::string GetLocalDateTimeWithMilliSecond()
    {
        return GetLocalDateTimeWithMilliSecond(GetCurrentTime());
    }

    std::string GetLocalDateTimeWithMilliSecond(const Now& now)
    {
        time_t t = std::chrono::system_clock::to_time_t(now);
        int milliSecond = now.time_since_epoch().count() % 1000;

        tm localTm = { 0 };
        auto ret = GetLocalTime(&t, &localTm);
        if (ret != 0) {
            return "";
        }

        char buff[32] = { 0 };
        int len = (int)strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &localTm);
        Sprintf(buff + len, sizeof(buff) - len, ".%03u", milliSecond);
        return std::string(buff);
    }

    std::string GetLocalDateFromUnixTimeStamp(long long timeStamp)
    {
        time_t time = timeStamp / 1000000000LL;
        static char buff[16] = { 0 };
        tm localTm = { 0 };
        auto ret = GetLocalTime(&time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%Y-%m-%d", &localTm);
        return std::string(buff);
    }

    std::string GetLocalTimeFromUnixTimeStamp(long long timeStamp)
    {
        time_t time = timeStamp / 1000000000LL;
        static char buff[16] = { 0 };
        tm localTm = { 0 };
        auto ret = GetLocalTime(&time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string ToLocalDateTime(const time_t* time)
    {
        char buff[32] = { 0 };
        tm localTm = { 0 };
        auto ret = GetLocalTime(time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &localTm);
        return std::string(buff);
    }

    std::string ToLocalDate(const time_t* time)
    {
        char buff[32] = { 0 };
        tm localTm = { 0 };
        auto ret = GetLocalTime(time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%Y-%m-%d", &localTm);
        return std::string(buff);
    }

    std::string ToLocalTime(const time_t* time)
    {
        char buff[32] = { 0 };
        tm localTm = { 0 };
        auto ret = GetLocalTime(time, &localTm);
        if (ret != 0) {
            return "";
        }

        strftime(buff, sizeof(buff), "%H:%M:%S", &localTm);
        return std::string(buff);
    }

    time_t GetTimeFromString(std::string dateTime, std::string format)
    {
        tm t;
        StrScanf(dateTime.c_str(), format.c_str(), &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
        t.tm_year -= 1900;
        t.tm_mon -= 1;

        return mktime(&t);
    }
}
