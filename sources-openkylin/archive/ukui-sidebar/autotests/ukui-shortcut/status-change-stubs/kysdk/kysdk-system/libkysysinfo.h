/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AUTOTESTS_STATUS_CHANGE_STUB_LIBKYSYSINFO_H
#define AUTOTESTS_STATUS_CHANGE_STUB_LIBKYSYSINFO_H

#include <cstdlib>
#include <cstring>

inline const char *&kdk_test_system_category_storage()
{
    static const char *category = "PC";
    return category;
}

inline void kdk_test_set_system_category(const char *category)
{
    kdk_test_system_category_storage() = category;
}

inline char *kdk_system_get_systemCategory()
{
    return strdup(kdk_test_system_category_storage());
}

#endif
