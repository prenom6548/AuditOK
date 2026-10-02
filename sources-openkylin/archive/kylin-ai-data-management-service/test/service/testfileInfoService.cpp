/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "fileInfoService.h"

class FileInfoServiceTest : public testing::Test {
public:
    static void SetUpTestSuite() {
        std::cout << "FileInfoServiceTest SetUpTestSuite..." << std::endl;

        // 先把本地的向量数据库删了
        std::string dbPath = FileDatabase::getDbFilePath();
        if (!dbPath.empty()) {
            std::filesystem::remove(dbPath);
        }
    }

    static void TearDownTestSuite() { std::cout << "FileInfoServiceTest TearDownTestSuite..." << std::endl; }

protected:
    void SetUp() override {
        std::cout << "FileInfoServiceTest SetUp..." << std::endl;
        fileInfoService = std::make_unique<FileInfoService>();

        // 创建临时目录 当前测试用例跑完后，会在TearDownTestSuite时删除该目录
        testDir = std::filesystem::current_path() / "FileInfoServiceTest";
        std::filesystem::create_directories(testDir);

        // 创建测试文件
        testFilePath1 = testDir / "test1.txt";
        testFilePath2 = testDir / "test2.txt";

        std::ofstream(testFilePath1) << "This is a test file content for similarity search.";
        std::ofstream(testFilePath2) << "Another test file with different content.";
    }

    void TearDown() override {
        std::cout << "FileInfoServiceTest TearDown..." << std::endl;
        fileInfoService.reset();
        // 清理测试环境
        std::filesystem::remove_all(testDir);
    }

    static inline std::filesystem::path testDir;
    static inline std::filesystem::path testFilePath1;
    static inline std::filesystem::path testFilePath2;

    std::unique_ptr<FileInfoService> fileInfoService;
};

TEST_F(FileInfoServiceTest, test01) {
    // 测试区分文件函数
    std::unordered_set<std::string> filePaths = {testFilePath1.string(), testFilePath2.string()};
    std::unordered_set<std::string> newFilePaths;
    std::vector<FileInfoDO> updateFileInfos;

    // 第一次分类 当前插入的都是新文件
    fileInfoService->classifyFile(filePaths, newFilePaths, updateFileInfos);
    EXPECT_EQ(newFilePaths.size(), 2);
    EXPECT_TRUE(updateFileInfos.empty());

    // 第二次分类，刚才插入的文件，现在应该都是更新文件
    newFilePaths.clear();
    fileInfoService->insertTask(filePaths);  // 先插入数据库

    fileInfoService->classifyFile(filePaths, newFilePaths, updateFileInfos);
    EXPECT_TRUE(newFilePaths.empty());
    EXPECT_EQ(updateFileInfos.size(), 2);
}

/*
*** debuild的时候获得不到这个hand.docx，所以这个测试用例就注释掉了
*** 如果手动生成文件，读取时间是一直变化的，所以当时才用的本地文件
*** 就导致了测试结果和我的预期不一样 
*/ 

// TEST_F(FileInfoServiceTest, test02) {
//     namespace fs = std::filesystem;
//     fs::path dir = fs::path(__FILE__).parent_path();
//     fs::path path = dir / "hand.docx";

//     // 测试插入和更新
//     std::unordered_set<std::string> filePaths = {path.string()};
//     fileInfoService->insertTask(filePaths);

//     // 验证文件是否已插入
//     FileInfoDO fileInfo;
//     SQLite::Database db = fileInfoService->getDatabase();
//     bool success = FileDatabase::getSingleFileInfo(db, path.string(), fileInfo);
//     EXPECT_TRUE(success);
//     EXPECT_EQ(fileInfo.filePath, path.string());

//     std::cout << fileInfo.fileId << " " << fileInfo.filePath << " " << fileInfo.enable << " " << fileInfo.modifyTime
//               << " " << fileInfo.fileSummary << std::endl;

