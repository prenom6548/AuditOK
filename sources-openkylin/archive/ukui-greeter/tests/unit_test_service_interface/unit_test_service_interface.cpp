/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QTextCodec>
#include "../../ukui-greeter-daemon/serviceinterface.h"
#include "stubext.h"

using namespace stub_ext;


// 测试 ServiceInterface 的构造函数
TEST(ServiceInterfaceTest, Constructor) {
    StubExt st;
    // 使用 std::bind 来绑定 QDBusConnection 实例和 registerService 函数
    std::function<bool(QDBusConnection&, const QString&)> originalRegisterService = std::bind(&QDBusConnection::registerService,
                                                                         std::placeholders::_1,
                                                                         std::placeholders::_2);
    // 获取 QDBusConnection 实例
    QDBusConnection dbusConn = QDBusConnection::systemBus();
    // 使用 lambda 函数来调用绑定的函数
    st.set_lamda([&originalRegisterService, &dbusConn](const QString& serviceName) {
        return originalRegisterService(dbusConn, serviceName);
    }, []() { return true; });

    ServiceInterface service;
    // 检查 QDBusConnection 注册是否成功
//    EXPECT_TRUE(QDBusConnection::systemBus().isConnected());
    // 检查 QSettings 初始化
    EXPECT_EQ(service.settings->fileName(), QString("/usr/share/ukui-greeter/ukui-greeter.conf"));
    EXPECT_EQ(service.settings->format(), QSettings::IniFormat);
    EXPECT_EQ(service.settings->iniCodec(), QTextCodec::codecForName("utf-8"));
    // 检查 m_willLoginUser 初始状态
    EXPECT_EQ(service.m_willLoginUser.strUserName, "");
    EXPECT_FALSE(service.m_willLoginUser.isOneKeyLogin);
}


// 测试 SwitchToUser 函数
TEST(ServiceInterfaceTest, SwitchToUser) {
//    ServiceInterface service;
//    QSignalSpy spy(&service, SIGNAL(userChanged(WillLoginUserInfo)));
//    service.SwitchToUser("testUser");
//    // 检查 m_willLoginUser 更新
//    EXPECT_EQ(service.m_willLoginUser.strUserName, "testUser");
//    EXPECT_TRUE(service.m_willLoginUser.isOneKeyLogin);
//    // 检查信号是否发出
//    EXPECT_EQ(spy.count(), 1);
}


// 测试 SwitchToGreeterUser 函数
TEST(ServiceInterfaceTest, SwitchToGreeterUser) {
//    ServiceInterface service;
//    QSignalSpy spy(&service, SIGNAL(userChanged(WillLoginUserInfo));
//    service.SwitchToGreeterUser("testUser");
//    // 检查 m_willLoginUser 更新
//    EXPECT_EQ(service.m_willLoginUser.strUserName, "testUser");
//    EXPECT_FALSE(service.m_willLoginUser.isOneKeyLogin);
//    // 检查信号是否发出
//    EXPECT_EQ(spy.count(), 1);
}


// 测试 getWillSwitchUser 函数
TEST(ServiceInterfaceTest, GetWillSwitchUser) {
    ServiceInterface service;
    service.m_willLoginUser.strUserName = "testUser";
    service.m_willLoginUser.isOneKeyLogin = true;
    WillLoginUserInfo userInfo = service.getWillSwitchUser();
    EXPECT_EQ(userInfo.strUserName, "testUser");
    EXPECT_TRUE(userInfo.isOneKeyLogin);
}


// 测试 getValue 和 setValue 函数
TEST(ServiceInterfaceTest, GetValueAndSetValue) {
    ServiceInterface service;
    service.setValue("testKey", "testValue");
    QVariant value = service.getValue("testKey");
    EXPECT_EQ(value.toString(), "testValue");

    service.setValue("boolKey", true);
    value = service.getValue("boolKey");
    EXPECT_EQ(value.toBool(), true);

    service.setValue("intKey", 123);
    value = service.getValue("intKey");
    EXPECT_EQ(value.toInt(), 123);

    // 测试不存在的键
    value = service.getValue("nonExistentKey");
    EXPECT_EQ(value.isValid(), false);
}


