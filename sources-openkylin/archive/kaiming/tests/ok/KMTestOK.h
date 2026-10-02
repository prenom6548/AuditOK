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

class KMTestOK : public ::testing::Test
{
public:
    KMTestOK() = default;
    ~KMTestOK() = default;

    /**
     * @brief : SetUp会在每个TEST_F之前执行
     */ 
    void SetUp() override;
    void prepareData();
    void findOk();

    /**
     * @brief : TearDown会在每个TEST_F之后执行
     */ 
    void TearDown() override;

    int cmd(std::vector<std::string>& args);

    int mountOAB(std::vector<std::string>& args);

    int extractOAB(std::vector<std::string>& args);

    int printDataDir(std::vector<std::string>& args);

    int printOABMeta(std::vector<std::string>& args);

    int repackageOAB(std::vector<std::string>& args);

protected:
    std::string m_directory;
    std::string m_stateDir;
    std::string m_ok;
};

