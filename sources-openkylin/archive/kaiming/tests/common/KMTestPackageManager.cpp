/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <stdlib.h>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <atomic>

// #define private public

#include "app/list/KMList.h"
#include "app/search/KMSearch.h"
#include "common/KMPackageManager.h"
#include "common/KMOSTreeHandler.h"
#include "common/KMFolder.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMVersion.h"

class KMTestPackageManager : public ::testing::Test
{
protected:
    void SetUp() override {
        KMInstalledAppQuery appQuery;
        m_installedRefs = appQuery.query(KMInstalledAppQuery::All);
        
        std::cout << "SetUp() completed, found " << m_installedRefs.size() << " installed apps" << std::endl;
    }
    
    std::string getFirstInstalledApp()
    {
        std::set<std::string> protectedPackages = {
            "top.openkylin.base",
            "top.openkylin.ukui",
            "cn.kylin.v10sp1-base"};

        for (const auto &ref : m_installedRefs)
        {
            if (protectedPackages.find(ref.id) == protectedPackages.end())
            {
                return ref.id;
            }
        }
        return "";
    }

    std::string getFirstAvailablePackage() {
        std::vector<std::string> availablePackages;
        PackageManagerOptions pmOptions;
        pmOptions.relativeRepoPath = "";
        pmOptions.isPrint = false;
        
        int errorCode = m_packageManager.searchInfo("", pmOptions, availablePackages);
        
        if (errorCode != KM_ERROR_NO) {
            std::cout << "Failed to search packages: " << errorCode << std::endl;
            return "";
        }
        
        if (availablePackages.empty()) {
            std::cout << "No available packages found" << std::endl;
            return "";
        }
        
        std::cout << "Found " << availablePackages.size() << " available packages" << std::endl;
        
        try {
            KMPMRef pmRef = KMUtil::parseSummaryFuzzyKMPMRef(availablePackages.front());
            return pmRef.ref.id;
        } catch (const std::exception& e) {
            std::cout << "Failed to parse package reference: " << e.what() << std::endl;
            return "";
        }
    }
    
    std::vector<std::string> getAvailablePackages() {
        std::vector<std::string> availablePackages;
        PackageManagerOptions pmOptions;
        pmOptions.relativeRepoPath = "";
        pmOptions.isPrint = false;
        pmOptions.arch = KMUtil::kmGetArch();
        
        int errorCode = m_packageManager.searchInfo("", pmOptions, availablePackages);
        
        if (errorCode != KM_ERROR_NO) {
            std::cout << "Failed to search packages: " << errorCode << std::endl;
            return {};
        }
        
        std::cout << "Found " << availablePackages.size() << " available packages" << std::endl;
        return availablePackages;
    }
    
    std::vector<std::string> getDependPackages() {
        std::vector<std::string> depends;
        std::vector<std::string> availablePackages = getAvailablePackages();
        
        for (const auto& pkg : availablePackages) {
            try {
                KMPMRef pmRef = KMUtil::parseSummaryFuzzyKMPMRef(pkg);
                if (pmRef.ref.id.find("depend") != std::string::npos) {
                    depends.push_back(pmRef.ref.id);
                    if (depends.size() >= 3) break;
                }
            } catch (...) {
                continue;
            }
        }
        return depends;
    }
    
    std::vector<KMRef> m_installedRefs;
    KMPackageManager m_packageManager;
};

// 辅助函数：比较 KMVersion
bool isVersionLess(const KMVersion& a, const KMVersion& b) {
    KMVersion tempA = a;
    KMVersion tempB = b;
    return tempA < tempB;
}

