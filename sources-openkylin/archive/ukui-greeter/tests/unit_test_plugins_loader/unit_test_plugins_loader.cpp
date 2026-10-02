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
#include <gtest/gtest-death-test.h>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <memory>
#include "../../ukui-greeter/greeter/pluginsloader.h"
#include "../../ukui-greeter/greeter/loginplugininterface.h"

#include "stubext.h"
using namespace stub_ext;


class PluginsLoaderTest : public ::testing::Test {
protected:
    PluginsLoader* pluginsLoader;


    void SetUp() override {

        pluginsLoader = &PluginsLoader::instance();
    }


    void TearDown() override {
        // 清理可能创建的测试文件和目录
//        QDir dir(gPluginDir);
//        if (dir.exists()) {
//            dir.removeRecursively();
//        }
    }
};


// 测试单例模式
TEST_F(PluginsLoaderTest, InstanceTest) {
    PluginsLoader* instance1 = &PluginsLoader::instance();
    PluginsLoader* instance2 = &PluginsLoader::instance();
    EXPECT_EQ(instance1, instance2);
}


// 测试 findModuleByName 函数
TEST_F(PluginsLoaderTest, FindModuleByNameTest) {
    EXPECT_EQ(pluginsLoader->findModuleByName("NonExistentModule"), nullptr);
}


// 测试 findModulesByType 函数
TEST_F(PluginsLoaderTest, FindModulesByTypeTest) {

    EXPECT_EQ(PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).size(), 0);
}


// 测试 run 和 findModule 函数
TEST_F(PluginsLoaderTest, RunAndFindModuleTest) {
    // 正常情况：模拟插件目录存在
    QDir dir("/usr/lib/ukui-greeter/plugins");
    dir.mkpath(".");


    QFile mockPluginFile(dir.absoluteFilePath("mockplugin.so"));
    mockPluginFile.open(QIODevice::WriteOnly);
    mockPluginFile.close();


    pluginsLoader->run();


    // 检查是否有模块被找到
    EXPECT_GT(pluginsLoader->m_plugins.size(), 0);


    // 异常情况：模拟插件目录不存在
    QDir invalidDir("/invalid/path");
    pluginsLoader->findModule(invalidDir.absolutePath());


    // 检查是否正确处理目录不存在的情况
    EXPECT_EQ(pluginsLoader->m_plugins.size(), 0);
}


// 测试 removeModule 函数
TEST_F(PluginsLoaderTest, RemoveModuleTest) {
    QString moduleName = "TestModule";

    pluginsLoader->removeModule(moduleName);


    EXPECT_EQ(pluginsLoader->m_plugins.contains(moduleName), false);
}
