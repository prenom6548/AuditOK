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

#include <filesystem>

#include "businessFrameworkTest.h"
#include "scene.h"

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(BusinessFrameworkTest, conf_Success_Basic) {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/";
    bool success = Configuration::parseConfFile(filePath.string(), "conf.json");
    EXPECT_TRUE(success);

    auto value = Configuration::getAllValues();
    EXPECT_STRNE(value.dump().c_str(), "");

    std::string serviceName = Configuration::getString("service_name");
    EXPECT_STREQ(serviceName.c_str(), "test");

    std::string getServiceName = Configuration::getServiceName();
    EXPECT_STREQ(getServiceName.c_str(), serviceName.c_str());

    int logLevel = Configuration::getInt("log_level");
    EXPECT_EQ(logLevel, 4);

    std::vector<int> intArray = Configuration::getIntArray("int_array", std::vector<int>());
    EXPECT_EQ(intArray.size(), 3);

    std::vector<std::string> strArray = Configuration::getStringArray("str_array", std::vector<std::string>());
    EXPECT_EQ(strArray.size(), 3);

    LogLevel log = Configuration::getLogLevel();
    EXPECT_NE((int)log, -1);
}

TEST_F(BusinessFrameworkTest, conf_Error_ErrorParserPath) {
    bool success = Configuration::parseConfFile("", "");
    EXPECT_FALSE(success);
}

TEST_F(BusinessFrameworkTest, conf_Error_ErrorJsonFile) {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/";
    bool success = Configuration::parseConfFile(filePath.string(), "error.json");
    EXPECT_FALSE(success);
}

TEST_F(BusinessFrameworkTest, conf_Error_GetErrorKey) {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/";
    bool success = Configuration::parseConfFile(filePath.string(), "conf.json");
    EXPECT_TRUE(success);

    std::vector<int> intArray = Configuration::getIntArray("ket", std::vector<int>());
    EXPECT_EQ(intArray.size(), 0);

    std::vector<std::string> strArray = Configuration::getStringArray("key", std::vector<std::string>());
    EXPECT_EQ(strArray.size(), 0);

    std::vector<int> intArray_1 = Configuration::getIntArray("str_array", std::vector<int>());
    EXPECT_EQ(intArray_1.size(), 0);

    std::vector<std::string> strArray_1 = Configuration::getStringArray("int_array", std::vector<std::string>());
    EXPECT_EQ(strArray_1.size(), 0);
}