#include <gtest/gtest.h>
#include "app/list/KMList.h"
#include "KMPackageManager.h"
#include "KMOSTreeHandler.h"
#include "KMFolder.h"

#define private public

class KMTestPackageManager
{
public:
    KMTestPackageManager() = default;
    ~KMTestPackageManager() = default;

public:
    KMPackageManager m_packageManager;
};

// 测试安装功能
TEST(KMTestPackageManager, InstallOnlinePackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::vector<std::string> onlineIds = {"top.openkylin.base", "com.daidouji.oneko", "org.gnome.Breakout/5.3.0", "depend.openkylin.zip"};
    std::vector<std::string> uninstallIds = {"com.daidouji.oneko"};

    // depend
    std::vector<std::string> dependId = {"depend.openkylin.libgsl-dev","depend.openkylin.libgslcblas0","depend.openkylin.libgsl27"};
    EXPECT_EQ(testManager.m_packageManager.install(dependId, options), KM_ERROR_NO);
    EXPECT_EQ(testManager.m_packageManager.uninstall(dependId, options), KM_ERROR_NO);

    // 测试在线安装
    EXPECT_EQ(testManager.m_packageManager.processTask(TASK_TYPE_INSTALL, onlineIds, options), KM_ERROR_NO);

    EXPECT_EQ(testManager.m_packageManager.isInstalled(onlineIds[0]),KM_ERROR_NO);
    EXPECT_EQ(testManager.m_packageManager.isKylinFileSignToolPresent(),false);
    EXPECT_EQ(testManager.m_packageManager.getSafeMode(),0);
    fs::path idPath;
    std::string id;
    EXPECT_EQ(testManager.m_packageManager.resolveFailedVerification(0, idPath, id),KM_ERROR_GET_SAFE_MODE_FAILED);
    EXPECT_EQ(testManager.m_packageManager.isUpgradabled(onlineIds[0]),KM_ERROR_NOT_NEED_UPDATE);

    // 验签
    options.assumeYes = false;
    options.skipSignatureVerification = false;
    std::vector<std::string> verIds = {"org.gnome.baobab"};
    EXPECT_EQ(testManager.m_packageManager.install(verIds, options), KM_ERROR_NO);

    KMList listIds;
    std::map<std::string, std::string> localAppsInfo = listIds.getLocalAppIds(false);

    options.assumeYes = true;
    options.arch = "x86_64";
    options.upgradable = false;
    options.isGetDevel = true;
    options.isPrint = false;
    options.skipSignatureVerification = true;

    EXPECT_EQ(testManager.m_packageManager.getUpgradableList(localAppsInfo, options), KM_ERROR_NO);

    KMFolder folder;
    fs::path repoPath = folder.getRepoPath();
    EXPECT_EQ(testManager.m_packageManager.verifySignature("top.openkylin.base", "base", "/opt/kaiming/layers/stable/x86_64/base/top.openkylin.base/binary/1.0.0"),KM_ERROR_NO);

    // testManager.m_packageManager.install(uninstallIds, options);
    options.assumeYes = false;
    EXPECT_EQ(testManager.m_packageManager.uninstall(uninstallIds, options), KM_ERROR_NO);
    
}

TEST(KMTestPackageManager, SearchInfoPackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::vector<std::string> onlineIds = {"com.daidouji.oneko"};
    testManager.m_packageManager.install(onlineIds, options);

    EXPECT_EQ(testManager.m_packageManager.dbusSearch(onlineIds[0], options),KM_ERROR_NO);
}

// 测试本地包安装
TEST(KMTestPackageManager, InstallLocalPackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;

    testManager.m_packageManager.uninstall({"depend.openkylin.zip"}, options);
    std::vector<std::string> offPackages = {"/home/lxy/下载/top.openkylin.Scanner-x86_64-3.2.1.0-ok2.22-stable-binary-squashfs.ok", "/home/lxy/下载/cn.feishu.Feishu-x86_64-7.18.11-stable-binary-squashfs.ok", "/home/lxy/下载/depend.openkylin.zip-x86_64-3.0.0-stable-devel-squashfs.ok"};
    std::vector<std::string> installedPackages = {"top.openkylin.Scanner"};


    // 测试本地包安装
    EXPECT_EQ(testManager.m_packageManager.installOfflinePackage(offPackages, options), KM_ERROR_NO);
    // EXPECT_EQ(testManager.m_packageManager.installOfflinePackage(installedPackages, options), KM_ERROR_ALREADY_INSTALLED);

    testManager.m_packageManager.uninstall({"depend.openkylin.libgsl-dev"}, options);
    std::vector<std::string> offdependPackages = {"/home/lxy/桌面/depend.openkylin.libgsl-dev-x86_64-2.7.1-stable-devel-squashfs.ok"};
    EXPECT_EQ(testManager.m_packageManager.installOfflinePackage(offdependPackages, options), KM_ERROR_NO);

    options.skipSignatureVerification = false;
    std::vector<std::string> baseId = {"top.openkylin.base"};
    testManager.m_packageManager.uninstall(baseId, options);
}

// 测试更新功能
TEST(KMTestPackageManager, UpdatePackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::vector<std::string> ids = {"org.gnome.Breakout"};

    std::vector<std::string> onlineIds = {"org.gnome.Breakout/5.3.0"};
    testManager.m_packageManager.install(onlineIds, options);

    EXPECT_EQ(testManager.m_packageManager.update(ids, options), KM_ERROR_NO);
}
TEST(KMTestPackageManager, SearchPackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::string onlineIds = "com.daidouji.oneko";

    EXPECT_EQ(testManager.m_packageManager.search(onlineIds, options), KM_ERROR_NO);
}

TEST(KMTestPackageManager, ReparePackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::vector<std::string> onlineIds = {"com.daidouji.oneko"};
    options.force = true;
    testManager.m_packageManager.install(onlineIds, options);

    EXPECT_EQ(testManager.m_packageManager.repair(onlineIds, options), KM_ERROR_NO);
}

TEST(KMTestPackageManager, UpgradePackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::vector<std::string> onlineIds = {"com.daidouji.oneko"};

    testManager.m_packageManager.install(onlineIds, options);
    EXPECT_EQ(testManager.m_packageManager.processTask(TASK_TYPE_UPGRADE, onlineIds, options),KM_ERROR_ALREADY_INSTALLED);

}

// 测试卸载功能
TEST(KMTestPackageManager, UninstallPackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::vector<std::string> onlineIds = {"com.daidouji.oneko"};

    testManager.m_packageManager.install(onlineIds, options);
    
    // 测试卸载
    EXPECT_EQ(testManager.m_packageManager.processTask(TASK_TYPE_UNINSTALL, onlineIds, options), KM_ERROR_NO);
}

TEST(KMTestPackageManager, SoftCenterPackage) {
    KMTestPackageManager testManager;
    PackageManagerOptions options;
    options.assumeYes = true;
    options.skipSignatureVerification = true;
    std::string name = "com.daidouji.oneko";
    KMPMProgressCallBack installCallback;

    testManager.m_packageManager.installSoftwareCenter(name, "", installCallback);
    testManager.m_packageManager.updateSoftwareCenter(name, "", installCallback);
    testManager.m_packageManager.uninstallSoftwareCenter(name);

    std::vector<std::string> uninstallIds = {"depend.openkylin.zip", "top.openkylin.base"};
    EXPECT_EQ(testManager.m_packageManager.uninstall(uninstallIds, options), KM_ERROR_NO);
}
