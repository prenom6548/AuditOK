/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <filesystem>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>

#define private public

#include "KMTestBuilder.h"
#include "common/KMStorageDir.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMUtil.h"
#include "common/KMInfoJson.h"
#include "common/KMException.h"
#include "common/KMJsonHelper.h"
#include "common/KMSubPackagesJson.h"
#include "common/KMLogger.h"
#include "app/builder/KMBuildInit.h"
#include "app/builder/KMBuild.h"
#include "app/builder/KMContainerWrap.h"
#include "app/builder/KMBuildFinish.h"
#include "app/builder/KMBuildExport.h"

namespace fs =  std::filesystem;

int KMTestBuilder::init(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    int ret = 0;
    {
        KMBuildInit init;
        ret = init.dispose(argv.size(), argv.data());
    }
    
    if (ret != 0)
    {
        initDepend();

        KMBuildInit init;
        ret = init.dispose(argv.size(), argv.data());
    }

    return ret;
}

void KMTestBuilder::initCache()
{
    std::string stageCache = KMStringUtils::buildFilename(m_cacheDir, "stage-cache.json");
    std::ofstream o(stageCache);

    nlohmann::json root;
    root["init"] = true;
    o << std::setw(4) << root << std::endl;
}

void KMTestBuilder::initDepend()
{
    std::string dependDir = KMStringUtils::buildFilename(KMStorageDir::getUserBaseDirLocation(), "stable", KMUtil::kmGetArch(), "depend/depend.openkylin.libgsl27/devel/2.7.0");
    
    std::string libDir = KMStringUtils::buildFilename(dependDir, "files/usr/lib");
    KMFileUtils::mkpath(libDir);
    std::string content("#!/bin/bash\n\n"
                        "echo 'This is a .so'");
    std::string so = KMStringUtils::buildFilename(libDir, "so.sh");
    KMFileUtils::syncWriteFile(so, content);
    fs::permissions(so, fs::perms::all);

    KMInfoJson info;
    info.schema_version = "1.0";
    info.id = "depend.openkylin.libgsl27";
    info.archs.push_back(KMUtil::kmGetArch());
    info.kind = "depend";
    info.module = "devel";
    info.base = "stable:top.openkylin.base/1/" + KMUtil::kmGetArch();
    info.runtime = "stable:top.openkylin.ukui/1/" + KMUtil::kmGetArch();
    info.channel = "stable";
    info.name = "libgsl27";
    info.description = "This is a test demo";
    info.version = "2.7.0";
    info.permissions.all = true;
    info.annotations.sysapp = true;
    info.size = 3041234;
    std::string json = KMStringUtils::buildFilename(dependDir, "info.json");
    info.saveFile(json);

    std::string deploy = KMStringUtils::buildFilename(dependDir, "deploy");
    KMFileUtils::syncWriteFile(deploy, "depend.openkylin.libgsl27");
}

int KMTestBuilder::build(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMBuild build;
    int ret = build.dispose(argv.size(), argv.data());
    build.cleanup();

    return ret;
}

int KMTestBuilder::finish(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMBuildFinish finish;
    return finish.dispose(argv.size(), argv.data());
}

