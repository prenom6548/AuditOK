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
#include <iostream>

#define private public

#include "KMTestOK.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMInfoJson.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "app/builder/KMBuildExport.h"
#include "ok/engine/KMOABApplication.h"
#include "ok/engine/KMOABUtils.h"
#include "ok/engine/KMOABContext.h"
#include "ok/engine/KMOABMount.h"
#include "ok/engine/KMOABExtract.h"
#include "ok/engine/KMOABPrintDataDir.h"
#include "ok/engine/KMOABPrintMeta.h"
#include "ok/engine/KMOABRepackage.h"
#include "ok/engine/KMOABElf.h"

namespace fs =  std::filesystem;

void KMTestOK::SetUp()
{
    m_directory = KMStringUtils::buildFilename(fs::current_path().string(), "build");
    m_stateDir = KMStringUtils::buildFilename(fs::current_path().string(), ".kaiming-builder");

    KMOABUtils::rmRfAll(m_directory);
    KMOABUtils::rmRfAll(m_stateDir);

    KMOABUtils::mkpath(m_directory);
    KMOABUtils::mkpath(m_stateDir, 0777);
    KMOABUtils::mkpath("/tmp/.ok/mountpoint", 0777);

    prepareData();
}

void KMTestOK::TearDown()
{
    KMOABUtils::rmRfAll(m_directory);
    KMOABUtils::rmRfAll(m_stateDir);
    KMOABUtils::rmRfAll("/tmp/.ok/mountpoint");
}

void KMTestOK::prepareData()
{
    std::string kaiming = KMStringUtils::buildFilename(m_directory, "KAIMING");
    KMFileUtils::mkpath(kaiming);
    std::string changelog = KMStringUtils::buildFilename(kaiming, "changelog");
    KMFileUtils::syncWriteFile(changelog, "kaiming");

    std::string binDir = KMStringUtils::buildFilename(m_directory, "files/bin");
    KMFileUtils::mkpath(binDir);
    std::string content("#!/bin/bash\n\n"
                        "echo 'Hello world!'");
    std::string hello = KMStringUtils::buildFilename(binDir, "hello.sh");
    KMFileUtils::syncWriteFile(hello, content);
    fs::permissions(hello, fs::perms::all);

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
    info.permissions.all = true;
    std::string json = KMStringUtils::buildFilename(m_directory, "info.json");
    info.saveFile(json);

    std::vector<std::string> args;
    args.push_back("build-export");
    args.push_back("--dest-dir=" + m_stateDir);
    args.push_back(m_directory);

    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMBuildExport exports;
    exports.dispose(argv.size(), argv.data());

    findOk();
}

void KMTestOK::findOk()
{
    fs::path dir(m_stateDir);
    for (auto const &dirEntry : fs::directory_iterator{ dir })
    {
        fs::path p = dirEntry.path();
        std::string filename = p.filename();
        if (filename == "." || filename == "..")
        {
            continue ;
        }

        if (fs::is_directory(p))
        {
            continue ;
        }

        if (KMStringUtils::endsWith(filename, ".ok"))
        {
            m_ok = p.string();

            return ;
        }
    }
}

int KMTestOK::cmd(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOABApplication &a = KMOABApplication::instance((int)argv.size(), argv.data());
    a.reset((int)argv.size(), argv.data());
    a.init();

    int ret = a.exec();
    return ret;
}

int KMTestOK::mountOAB(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOABMount oab;
    return oab.dispose(argv.size(), argv.data());
}

int KMTestOK::extractOAB(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOABExtract oab;
    return oab.dispose(argv.size(), argv.data());
}

int KMTestOK::printDataDir(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOABPrintDataDir oab;
    return oab.dispose(argv.size(), argv.data());
}

int KMTestOK::printOABMeta(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOABPrintMeta oab;
    return oab.dispose(argv.size(), argv.data());
}

int KMTestOK::repackageOAB(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOABRepackage oab;
    return oab.dispose(argv.size(), argv.data());
}

