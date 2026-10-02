/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#define private public

#include "common/KMInfoJson.h"

TEST(KMInfoJsonTest, BaseParseBasicTest)
{
    KMInfoJson::InfoBase info;

    // 构造符合格式的 baseString，例如 "channel:id/version"
    info.m_baseString = "stable:core/1.0.2";

    // 调用函数
    info.baseParse();

    // 断言字段是否被正确分离
    EXPECT_EQ(info.m_baseId, "core");
    EXPECT_EQ(info.m_baseVersion, "1.0.2");
}

TEST(KMInfoJsonTest, VersionParseBasicTest)
{
    KMInfoJson::InfoBase info;

    // 设置 m_baseVersion 为合法格式
    info.m_baseVersion = "2.5";

    // 执行解析逻辑
    info.versionParse();

    // 验证 major 和 minor 是否被正确提取
    EXPECT_EQ(info.m_versionMajor, "2");
    EXPECT_EQ(info.m_versionMinor, "5");
}

TEST(KMInfoJsonTest, CheckVersionTest)
{
    KMInfoJson::InfoBase info1;
    KMInfoJson::InfoBase info2;

    // 设置两个对象的主版本号
    info1.m_versionMajor = "1";
    info2.m_versionMajor = "1";

    // 应返回 true
    EXPECT_TRUE(info1.checkVersion(info2));

    // 设置不同主版本号
    info2.m_versionMajor = "2";

    // 应返回 false
    EXPECT_FALSE(info1.checkVersion(info2));
}

TEST(KMInfoJsonTest, SetAndGetRootTest)
{
    KMInfoJson infoJson;

    // 构造一个 JSON 对象并设置
    nlohmann::json rootJson;
    rootJson["name"] = "test";
    rootJson["version"] = "1.0";

    infoJson.setRoot(rootJson);

    // 获取设置后的 root，并断言值是否一致
    nlohmann::json result = infoJson.root();
    EXPECT_EQ(result["name"], "test");
    EXPECT_EQ(result["version"], "1.0");
}

TEST(KMInfoJsonTest, SaveFileTest)
{
    KMInfoJson infoJson;

    // 构造 root 内容
    nlohmann::json root;
    root["test_key"] = "test_value";
    infoJson.setRoot(root);

    // 保存文件路径
    std::string filePath = "test_output/test_file.json";

    // 调用保存函数
    ASSERT_NO_THROW(infoJson.saveFile(filePath));

    // 检查文件是否存在
    std::ifstream inFile(filePath);
    ASSERT_TRUE(inFile.is_open());

    // 验证 JSON 内容
    nlohmann::json loadedJson;
    inFile >> loadedJson;
    EXPECT_EQ(loadedJson["test_key"], "test_value");

    inFile.close();

    // 清理测试文件
    std::remove(filePath.c_str());
    std::filesystem::remove_all("test_output");
}

TEST(KMInfoJsonTest, OperatorSquareBracketsTest)
{
    KMInfoJson info;

    // base 访问与赋值
    info["base"] = "base_value";
    EXPECT_EQ(info["base"], "base_value");

    // runtime 访问与赋值
    info["runtime"] = "runtime_value";
    EXPECT_EQ(info["runtime"], "runtime_value");

    // depend 访问与赋值
    info["depend"] = "depend_value";
    EXPECT_EQ(info["depend"], "depend_value");

    // 非法 key 抛出异常
    EXPECT_THROW({
        info["invalid"];
    },
                 std::invalid_argument);
}

