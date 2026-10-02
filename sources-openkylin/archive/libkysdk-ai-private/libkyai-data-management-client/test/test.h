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

#pragma once

#include <gtest/gtest.h>
#include <kylin-ai/business/data-management/datamanagement.h>

class DataManagementTest : public testing::Test {
protected:
    // 每个案例之前执行
    void SetUp() override {
        std::cout << "SetUp..." << std::endl;
        session = data_management_create_session();
        result = nullptr;
    }
    // 每个案例之后执行
    void TearDown() override {
        std::cout << "TearDown..." << std::endl;
        data_management_destroy_session(session);
        if (result != nullptr) {
            std::cout << "Result is " << result << std::endl;
            data_management_destroy_result(result);
        }
    }

public:
    DataManagementSession* session;
    char* result;
};
