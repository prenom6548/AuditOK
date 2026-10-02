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

#ifndef AUTOTESTS_STUB_LIBKYDATE_H
#define AUTOTESTS_STUB_LIBKYDATE_H

#ifdef __cplusplus
extern "C" {
#endif

char *kdk_system_get_shortformat(void);
void kdk_test_set_shortformat(const char *format);

#ifdef __cplusplus
}
#endif

#endif
