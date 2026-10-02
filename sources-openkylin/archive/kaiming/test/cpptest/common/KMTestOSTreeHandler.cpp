#include "KMOSTreeHandler.h"
#include <gtest/gtest.h>
#include <string>
#include <map>

#define private public

class KMTestOSTree
{
public:
    KMTestOSTree() = default;
    ~KMTestOSTree() = default;

public:
    // KMTestOSTree() : m_handler(ostreeManager("/opt/kaiming/repo")) {}
    KMOSTreeHandler m_handler{"/opt/kaiming/repo"}; // 假设初始化路径为 /path/to/repo
    // KMOSTreeHandler m_handler;
};

TEST(KMTestOSTree, test_ostreehandler)
{
    KMTestOSTree testOSTree;

    // 测试 readLatestCommit
    std::string refCompletedId = "stable/x86_64/app/com.daidouji.oneko/binary/1.2.0";
    EXPECT_EQ(testOSTree.m_handler.readLatestCommit(refCompletedId),"");

    // 测试 getRemoteUrl
    std::string remoteName = "kaiming-repo";
    // result = "http://localhost/repo/"
    EXPECT_EQ(testOSTree.m_handler.getRemoteUrl(remoteName),"http://localhost/repo/");

    // 测试 updateDeployRef
    std::string completedRef = "stable/x86_64/app/com.daidouji.oneko/binary/1.2.0.0";
    EXPECT_EQ(testOSTree.m_handler.updateDeployRef(completedRef, "2226b62b017e237e51ef96e6b8e74e7689322b63e90b40e5345b49deaf511cfa"),true);

    // 测试 deleteRefTag
    EXPECT_EQ(testOSTree.m_handler.deleteRefTag(remoteName, completedRef, "2226b62b017e237e51ef96e6b8e74e7689322b63e90b40e5345b49deaf511cfa"),true);

    // 测试 repoPrune
    EXPECT_EQ(testOSTree.m_handler.repoPrune(),true);

}