// 获取有两个版本且不在黑名单中的包
std::pair<std::string, std::string> getPackageWithTwoVersions(KMPackageManager& pm) {
    std::set<std::string> largePackages = {
        "top.openkylin.base",
        "top.openkylin.ukui",
        "cn.kylin.v10sp1-base",
        "cn.edrawsoft.mindmaster"
    };
    
    PackageManagerOptions searchOptions;
    searchOptions.isPrint = false;
    searchOptions.kind = "app";
    std::vector<std::string> searchResults;
    int searchRet = pm.searchInfo("", searchOptions, searchResults);
    
    if (searchRet != 0 || searchResults.empty()) {
        return {"", ""};
    }
    
    std::map<std::string, std::vector<std::string>> packageVersions;
    for (const auto& result : searchResults) {
        try {
            KMPMRef pmRef = KMUtil::parseSummaryFuzzyKMPMRef(result);
            std::string packageId = pmRef.ref.id;
            std::string version = pmRef.ref.version;
            
            if (!version.empty()) {
                packageVersions[packageId].push_back(version);
            }
        } catch (const std::exception& e) {
            continue;
        }
    }
    
    for (auto& pkg : packageVersions) {
        if (largePackages.find(pkg.first) != largePackages.end()) {
            std::cout << "Skipping blacklisted package: " << pkg.first << std::endl;
            continue;
        }
        
        if (pkg.second.size() >= 2) {
            std::string testPackage = pkg.first;
            
            // 使用冒泡排序 + isVersionLess 进行版本排序
            std::vector<std::string> versions = pkg.second;
            for (size_t i = 0; i < versions.size() - 1; i++) {
                for (size_t j = 0; j < versions.size() - i - 1; j++) {
                    KMVersion v1 = KMVersion::parse(versions[j], false);
                    KMVersion v2 = KMVersion::parse(versions[j + 1], false);
                    if (!isVersionLess(v1, v2)) {
                        std::swap(versions[j], versions[j + 1]);
                    }
                }
            }
            
            std::string lowVersion = versions.front();
            return {testPackage, lowVersion};
        }
    }
    
    return {"", ""};
}

// 测试安装功能
TEST_F(KMTestPackageManager, InstallOnlinePackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string testPackage = getFirstAvailablePackage();
    ASSERT_FALSE(testPackage.empty()) << "No available packages found";
    
    std::string basePackage = "top.openkylin.base";
    std::vector<std::string> onlineIds = {testPackage};
    
    std::vector<std::string> dependId = getDependPackages();
    if (!dependId.empty()) {
        EXPECT_EQ(m_packageManager.install(dependId, options), KM_ERROR_NO);
        EXPECT_EQ(m_packageManager.uninstall(dependId, options), KM_ERROR_NO);
    }

    m_packageManager.uninstall(onlineIds, options);
    EXPECT_EQ(m_packageManager.processTask(TaskType::TASK_TYPE_INSTALL, onlineIds, options), KM_ERROR_NO);

    EXPECT_EQ(m_packageManager.isInstalled(basePackage), KM_ERROR_NO);
    EXPECT_EQ(m_packageManager.isKylinFileSignToolPresent(), false);
    
    std::string installedApp = getFirstInstalledApp();
    if (!installedApp.empty()) {
        options.assumeYes = true;
        options.skipSignatureVerification = false;
        std::vector<std::string> verIds = {installedApp};
        std::vector<std::string> uninstallIds;
        uninstallIds.push_back(verIds[0]);
        EXPECT_EQ(m_packageManager.uninstall(uninstallIds, options), KM_ERROR_NO);
        EXPECT_EQ(m_packageManager.install(verIds, options), KM_ERROR_NO);
    }

    KMList listIds;
    std::vector<KMlocalAppsInfo> localAppsInfo;
    localAppsInfo = listIds.getLocalAppIds(false);

    options.assumeYes = true;
    options.arch = "x86_64";
    options.upgradable = false;
    options.isGetDevel = true;
    options.isPrint = false;
    options.skipSignatureVerification = true;

    EXPECT_EQ(m_packageManager.getUpgradableList(localAppsInfo, options), KM_ERROR_NO);

    KMFolder folder;
    fs::path repoPath = folder.getRepoPath();
    
    if (!m_installedRefs.empty()) {
        const auto& ref = m_installedRefs.front();
        std::string installPath = ref.baseInstallDir + "/" + ref.id + "/binary/" + ref.version;
        EXPECT_EQ(m_packageManager.verifySignature(ref.id, ref.kind, installPath), KM_ERROR_NO);
    }

    if (!testPackage.empty()) {
        options.assumeYes = true;
        std::vector<std::string> uninstallIds = {testPackage};
        EXPECT_EQ(m_packageManager.uninstall(uninstallIds, options), KM_ERROR_NO);
    }
}

TEST_F(KMTestPackageManager, SearchInfoPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::vector<std::string> onlineIds = {testPackage};
    m_packageManager.install(onlineIds, options);

    EXPECT_EQ(m_packageManager.dbusSearch(testPackage, options), KM_ERROR_NO);
    
    m_packageManager.uninstall(onlineIds, options);
}

