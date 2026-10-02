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

class KMTestBuilder : public ::testing::Test
{
public:
    KMTestBuilder() = default;
    ~KMTestBuilder() = default;

    /**
     * @brief : SetUp会在每个TEST_F之前执行
     */ 
    void SetUp() override;

    /**
     * @brief : TearDown会在每个TEST_F之后执行
     */ 
    void TearDown() override;

    // build-init
    int init(std::vector<std::string>& args);
    void initCache();
    void initDepend();

    // build
    int build(std::vector<std::string>& args);

    // build-finish
    int finish(std::vector<std::string>& args);
    void prepareFinishData();

    // build-export
    int exports(std::vector<std::string>& args);

protected:
    std::string m_directory;
    std::string m_stateDir;
    std::string m_cacheDir;
};

