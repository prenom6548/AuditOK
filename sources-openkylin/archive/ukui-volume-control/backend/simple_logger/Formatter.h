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


#ifndef FORMATTER_H
#define FORMATTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <type_traits>

#ifdef HAS_STD_FORMAT
#include <format>
#include <concepts>
#else
#include <string>
#include <sstream>
#endif

namespace simple_logger
{

#ifdef HAS_STD_FORMAT

#define FORMAT std::format

    template <typename T>
    concept EnumType = std::is_enum_v<T>;

    template <EnumType EnumValue, typename CharType>
    struct std::formatter<EnumValue, CharType> : std::formatter<int, CharType>
    {
        template <typename FormatContext>
        typename FormatContext::iterator format(const EnumValue& v, FormatContext& formatContext)
        {
            typename FormatContext::iterator itr = std::formatter<int, CharType>().format(static_cast<int>(v), formatContext);
            return itr;
        }
    };


#else

#define FORMAT simple_logger::format

    void FormatExpand(std::stringstream& s, const char* fmt);

    template<typename T, typename = typename std::enable_if<std::is_enum_v<T>>::type>
    inline std::stringstream& operator<<(std::stringstream& ss, T& value)
    {
        ss << static_cast<int>(value);
        return ss;
    }

    template<typename T, typename... Args>
    void FormatExpand(std::stringstream& ss, const char* fmt, T& value, Args... args)
    {
        while (*fmt) {
            // 处理参数打印
            if (*fmt == '{' && *(++fmt) == '}') {
                ss << value;
                FormatExpand(ss, ++fmt, args...);
                return;
            }

            ss << *fmt++;
        }

#ifndef NDEBUG
        // 调试版本抛出异常（如果异常已启用），否则终止进程
        #if defined(__EXCEPTIONS)
        throw std::logic_error("Invalid formatting: too much arguments are provided to format");
        #else
        std::terminate();
        #endif
#endif
    }

    template<typename ... Args>
    std::string format(const char* fmt, Args ... args)
    {
        std::stringstream ss;
        FormatExpand(ss, fmt, args...);
        return ss.str();
    }

#endif
}
#endif // !FORMATTER_H
