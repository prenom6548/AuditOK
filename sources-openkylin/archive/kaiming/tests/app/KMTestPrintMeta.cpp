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

#include "KMTestPrintMeta.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMInfoJson.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "app/builder/KMBuildExport.h"
#include "app/print-meta/KMOKPrintMeta.h"

namespace fs =  std::filesystem;

void KMTestPrintMeta::SetUp()
{
    m_directory = KMStringUtils::buildFilename(fs::current_path().string(), "build");
    m_stateDir = KMStringUtils::buildFilename(fs::current_path().string(), ".kaiming-builder");

    KMFileUtils::mkpath(m_directory);
    KMFileUtils::mkpath(m_stateDir, 0777);

    prepareData();
}

void KMTestPrintMeta::TearDown()
{
    KMFileUtils::rmRfAll(m_directory);
    KMFileUtils::rmRfAll(m_stateDir);
}

void KMTestPrintMeta::prepareData()
{
    std::string kaiming = KMStringUtils::buildFilename(m_directory, "KAIMING");
    KMFileUtils::mkpath(kaiming);
    std::string changelog = KMStringUtils::buildFilename(kaiming, "changelog");
    KMFileUtils::syncWriteFile(changelog, "top.openkylin.hello (1.0.0) v11-2603-preview; urgency=medium\n"
                                          "  * BUG: #518255\n"
                                          " -- tom <tom@star.cn>  Wed, 8 Apr 2026 10:00:00 +0800\n");

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

void KMTestPrintMeta::findOk()
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

int KMTestPrintMeta::printMeta(std::vector<std::string>& args)
{
    std::vector<char *> argv;
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    KMOKPrintMeta meta;
    return meta.dispose(argv.size(), argv.data());
}

TEST_F(KMTestPrintMeta, printMeta)
{
    std::vector<std::string> args;
    args.push_back("print-meta");
    args.push_back(m_ok);
    EXPECT_EQ(0u, printMeta(args));
}

TEST_F(KMTestPrintMeta, printMeta_throw)
{
    std::vector<std::string> args;
    args.push_back("print-meta");
    try
    {
        printMeta(args);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    args.push_back("/tmp/akjlakjldkhlakdas/changelog");
    try
    {
        printMeta(args);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

