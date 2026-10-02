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

#include <gtest/gtest.h>

#include "scene.h"

TEST(Utils, UUID) {
    std::string UUID_1 = Utils::generateUUID();
    std::string UUID_2 = Utils::generateUUID();
    EXPECT_NE(UUID_1, UUID_2);
}

TEST(Utils, camelCaseToKebab) {
    std::string str = "HelloWorld";
    std::string result = Utils::camelCaseToKebab(str);
    EXPECT_STREQ(result.c_str(), "hello-world");
}

TEST(Utils, joinIds) {
    std::unordered_set<int64_t> ids = {1, 2, 3, 4, 5};
    std::string result = Utils::joinIds(ids);
    EXPECT_STRNE(result.c_str(), "");
}