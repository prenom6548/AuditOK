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

// #include <gtest/gtest.h>

// #include "utils.h"
// #include <nlohmann/json.hpp>

// TEST(Utils, UUID) {
//     std::string UUID_1 = Utils::generateUUID();
//     std::string UUID_2 = Utils::generateUUID();
//     EXPECT_NE(UUID_1, UUID_2);

//     Json::Value root;
//     root["err_no"] = 0;
//     root["err_msg"] = 0;
//     root["data"]["test"] = "hello";
//     root["data"]["haha"] = "world";
//     std::unordered_map<std::string, Json::Value> ret = Utils::jsonToMap(root);

//     EXPECT_STREQ(ret["test"].asCString(), "hello");
//     // EXPECT_EQ(SQLITE_VERSION_NUMBER, SQLite::VERSION_NUMBER);
// }

// TEST(Utils, mapToJson) {
//     std::unordered_map<std::string, Json::Value> root = {{"0", "hello"}, {"1", "world"}};

//     Json::Value ret = Utils::mapToJson(root);
//     std::cout << ret.toStyledString() << std::endl;

//     EXPECT_EQ(ret.type(), Json::ValueType::arrayValue);
//     // EXPECT_STREQ(ret[0].asString(), "hello");
//     // EXPECT_EQ(SQLITE_VERSION_NUMBER, SQLite::VERSION_NUMBER);
// }