//     // 测试更新任务
//     // 由于前面把db给拿出来了，所以之前的FileInfoService中私有成员db用不了，需要重新创建FileInfoService对象
//     fileInfoService.reset();
//     std::unique_ptr<FileInfoService> fileInfoServiceTmp = std::make_unique<FileInfoService>();
//     std::vector<FileInfoDO> fileInfos = {fileInfo};
//     auto updateIds = fileInfoServiceTmp->updateTask(fileInfos);
//     EXPECT_EQ(updateIds.size(), 0);  // 这里是0 如果文件没有修改，不会更新，这里应该为0

//     for (auto& fileInfo : fileInfos) {
//         std::cout << fileInfo.fileId << " " << fileInfo.filePath << " " << fileInfo.enable << " " << fileInfo.modifyTime
//                   << " " << fileInfo.fileSummary << std::endl;
//     }

//     // 修改文件内容，在插入，这个时候updateIds.size为1
//     std::ofstream(path, std::ios::app) << "This is an updated test file content.";
//     updateIds = fileInfoServiceTmp->updateTask(fileInfos);
//     EXPECT_EQ(updateIds.size(), 1);
// }

TEST_F(FileInfoServiceTest, test03) {
    // 测试获取文件最后修改时间
    auto modifyTime = fileInfoService->getLastModifyTime(testFilePath1.string());
    EXPECT_NE(modifyTime, "0000-00-00 00:00:00");

    // 测试不存在的文件
    auto nonExistTime = fileInfoService->getLastModifyTime((testDir / "nonexist.txt").string());
    EXPECT_EQ(nonExistTime, "0000-00-00 00:00:00");
}

// TEST_F(FileInfoServiceTest, test04) {
//     // 测试搜索文件
//     std::unordered_set<std::string> filePaths = {testFilePath1.string(), testFilePath2.string()};
//     fileInfoService->insertTask(filePaths);

//     // 由于索引是闲时创建，所以这里暂时只判断接口是否成功
//     SimilaritySearchResult result;
//     auto ret = fileInfoService->searchFilesByText("test", result);
//     EXPECT_EQ(ret, SUCCESS);

//     // 对于空搜索
//     SimilaritySearchResult emptyResult;
//     ret = fileInfoService->searchFilesByText("", emptyResult);
//     EXPECT_EQ(ret, EMBEDDING_FAILED);  // 会返回向量化错误码
//     EXPECT_TRUE(emptyResult.empty());
// }

TEST_F(FileInfoServiceTest, test05) {
    // 测试 DeleteTextFileVectorInfo
    std::unordered_set<std::string> filePaths = {testFilePath1.string()};
    fileInfoService->insertTask(filePaths);

    SQLite::Database db = fileInfoService->getDatabase();
    FileInfoDO fileInfo;
    FileDatabase::getSingleFileInfo(db, testFilePath1.string(), fileInfo);
    std::unordered_set<int64_t> fileIds = {fileInfo.fileId};

    // 测试删除向量信息
    // insertTask闲时后 服务才会去插入向量信息 测试用例虽然没有实际插入，但是删除不存的向量，要返回true
    bool success = FileInfoService::deleteTextFileVectorInfo(fileIds);
    EXPECT_TRUE(success);

    // 测试空输入
    success = FileInfoService::deleteTextFileVectorInfo({});
    EXPECT_TRUE(success);
}

TEST_F(FileInfoServiceTest, test06) {
    // 先插入测试文件
    fileInfoService->insertTask({testFilePath1.string()});
    SQLite::Database db = fileInfoService->getDatabase();
    FileInfoDO fileInfo;
    FileDatabase::getSingleFileInfo(db, testFilePath1.string(), fileInfo);

    // 测试删除文件
    fileInfoService.reset();
    std::unique_ptr<FileInfoService> fileInfoServiceTmp = std::make_unique<FileInfoService>();
    bool success = fileInfoServiceTmp->deleteFiles({fileInfo.fileId});
    EXPECT_TRUE(success);

    // 验证文件是否已删除
    FileInfoDO deletedInfo;
    success = FileDatabase::getSingleFileInfo(db, testFilePath1.string(), deletedInfo);
    EXPECT_FALSE(success);

    // 测试空输入
    success = fileInfoServiceTmp->deleteFiles({});
    EXPECT_TRUE(success);
}