void KMTestBuilder::prepareFinishData()
{
    std::string kaiming = KMStringUtils::buildFilename(m_directory, "KAIMING");
    KMFileUtils::mkpath(kaiming);
    std::string changelog = KMStringUtils::buildFilename(kaiming, "changelog");
    KMFileUtils::syncWriteFile(changelog, "kaiming");

    std::string templateDir = KMStringUtils::buildFilename(m_directory, "files/home/模板");
    KMFileUtils::mkpath(templateDir);
    std::string doc = KMStringUtils::buildFilename(templateDir, "top.openkylin.hello.doc");
    KMFileUtils::syncWriteFile(doc, "template");

    std::string autostart = KMStringUtils::buildFilename(m_directory, "files/etc/xdg/autostart");
    KMFileUtils::mkpath(autostart);
    std::string desktop = KMStringUtils::buildFilename(autostart, "top.openkylin.hello.desktop");
    KMFileUtils::syncWriteFile(desktop, "[Desktop Entry]");

    std::string binDir = KMStringUtils::buildFilename(m_directory, "files/usr/local/bin");
    KMFileUtils::mkpath(binDir);
    std::string content("#!/bin/bash\n\n"
                        "echo 'Hello world!'");
    std::string hello = KMStringUtils::buildFilename(binDir, "hello.sh");
    KMFileUtils::syncWriteFile(hello, content);
    fs::permissions(hello, fs::perms::all);

    std::string applications = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/applications");
    KMFileUtils::mkpath(applications);
    desktop = KMStringUtils::buildFilename(applications, "top.openkylin.hello.desktop");
    KMFileUtils::syncWriteFile(desktop, "[Desktop Entry]");

    std::string icons = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/icons");
    KMFileUtils::mkpath(icons);
    std::string svg = KMStringUtils::buildFilename(icons, "top.openkylin.hello.svg");
    KMFileUtils::syncWriteFile(svg, "1010101010");

    std::string pixmaps = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/pixmaps");
    KMFileUtils::mkpath(pixmaps);
    std::string xpm = KMStringUtils::buildFilename(pixmaps, "top.openkylin.hello.xpm");
    KMFileUtils::syncWriteFile(xpm, "1010101010");

    std::string dbus1 = KMStringUtils::buildFilename(m_directory, "files/etc/dbus-1");
    KMFileUtils::mkpath(dbus1);
    std::string limit = KMStringUtils::buildFilename(dbus1, "top.openkylin.hello.limit");
    KMFileUtils::syncWriteFile(limit, "1010101010");

    std::string search = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/gnome-shell/search-providers");
    KMFileUtils::mkpath(search);
    std::string ini = KMStringUtils::buildFilename(search, "top.openkylin.hello.ini");
    KMFileUtils::syncWriteFile(ini, "1010101010");

    std::string packages = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/mime/packages");
    KMFileUtils::mkpath(packages);
    std::string xml = KMStringUtils::buildFilename(packages, "top.openkylin.hello.xml");
    KMFileUtils::syncWriteFile(xml, "1010101010");

    std::string appdata = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/appdata");
    KMFileUtils::mkpath(appdata);
    std::string appdata_xml = KMStringUtils::buildFilename(appdata, "top.openkylin.hello.xml");
    KMFileUtils::syncWriteFile(appdata_xml, "1010101010");

    std::string systemd = KMStringUtils::buildFilename(m_directory, "files/usr/local/lib/systemd/system");
    KMFileUtils::mkpath(systemd);
    std::string service = KMStringUtils::buildFilename(systemd, "top.openkylin.hello.service");
    KMFileUtils::syncWriteFile(service, "1010101010");

    std::string guide = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/kylin-user-guide/data/guide");
    KMFileUtils::mkpath(guide);
    std::string md = KMStringUtils::buildFilename(guide, "top.openkylin.hello.md");
    KMFileUtils::syncWriteFile(md, "1010101010");

    std::string schemas = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/glib-2.0/schemas");
    KMFileUtils::mkpath(schemas);
    std::string schemas_xml = KMStringUtils::buildFilename(schemas, "top.openkylin.hello.xml");
    KMFileUtils::syncWriteFile(schemas_xml, "1010101010");

    std::string polkit1 = KMStringUtils::buildFilename(m_directory, "files/usr/local/share/polkit-1");
    KMFileUtils::mkpath(polkit1);
    std::string policy = KMStringUtils::buildFilename(polkit1, "top.openkylin.hello.policy");
    KMFileUtils::syncWriteFile(policy, "1010101010");

    KMInfoJson info;
    info.schema_version = "1.0";
    info.id = "top.openkylin.hello";
    info.archs.push_back(KMUtil::kmGetArch());
    info.kind = "app";
    info.module = "binary";
    info.base = "stable:top.openkylin.base/1/" + KMUtil::kmGetArch();
    info.runtime = "stable:top.openkylin.ukui/1/" + KMUtil::kmGetArch();
    info.channel = "stable";
    info.name = "Hello";
    info.description = "This is a demo";
    info.version = "1.0.0";
    info.license = "Apache-2.0";
    info.homepage = "http://www.xxx.xxx";
    info.maintainers.push_back("name@kylinos.cn");
    info.permissions.filesystemDirs.push_back("/data");
    info.permissions.xdgUserDirs = true;
    info.permissions.network = true;
    info.permissions.scanning = true;
    info.permissions.bluetooth = true;
    info.permissions.usb = true;
    info.permissions.autostart = true;
    info.permissions.systemNotice = true;
    info.annotations.sysapp = true;
    info.annotations.depends.push_back("depend.openkylin.libgsl27");
    std::string json = KMStringUtils::buildFilename(m_directory, "info.json");
    info.saveFile(json);
}

