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

#include <filesystem>
#include <iostream>

#include "imageService.h"

class ImageServiceTest : public testing::Test {
public:
    // 所有案例执行之前
    static void SetUpTestSuite();
    // 所有案例执行之后
    static void TearDownTestSuite();

protected:
    // 每个案例之前执行
    void SetUp() override { std::cout << "ImageServiceTest SetUp..." << std::endl; }
    // 每个案例之后执行
    void TearDown() override { std::cout << "ImageServiceTest TearDown..." << std::endl; }

private:
};

void ImageServiceTest::SetUpTestSuite() { std::cout << "ImageServiceTest SetUpTestSuite..." << std::endl; }

void ImageServiceTest::TearDownTestSuite() { std::cout << "ImageServiceTest TearDownTestSuite..." << std::endl; }

// TEST_F(ImageServiceTest, test01) {
//     namespace fs = std::filesystem;
//     fs::path dir = fs::path(__FILE__).parent_path().parent_path();
//     fs::path path = dir / "20250626.jpg";

//     ImageService imageService;
//     imageService.embedding({path.string()});
// }
// TEST_F(ImageServiceTest, test02) {
//     namespace fs = std::filesystem;
//     fs::path dir = fs::path(__FILE__).parent_path().parent_path();
//     fs::path path = dir / "20250626.jpg";

//     ImageService imageService;
//     imageService.embedding({path.string(), path.string(), path.string(), path.string(), path.string(), path.string()});
// }

// TEST_F(ImageServiceTest, test03) {
//     ImageService imageService;
//     imageService.embedding({});
// }