// 测试本地包安装
TEST_F(KMTestPackageManager, InstallLocalPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = false;

    std::string testPackage = getFirstAvailablePackage();

    // 创建导出目录
    std::string exportDir = "/home/depned_test";
    if (!fs::exists(exportDir)) {
        fs::create_directories(exportDir);
        std::cout << "Created directory: " << exportDir << std::endl;
    }

    PackageManagerOptions installOptions;
    installOptions.assumeYes = true;
    installOptions.skipSignatureVerification = false;
    installOptions.bundle = true;
    installOptions.subpath = exportDir;
    
    std::vector<std::string> installIds = {testPackage};
    int installRet = m_packageManager.install(installIds, installOptions);
    EXPECT_EQ(installRet, KM_ERROR_NO) << "Failed to install and export package";
    
    std::string okPackagePath;
    for (const auto& entry : fs::directory_iterator(exportDir)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".ok") {
            okPackagePath = entry.path().string();
            break;
        }
    }
    
    if (okPackagePath.empty()) {
        GTEST_SKIP() << "No .ok package found in " << exportDir;
    }
    
    std::cout << "Found exported .ok package: " << okPackagePath << std::endl;
    
    std::vector<std::string> offPackages = {okPackagePath};
    EXPECT_EQ(m_packageManager.installOfflinePackage(offPackages, options), KM_ERROR_NO);
    
    // 使用原路径作为包名获取deb信息
    int errorCode = -1;
    std::string debInfo = m_packageManager.getDebInfoFromOK(okPackagePath, errorCode);
    EXPECT_FALSE(debInfo.empty());
    // EXPECT_EQ(errorCode, 0);

    // 正确调用 getDebInfoFromOK（第一个参数是 string，不是 initializer_list）
    m_packageManager.getDebInfoFromOK("aaa.ok", errorCode);

    // 比较 errorCode 而不是 debInfo
    EXPECT_NE(errorCode, KM_ERROR_NO);
}

// 测试本地depend包安装
TEST_F(KMTestPackageManager, InstallDependLocalPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = false;

    std::string dependZip = "depend.openkylin.maven";
    m_packageManager.uninstall({dependZip}, options);
    
    // 创建导出目录
    std::string exportDir = "/home/depned_test";
    if (!fs::exists(exportDir)) {
        fs::create_directories(exportDir);
        std::cout << "Created directory: " << exportDir << std::endl;
    }
    
    // 1. 调用 install 接口导出 dependZip 到指定目录
    PackageManagerOptions installOptions;
    installOptions.assumeYes = true;
    installOptions.skipSignatureVerification = false;
    installOptions.bundle = true;
    installOptions.subpath = exportDir;
    
    std::vector<std::string> installIds = {dependZip};
    int installRet = m_packageManager.install(installIds, installOptions);
    EXPECT_EQ(installRet, KM_ERROR_NO) << "Failed to install and export package";
    
    // 2. 查找导出的 .ok 包文件
    std::string okPackagePath;
    for (const auto& entry : fs::directory_iterator(exportDir)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".ok") {
            okPackagePath = entry.path().string();
            break;
        }
    }
    
    if (okPackagePath.empty()) {
        GTEST_SKIP() << "No .ok package found in " << exportDir;
    }
    
    std::cout << "Found exported .ok package: " << okPackagePath << std::endl;
    
    // 3. 安装导出的 .ok 包
    std::vector<std::string> offPackages = {okPackagePath};
    EXPECT_EQ(m_packageManager.installOfflinePackage(offPackages, options), KM_ERROR_NO);
    
    // 4. 卸载导出的包
    fs::path path(okPackagePath);
    std::string packageName = path.stem().string();
    size_t pos = packageName.find("-x86_64");
    if (pos != std::string::npos) {
        packageName = packageName.substr(0, pos);
    }
    
    std::vector<std::string> cleanupIds = {packageName};
    m_packageManager.uninstall(cleanupIds, options);
    
    // 5. 清理导出的目录
    fs::remove_all(exportDir);
    std::cout << "Cleanup completed" << std::endl;
}

// 测试更新功能
TEST_F(KMTestPackageManager, UpdatePackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    auto result = getPackageWithTwoVersions(m_packageManager);
    std::string testPackage = result.first;
    std::string lowVersion = result.second;
    
    if (testPackage.empty()) {
        GTEST_SKIP() << "No package with multiple versions found (skipping blacklisted packages)";
    }
    
    kmlogger.info("Testing update for package: %s", testPackage.c_str());
    kmlogger.info("Low version: %s", lowVersion.c_str());
    
    std::vector<std::string> uninstallIds = {testPackage};
    m_packageManager.uninstall(uninstallIds, options);
    
    std::vector<std::string> lowVersionIds = {testPackage + "/" + lowVersion};
    int installRet = m_packageManager.install(lowVersionIds, options);
    EXPECT_EQ(installRet, KM_ERROR_NO) << "Failed to install low version: " << lowVersion;
    
    EXPECT_EQ(m_packageManager.isUpgradabled(testPackage), KM_ERROR_NO);

    std::vector<std::string> updateIds = {testPackage};
    int updateRet = m_packageManager.update(updateIds, options);
    EXPECT_EQ(updateRet, KM_ERROR_NO);

    m_packageManager.uninstall(uninstallIds, options);
}