int KMTestBuilder::exports(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMBuildExport exports;
    return exports.dispose(argv.size(), argv.data());
}

void KMTestBuilder::SetUp()
{
    m_directory = KMStringUtils::buildFilename(fs::current_path().string(), "build");
    m_stateDir = KMStringUtils::buildFilename(fs::current_path().string(), ".kaiming-builder");
    m_cacheDir = KMStringUtils::buildFilename(m_stateDir, "cache");

    KMFileUtils::mkpath(m_directory);
    KMFileUtils::mkpath(m_cacheDir);

    initCache();

    // 清理已经安装的depend包，增加覆盖率
    std::string dependDir = KMStringUtils::buildFilename(KMStorageDir::getUserBaseDirLocation(), "stable", KMUtil::kmGetArch(), "depend");
    KMFileUtils::rmRfAll(dependDir);
}

void KMTestBuilder::TearDown()
{
    KMFileUtils::rmRfAll(m_directory);
    KMFileUtils::rmRfAll(m_stateDir);
}

TEST_F(KMTestBuilder, BuildInit_AddEnv)
{
    KMBuildInit init;
    std::map<std::string, std::string> envs;
    init.addEnv(envs, "UNIT_TEST", "PREFIX:");
    init.addEnv(envs, "UNIT_TEST", "$UNIT_TEST:APPEND1");
    init.addEnv(envs, "UNIT_TEST", ":SUXFIX");
    init.addEnv(envs, "UNIT_TEST1", "$UNIT_TEST/ADD:APPEND2");
    EXPECT_STREQ(std::string("PREFIX:APPEND1:SUXFIX").data(), envs.at("UNIT_TEST").data());
    EXPECT_STREQ(std::string("PREFIX:APPEND1:SUXFIX/ADD:APPEND2").data(), envs.at("UNIT_TEST1").data());
}

TEST_F(KMTestBuilder, BuildInit)
{
    std::vector<std::string> args;
    args.push_back("build-init");
    args.push_back("--kind=depend");
    args.push_back("--channel=stable");
    args.push_back("--version=1.0.0");
    args.push_back("--kylin-sysapp");
    args.push_back("--base=stable:top.openkylin.base/1");
    args.push_back("--runtime=stable:top.openkylin.ukui/1");
    args.push_back("--depend=depend.openkylin.libgsl27");
    args.push_back("--cacheDir=" + m_cacheDir);
    args.push_back(m_directory);
    args.push_back("depend.openkylin.hello");
    EXPECT_EQ(0u, init(args));
}

TEST_F(KMTestBuilder, Build_Sysapp)
{
    // 前置步骤
    std::vector<std::string> args;
    args.push_back("build-init");
    args.push_back("--kind=app");
    args.push_back("--channel=stable");
    args.push_back("--version=1.0.0");
    args.push_back("--kylin-sysapp");
    args.push_back("--base=stable:top.openkylin.base/1");
    args.push_back("--runtime=stable:top.openkylin.ukui/1");
    args.push_back("--depend=depend.openkylin.libgsl27");
    args.push_back("--cacheDir=" + m_cacheDir);
    args.push_back(m_directory);
    args.push_back("top.openkylin.hello");

    EXPECT_EQ(0u, init(args));

    // 数据准备
    std::string sourcesDir = KMStringUtils::buildFilename(m_stateDir, "sources/hello");
    KMFileUtils::mkpath(sourcesDir);
    std::string content("#!/bin/bash\n\n"
                        "echo 'Hello world!'");
    std::string hello = KMStringUtils::buildFilename(sourcesDir, "hello.sh");
    KMFileUtils::syncWriteFile(hello, content);
    fs::permissions(hello, fs::perms::all);
    std::string script = KMStringUtils::buildFilename(sourcesDir, "kaiming_buildScript.sh");
    content = "#!/bin/bash\n\n"
              "install -D hello.sh ${APP_INSTALL_PREFIX}/bin/hello";
    KMFileUtils::syncWriteFile(script, content);
    fs::permissions(script, fs::perms::all);

    args.clear();
    args.push_back("build");
    args.push_back("--env=KAIMING_BUILDER_BUILDDIR=/run/build/hello");
    args.push_back("--env=KAIMING_BUILDER_N_JOB=12");
    args.push_back("--build-dir=/run/build/hello");
    args.push_back("--bind-mount=/run/build/hello=" + sourcesDir);
    args.push_back(m_directory);
    args.push_back("./kaiming_buildScript.sh");
    args.push_back("more");

    EXPECT_EQ(0u, build(args));
}

