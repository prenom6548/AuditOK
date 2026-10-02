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

#ifndef UTILS_PLATFORM_H
#define UTILS_PLATFORM_H

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
#define ARCH_AMD64
#elif defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
#define ARCH_ARM64
#else
#define ARCH_OTHER
#endif

#endif  // UTILS_PLATFORM_H