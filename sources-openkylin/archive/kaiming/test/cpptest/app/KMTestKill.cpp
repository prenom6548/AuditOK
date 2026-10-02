/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMKill.h"
#include "KMRunner.h"
#include "KMInstalledAppQuery.h"

#include <gtest/gtest.h>

#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::seconds
#include <vector>
#include <string>

#define private public

class KMTestKill
{
public:
    KMTestKill() = default;
    ~KMTestKill() = default;

public:
    KMKill m_killer;
    KMRunner m_runner;
};

TEST(KMTestKill, test_kill)
{
    KMTestKill testKiller;
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.kind("app").query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";
    std::string appid = refs.front().id;
    const char *argv[]  = {"run",  appid.c_str()};
    const char *argv1[] = {"kill", appid.c_str()};

    std::thread runThread([&]() 
        {
            testKiller.m_runner.dispose(2, const_cast<char**>(argv));
        }
    ); 
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(testKiller.m_killer.dispose(2, const_cast<char**>(argv1)), 1);
    runThread.join();
}