TEST_F(KMTestBuilder, Build)
{
    // 前置步骤
    std::vector<std::string> args;
    args.push_back("build-init");
    args.push_back("--kind=app");
    args.push_back("--channel=stable");
    args.push_back("--version=1.0.0");
    args.push_back("--base=stable:top.openkylin.base/1");
    args.push_back("--runtime=stable:top.openkylin.ukui/1");
    args.push_back("--depend=depend.openkylin.libgsl27");
    args.push_back("--cacheDir=" + m_cacheDir);
    args.push_back(m_directory);
    args.push_back("top.openkylin.hello");

    EXPECT_EQ(0u, init(args));

    // 数据准备
    std::string sourcesDir = KMStringUtils::buildFilename(m_stateDir, "sources/hello");
    KMFileUtils::mkpath(sourcesDir);
    std::string content("#!/bin/bash\n\n"
                        "echo 'Hello world!'");
    std::string hello = KMStringUtils::buildFilename(sourcesDir, "hello.sh");
    KMFileUtils::syncWriteFile(hello, content);
    fs::permissions(hello, fs::perms::all);
    std::string script = KMStringUtils::buildFilename(sourcesDir, "kaiming_buildScript.sh");
    content = "#!/bin/bash\n\n"
              "install -D hello.sh ${APP_INSTALL_PREFIX}/bin/hello";
    KMFileUtils::syncWriteFile(script, content);
    fs::permissions(script, fs::perms::all);

    int rows=0, columns=0;
    KMBuild::getTerminalSize(rows, columns);
    bool remote = false;
    KMContainerWrap::parsePulseServer("{server}tcp://172.30.60.71", remote);
    KMContainerWrap::cupsCheckServerIsSocket("/var/run/lightdm/greeter-socket");
    KMContainerWrap::getCupsServerNameConfig("/etc/profile");

    args.clear();
    args.push_back("build");
    args.push_back("--env=KAIMING_BUILDER_BUILDDIR=/run/build/hello");
    args.push_back("--env=KAIMING_BUILDER_N_JOB=12");
    args.push_back("--env=VAR1=VALUE1");
    args.push_back("--env=VAR2=VALUE2");
    args.push_back("--unset-env=VAR1");
    args.push_back("--env=UNIT_TEST=PREFIX:");
    args.push_back("--env=UNIT_TEST=$UNIT_TEST:APPEND1");
    args.push_back("--env=UNIT_TEST=:SUXFIX");
    args.push_back("--env=UNIT_TEST1=$UNIT_TEST/ADD:APPEND2");
    args.push_back("--build-dir=/run/build/hello");
    args.push_back("--bind-mount=/run/build/hello=" + sourcesDir);
    args.push_back(m_directory);
    args.push_back("ls");
    args.push_back("-al");

    EXPECT_EQ(0u, build(args));
}

TEST_F(KMTestBuilder, BuildFinish)
{
    // 数据准备
    prepareFinishData();

    std::vector<std::string> args;
    args.push_back("build-finish");
    args.push_back(m_directory);
    EXPECT_EQ(0u, finish(args));
}

TEST_F(KMTestBuilder, BuildExport_Ref)
{
    // 前置步骤
    std::vector<std::string> args;
    args.push_back("build-init");
    args.push_back("--kind=app");
    args.push_back("--channel=stable");
    args.push_back("--version=1.0.0");
    args.push_back("--kylin-sysapp");
    args.push_back("--base=stable:top.openkylin.base/1");
    args.push_back("--runtime=stable:top.openkylin.ukui/1");
    args.push_back("--depend=depend.openkylin.libgsl27");
    args.push_back("--cacheDir=" + m_cacheDir);
    args.push_back(m_directory);
    args.push_back("top.openkylin.hello");

    EXPECT_EQ(0u, init(args));

    args.clear();
    args.push_back("build-export");
    args.push_back("--ref-mode");
    args.push_back("--dest-dir=" + m_cacheDir);
    args.push_back("depend.openkylin.libgsl27");
    EXPECT_EQ(0u, exports(args));
}

