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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <stdio.h>

#include <iostream>
#include <memory>

#include "pbkdf2.h"

TEST(Pbkdf2Test, SamePasswordProducesDifferentKeys) {
    auto pbkdf2 = std::make_unique<Pbkdf2>();

    std::string password = "ab";
    auto result1 = pbkdf2->generate(password);
    auto result2 = pbkdf2->generate(password);

    ASSERT_EQ(result1.success, result2.success);
    ASSERT_TRUE(result1.success);
    ASSERT_EQ(result1.key.size(), result2.key.size());
    bool same = true;
    for (auto i = 0; i < result1.key.size(); i++) {
        if (result1.key[i] != result2.key[i]) {
            same = false;
            break;
        }
    }

    EXPECT_FALSE(same);
}