// 测试 getGreeterShowPrompt 函数
TEST(ServiceInterfaceTest, GetGreeterShowPrompt) {
    ServiceInterface service;
    // 测试开关为关
    service.setValue("Greeter/showLoginPrompt", false);
    EXPECT_FALSE(service.getGreeterShowPrompt());

    // 测试标题为空
    service.setValue("Greeter/showLoginPrompt", true);
    service.setValue("Greeter/promptTitle", "");
    EXPECT_FALSE(service.getGreeterShowPrompt());

    // 测试 promptText 不为空
    service.setValue("Greeter/showLoginPrompt", true);
    service.setValue("Greeter/promptTitle", "title");
    service.setValue("Greeter/promptText", "text");
    EXPECT_TRUE(service.getGreeterShowPrompt());

    // 测试文件存在
    service.setValue("Greeter/showLoginPrompt", true);
    service.setValue("Greeter/promptTitle", "title");
    service.setValue("Greeter/promptTextFilePath", "/path/to/file");
    QFile file("/path/to/file");
    file.open(QIODevice::WriteOnly);
    file.close();
    EXPECT_TRUE(service.getGreeterShowPrompt());
}


// 测试 getShowLoginPrompt 函数
TEST(ServiceInterfaceTest, GetShowLoginPrompt) {
    ServiceInterface service;
    // 测试键存在
    service.setValue("Greeter/showLoginPrompt", true);
    EXPECT_TRUE(service.getShowLoginPrompt());
    // 测试键不存在
    EXPECT_FALSE(service.getShowLoginPrompt());
}


// 测试 getHideTitle 函数
TEST(ServiceInterfaceTest, GetHideTitle) {
    ServiceInterface service;
    // 测试键存在
    service.setValue("Greeter/hideTitle", true);
    EXPECT_TRUE(service.getHideTitle());
    // 测试键不存在
    EXPECT_TRUE(service.getHideTitle());
}


// 测试 getPromptTitle 函数
TEST(ServiceInterfaceTest, GetPromptTitle) {
    ServiceInterface service;
    // 测试键存在
    service.setValue("Greeter/promptTitle", "title");
    EXPECT_EQ(service.getPromptTitle(), "title");
    // 测试键不存在
    EXPECT_EQ(service.getPromptTitle(), "");
}


// 测试 getPromptText 函数
TEST(ServiceInterfaceTest, GetPromptText) {
    ServiceInterface service;
    // 测试键存在
    service.setValue("Greeter/promptText", "text");
    EXPECT_EQ(service.getPromptText(), "text");
    // 测试键不存在
    EXPECT_EQ(service.getPromptText(), "");
}


// 测试 getPromptTextFilePath 函数
TEST(ServiceInterfaceTest, GetPromptTextFilePath) {
    ServiceInterface service;
    // 测试键存在
    service.setValue("Greeter/promptTextFilePath", "/path/to/file");
    EXPECT_EQ(service.getPromptTextFilePath(), "/path/to/file");
    // 测试键不存在
    EXPECT_EQ(service.getPromptTextFilePath(), "");
}


// 测试 setShowLoginPrompt 函数
TEST(ServiceInterfaceTest, SetShowLoginPrompt) {
    ServiceInterface service;
    service.setShowLoginPrompt(true);
    EXPECT_TRUE(service.getShowLoginPrompt());
}


// 测试 setHideTitle 函数
TEST(ServiceInterfaceTest, SetHideTitle) {
    ServiceInterface service;
    service.setHideTitle(true);
    EXPECT_TRUE(service.getHideTitle());
}


// 测试 setPromptTitle 函数
TEST(ServiceInterfaceTest, SetPromptTitle) {
    ServiceInterface service;
    service.setPromptTitle("title");
    EXPECT_EQ(service.getPromptTitle(), "title");
}


// 测试 setPromptText 函数
TEST(ServiceInterfaceTest, SetPromptText) {
    ServiceInterface service;
    service.setPromptText("text");
    EXPECT_EQ(service.getPromptText(), "text");
}


// 测试 setPromptTextFilePath 函数
TEST(ServiceInterfaceTest, SetPromptTextFilePath) {
    ServiceInterface service;
    service.setPromptTextFilePath("/path/to/file");
    EXPECT_EQ(service.getPromptTextFilePath(), "/path/to/file");
}
