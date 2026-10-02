/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

/**
 * 环境准备：
 * sudo apt install erofs-tools erofsfuse
 * 安装：sudo kaiming install depend.openkylin.libzen-dev，top.openkylin.demo-x86_64-1.0.0-stable-binary-squashfs.ok
 * 将top.openkylin.demo-x86_64-1.0.0-stable-binary-squashfs-all.ok放到homepath/top.openkylin.demo/下
 */
#include "KMBuildInit.h"
#include "KMBuild.h"
#include "KMBuildFinish.h"
#include "KMBuildExport.h"
#include "KMContainerWrap.h"

#include "KMOABContext.h"
#include "KMOABPrintDataDir.h"
#include "KMOABPrintMeta.h"
#include "KMOABExtract.h"
#include "KMOABMount.h"
#include "KMOABRepackage.h"
#include "KMOABRun.h"
#include "KMOABUtils.h"
#include "KMOABElf.h"
#include "KMFolder.h"

#include "KMInfoJson.h"
#include "KMStringUtils.h"
#include "KMUtil.h"


#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace fs = std::filesystem;

#define private public

class KMTestBuildInit
{
public:
    KMTestBuildInit() = default;
    ~KMTestBuildInit() = default;

public:
    KMBuildInit m_buildinit;
};

class KMTestBuild
{
public:
    KMTestBuild() = default;
    ~KMTestBuild() = default;

public:
    KMBuild m_build;
};

class KMTestBuildFinish
{
public:
    KMTestBuildFinish() = default;
    ~KMTestBuildFinish() = default;

public:
    KMBuildFinish m_buildfinish;
};

class KMTestBuildExport {
public:
    KMTestBuildExport() = default;
    ~KMTestBuildExport() = default;

public:
    KMBuildExport m_buildExport;
};

class KMTestContainerWrap{
public:
    KMTestContainerWrap() = default;
    ~KMTestContainerWrap() = default;

public:
    KMContainerWrap m_containerWrap;
};

class KMTestOABPrintMeta
{
public:
    KMTestOABPrintMeta() = default;
    ~KMTestOABPrintMeta() = default;

public:
    KMOABPrintMeta m_oab_print_meta;
};

class KMTestOABPrintDataDir
{
public:
    KMTestOABPrintDataDir() = default;
    ~KMTestOABPrintDataDir() = default;

public:
    KMOABPrintDataDir m_oab_print_datadir;
};

class KMTestOABExtract
{
public:
    KMTestOABExtract() = default;
    ~KMTestOABExtract() = default;

public:
    KMOABExtract m_oab_extract;
};

class KMTestOABMount
{
public:
    KMTestOABMount() = default;
    ~KMTestOABMount() = default;

public:
    KMOABMount m_oab_mount;
};

class KMTestOABRepackage
{
public:
    KMTestOABRepackage() = default;
    ~KMTestOABRepackage() = default;

public:
    KMOABRepackage m_oab_repackage;
};

class KMTestOABRun
{
public:
    KMTestOABRun() = default;
    ~KMTestOABRun() = default;

public:
    KMOABRun m_oab_run;
};