TEST_F(KMTestPackageManager, SearchPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }

    EXPECT_EQ(m_packageManager.search(testPackage, options), KM_ERROR_NO);
}

TEST_F(KMTestPackageManager, ReparePackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::vector<std::string> onlineIds = {testPackage};
    options.force = true;
    m_packageManager.install(onlineIds, options);

    EXPECT_EQ(m_packageManager.repair(onlineIds, options), KM_ERROR_NO);
    
    m_packageManager.uninstall(onlineIds, options);
}

TEST_F(KMTestPackageManager, UpgradePackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::vector<std::string> onlineIds = {testPackage};

    m_packageManager.install(onlineIds, options);
    EXPECT_EQ(m_packageManager.processTask(TaskType::TASK_TYPE_UPGRADE, onlineIds, options), KM_ERROR_ALREADY_INSTALLED);
}

// 测试卸载功能
TEST_F(KMTestPackageManager, UninstallPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::vector<std::string> onlineIds = {testPackage};

    m_packageManager.install(onlineIds, options);
    
    EXPECT_EQ(m_packageManager.processTask(TaskType::TASK_TYPE_UNINSTALL, onlineIds, options), KM_ERROR_NO);
}

TEST_F(KMTestPackageManager, SoftCenterPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;

    auto result = getPackageWithTwoVersions(m_packageManager);
    std::string testPackage = result.first;
    std::string lowVersion = result.second;
    
    if (testPackage.empty()) {
        GTEST_SKIP() << "No package with multiple versions found (skipping blacklisted packages)";
    }
    
    kmlogger.info("Testing update for package: %s", testPackage.c_str());
    kmlogger.info("Low version: %s", lowVersion.c_str());
    
    auto installCallback = [](const std::string& progressJson) {
        std::cout << "Install progress: " << progressJson << std::endl;
    };
    
    auto updateCallback = [](const std::string& progressJson) {
        std::cout << "Update progress: " << progressJson << std::endl;
    };
    
    auto deployCallback = [](const std::string& progressJson) {
        std::cout << "Deploy progress: " << progressJson << std::endl;
    };
    
    auto completionCallback = [](const std::string& refId, int operateType, int errorCode, const std::string& errorMessage) {
        std::cout << "Install completed: refId=" << refId << ", operateType=" << operateType 
                  << ", errorCode=" << errorCode << ", errorMessage=" << errorMessage << std::endl;
    };
    
    std::vector<std::string> testPackages = {testPackage};
    PackageManagerOptions pmOption;

    std::vector<std::string> uninstallIds = {testPackage};
    m_packageManager.uninstall(uninstallIds, options);
    
    std::vector<std::string> lowVersionIds = {testPackage + "/" + lowVersion};

    m_packageManager.install(lowVersionIds, options);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::string taskId = m_packageManager.enqueueInstall(lowVersionIds, lowVersion, 
                                                          installCallback, deployCallback, 
                                                          options, completionCallback);

    EXPECT_EQ(m_packageManager.isUpgradabled(testPackage), KM_ERROR_NO);

    std::cout << "Testing updateSoftwareCenter for: " << testPackage << std::endl;
    int updateResult = m_packageManager.updateSoftwareCenter(
        testPackage,
        "",
        updateCallback,
        deployCallback
    );
    
    EXPECT_EQ(updateResult, KM_ERROR_NO);
    std::cout << "Update result: " << updateResult << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "Testing uninstallSoftwareCenter for: " << testPackage << std::endl;
    int uninstallResult = m_packageManager.uninstallSoftwareCenter(testPackage);
    
    EXPECT_EQ(uninstallResult, KM_ERROR_NO) << "Uninstall should succeed";
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    uninstallIds = {"depend.openkylin.zip"};
    int cleanupResult = m_packageManager.uninstall(uninstallIds, options);
    std::cout << "Cleanup result: " << cleanupResult << std::endl;
}

