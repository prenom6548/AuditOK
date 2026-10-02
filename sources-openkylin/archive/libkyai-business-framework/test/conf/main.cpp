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

// #include <iostream>

// #include "conf.h"

// int main() {
//     std::string path = "/home/hezhe/code/kylin-ai-business-framework/test/conf/";
//     std::string filename = "conf.json";
//     bool ret = Configuration::parseConfFile(path, filename);
//     if (!ret) {
//         std::cout << "parseConfFile failed." << std::endl;
//         return -1;
//     }

//     std::string value = Configuration::getString("address");
//     std::cout << value << std::endl;

//     std::vector<std::string> allTags{"hello", "world", "hehe"};
//     nlohmann::json output = allTags;
//     std::cout << output << std::endl;

//     return 0;
// }