TEST(KMInfoJsonTest, ParsePermissionsTest)
{
    KMInfoJson info;

    // 构造一个完整的 permission JSON，覆盖所有字段
    nlohmann::json j = R"({
        "permission": {
            "filesystem": {
                "xdg-user-dirs": true,
                "filesystem-dirs": ["dir1","dir2"]
            },
            "net": {
                "network": true,
                "scanning": true
            },
            "device": {
                "bluetooth": true,
                "microphone": true,
                "camera": true,
                "usb": true,
                "serial": true
            },
            "processManager": {
                "autostart": true
            },
            "others": {
                "system-notice": true,
                "system-time": true,
                "system-server": true,
                "window-top": true,
                "prevent-sleep": true,
                "system-display-resolution": true,
                "system-display-brightness": true,
                "system-audio-settings": true,
                "system-audio-size": true,
                "system-crontab": true,
                "system-power-mode": true,
                "system-power-policy": true
            }
        }
    })"_json;

    // 注入 root
    info.setRoot(j);

    // 调用解析函数
    info.parsePermissions();

    // 断言每个字段是否被正确解析
    EXPECT_FALSE(info.permissions.all); 
    EXPECT_TRUE(info.permissions.xdgUserDirs);
    EXPECT_EQ(info.permissions.filesystemDirs.size(), 2);
    EXPECT_TRUE(info.permissions.network);
    EXPECT_TRUE(info.permissions.scanning);
    EXPECT_TRUE(info.permissions.bluetooth);
    EXPECT_TRUE(info.permissions.microphone);
    EXPECT_TRUE(info.permissions.camera);
    EXPECT_TRUE(info.permissions.usb);
    EXPECT_TRUE(info.permissions.serial);
    EXPECT_TRUE(info.permissions.autostart);
    EXPECT_TRUE(info.permissions.systemNotice);
    EXPECT_TRUE(info.permissions.systemTime);
    EXPECT_TRUE(info.permissions.systemServer);
    EXPECT_TRUE(info.permissions.windowTop);
    EXPECT_TRUE(info.permissions.preventSleep);
    EXPECT_TRUE(info.permissions.systemDisplayResolution);
    EXPECT_TRUE(info.permissions.systemDisplayBrightness);
    EXPECT_TRUE(info.permissions.systemAudioSettings);
    EXPECT_TRUE(info.permissions.systemAudioSize);
    EXPECT_TRUE(info.permissions.systemCrontab);
    EXPECT_TRUE(info.permissions.systemPowerMode);
    EXPECT_TRUE(info.permissions.systemPowerPolicy);
}