TEST_F(KMTestBuilder, BuildExport_SplitsModes)
{
    prepareFinishData();

    std::string modules_json = KMStringUtils::buildFilename(m_directory, "modules.json");
    nlohmann::json root;
    KMJsonHelper helper(root);
    std::map<std::string, std::string> modules;
    modules.insert({ "binary", "/usr/local/bin" });
    modules.insert({ "share", "/usr/local/share\n\".*xml\"" });
    helper.setStringMapValue("modules", modules);
    std::ofstream o(modules_json);
    o << std::setw(4) << root << std::endl;

    std::vector<std::string> args;
    args.push_back("build-finish");
    args.push_back(m_directory);
    EXPECT_EQ(0u, finish(args));

    args.clear();
    args.push_back("build-export");
    args.push_back("--dest-dir=" + m_cacheDir);
    args.push_back(m_directory);
    EXPECT_EQ(0u, exports(args));
}

TEST_F(KMTestBuilder, BuildExport_SubPackages)
{
    // 前置步骤
    std::vector<std::string> args;
    args.push_back("build-init");
    args.push_back("--kind=app");
    args.push_back("--channel=stable");
    args.push_back("--version=1.0.0");
    args.push_back("--kylin-sysapp");
    args.push_back("--base=stable:top.openkylin.base/1");
    args.push_back("--runtime=stable:top.openkylin.ukui/1");
    args.push_back("--depend=depend.openkylin.libgsl27");
    args.push_back("--cacheDir=" + m_cacheDir);
    args.push_back(m_directory);
    args.push_back("top.openkylin.hello");

    EXPECT_EQ(0u, init(args));

    prepareFinishData();

    KMInfoJson info;
    info.loadFile(KMStringUtils::buildFilename(m_directory, "info.json"));

    KMSubPackagesJson json;
    KMSubPackagesJson::SubPackage subpackage1;
    subpackage1.id = "top.openkylin.hello.share";
    subpackage1.kind = "depend";
    subpackage1.module = "devel";
    subpackage1.name = "Share";
    subpackage1.version = info.version;
    subpackage1.description =  info.description;
    subpackage1.depends = info.annotations.depends;
    subpackage1.includes.push_back("/usr/share");
    subpackage1.includes.push_back("/usr/local/share");
    json.subpackages.push_back(subpackage1);

    KMSubPackagesJson::SubPackage subpackage2;
    subpackage2.id = info.id;
    subpackage2.kind = info.kind;
    subpackage2.module = "binary";
    subpackage2.name = info.name;
    subpackage2.version = info.version;
    subpackage2.description =  info.description;
    subpackage2.names.insert({ "name[zh_CN]", "Hello样例" });
    subpackage2.descriptions.insert({ "description[zh_CN]", "这是一个样例。" });
    subpackage2.depends = info.annotations.depends;
    subpackage2.depends.push_back("top.openkylin.hello.share");
    subpackage2.includes.push_back("/usr");
    subpackage2.includes.push_back("/etc");
    subpackage2.includes.push_back("/home");
    json.subpackages.push_back(subpackage2);

    std::string subpackages_json = KMStringUtils::buildFilename(m_directory, "subpackages.json");
    json.saveFile(subpackages_json);

    args.clear();
    args.push_back("build-export");
    args.push_back("--dest-dir=" + m_cacheDir);
    args.push_back(m_directory);
    EXPECT_EQ(0u, exports(args));
}

TEST_F(KMTestBuilder, BuildExport_Erofs)
{
    prepareFinishData();

    std::vector<std::string> args;
    args.push_back("build-finish");
    args.push_back(m_directory);
    EXPECT_EQ(0u, finish(args));

    args.clear();
    args.push_back("build-export");
    args.push_back("--bundle-filesystem=erofs");
    args.push_back("--dest-dir=" + m_cacheDir);
    args.push_back(m_directory);
    
    try
    {
        exports(args);
    }
    catch(const std::exception& e)
    {
        // 系统不一定支持erofs，此处不再判断.
    }
}

TEST_F(KMTestBuilder, BuildExport)
{
    prepareFinishData();

    std::vector<std::string> args;
    args.push_back("build-finish");
    args.push_back(m_directory);
    EXPECT_EQ(0u, finish(args));

    args.clear();
    args.push_back("build-export");
    args.push_back("--comp=gzip -Xcompression-level 9");
    args.push_back("--dest-dir=" + m_cacheDir);
    args.push_back(m_directory);
    EXPECT_EQ(0u, exports(args));
}