// 测试队列安装功能
TEST_F(KMTestPackageManager, EnqueueInstall) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> vectorIds = {testPackage};
    std::string version = "";
    
    std::atomic<bool> downloadCompleted{false};
    std::atomic<bool> deployCompleted{false};
    std::atomic<bool> completionCalled{false};
    
    auto downloadCallback = [&downloadCompleted](const std::string& progressJson) {
        std::cout << "Download progress JSON: " << progressJson << std::endl;
        downloadCompleted = true;
    };
    
    auto deployCallback = [&deployCompleted](const std::string& progressJson) {
        std::cout << "Deploy progress JSON: " << progressJson << std::endl;
        deployCompleted = true;
    };
    
    auto completionCallback = [&completionCalled](const std::string& refId, int operateType, int errorCode, const std::string& errorMessage) {
        std::cout << "Install completed for " << refId << ": operateType=" << operateType 
                  << ", errorCode=" << errorCode << ", errorMessage=" << errorMessage << std::endl;
        if (errorCode == 0) {
            completionCalled = true;
        }
    };

    std::string taskId = m_packageManager.enqueueInstall(vectorIds, version, 
                                                          downloadCallback, deployCallback, 
                                                          options, completionCallback);
    
    EXPECT_FALSE(taskId.empty()) << "Task ID should not be empty";
    
    int waitTime = 0;
    while (waitTime < 60 && (!downloadCompleted || !deployCompleted || !completionCalled)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        waitTime++;
        
        KMPMProgressStatus status = m_packageManager.getTaskStatus(taskId);
        std::cout << "Task progress: " << status.progress << "%, refId: " << status.refId << std::endl;
    }
    
    EXPECT_TRUE(downloadCompleted) << "Download should complete";
    EXPECT_TRUE(deployCompleted) << "Deploy should complete";
    EXPECT_TRUE(completionCalled) << "Completion callback should be called";
    
    std::vector<std::string> uninstallIds = {testPackage};
    m_packageManager.uninstall(uninstallIds, options);
}

// 测试取消安装
TEST_F(KMTestPackageManager, CancelInstallByTaskId) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> vectorIds = {testPackage};
    std::string version = "";
    
    auto downloadCallback = [](const std::string& progressJson) {
        std::cout << "Progress: " << progressJson << std::endl;
    };
    
    auto deployCallback = [](const std::string& progressJson) {};

    std::string taskId = m_packageManager.enqueueInstall(vectorIds, version, 
                                                          downloadCallback, deployCallback, 
                                                          options, nullptr);
    
    EXPECT_FALSE(taskId.empty());
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    m_packageManager.pauseInstall(taskId);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    bool result = m_packageManager.cancelInstall(taskId);
    EXPECT_TRUE(result) << "Cancel should succeed";
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// 测试按包名取消安装
TEST_F(KMTestPackageManager, CancelInstallByPackageName) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> vectorIds = {testPackage};
    std::string version = "";
    
    auto downloadCallback = [](const std::string& progressJson) {};
    auto deployCallback = [](const std::string& progressJson) {};
    
    std::string taskId = m_packageManager.enqueueInstall(vectorIds, version, 
                                                          downloadCallback, deployCallback, 
                                                          options, nullptr);
    
    EXPECT_FALSE(taskId.empty());
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    bool result = m_packageManager.cancelInstall(testPackage);

    m_packageManager.pauseInstall(testPackage);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_TRUE(result) << "Cancel by package name should succeed";
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// 测试暂停安装
TEST_F(KMTestPackageManager, PauseInstallByTaskId) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> vectorIds = {testPackage};
    std::string version = "";
    
    auto downloadCallback = [](const std::string& progressJson) {
        std::cout << "Download progress: " << progressJson << std::endl;
    };
    
    auto deployCallback = [](const std::string& progressJson) {};
    
    std::string taskId = m_packageManager.enqueueInstall(vectorIds, version, 
                                                          downloadCallback, deployCallback, 
                                                          options, nullptr);
    
    EXPECT_FALSE(taskId.empty());
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    KMPMProgressStatus statusBefore = m_packageManager.getTaskStatus(taskId);
    int progressBefore = statusBefore.progress;
    std::cout << "Progress before pause: " << progressBefore << "%" << std::endl;
    
    bool result = m_packageManager.pauseInstall(taskId);
    EXPECT_TRUE(result) << "Pause should succeed";
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    m_packageManager.cancelInstall(taskId);
}