TEST(KMTestBuildInit, test_builder_ok)
{
    KMTestContainerWrap test_wrap;
    std::string key = "TEST";
    std::string value = "$TEST";
    test_wrap.m_containerWrap.addEnv(key, value);

    std::string current_path = fs::current_path();
    std::string build_path = KMStringUtils::buildFilename(current_path,".kaiming-builder-testing");
    if (!fs::exists(build_path))
    {
        fs::create_directories(build_path);
    }
    std::string build_path_init = KMStringUtils::buildFilename(build_path,"build");

    //build_init app
    KMTestBuildInit testBuildInit;
    char * init_argv[] = 
    {
        const_cast<char*>("build-init"),
        const_cast<char*>("--arch=x86_64"),
        const_cast<char*>("--kind=app"),
        const_cast<char*>("--channel=stable"),
        const_cast<char*>("--base=stable:top.openkylin.base/1.0.0"),
        const_cast<char*>("--depend=depend.openkylin.zip"),
        const_cast<char*>("--version=0.0.1"),
        const_cast<char*>("--runtime=stable:top.openkylin.ukui/1.0.0"),
        const_cast<char*>(build_path_init.c_str()),
        const_cast<char*>("top.openkylin.demo"),
    };
    int init_argc = 10;
    EXPECT_EQ(testBuildInit.m_buildinit.dispose(init_argc, init_argv), 0);

    //build
    std::string build_path_build = KMStringUtils::buildFilename(build_path,"build");
    std::string bind_mount = "--bind-mount=/run/build/hello=";
    std::string bind_mount_path = KMStringUtils::buildFilename(build_path, ".kaiming-builder/sources/hello-1");
    bind_mount = bind_mount + bind_mount_path;

    //mkdir sources/component-name
    if ( !fs::exists(bind_mount_path))
    {
        fs::create_directories(bind_mount_path);
    }
    if ( !fs::exists(KMStringUtils::buildFilename(bind_mount_path, "hello.cpp")))
    {
        std::string filename = KMStringUtils::buildFilename(bind_mount_path, "hello.cpp");
        std::ofstream outfile(filename);
        std::string content = R"(
#include <iostream>
int main() {
std::cout << "Hello kaiming!" << std::endl;
return 0;
}
)";
        outfile << content;
        outfile.close();
    }
    if ( !fs::exists(KMStringUtils::buildFilename(bind_mount_path, "CMakeLists.txt")))
    {
        std::string filename = KMStringUtils::buildFilename(bind_mount_path, "CMakeLists.txt");
        std::ofstream outfile(filename);
        std::string content = "\
cmake_minimum_required(VERSION 3.28.3)\n\
project(kaiming-builder-demo)\n\
add_executable(kaiming-builder-demo hello.cpp)\n\
install(TARGETS kaiming-builder-demo)\n\
            ";
        outfile << content;
        outfile.close();
    }


    KMTestBuild testBuild;
    char * build_argv[] = 
    {
        const_cast<char*>("build"),
        const_cast<char*>("--env=KAIMING_BUILDER_BUILDDIR=/run/build/hello"),  
        const_cast<char*>("--env=TEST=$TEST"),  
        const_cast<char*>("-d"),     
        const_cast<char*>(bind_mount.c_str()),
        const_cast<char*>("--build-dir=/run/build/hello"),
        const_cast<char*>("--env=CCACHE_DIR=/run/ccache/disabled"),
        const_cast<char*>("--env=KAIMING_BUILDER_N_JOB=8"),
        const_cast<char*>(build_path_build.c_str()),
        const_cast<char*>("cmake"),
        const_cast<char*>("--"),
        const_cast<char*>("-DCMAKE_INSTALL_PREFIX:PATH=/usr"),
        const_cast<char*>("'-G Unix Makefiles'"),
        const_cast<char*>(".")
    };
    int build_argc = 14;
    EXPECT_EQ(testBuild.m_build.dispose(build_argc, build_argv), 0);
    //build_finish

    //mkdir build
    if ( !fs::exists(KMStringUtils::buildFilename(build_path_build, "files/bin/hello")))
    {
        fs::create_directories(KMStringUtils::buildFilename(build_path_build, "files/bin"));
        std::string filename = KMStringUtils::buildFilename(build_path_build, "files/bin/hello");
        std::ofstream outfile(filename);
        std::string content = R"(echo "Hello Kaiming!")";
        outfile << content;
        outfile.close();
        std::string command = "chmod +x " + filename;
        std::cout << filename << std::endl;
        system(command.c_str());
    }

     std::vector<std::string> paths = {
        "share/applications",                 /* Copy desktop files */
        "share/mime/packages",                /* Copy MIME Type files */
        "share/icons",                        /* Icons */
        "share/dbus-1/services",              /* D-Bus service files */
        "share/gnome-shell/search-providers", /* Search providers */
        "share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "share/metainfo",                     /* Copy appdata/metainfo files */
        "lib/systemd",                        /* .service, .target, .timer, .socket, .conf */
        "etc/systemd",                        /* .service, .target, .timer, .socket, .conf */

        // 下面/usr和/usr/local路径是为sysapp做的兼容
        "usr/share/applications",                 /* Copy desktop files */
        "usr/share/mime/packages",                /* Copy MIME Type files */
        "usr/share/icons",                        /* Icons */
        "usr/share/dbus-1/services",              /* D-Bus service files */
        "usr/share/gnome-shell/search-providers", /* Search providers */
        "usr/share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "usr/share/metainfo",                     /* Copy appdata/metainfo files */
        "usr/lib/systemd",                        /* .service, .target, .timer, .socket, .conf */

        "usr/local/share/applications",                 /* Copy desktop files */
        "usr/local/share/mime/packages",                /* Copy MIME Type files */
        "usr/local/share/icons",                        /* Icons */
        "usr/local/share/dbus-1/services",              /* D-Bus service files */
        "usr/local/share/gnome-shell/search-providers", /* Search providers */
        "usr/local/share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "usr/local/share/metainfo",                     /* Copy appdata/metainfo files */
        "usr/local/lib/systemd",                        /* .service, .target, .timer, .socket, .conf */
    };
    for (const std::string &path : paths)
    {
        fs::create_directories(KMStringUtils::buildFilename(build_path_build, "files/",path));
        std::string dest_path = KMStringUtils::buildFilename(build_path_build,"files/",path);
        std::string src_path = KMStringUtils::buildFilename(build_path_build, "files/bin/hello");
        std::string command = "cp " + src_path + "  " + dest_path;
        system(command.c_str());
    }

    KMInfoJson infoJson;
    std::string infoPath = KMStringUtils::buildFilename(build_path_build, "info.json");
    std::string arch = KMUtil::kmGetArch();
    infoJson.loadFile(infoPath);
    infoJson.version = "0.0.1";
    infoJson.channel = "stable";
    infoJson.module = "binary";
    infoJson.base = "stable:top.openkylin.base/1.0.0/" + arch;
    infoJson.runtime = "stable:top.openkylin.ukui/1.0.0/" + arch;
    
    infoJson.saveFile(infoPath);

    std::string build_path_finish = KMStringUtils::buildFilename(build_path, "build");

    KMTestBuildFinish testBuildFinish;
    char * finish_argv[] = 
    {
        const_cast<char*>("build-finish"),
        const_cast<char*>(build_path_finish.c_str()),
    };
    int finish_argc = 2;
    EXPECT_EQ(testBuildFinish.m_buildfinish.dispose(finish_argc, finish_argv), 0);

    std::string modulesPath = KMStringUtils::buildFilename(build_path_build, "modules.json");
    std::ofstream file(modulesPath);
    if (file.is_open()) 
    {
        file << "{\n";
        file << "    \"modules\": {\n";
        file << "        \"binary\": \".*\\n\",\n";
        file << "        \"lang-zh_CN\": \"/bin\\n\"\n";
        file << "    }\n";
        file << "}";
        file.close();
    } 
    else 
    {
        std::cerr << "Unable to open file: " << modulesPath << std::endl;
    }
    
    //build_export squashfs
    KMTestBuildExport testBuildExport;
    std::string icon_path = "--icon=";
    icon_path = icon_path + build_path;
    std::string dest_path = "--dest-dir=";
    dest_path = dest_path + build_path;
    std::string build_path_export = KMStringUtils::buildFilename(build_path, "build");
    char* export_argv[] = {
        const_cast<char*>("build-export"),
        const_cast<char*>(dest_path.c_str()),
        const_cast<char*>(icon_path.c_str()),
        const_cast<char*>("--bundle-filesystem=squashfs"),
        const_cast<char*>(build_path_export.c_str())
    };
    int export_argc = 5;
    EXPECT_EQ(testBuildExport.m_buildExport.dispose(export_argc, export_argv), 0);

    // build_export --all
    KMTestBuildExport testBuildExport_all;
    char* export_argv_all[] = {
        const_cast<char*>("build-export"),
        const_cast<char*>(dest_path.c_str()),
        const_cast<char*>(icon_path.c_str()),
        const_cast<char*>("--all"),
        const_cast<char*>("--bundle-filesystem=squashfs"),
        const_cast<char*>(build_path_export.c_str())
    };
    int export_argc_all = 6;

    EXPECT_EQ(testBuildExport_all.m_buildExport.test(export_argc_all, export_argv_all), 0);

    //build_export --ref-mode
    KMTestBuildExport testBuildExport_ref;
    char* export_argv2[] = {
        const_cast<char*>("build-export"),
        const_cast<char*>(dest_path.c_str()),
        const_cast<char*>(icon_path.c_str()), 
        const_cast<char*>("--bundle-filesystem=squashfs"),
        const_cast<char*>("--ref-mode"),
        const_cast<char*>("top.openkylin.demo")
    };
    int export_argc2 = 6;
    EXPECT_EQ(testBuildExport_ref.m_buildExport.dispose(export_argc2, export_argv2), 0);

    // build_export erofs
    KMTestBuildExport testBuildExport_erofs;
    char* export_argv1[] = {
        const_cast<char*>("build-export"),
        const_cast<char*>(dest_path.c_str()),
        const_cast<char*>(icon_path.c_str()),
        const_cast<char*>("--bundle-filesystem=erofs"),
        const_cast<char*>(build_path_export.c_str())
    };
    int export_argc1 = 5;
    EXPECT_EQ(testBuildExport_erofs.m_buildExport.dispose(export_argc1, export_argv1), 0);

    //oab_print_mate
    std::string appName = KMStringUtils::buildFilename(build_path, "top.openkylin.demo-x86_64-0.0.1-stable-binary-squashfs.ok");

    KMOABContext::instance().setAppName(appName);
    
    KMTestOABPrintMeta m_test_oab_print_meta;

    char * print_meta_argv[] = {
        const_cast<char*>("print-meta")
    };
    int print_meta_argc = 1;
    EXPECT_EQ(m_test_oab_print_meta.m_oab_print_meta.dispose(print_meta_argc,print_meta_argv), 0);

    //oab_print_data_dir

    KMTestOABPrintDataDir m_test_oab_print_datadir;
    int print_data_argc = 1;
    char * print_data_argv[] = {
        const_cast<char*>("print-data")
    };

    EXPECT_EQ(m_test_oab_print_datadir.m_oab_print_datadir.dispose(print_data_argc,print_data_argv), 0);

    //oab_extract squashfs
    KMTestOABExtract m_test_oab_extract;

    std::string extract_path = KMStringUtils::buildFilename(build_path, "extract_test");
    int extract_argc = 2;
    char * extract_argv[] = {
        const_cast<char*>("extract"),
        const_cast<char*>(extract_path.c_str())
    };

    EXPECT_EQ(m_test_oab_extract.m_oab_extract.dispose(extract_argc,extract_argv), 0);

    //oab_mount
    KMTestOABMount m_test_oab_mount;
    KMOABContext::instance().setAppName(appName);
    std::string mount_path = KMStringUtils::buildFilename(build_path, "mount_test");
    int mount_argc = 2;
    char * mount_argv[] = {
        const_cast<char*>("mount"),
        const_cast<char*>(mount_path.c_str())
    };

    EXPECT_EQ(m_test_oab_mount.m_oab_mount.dispose(mount_argc,mount_argv), 0);

    std::string umount_path = "umount " + mount_path;
    std::system(umount_path.c_str());

    //oab_repackage
    KMTestOABRepackage m_test_oab_repackage;

    std::string repackage_path = KMStringUtils::buildFilename(build_path, "extract_test");
    int repackage_argc = 2;
    char * repackage_argv[] = {
        const_cast<char*>("repackage"),
        const_cast<char*>(repackage_path.c_str())
    };

    EXPECT_EQ(m_test_oab_repackage.m_oab_repackage.dispose(repackage_argc, repackage_argv), 0);

    //oab_utils
    std::string test_path = KMStringUtils::buildFilename(build_path, ".test");
    KMOABUtils::mkpath(test_path);
    KMOABUtils::getPulseHome();
    KMOABUtils::getPulseMachineId();
    
    //oab_elf
    KMOABElf elf(appName);
    elf.test(build_path);
    
    //oab_run_squashfs
    std::string home = KMFolder::getHomePath();
    appName = KMStringUtils::buildFilename(home, "top.openkylin.demo/top.openkylin.demo-x86_64-1.0.0-stable-binary-squashfs-all.ok");
    KMOABContext::instance().setAppName(appName);
    KMTestOABRun m_test_oab_run;

    int run_argc = 1;
    char * run_argv[] = {
        const_cast<char*>("run"),
    };

    EXPECT_EQ(m_test_oab_run.m_oab_run.dispose(run_argc, run_argv), 0);

    //del .kaiming-builder-testing
    std::string del_build_path = "rm -rf " + build_path;
    std::system(del_build_path.c_str());
}

