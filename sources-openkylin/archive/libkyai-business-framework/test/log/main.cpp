/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

// #include <ctime>
// #include <sstream>
// #include <thread>

// #include "log.h"

// void testLog01() {
//     PRINT_ERROR("test error: %d\n", LOG_LEVEL);
//     PRINT_ERROR("test error\n");
//     PRINT_INFO("test info: %d\n", LOG_LEVEL);
//     PRINT_DEBUG("test debug: %d\n", LOG_LEVEL);

//     setLogLevel(LOG_LEVEL_DEBUG);
//     PRINT_DEBUG("test debug: %d\n", LOG_LEVEL);

//     time_t now;
//     time(&now);
//     tm *p = localtime(&now);
//     PRINT_DEBUG("%04d-%02d-%02d %02d:%02d:%02d\n", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min,
//                 p->tm_sec);
// }

// void testLog02() {
//     PRINT_FATAL("hello %s\n", "world");
//     PRINT_FATAL("hello world");
// }

// int main() {
//     // testLog01();
//     setvbuf(stdout, NULL, _IOLBF, 0);  // 设置为行缓冲模式

//     auto pid = std::this_thread::get_id();
//     std::stringstream sin;
//     sin << pid;

//     fprintf(stdout, "[WARN] pid: %s\n", sin.str().c_str());

//     return 0;
// }