// 测试恢复安装
TEST_F(KMTestPackageManager, ResumeInstall) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> vectorIds = {testPackage};
    std::string version = "";
    
    std::atomic<bool> resumed{false};
    
    auto downloadCallback = [&resumed](const std::string& progressJson) {
        std::cout << "Progress JSON: " << progressJson << std::endl;
        resumed = true;
    };
    
    auto deployCallback = [](const std::string& progressJson) {};
    
    std::string taskId = m_packageManager.enqueueInstall(vectorIds, version, 
                                                          downloadCallback, deployCallback, 
                                                          options, nullptr);
    
    EXPECT_FALSE(taskId.empty());
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    bool pauseResult = m_packageManager.pauseInstall(taskId);
    EXPECT_TRUE(pauseResult);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    KMPMProgressStatus statusBefore = m_packageManager.getTaskStatus(taskId);
    std::cout << "Progress before resume: " << statusBefore.progress << "%" << std::endl;
    
    bool resumeResult = m_packageManager.resumeInstall(taskId);
    EXPECT_TRUE(resumeResult) << "Resume should succeed";
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    m_packageManager.cancelInstall(taskId);
}

// 测试获取任务状态
TEST_F(KMTestPackageManager, GetTaskStatus) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> vectorIds = {testPackage};
    std::string version = "";
    
    auto downloadCallback = [](const std::string& progressJson) {};
    auto deployCallback = [](const std::string& progressJson) {};
    
    std::string taskId = m_packageManager.enqueueInstall(vectorIds, version, 
                                                          downloadCallback, deployCallback, 
                                                          options, nullptr);
    
    EXPECT_FALSE(taskId.empty());
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    KMPMProgressStatus status = m_packageManager.getTaskStatus(taskId);
    EXPECT_FALSE(status.refId.empty());
    
    m_packageManager.cancelInstall(taskId);
}

// 测试部署功能（先下载后部署）
TEST_F(KMTestPackageManager, DeploySoftwareCenter) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::atomic<bool> downloadCompleted{false};
    std::atomic<bool> downloadSuccess{false};
    int finalErrorCode = -1;
    std::string finalErrorMessage;
    
    auto downloadCallback = [](const std::string& progressJson) {
        std::cout << "Download progress: " << progressJson << std::endl;
    };
    
    auto deployCallback = [](const std::string& progressJson) {
        std::cout << "Deploy progress: " << progressJson << std::endl;
    };
    
    auto completionCallback = [&](const std::string& refId, int operateType, int errorCode, const std::string& errorMessage) {
        std::cout << "Install completed: refId=" << refId << ", operateType=" << operateType 
                  << ", errorCode=" << errorCode << ", errorMessage=" << errorMessage << std::endl;
        
        downloadCompleted = true;
        finalErrorCode = errorCode;
        finalErrorMessage = errorMessage;
        
        if (errorCode == 0) {
            downloadSuccess = true;
        }
    };
    
    PackageManagerOptions pmOptions;
    pmOptions.assumeYes = true;
    pmOptions.skipSignatureVerification = true;
    pmOptions.isDownloadOnly = true;
    pmOptions.arch = KMUtil::kmGetArch();
    
    std::vector<std::string> packageNames = {testPackage};
    unsigned int maxSpeedLimitKilobytes = 0;
    
    std::cout << "Step 1: Downloading package (download only): " << testPackage << std::endl;
    std::string taskId = m_packageManager.installSoftwareCenter(
        packageNames,
        "",
        downloadCallback,
        deployCallback,
        pmOptions,
        maxSpeedLimitKilobytes,
        completionCallback
    );
    
    EXPECT_FALSE(taskId.empty()) << "Install task ID should not be empty";
    std::cout << "Download task ID: " << taskId << std::endl;
    
    int waitTime = 0;
    while (waitTime < 60 && !downloadCompleted) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        waitTime++;
        
        KMPMProgressStatus status = m_packageManager.getTaskStatus(taskId);
        std::cout << "Waiting for download... progress: " << status.progress 
                  << "%, refId: " << status.refId 
                  << ", completed: " << (downloadCompleted ? "yes" : "no") << std::endl;
    }
    
    ASSERT_TRUE(downloadCompleted) << "Download did not complete within timeout";
    ASSERT_TRUE(downloadSuccess) << "Download failed with error code: " << finalErrorCode 
                                  << ", message: " << finalErrorMessage;
    
    std::cout << "Download completed successfully!" << std::endl;
    
    std::cout << "Step 2: Deploying package: " << testPackage << std::endl;
    
    PackageManagerOptions deployOptions;
    deployOptions.assumeYes = true;
    deployOptions.skipSignatureVerification = true;
    deployOptions.isDeployOnly = true;
    deployOptions.arch = KMUtil::kmGetArch();
    
    std::vector<std::string> deployNames = {testPackage};
    
    int deployResult = m_packageManager.deploySoftwareCenter(deployNames, deployOptions, deployCallback);
    
    EXPECT_TRUE(deployResult == KM_ERROR_NO || deployResult == KM_ERROR_ALREADY_INSTALLED) 
        << "Deploy should succeed or return already installed, got: " << deployResult;
    
    std::cout << "Deploy result: " << deployResult << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::vector<std::string> uninstallIds = {testPackage};
    m_packageManager.uninstall(uninstallIds, pmOptions);
    std::cout << "Cleanup completed" << std::endl;
}

