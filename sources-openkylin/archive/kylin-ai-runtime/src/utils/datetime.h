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

#ifndef UTILS_DATETIME_H
#define UTILS_DATETIME_H

#include <string>

class DateTime {
public:
    DateTime();
    int year() const { return year_; }
    int month() const { return month_; }
    int day() const { return day_; }
    int hour() const { return hour_; }
    int min() const { return min_; }
    int sec() const { return sec_; }
    int ms() const { return ms_; }
    int us() const { return us_; }
    std::string weekday() const;

private:
    int year_{};
    int month_{};
    int day_{};
    int hour_{};
    int min_{};
    int sec_{};
    int ms_{};
    int us_{};
    int weekday_{};  // 0-6, 其中0代表周日，1代表周一，以此类推
};

#endif  // UTILS_DATETIME_H