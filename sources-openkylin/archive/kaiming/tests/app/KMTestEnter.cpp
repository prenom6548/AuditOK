/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <thread>
#include <chrono>
#include <gtest/gtest.h>

#include "app/enter/KMEnter.h"
#include "app/run/KMRunner.h"
#include "app/kill/KMKill.h"
#include "common/KMInstalledAppQuery.h"

class KMTestEnter
{
public:
    KMTestEnter() = default;
    ~KMTestEnter() = default;

public:
    KMEnter m_enter;
    KMRunner m_runner;
    KMKill m_kill;
};

TEST(KMTestEnter, test_enter)
{
    KMTestEnter testenter;
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.kind("app").query(KMInstalledAppQuery::All);
    if (refs.empty())
    {
        GTEST_SKIP() << "No installed apps found";
    }
    std::string appid = refs.front().id;
    std::cout << "test appid" << appid << std::endl;

    const char *argv[]  = {"run",  appid.c_str()};
    const char *argv1[] = {"enter",   appid.c_str(),"ls"};
    const char *argv2[] = {"kill", appid.c_str()};

    std::thread runThread([&]() 
        {
            testenter.m_runner.dispose(2, const_cast<char**>(argv));
        }
    ); 

    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_GE(testenter.m_enter.dispose(3,const_cast<char**>(argv1)), 0);   
    EXPECT_GE(testenter.m_kill.dispose(2,const_cast<char**>(argv2)), 0);
    runThread.join();
}