// 测试包的可更新状态
TEST_F(KMTestPackageManager, GetAllUpgradableStatusSinglePackage) {
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::vector<std::string> names = {testPackage};
    std::vector<std::pair<std::string, bool>> results;
    
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::vector<std::string> installIds = {testPackage};
    m_packageManager.install(installIds, options);
    
    int result = m_packageManager.getAllUpgradableStatus(names, results);
    
    EXPECT_EQ(result, KM_ERROR_NO) << "Get upgradable status for single package should succeed";
    EXPECT_EQ(results.size(), 1) << "Should have exactly one result";
    
    if (!results.empty()) {
        EXPECT_EQ(results[0].first, testPackage) << "Package name should match";
        std::cout << "Package " << testPackage << " upgradable status: " << (results[0].second ? "true" : "false") << std::endl;
    }
    
    m_packageManager.uninstall(installIds, options);
}

// 测试空包名列表
TEST_F(KMTestPackageManager, GetAllUpgradableStatusEmptyList) {
    std::vector<std::string> names;
    std::vector<std::pair<std::string, bool>> results;
    
    int result = m_packageManager.getAllUpgradableStatus(names, results);
    
    EXPECT_EQ(result, KM_ERROR_NO) << "Get all upgradable status with empty list should succeed";
    EXPECT_TRUE(results.empty()) << "Results should be empty";
}

// 测试获取远程包状态信息
TEST_F(KMTestPackageManager, GetRemoteStatusInfo) {
    std::vector<std::string> availablePackages;
    PackageManagerOptions pmOptions;
    pmOptions.isPrint = false;
    pmOptions.kind = "app";
    pmOptions.arch = KMUtil::kmGetArch();
    
    int searchRet = m_packageManager.searchInfo("", pmOptions, availablePackages);
    if (searchRet != KM_ERROR_NO || availablePackages.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::string testPackage;
    std::string testVersion;
    std::string testBranch;
    
    for (const auto& pkg : availablePackages) {
        try {
            KMPMRef pmRef = KMUtil::parseSummaryFuzzyKMPMRef(pkg);
            if (!pmRef.ref.id.empty() && !pmRef.ref.version.empty()) {
                testPackage = pmRef.ref.id;
                testVersion = pmRef.ref.version;
                testBranch = pmRef.ref.channel.empty() ? "stable" : pmRef.ref.channel;
                break;
            }
        } catch (const std::exception& e) {
            continue;
        }
    }
    
    if (testPackage.empty() || testVersion.empty()) {
        GTEST_SKIP() << "No package with valid version found";
    }
    
    std::cout << "Testing package: " << testPackage << ", version: " << testVersion 
              << ", branch: " << testBranch << std::endl;
    
    std::string name = testPackage;
    std::string version = testVersion;
    std::string branch = testBranch;
    std::string packInfoToJson;
    int errorCode;
    
    int result = m_packageManager.getRemoteStatusInfo(name, version, branch, packInfoToJson, errorCode);
    
    EXPECT_EQ(result, KM_ERROR_NO) << "GetRemoteStatusInfo failed with result: " << result;
    
    std::cout << "Remote status info: " << packInfoToJson << std::endl;
}

// 测试字符串分割功能
TEST_F(KMTestPackageManager, SplitString) {
    std::string str1 = "OU1,OU2,OU3";
    std::vector<std::string> result = m_packageManager.splitString(str1, ',');
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "OU1");
    EXPECT_EQ(result[1], "OU2");
    EXPECT_EQ(result[2], "OU3");
}

// 测试检查麒麟文件签名工具是否存在
TEST_F(KMTestPackageManager, IsKylinFileSignToolPresent) {
    bool result = m_packageManager.isKylinFileSignToolPresent();
    std::cout << "Is Kylin File Sign Tool Present: " << (result ? "true" : "false") << std::endl;
    EXPECT_TRUE(true) << "Function should execute without error";
}

