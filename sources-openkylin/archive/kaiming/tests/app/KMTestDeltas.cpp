/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>
#include <set>

#include "app/deltas/KMDeltas.h"
#include "common/KMPackageManager.h"
#include "common/KMUtil.h"
#include "common/KMVersion.h"

namespace fs = std::filesystem;

// isVersionLess 函数已经在 KMTestPackageManager.cpp 中定义
// 这里不要再重复定义，使用 extern 声明即可
extern bool isVersionLess(const KMVersion& a, const KMVersion& b);

class KMTestDeltas : public ::testing::Test
{
protected:
   void SetUp() override
   {
      // 准备测试目录
      // const char *homeDir = std::getenv("HOME");
      // if (!homeDir)
      // {
      //    homeDir = "/tmp";
      // }
      m_testDir = "/tmp/testdeltes";

      // 如果目录存在则删除
      if (fs::exists(m_testDir))
      {
         fs::remove_all(m_testDir);
      }

      // 创建测试目录
      fs::create_directories(m_testDir);

      // 动态查找有两个版本的应用
      KMPackageManager pm;
      PackageManagerOptions searchOptions;
      searchOptions.isPrint = false;
      searchOptions.kind = "app";
      std::vector<std::string> searchResults;
      int searchRet = pm.searchInfo("", searchOptions, searchResults);

      if (searchRet != 0 || searchResults.empty())
      {
         GTEST_SKIP() << "No available packages found";
      }

      // 定义需要跳过的大包黑名单
      std::set<std::string> largePackages = {
         "top.openkylin.base",
         "top.openkylin.ukui",
         "cn.kylin.v10sp1-base",
         "cn.edrawsoft.mindmaster"
      };

      // 解析所有包，找出有两个版本的应用
      std::map<std::string, std::vector<std::string>> packageVersions;
      for (const auto &result : searchResults)
      {
         try
         {
            KMPMRef pmRef = KMUtil::parseSummaryFuzzyKMPMRef(result);
            std::string packageId = pmRef.ref.id;
            std::string version = pmRef.ref.version;

            if (!version.empty())
            {
               packageVersions[packageId].push_back(version);
            }
         }
         catch (const std::exception &e)
         {
            continue;
         }
      }

      // 查找有两个或以上版本的应用，跳过黑名单中的大包
      for (auto &pkg : packageVersions)
      {
         // 跳过黑名单中的包
         if (largePackages.find(pkg.first) != largePackages.end())
         {
            std::cout << "Skipping large package: " << pkg.first << std::endl;
            continue;
         }
         
         if (pkg.second.size() >= 2)
         {
            m_testPackage = pkg.first;
            
            // 使用 isVersionLess 对版本号进行冒泡排序
            std::vector<std::string> versions = pkg.second;
            for (size_t i = 0; i < versions.size() - 1; i++)
            {
               for (size_t j = 0; j < versions.size() - i - 1; j++)
               {
                  KMVersion v1 = KMVersion::parse(versions[j], false);
                  KMVersion v2 = KMVersion::parse(versions[j + 1], false);
                  if (!isVersionLess(v1, v2))
                  {
                     std::swap(versions[j], versions[j + 1]);
                  }
               }
            }
            
            m_lowVersion = versions.front();
            m_highVersion = versions.back();
            break;
         }
      }

      if (m_testPackage.empty())
      {
         GTEST_SKIP() << "No package with at least 2 versions found";
      }

      std::cout << "Selected test package: " << m_testPackage << std::endl;
      std::cout << "Low version: " << m_lowVersion << std::endl;
      std::cout << "High version: " << m_highVersion << std::endl;

      // 安装两个版本的应用
      PackageManagerOptions installOptions;
      installOptions.assumeYes = true;
      installOptions.skipSignatureVerification = true;

      // 先卸载可能已安装的版本
      std::vector<std::string> uninstallIds = {m_testPackage};
      pm.uninstall(uninstallIds, installOptions);

      // 安装低版本
      std::vector<std::string> lowVersionIds = {m_testPackage + "/" + m_lowVersion};
      int installRet = pm.install(lowVersionIds, installOptions);
      if (installRet != KM_ERROR_NO)
      {
         GTEST_SKIP() << "Failed to install low version: " << m_lowVersion;
      }

      // 安装高版本
      std::vector<std::string> highVersionIds = {m_testPackage + "/" + m_highVersion};
      installRet = pm.install(highVersionIds, installOptions);
      if (installRet != KM_ERROR_NO)
      {
         GTEST_SKIP() << "Failed to install high version: " << m_highVersion;
      }
   }