TEST(KMInfoJsonTest, ParseAnnotationsTest)
{
    // 构造完整的 JSON，触发 parseAnnotations 所有分支
    nlohmann::json j = {
        {"annotations", {
            {"kylin.privilege", true},
            {"kylin.sysapp", true},
            {"kylin.gpu", true},
            {"kylin.cover", {{"key1", "value1"}, {"key2", "value2"}}},
            {"kylin.polkit", true},
            {"kylin.tray", true},
            {"kylin.deltas", true},
            {"kylin.depends", {"dep1", "dep2"}},
            {"kylin.depend-apps", {"app1", "app2"}},
            {"kylin.desktop", {"desk1"}},
            {"kylin.dbus-whitelist", {"dbus1"}},
            {"kylin.preVersion", {"v1.0"}},

            {"kylin.hostapp", true},
            {"kylin.script", true},
            {"service", {"svc1", "svc2"}},
            {"kylin.addsystemgroup", {"group1"}},
            {"kylin.autostart", {"auto1"}},
            {"kylin.chmod", {"chmod1"}},
            {"kylin.installkill", {"ik1"}},
            {"kylin.uninstallkill", {"uk1"}},
            {"kylin.stopservice", {"ss1"}},
            {"kylin.rmdir", {"rmdir1"}},
            {"kylin.unlink", {"unlink1"}},

            // install 分支
            {"kylin.install", {
                {"start_service", {"start1"}},
                {"stop_service", {"stop1"}},
                {"stop_process", {"proc1"}},
                {"chmod", {"chmod_inst"}},
                {"add_group", {"grp1"}},
                {"autostart", {"auto_inst"}},
                {"remove", {"rm1"}}
            }},

            // uninstall 分支
            {"kylin.uninstall", {
                {"stop_service", {"ustop1"}},
                {"stop_process", {"uproc1"}},
                {"remove", {"urm1"}}
            }},

            // debinfo 分支
            {"kylin.debinfo", {
                {"Package", "pkg1"},
                {"Version", "1.2.3"},
                {"Section", "utils"},
                {"Package-Type", "deb"},
                {"Recommends", "rec1"},
                {"Uploaders", "up1"},
                {"Provides", "prov1"},
                {"Origin", "origin1"},
                {"Multi-Arch", "all"},
                {"Build-Depends", "bd1"},
                {"Source", "src1"},
                {"Suggests", "sug1"},
                {"Conflicts", "conf1"},
                {"Breaks", "break1"},
                {"Replaces", "rep1"},
                {"Vcs-Git", "git1"},
                {"Vcs-Browser", "browser1"}
            }}
        }}
    };

    KMInfoJson info;
    info.setRoot(j);  // 设置私有 m_root

    info.parseAnnotations();

    // 验证基本 bool 字段
    EXPECT_TRUE(info.annotations.privilege);
    EXPECT_TRUE(info.annotations.sysapp);
    EXPECT_TRUE(info.annotations.gpu);
    EXPECT_TRUE(info.annotations.polkit);
    EXPECT_TRUE(info.annotations.tray);
    EXPECT_TRUE(info.annotations.deltas);
    EXPECT_TRUE(info.annotations.hostapp);
    EXPECT_TRUE(info.annotations.hostscript);

    // 验证列表字段
    EXPECT_EQ(info.annotations.depends.size(), 2);
    EXPECT_EQ(info.annotations.dependApps.size(), 2);
    EXPECT_EQ(info.annotations.desktops.size(), 1);
    EXPECT_EQ(info.annotations.kylin_dbus_whitelist.size(), 1);
    EXPECT_EQ(info.annotations.preVersion.size(), 1);

    EXPECT_EQ(info.annotations.service.size(), 2);
    EXPECT_EQ(info.annotations.sysgroup.size(), 1);
    EXPECT_EQ(info.annotations.autostart.size(), 1);
    EXPECT_EQ(info.annotations.chmods.size(), 1);
    EXPECT_EQ(info.annotations.installkills.size(), 1);
    EXPECT_EQ(info.annotations.uninstallkills.size(), 1);
    EXPECT_EQ(info.annotations.stopservices.size(), 1);
    EXPECT_EQ(info.annotations.rmdir.size(), 1);
    EXPECT_EQ(info.annotations.unlink.size(), 1);

    // 验证 install 分支
    EXPECT_EQ(info.annotations.install_actions.start_service[0], "start1");
    EXPECT_EQ(info.annotations.install_actions.stop_service[0], "stop1");
    EXPECT_EQ(info.annotations.install_actions.stop_process[0], "proc1");
    EXPECT_EQ(info.annotations.install_actions.chmod[0], "chmod_inst");
    EXPECT_EQ(info.annotations.install_actions.add_group[0], "grp1");
    EXPECT_EQ(info.annotations.install_actions.autostart[0], "auto_inst");
    EXPECT_EQ(info.annotations.install_actions.remove[0], "rm1");

    // 验证 uninstall 分支
    EXPECT_EQ(info.annotations.uninstall_actions.stop_service[0], "ustop1");
    EXPECT_EQ(info.annotations.uninstall_actions.stop_process[0], "uproc1");
    EXPECT_EQ(info.annotations.uninstall_actions.remove[0], "urm1");

    // 验证 debinfo
    EXPECT_EQ(info.annotations.debinfo.package, "pkg1");
    EXPECT_EQ(info.annotations.debinfo.version, "1.2.3");
    EXPECT_EQ(info.annotations.debinfo.section, "utils");
    EXPECT_EQ(info.annotations.debinfo.package_type, "deb");
    EXPECT_EQ(info.annotations.debinfo.recommends, "rec1");
    EXPECT_EQ(info.annotations.debinfo.uploaders, "up1");
    EXPECT_EQ(info.annotations.debinfo.provides, "prov1");
    EXPECT_EQ(info.annotations.debinfo.origin, "origin1");
    EXPECT_EQ(info.annotations.debinfo.multi_arch, "all");
    EXPECT_EQ(info.annotations.debinfo.build_depends, "bd1");
    EXPECT_EQ(info.annotations.debinfo.source, "src1");
    EXPECT_EQ(info.annotations.debinfo.suggests, "sug1");
    EXPECT_EQ(info.annotations.debinfo.conflicts, "conf1");
    EXPECT_EQ(info.annotations.debinfo.breaks, "break1");
    EXPECT_EQ(info.annotations.debinfo.replaces, "rep1");
    EXPECT_EQ(info.annotations.debinfo.vcs_git, "git1");
    EXPECT_EQ(info.annotations.debinfo.vcs_browser, "browser1");
}