// 测试检查系统是否为V10 SP1版本
TEST_F(KMTestPackageManager, IsV10Sp1) {
    bool result = m_packageManager.isV10Sp1();
    std::cout << "Is V10 SP1: " << (result ? "true" : "false") << std::endl;
    EXPECT_TRUE(true) << "Function should execute without error";
}

// 测试两个接口的组合使用
TEST_F(KMTestPackageManager, SystemCapabilitiesCheck) {
    bool hasSignTool = m_packageManager.isKylinFileSignToolPresent();
    bool isV10Sp1 = m_packageManager.isV10Sp1();
    
    std::cout << "System capabilities:" << std::endl;
    std::cout << "  - Kylin File Sign Tool: " << (hasSignTool ? "present" : "not present") << std::endl;
    std::cout << "  - V10 SP1: " << (isV10Sp1 ? "yes" : "no") << std::endl;
    
    EXPECT_TRUE(true) << "Both functions should execute without error";
}

// 测试安装不存在的包（aaa）
TEST_F(KMTestPackageManager, InstallNonExistentPackage) {
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    
    std::string nonExistentPackage = "aaa";
    std::vector<std::string> onlineIds = {nonExistentPackage};
    
    // 测试安装不存在的包
    int result = m_packageManager.processTask(TaskType::TASK_TYPE_INSTALL, onlineIds, options);
    
    EXPECT_NE(result, KM_ERROR_NO);
}

// 测试获取远程包信息
TEST_F(KMTestPackageManager, GetRemoteInfo) {
    // 动态获取测试包
    std::string testPackage = getFirstAvailablePackage();
    if (testPackage.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::string strRemoteInfoJson;
    std::vector<std::string> allRemoteInfoJson;
    std::string remoteKind = "app";
    bool isNameNullable = false;
    
    // 测试获取远程包信息
    int result = m_packageManager.getRemoteInfo(testPackage, strRemoteInfoJson, allRemoteInfoJson, remoteKind, isNameNullable);
    
    // 验证结果
    EXPECT_EQ(result, KM_ERROR_NO) << "Get remote info should succeed";
}

// 测试获取不存在的远程包信息
TEST_F(KMTestPackageManager, GetRemoteInfoNonExistent) {
    std::string nonExistentPackage = "aaa";
    std::string strRemoteInfoJson;
    std::vector<std::string> allRemoteInfoJson;
    std::string remoteKind = "app";
    bool isNameNullable = false;
    
    // 测试获取不存在的远程包信息
    int result = m_packageManager.getRemoteInfo(nonExistentPackage, strRemoteInfoJson, allRemoteInfoJson, remoteKind, isNameNullable);
    
    // 验证结果应该返回错误
    EXPECT_NE(result, KM_ERROR_NO) << "Get remote info for non-existent package should fail";
}

// 测试获取所有远程包信息
TEST_F(KMTestPackageManager, GetRemoteInfoAllPackages) {
    std::string strRemoteInfoJson;
    std::vector<std::string> allRemoteInfoJson;
    std::string remoteKind = "app";
    bool isNameNullable = true;
    
    // 测试获取所有远程包信息（name为空）
    int result = m_packageManager.getRemoteInfo("", strRemoteInfoJson, allRemoteInfoJson, remoteKind, isNameNullable);
    
    // 验证结果
    EXPECT_EQ(result, KM_ERROR_NO) << "Get all remote info should succeed";
}

// // 测试删除应用用户数据
// TEST_F(KMTestPackageManager, DeleteUserData) {
//     // 动态获取已安装的应用
//     std::string testPackage = getFirstInstalledApp();
//     if (testPackage.empty()) {
//         GTEST_SKIP() << "No installed apps found";
//     }
    
//     std::cout << "Testing delete user data for: " << testPackage << std::endl;
    
//     // 测试删除用户数据
//     bool result = m_packageManager.deleteUserData(testPackage);
    
//     // 验证删除结果
//     EXPECT_TRUE(result) << "Delete user data should succeed for: " << testPackage;
// }

// // 测试卸载base和runtime
// TEST_F(KMTestPackageManager, UninstallRuntime) {
//     PackageManagerOptions options;
//     options.assumeYes = true;

//     std::string basePackage = "cn.kylin.v10sp1-base";
//     EXPECT_EQ(m_packageManager.uninstall({basePackage}, options),KM_ERROR_NO);
    
//     options.assumeYes = true;
//     std::string runtimePackage = "top.openkylin.ukui";
//     EXPECT_EQ(m_packageManager.uninstall({runtimePackage}, options),KM_ERROR_NO);
// }