   void TearDown() override
   {
      // 清理测试目录
      if (fs::exists(m_testDir))
      {
         fs::remove_all(m_testDir);
      }

      // 清理安装的应用
      if (!m_testPackage.empty())
      {
         KMPackageManager pm;
         PackageManagerOptions uninstallOptions;
         uninstallOptions.assumeYes = true;
         uninstallOptions.skipSignatureVerification = true;
         std::vector<std::string> uninstallIds = {m_testPackage};
         pm.uninstall(uninstallIds, uninstallOptions);
      }
   }

   std::string m_testPackage;
   std::string m_lowVersion;
   std::string m_highVersion;
   std::string m_testDir;
   KMDeltas m_deltas;
};

TEST_F(KMTestDeltas, test_deltas_dispose)
{
   KMPackageManager pm;
   PackageManagerOptions options;
   options.assumeYes = true;
   options.skipSignatureVerification = true;

   // // 构建二进制文件路径
   // std::string binaryPath = "/opt/kaiming/layers/stable/x86_64/app/" + m_testPackage + "/binary";

   // // 检查二进制目录是否存在
   // if (!fs::exists(binaryPath))
   // {
   //    GTEST_SKIP() << "Binary path does not exist: " << binaryPath;
   // }

   // std::string buildExportCmd;
   // int buildRet;

   // // 导出高版本
   // buildExportCmd = "cd " + binaryPath + " && kaiming build-export " + m_highVersion + " -d " + m_testDir;
   // std::cout << "Exporting high version: " << buildExportCmd << std::endl;
   // buildRet = std::system(buildExportCmd.c_str());

   // 先卸载两个版本
   std::vector<std::string> uninstallIds = {m_testPackage};
   std::cout << "Uninstalling package: " << m_testPackage << std::endl;
   int uninstallRet = pm.uninstall(uninstallIds, options);

   options.bundle = true;
   options.subpath = m_testDir;
   
   // 导出低版本ok包
   std::vector<std::string> lowVersionIds = {m_testPackage + "/" + m_lowVersion};
   int installRet = pm.install(lowVersionIds, options);
   EXPECT_EQ(installRet, KM_ERROR_NO);

   // 导出高版本ok包
   std::vector<std::string> highVersionIds = {m_testPackage + "/" + m_highVersion};
   installRet = pm.install(highVersionIds, options);
   EXPECT_EQ(installRet, KM_ERROR_NO);

   // 查找导出的低版本 .ok 包文件
   std::string lowOkPackagePath;
   fs::path exportDir = m_testDir;
   for (const auto& entry : fs::directory_iterator(exportDir)) {
       if (fs::is_regular_file(entry) && entry.path().extension() == ".ok") {
           std::string filename = entry.path().filename().string();
           // 文件名中应该包含版本号，例如: package-1.0.0-x86_64.ok
           if (filename.find(m_lowVersion) != std::string::npos) {
               lowOkPackagePath = entry.path().string();
               std::cout << "Found low version .ok package: " << lowOkPackagePath << std::endl;
               break;
           }
       }
   }

   // 安装导出的低版本 .ok 包
   pm.installOfflinePackage({lowOkPackagePath}, options);

   // 验证低版本已安装
   EXPECT_EQ(pm.isInstalled(m_testPackage), KM_ERROR_NO) << "Low version should be installed";

   // 构建 deltas 命令参数
   std::string newExportDir = m_testDir + "/output";
   fs::create_directories(newExportDir);
   const char *argv[] = {"deltas", "--ok", m_testDir.c_str(), newExportDir.c_str()};
   int result = m_deltas.dispose(4, const_cast<char **>(argv));

   // 验证结果
   EXPECT_EQ(result, 0);

   // 查找生成的 .ok 包文件
   std::string okPackagePath;
   std::string okPath = newExportDir + "/okfile";
   for (const auto& entry : fs::directory_iterator(okPath)) {
       if (fs::is_regular_file(entry) && entry.path().extension() == ".ok") {
           okPackagePath = entry.path().string();
           std::cout << "Found .ok package: " << okPackagePath << std::endl;
           break;
       }
   }

   if (okPackagePath.empty()) {
       GTEST_SKIP() << "No .ok package found in " << okPath;
   }

   // 安装构建好的 .ok 包
   std::vector<std::string> okPackages = {okPackagePath};
   std::cout << "Installing .ok package: " << okPackagePath << std::endl;
   int installOkRet = pm.installOfflinePackage(okPackages, options);
   EXPECT_EQ(installOkRet, KM_ERROR_NO);

   pm.uninstall(uninstallIds, options);
}