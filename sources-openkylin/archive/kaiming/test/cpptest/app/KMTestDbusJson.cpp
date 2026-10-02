#include "KMDbusJson.h"
#include <gtest/gtest.h>

class KMTestDbusJson
{
public:
    KMTestDbusJson() = default;
    ~KMTestDbusJson() = default;

public:
    KMDbusJson m_dbusJson;
};

TEST(KMTestDbusJson, test_dbusJson_packLocalInfoToJson)
{
    KMTestDbusJson packLocalInfoToJson;
    const char *argv[] = {"app", "1.0.0", "222222", "stable","x86_64","binary"};
    std::string name = argv[0];
    std::string version = argv[1];
    std::string size = argv[2];
    std::string branch = argv[3];
    std::string arch = argv[4];
    std::string module = argv[5];
    std::string result = packLocalInfoToJson.m_dbusJson.packLocalInfoToJson(name,version,size,branch,arch,module);
    std::cout << result <<std::endl;
    EXPECT_EQ(result, "{\"branch\":\"stable\",\"name\":\"app\",\"size\":\"222222\",\"version\":\"1.0.0\"}");
}

TEST(KMTestDbusJson, test_dbusJson_packProgressInfoToJson)
{
    KMTestDbusJson packProgressInfoToJson;
    const char *argv[] = {"app", "1.0.0"};
    std::string refId = argv[0];
    std::string speed = argv[1];
    std::string result = packProgressInfoToJson.m_dbusJson.packProgressInfoToJson(refId,1,speed,2);
    EXPECT_EQ(result, "{\"progress\":1,\"refId\":\"app\",\"speed\":\"1.0.0\",\"timeout\":2}");
}

TEST(KMTestDbusJson, test_dbusJson_errorLocalJson)
{
    KMTestDbusJson errorLocalJson;
    std::string result = errorLocalJson.m_dbusJson.errorLocalJson();
    EXPECT_EQ(result, "{\"branch\":\"\",\"name\":\"\",\"size\":\"\",\"version\":\"\"}");
}

TEST(KMTestDbusJson, test_dbusJson_parseRemotePathToJson)
{
    KMTestDbusJson parseRemotePathToJson;
    std::string ref = "stable/x86_64/depend/depend.openkylin.libgsl-dev/devel/2.7.1/hello/2222";
    std::string result = parseRemotePathToJson.m_dbusJson.parseRemotePathToJson(ref);
    EXPECT_EQ(result, "{\"branch\":\"stable\",\"id\":\"depend.openkylin.libgsl-dev\",\"size\":\"2222\"}");
}

TEST(KMTestDbusJson, test_dbusJson_errorRemoteJson)
{
    KMTestDbusJson errorRemoteJson;
    std::string result = errorRemoteJson.m_dbusJson.errorRemoteJson();
    EXPECT_EQ(result, "{\"branch\":\"\",\"id\":\"\",\"size\":\"\"}");
}