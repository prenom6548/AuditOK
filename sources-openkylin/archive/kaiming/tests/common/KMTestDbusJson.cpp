/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "common/KMDbusJson.h"
#include "common/KMPackageManager.h"
#include "common/KMUtil.h"
#include "common/KMInstalledAppQuery.h"

class KMTestDbusJson
{
public:
    KMTestDbusJson() = default;
    ~KMTestDbusJson() = default;

public:
    KMDbusJson m_dbusJson;
    KMPackageManager m_pm;
};

TEST(KMTestDbusJson, test_dbusJson_packLocalInfoToJson)
{
    KMTestDbusJson packLocalInfoToJson;
    
    // 动态获取已安装的应用
    KMInstalledAppQuery appQuery;
    std::vector<KMRef> refs = appQuery.query(KMInstalledAppQuery::All);
    ASSERT_FALSE(refs.empty()) << "No installed apps found";
    
    std::string id = refs.front().id;
    std::string name = refs.front().id;
    std::string version = refs.front().version;
    std::string size = "2222";
    std::string branch = refs.front().branch;
    std::string arch = refs.front().arch;
    std::string module = refs.front().module;
    std::string description = "";
    
    std::string result = packLocalInfoToJson.m_dbusJson.packLocalInfoToJson(id, name, version, size, branch, arch, module, description);
    std::cout << "packLocalInfoToJson result: " << result << std::endl;
    
    // 验证JSON格式
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("\"name\":\"" + name + "\""), std::string::npos);
    EXPECT_NE(result.find("\"version\":\"" + version + "\""), std::string::npos);
    EXPECT_NE(result.find("\"branch\":\"" + branch + "\""), std::string::npos);
}

TEST(KMTestDbusJson, test_dbusJson_packProgressInfoToJson)
{
    KMTestDbusJson packProgressInfoToJson;
    const char *argv[] = {"app", "1.0.0"};
    std::string refId = argv[0];
    std::string speed = argv[1];
    std::string result = packProgressInfoToJson.m_dbusJson.packProgressInfoToJson(refId, 1, speed, 2);
    EXPECT_EQ(result, "{\"progress\":1,\"refId\":\"app\",\"speed\":\"1.0.0\",\"timeout\":2}");
}

TEST(KMTestDbusJson, test_dbusJson_errorLocalJson)
{
    KMTestDbusJson errorLocalJson;
    std::string result = errorLocalJson.m_dbusJson.errorLocalJson();
    EXPECT_EQ(result, "{\"arch\":\"\",\"branch\":\"\",\"description\":\"\",\"id\":\"\",\"module\":\"\",\"name\":\"\",\"size\":\"\",\"version\":\"\"}");
}

TEST(KMTestDbusJson, test_dbusJson_parseRemotePathToJson)
{
    KMTestDbusJson parseRemotePathToJson;
    
    // 动态获取一个远程包路径
    std::vector<std::string> searchResults;
    PackageManagerOptions pmOptions;
    pmOptions.isPrint = false;
    pmOptions.kind = "app";
    
    int searchRet = parseRemotePathToJson.m_pm.searchInfo("", pmOptions, searchResults);
    if (searchRet != KM_ERROR_NO || searchResults.empty()) {
        GTEST_SKIP() << "No available packages found";
    }
    
    std::string ref = searchResults.front();
    std::cout << "Original ref: " << ref << std::endl;
    
    std::string result = parseRemotePathToJson.m_dbusJson.parseRemotePathToJson(ref);
    std::cout << "parseRemotePathToJson result: " << result << std::endl;
    
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("\"branch\""), std::string::npos);
    EXPECT_NE(result.find("\"id\""), std::string::npos);
    EXPECT_NE(result.find("\"size\""), std::string::npos);
}

TEST(KMTestDbusJson, test_dbusJson_errorRemoteJson)
{
    KMTestDbusJson errorRemoteJson;
    std::string result = errorRemoteJson.m_dbusJson.errorRemoteJson();
    EXPECT_EQ(result, "{\"branch\":\"\",\"debinfo\":{\"package\":\"\",\"version\":\"\"},\"id\":\"\",\"size\":\"\",\"version\":\"\"}");
}