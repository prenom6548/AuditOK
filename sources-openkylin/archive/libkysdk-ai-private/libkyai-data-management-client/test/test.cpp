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


#include "test.h"

void checkStatus(DataManagementStatus status) {
    ASSERT_EQ(status.success, true) << "errno: " << status.errorCode << ", errmsg: " << status.errorMsg;
}

// 清空数据
TEST_F(DataManagementTest, delete_files) {
    std::cout << "Testing delete_files..." << std::endl;
    std::string input = R"([
        {
            "filepath": "/usr/lib/xorg/protocol.txt"
        },
        {
            "filepath": "/"
        }
    ])";
    DataManagementStatus status = data_management_delete_files(session, input.c_str());
    checkStatus(status);

    // 所有数据库应该都没有数据
    status = data_management_get_all_fileinfos(session, &result);
    checkStatus(status);
    ASSERT_STREQ("[]", result) << result;
}

TEST_F(DataManagementTest, add_text_files) {
    std::cout << "Testing add_text_files..." << std::endl;
    std::string input = R"([
        {
          "filepath": "/home/hezhe/文档/操作系统大会演示素材v0.2/待办.txt",
          "fileformat": "txt"
        },
        {
          "filepath": "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txt",
          "fileformat": "txt"
        },
        {
          "filepath": "/home/hezhe/文档/操作系统大会演示素材v0.2/测试素材/纯图片文件.docx",
          "fileformat": "docx"
        }
    ])";
    DataManagementStatus status = data_management_add_text_files(session, input.c_str());
    checkStatus(status);
}

TEST_F(DataManagementTest, get_all_fileinfos) {
    std::cout << "Testing get_all_fileinfos..." << std::endl;
    DataManagementStatus status = data_management_get_all_fileinfos(session, &result);
    checkStatus(status);
    // [{"filepath":"/home/hezhe/文档/操作系统大会演示素材v0.2/待办.txt","timestamp":1721056006},{"filepath":"/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txt","timestamp":1721931246}]
}

TEST_F(DataManagementTest, get_tags_of_files) {
    std::cout << "Testing get_tags_of_files..." << std::endl;
    std::string input = R"({
        "files": [
            "/home/hezhe/文档/操作系统大会演示素材v0.2/待办.txt",
            "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txt"
        ]
    })";
    DataManagementStatus status = data_management_get_tags_of_files(session, input.c_str(), &result);
    checkStatus(status);
}

TEST_F(DataManagementTest, get_all_tags) {
    std::cout << "Testing get_all_tags..." << std::endl;
    DataManagementStatus status = data_management_get_all_tags(session, &result);
    checkStatus(status);
}

TEST_F(DataManagementTest, search_similar_tags) {
    std::cout << "Testing search_similar_tags..." << std::endl;
    std::string input = R"({
        "tags": [
            "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txttag1",
            "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txttag2"
        ]
    })";
    DataManagementStatus status = data_management_search_similar_tags(session, input.c_str(), &result);
    checkStatus(status);
}

TEST_F(DataManagementTest, search_files_about_tags) {
    std::cout << "Testing search_files_about_tags..." << std::endl;
    std::string input = R"({
        "tags": [
            "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txttag1",
            "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txttag2"
        ]
    })";
    DataManagementStatus status = data_management_search_files_about_tags(session, input.c_str(), &result);
    checkStatus(status);
}

TEST_F(DataManagementTest, is_file_relevant_to_tags) {
    std::cout << "Testing is_file_relevant_to_tags..." << std::endl;
    std::string input = R"({
        "file": "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txt",
        "tags": [
            "/home/hezhe/文档/操作系统大会演示素材v0.2/待办.txt",
            "/home/hezhe/文档/操作系统大会演示素材v0.2/待办.txttag"
        ]
    })";
    bool isRelevant;
    DataManagementStatus status = data_management_is_file_relevant_to_tags(session, input.c_str(), &isRelevant);
    checkStatus(status);
    std::cout << "isRelevant: " << isRelevant << std::endl;
    ASSERT_TRUE(isRelevant);
}

TEST_F(DataManagementTest, is_file_relevant_to_description) {
    std::cout << "Testing is_file_relevant_to_description..." << std::endl;
    std::string input = R"({
        "file": "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txt",
        "description": "定期定额投资"
    })";
    bool isRelevant;
    DataManagementStatus status = data_management_is_file_relevant_to_description(session, input.c_str(), &isRelevant);
    checkStatus(status);
    std::cout << "isRelevant: " << isRelevant << std::endl;
    ASSERT_TRUE(isRelevant);
}

TEST_F(DataManagementTest, extract_search_conditions) {
    std::cout << "Testing extract_search_conditions..." << std::endl;
    std::string input = R"({
        "description": "搜索一下狗狗的图片"
    })";
    DataManagementStatus status = data_management_extract_search_conditions(session, input.c_str(), &result);
    checkStatus(status);
    EXPECT_STREQ(
        result,
        R"({"0":{"condition type":3,"content":"搜索一下狗狗的图片","orCondition":false},"conditions count":1,"labels":[]})");
}

TEST_F(DataManagementTest, get_summarys_of_files) {
    std::cout << "Testing get_summarys_of_files..." << std::endl;
    std::string input = R"({
        "files": [
            "/home/hezhe/文档/操作系统大会演示素材v0.2/待办.txt",
            "/home/hezhe/文档/操作系统大会演示素材v0.2/金信-金融问答.txt"
        ]
    })";
    DataManagementStatus status = data_management_get_summarys_of_files(session, input.c_str(), &result);
    checkStatus(status);
}

TEST_F(DataManagementTest, get_feature_status) {
    std::cout << "Testing get_feature_status..." << std::endl;
    FeatureStatus status;
    DataManagementStatus ret = data_management_get_feature_status(session, &status);
    std::cout << "FeatureStatus: " << status << std::endl;
    checkStatus(ret);
}

TEST_F(DataManagementTest, is_feature_supported) {
    std::cout << "Testing is_feature_supported..." << std::endl;
    EXPECT_TRUE(data_management_is_feature_supported());
}
