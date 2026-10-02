/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <vector>
#include <string>
#include <gtest/gtest.h>

class KMTestCompareVersions : public ::testing::Test
{
public:
    KMTestCompareVersions() = default;
    ~KMTestCompareVersions() = default;

    int compareVersions(std::vector<std::string>& args);
};

