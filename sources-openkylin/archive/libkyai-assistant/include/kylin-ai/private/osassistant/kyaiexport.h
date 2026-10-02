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

#ifndef __KYLIN_AI_PRIVATE_EXPORT_H__
#define __KYLIN_AI_PRIVATE_EXPORT_H__

// 定义导出宏
#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef KYAI_ASSISTANT_BUILDING_LIB
        #ifdef __GNUC__
            #define KYAI_ASSISTANT_EXPORT __attribute__ ((dllexport))
        #else
            #define KYAI_ASSISTANT_EXPORT __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define KYAI_ASSISTANT_EXPORT __attribute__ ((dllimport))
        #else
            #define KYAI_ASSISTANT_EXPORT __declspec(dllimport)
        #endif
    #endif
#else
    #if __GNUC__ >= 4
        #define KYAI_ASSISTANT_EXPORT __attribute__ ((visibility ("default")))
    #else
        #define KYAI_ASSISTANT_EXPORT
    #endif
#endif

#endif // __KYLIN_AI_PRIVATE_EXPORT_H__