/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::seconds
#include <vector>
#include <string>
#include <gtest/gtest.h>
 
#include "app/ps/KMPS.h"
#include "app/run/KMRunner.h"
#include "app/kill/KMKill.h"
#include "common/KMInstalledAppQuery.h"
 
class KMTestPS
{
public:
    KMPS m_ps;
    KMRunner m_runner;
    KMKill m_kill;
};
 
TEST(KMTestPS, test_ps)
{
    KMTestPS testps;
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.kind("app").query(KMInstalledAppQuery::All);
    if (refs.empty())
    {
        GTEST_SKIP() << "No installed apps found";
    }
    
    std::string appid = refs.front().id;
    const char *argv[]  = {"run",  appid.c_str()};
    const char *argv1[] = {"ps",   appid.c_str()};
    const char *argv2[] = {"kill", appid.c_str()};

    std::thread runThread([&]() 
        {
            testps.m_runner.dispose(2, const_cast<char**>(argv));
        }
    ); 
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(testps.m_ps.dispose(2,const_cast<char**>(argv1)), 0);
    EXPECT_EQ(testps.m_kill.dispose(2, const_cast<char**>(argv2)), 0);

    runThread.join();
}