TEST_F(KMTestOK, mountOAB)
{
    KMOABContext::instance().setAppName(m_ok);

    std::string mountPoint = KMStringUtils::buildFilename(m_stateDir, "mount-point");
    try
    {
        std::vector<std::string> args;
        args.push_back("mount");
        args.push_back(mountPoint);

        EXPECT_EQ(0u, mountOAB(args));

        EXPECT_FALSE(KMOABUtils::isEmptyDirectory(mountPoint));

        EXPECT_TRUE(KMOABUtils::spawn("umount", { mountPoint }));
    }
    catch(const std::exception& e)
    {
        // 不允许重复挂载
    }
}

// TEST_F(KMTestOK, extractOAB)
// {
//     KMOABContext::instance().setAppName(m_ok);

//     std::vector<std::string> args;
//     args.push_back("extract");
//     args.push_back(m_stateDir);
//     EXPECT_EQ(0u, extractOAB(args));
// }

TEST_F(KMTestOK, printDataDir)
{
    KMOABContext::instance().setAppName(m_ok);

    std::vector<std::string> args;
    args.push_back("data-dir");
    EXPECT_EQ(0u, printDataDir(args));
}

TEST_F(KMTestOK, printOABMeta)
{
    KMOABContext::instance().setAppName(m_ok);

    std::vector<std::string> args;
    args.push_back("print-meta");
    EXPECT_EQ(0u, printOABMeta(args));
}

TEST_F(KMTestOK, repackageOAB)
{
    KMOABContext::instance().setAppName(m_ok);

    std::vector<std::string> args;
    args.push_back("extract");
    args.push_back(m_stateDir);
    EXPECT_EQ(0u, extractOAB(args));

    args.clear();
    args.push_back("repackage");
    args.push_back(m_stateDir);
    EXPECT_EQ(0u, repackageOAB(args));
}

TEST_F(KMTestOK, repackageOAB_erofs)
{
    KMOABContext::instance().setAppName(m_ok);

    std::vector<std::string> args;
    args.push_back("extract");
    args.push_back(m_stateDir);
    EXPECT_EQ(0u, extractOAB(args));

    try
    {
        args.clear();
        args.push_back("repackage");
        args.push_back("--bundle-filesystem=erofs");
        args.push_back(m_stateDir);
        repackageOAB(args);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

TEST_F(KMTestOK, OABElf)
{
    {
        KMOABElf elf(m_ok);
        elf.open();

        std::string mountPoint = KMStringUtils::buildFilename(m_stateDir, "mount-point");
        elf.mountBundle(mountPoint);
    }

    {
        KMOABElf noelf("asdfasdfasljlki");
        EXPECT_FALSE(noelf.open());
        noelf.errorMsg();
        noelf.getSection("noexists");
    }

    {
        KMOABElf elf(m_ok);
        EXPECT_TRUE(elf.open());
        elf.getSection("noexists");

        KMOABMetadata meta = elf.metadata();
        meta.dump();

        std::string metaFile = KMStringUtils::buildFilename(m_stateDir, "tmp", "meta.json");
        meta.saveFile(metaFile);

        std::string mountPoint = KMStringUtils::buildFilename(m_stateDir, "mount-point");
        elf.mountBundle(mountPoint);
        elf.mountErofsBundle();
        
        KMOABUtils::spawn("umount", { mountPoint });
    }
}

TEST_F(KMTestOK, TestKMOABApplication)
{
    std::vector<std::string> args;
    args.push_back(m_ok);
    args.push_back("-v");
    args.push_back("--");
    args.push_back("-l");
    EXPECT_EQ(1u, cmd(args));

    args.clear();
    args.push_back(m_ok);
    args.push_back("--oab-mount=");
    EXPECT_EQ(1u, cmd(args));

    args.clear();
    args.push_back(m_ok);
    args.push_back("--print-meta");
    // 注意：oab包的子命令和kaiming的子命令print-meta冲突
    cmd(args);

    args.clear();
    args.push_back(m_ok);
    args.push_back("--oab-extract=" + m_stateDir);
    EXPECT_EQ(0u, cmd(args));
}

