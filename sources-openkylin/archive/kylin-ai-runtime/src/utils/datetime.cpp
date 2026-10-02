/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "datetime.h"

#include <sys/time.h>

#include <ctime>

DateTime::DateTime() {
    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    struct tm* tm = localtime(&tv.tv_sec);
    year_ = tm->tm_year + 1900;
    month_ = tm->tm_mon + 1;
    day_ = tm->tm_mday;
    hour_ = tm->tm_hour;
    min_ = tm->tm_min;
    sec_ = tm->tm_sec;
    ms_ = int(tv.tv_usec / 1000);
    us_ = int(tv.tv_usec % 1000);
    weekday_ = tm->tm_wday;
}

std::string DateTime::weekday() const {
    switch (weekday_) {
        case 0:
        case 7:
            return "日";
        case 1:
            return "一";
        case 2:
            return "二";
        case 3:
            return "三";
        case 4:
            return "四";
        case 5:
            return "五";
        case 6:
            return "六";
        default:
            return "未知";
    }
}
