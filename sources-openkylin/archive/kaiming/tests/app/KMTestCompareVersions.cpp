/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */
#include <map>

#include "KMTestCompareVersions.h"
#include "app/compare-versions/KMCompareVersions.h"
#include "common/KMException.h"

int KMTestCompareVersions::compareVersions(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMCompareVersions comparer;
    return comparer.dispose(argv.size(), argv.data());
}


TEST_F(KMTestCompareVersions, compareVersions)
{
    std::vector<std::string> args;
    args.push_back("compare-versions");
    args.push_back("1.0.0");
    args.push_back("<");
    args.push_back("1.0.1");
    EXPECT_EQ(0u, compareVersions(args));

    args.clear();
    args.push_back("compare-versions");
    args.push_back("1.0.0");
    args.push_back("<=");
    args.push_back("1.0.1");
    EXPECT_EQ(0u, compareVersions(args));

    args.clear();
    args.push_back("compare-versions");
    args.push_back("1.0.0");
    args.push_back("==");
    args.push_back("1.0.0");
    EXPECT_EQ(0u, compareVersions(args));

    args.clear();
    args.push_back("compare-versions");
    args.push_back("1.0.0");
    args.push_back("!=");
    args.push_back("1.0.1");
    EXPECT_EQ(0u, compareVersions(args));

    args.clear();
    args.push_back("compare-versions");
    args.push_back("1.0.1");
    args.push_back(">=");
    args.push_back("1.0.0");
    EXPECT_EQ(0u, compareVersions(args));

    args.clear();
    args.push_back("compare-versions");
    args.push_back("1.0.1");
    args.push_back(">");
    args.push_back("1.0.0");
    EXPECT_EQ(0u, compareVersions(args));
}

TEST_F(KMTestCompareVersions, compareVersions_throw)
{
    std::vector<std::string> args;
    try
    {
        args.push_back("compare-versions");
        compareVersions(args);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    try
    {
        args.clear();
        args.push_back("compare-versions");
        args.push_back("1.0.0");
        args.push_back("~=");
        args.push_back("1.0.1");
        compareVersions(args);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    args.clear();
    args.push_back("compare-versions");
    args.push_back("1.0.0");
    args.push_back(">");
    args.push_back("1.0.1");
    EXPECT_EQ(1u, compareVersions(args));
}

