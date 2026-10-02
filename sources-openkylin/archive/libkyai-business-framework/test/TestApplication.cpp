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

#include <gio/gio.h>

#include <chrono>
#include <filesystem>
#include <thread>

#include "TestclientServer/clientTest.h"
#include "businessFrameworkTest.h"
#include "scene.h"

class MyTest {
public:
    static MyTest& getInstance() {
        static MyTest instance;
        return instance;
    }
    int print() {
        std::cout << "print:MyTest" << std::endl;
        return 0;
    }
};

// 声明外部全局变量 为了拿到事件循环变量
extern GMainLoop* loop;

// ==================== 正常功能测试 (Happy Path) ====================

// TEST_F(BusinessFrameworkTest, Application_Exec_Success_Basic_1) {
//     scene::Application::getInstance().setFunc(std::bind(&MyTest::print, MyTest::getInstance()));

//     namespace fs = std::filesystem;
//     fs::path dir = fs::path(__FILE__).parent_path();
//     fs::path filePath = dir / "conf/";

//     // 测试时，只能通过-d -f 去指定配置文件 因为-s时解析文件的路径默认是本地的
//     char arg1[] = "program_name";
//     char arg2[] = "-d";

//     // 使用 std::vector<char> 来管理字符串内存
//     std::string arg3_str = filePath.string();
//     std::vector<char> arg3(arg3_str.begin(), arg3_str.end());
//     arg3.push_back('\0');  // 确保以null结尾

//     char arg4[] = "-f";
//     char arg5[] = "conf.json";

//     char* argv[] = {arg1, arg2, arg3.data(), arg4, arg5, nullptr};

//     // 3s后退出事件循环
//     std::thread t([]() {
//         std::this_thread::sleep_for(std::chrono::seconds(3));
//         scene::Application::getInstance().notifyAllConnections("test", "test");
//         g_main_loop_quit(loop);
//     });
//     t.detach();

//     int res = scene::Application::getInstance().exec(5, argv);
//     EXPECT_EQ(res, 0);
// }

// 现在框架没有对外暴露主动停止服务的接口，所以客户端请求都会在这一个测试用例中执行完毕

TEST_F(BusinessFrameworkTest, Application_Exec_Success_Basic_2) {
    scene::Application::getInstance().setFunc(std::bind(&MyTest::print, MyTest::getInstance()));

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/";

    // 测试时，只能通过-d -f 去指定配置文件 因为-s时解析文件的路径默认是本地的
    char arg1[] = "program_name";
    char arg2[] = "-d";

    // 使用 std::vector<char> 来管理字符串内存
    std::string arg3_str = filePath.string();
    std::vector<char> arg3(arg3_str.begin(), arg3_str.end());
    arg3.push_back('\0');  // 确保以null结尾

    char arg4[] = "-f";
    char arg5[] = "conf.json";

    char* argv[] = {arg1, arg2, arg3.data(), arg4, arg5, nullptr};

    // 3s 后开始调用客户端同步接口，调用完毕后退出事件循环
    std::thread t([]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        ClientTest client;
        EXPECT_TRUE(client.connect());
        client.callMethod1();
        client.callMethod2();
        client.callMethod3();
        scene::Application::getInstance().notifyAllConnections("test", "test");
        sleep(12);
        client.destroy();
        g_main_loop_quit(loop);
    });
    t.detach();
    int res = scene::Application::getInstance().exec(5, argv);
    EXPECT_EQ(res, 0);
    setLogLevel(LogLevel(-1));
}

// TEST_F(BusinessFrameworkTest, Application_Exec_Success_Basic_3) {
//     scene::Application::getInstance().setFunc(std::bind(&MyTest::print, MyTest::getInstance()));

//     namespace fs = std::filesystem;
//     fs::path dir = fs::path(__FILE__).parent_path();
//     fs::path filePath = dir / "conf/";

//     // 测试时，只能通过-d -f 去指定配置文件 因为-s时解析文件的路径默认是本地的
//     char arg1[] = "program_name";
//     char arg2[] = "-d";

//     // 使用 std::vector<char> 来管理字符串内存
//     std::string arg3_str = filePath.string();
//     std::vector<char> arg3(arg3_str.begin(), arg3_str.end());
//     arg3.push_back('\0');  // 确保以null结尾

//     char arg4[] = "-f";
//     char arg5[] = "conf.json";

//     char* argv[] = {arg1, arg2, arg3.data(), arg4, arg5, nullptr};

//     // 调用客户端异步接口，5s后退出事件循环
//     std::thread t([]() {
//         ClientTest client;
//         EXPECT_TRUE(client.connect());
//         client.callMethod2();
//         std::this_thread::sleep_for(std::chrono::seconds(5));
//         g_main_loop_quit(loop);
//     });
//     t.detach();

//     int res = scene::Application::getInstance().exec(5, argv);
//     EXPECT_EQ(res, 0);
// }

TEST_F(BusinessFrameworkTest, Application_Exec_Error_ErrorArgv_1) {
    scene::Application::getInstance().setFunc(std::bind(&MyTest::print, MyTest::getInstance()));

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/";

    // 测试时，只能通过-d -f 去指定配置文件 因为-s时解析文件的路径默认是本地的
    char arg1[] = "program_name";
    char arg2[] = "-d";

    // 使用 std::vector<char> 来管理字符串内存
    std::string arg3_str = filePath.string();
    std::vector<char> arg3(arg3_str.begin(), arg3_str.end());
    arg3.push_back('\0');  // 确保以null结尾

    char arg4[] = "-f";
    char arg5[] = "error.json";

    char* argv[] = {arg1, arg2, arg3.data(), arg4, arg5, nullptr};
    int res = scene::Application::getInstance().exec(5, argv);
    EXPECT_EQ(res, -1);
}

TEST_F(BusinessFrameworkTest, Application_Exec_Error_ErrorArgv_2) {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path filePath = dir / "conf/";

    char arg1[] = "program_name";
    char arg2[] = "-s";
    char arg3[] = "error";

    char* argv[] = {arg1, arg2, arg3, nullptr};
    int res = scene::Application::getInstance().exec(3, argv);
    EXPECT_EQ(res, -1);
}

TEST_F(BusinessFrameworkTest, Application_Exec_Error_ErrorArgv_3) {
    char arg4[] = "program_name";
    char arg5[] = "-h";
    char* argv_1[] = {arg4, arg5, nullptr};
    int res = scene::Application::getInstance().exec(2, argv_1);
    EXPECT_EQ(res, -1);
}