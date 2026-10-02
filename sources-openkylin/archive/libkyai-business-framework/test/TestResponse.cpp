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

#include "businessFrameworkTest.h"
#include "scene.h"

// ==================== 正常功能测试 (Happy Path) ====================

TEST_F(BusinessFrameworkTest, Response_Success_Basic_1) {
    Response response = Response::success();
    EXPECT_TRUE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), 0);
    EXPECT_STREQ(response.getErrMsg().c_str(), "请求成功");
    response.add("p1", 11);
    response.add("p2", "hehe");

    EXPECT_EQ(response.getInt("p1"), 11);
    EXPECT_STREQ(response.getData()["p2"].get<std::string>().c_str(), "hehe");
    EXPECT_STREQ(response.getString("p2").c_str(), "hehe");

    response.clear();

    Response response_1 = Response::success(nlohmann::json{
        {"aa", 11},
        {"bb", "asdfg"},
        {"cc", 33},
    });
}

// ==================== 成功和异步相关错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Success_With_Data) {
    nlohmann::json data = {
        {"user", "Alice"},
        {"age", 25},
        {"scores", {90, 85, 95}}
    };
    
    Response response = Response::success(data);
    EXPECT_TRUE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), SUCCESS);
    EXPECT_STREQ(response.getErrMsg().c_str(), "请求成功");
    EXPECT_EQ(response.getData()["user"], "Alice");
    EXPECT_EQ(response.getData()["age"], 25);
}

TEST_F(BusinessFrameworkTest, Response_Async_Flow_End) {
    Response response(ASYNC_FLOW_END);
    EXPECT_FALSE(response.isSuccess());  // 注意：ASYNC_FLOW_END != 0
    EXPECT_EQ(response.getErrCode(), ASYNC_FLOW_END);
    EXPECT_STREQ(response.getErrMsg().c_str(), "异步流式返回结束");
}

// ==================== 系统级错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_System_Error) {
    Response response(SYSTEM_ERR);
    EXPECT_FALSE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), SYSTEM_ERR);
    EXPECT_STREQ(response.getErrMsg().c_str(), "系统默认错误");
}

// ==================== 配置相关错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Config_Errors) {
    // 测试解析配置文件失败
    Response response1(PARSE_CONFIG_FILE_FAIL);
    EXPECT_FALSE(response1.isSuccess());
    EXPECT_EQ(response1.getErrCode(), PARSE_CONFIG_FILE_FAIL);
    EXPECT_STREQ(response1.getErrMsg().c_str(), "解析配置文件失败");
    
    // 测试无效配置
    Response response2(INVALID_CONFIG);
    EXPECT_FALSE(response2.isSuccess());
    EXPECT_EQ(response2.getErrCode(), INVALID_CONFIG);
    EXPECT_STREQ(response2.getErrMsg().c_str(), "缺少必要配置");
    
    // 测试D-Bus服务初始化失败
    Response response3(INIT_DBUS_SERVER_FAIL);
    EXPECT_FALSE(response3.isSuccess());
    EXPECT_EQ(response3.getErrCode(), INIT_DBUS_SERVER_FAIL);
    EXPECT_STREQ(response3.getErrMsg().c_str(), "初始化GDBUS服务失败");
}

// ==================== 参数解析错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Parse_Errors) {
    Response response1(PARSE_PARAM_FAILED);
    EXPECT_FALSE(response1.isSuccess());
    EXPECT_EQ(response1.getErrCode(), PARSE_PARAM_FAILED);
    EXPECT_STREQ(response1.getErrMsg().c_str(), "解析参数失败");
    
    Response response2(PARSE_RESPONSE_FAILED);
    EXPECT_FALSE(response2.isSuccess());
    EXPECT_EQ(response2.getErrCode(), PARSE_RESPONSE_FAILED);
    EXPECT_STREQ(response2.getErrMsg().c_str(), "解析Response失败");
}

// ==================== 文件处理错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_File_Processing_Errors) {
    // 文件分段失败
    Response response1(CHUNK_FILE_INFO_FAILED);
    EXPECT_FALSE(response1.isSuccess());
    EXPECT_EQ(response1.getErrCode(), CHUNK_FILE_INFO_FAILED);
    EXPECT_STREQ(response1.getErrMsg().c_str(), "文件内容分段失败");
    
    // 向量化失败
    Response response2(EMBEDDING_FAILED);
    EXPECT_FALSE(response2.isSuccess());
    EXPECT_EQ(response2.getErrCode(), EMBEDDING_FAILED);
    EXPECT_STREQ(response2.getErrMsg().c_str(), "向量化失败");
    
    // NLP处理失败
    Response response3(NLP_FAILED);
    EXPECT_FALSE(response3.isSuccess());
    EXPECT_EQ(response3.getErrCode(), NLP_FAILED);
    EXPECT_STREQ(response3.getErrMsg().c_str(), "获取标签，总结失败");
}

// ==================== 数据库操作错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Database_Operation_Errors) {
    // SQLite数据库操作错误
    std::vector<ERROR_CODE> sqlite_errors = {
        INSERT_FILE_INFO_SQLITE_FAILED,
        DELETE_FILE_INFO_SQLITE_FAILED,
        UPDATE_FILE_INFO_SQLITE_FAILED
    };
    
    std::vector<std::string> sqlite_messages = {
        "数据库存入文件信息失败",
        "数据库删除文件信息失败", 
        "数据库更新文件信息失败"
    };
    
    for (size_t i = 0; i < sqlite_errors.size(); ++i) {
        Response response(sqlite_errors[i]);
        EXPECT_FALSE(response.isSuccess());
        EXPECT_EQ(response.getErrCode(), sqlite_errors[i]);
        EXPECT_STREQ(response.getErrMsg().c_str(), sqlite_messages[i].c_str());
    }
    
    // 向量数据库操作错误
    std::vector<ERROR_CODE> vector_db_errors = {
        CREATE_VECTOR_DATABASE_FAILED,
        INSERT_VECTOR_DATABASE_FAILED, 
        DELETE_VECTOR_DATABASE_FAILED,
        UPDATE_VECTOR_DATABASE_FAILED,
        SEARCH_FILE_INFO_FAILED
    };
    
    std::vector<std::string> vector_db_messages = {
        "创建向量数据库失败",
        "向量数据库存入信息失败",
        "向量数据库删除信息失败", 
        "向量数据库更新信息失败",
        "搜索文件信息失败"
    };
    
    for (size_t i = 0; i < vector_db_errors.size(); ++i) {
        Response response(vector_db_errors[i]);
        EXPECT_FALSE(response.isSuccess());
        EXPECT_EQ(response.getErrCode(), vector_db_errors[i]);
        EXPECT_STREQ(response.getErrMsg().c_str(), vector_db_messages[i].c_str());
    }
}

// ==================== 业务场景错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Business_Scenario_Errors) {
    // 数据管理场景错误
    Response response1(DM_ERROR_UNKNOWN);
    EXPECT_FALSE(response1.isSuccess());
    EXPECT_EQ(response1.getErrCode(), DM_ERROR_UNKNOWN);
    
    Response response2(DM_SESSION_ERROR);
    EXPECT_FALSE(response2.isSuccess());
    EXPECT_EQ(response2.getErrCode(), DM_SESSION_ERROR);
    
    Response response3(DM_ERROR_INVALID_INPUT);
    EXPECT_FALSE(response3.isSuccess());
    EXPECT_EQ(response3.getErrCode(), DM_ERROR_INVALID_INPUT);
    
    Response response4(DM_ERROR_PERMISSION_DENIED);
    EXPECT_FALSE(response4.isSuccess());
    EXPECT_EQ(response4.getErrCode(), DM_ERROR_PERMISSION_DENIED);
}

// ==================== 文档问答错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Document_QA_Errors) {
    Response response(TEXT_CHUNKS_UNSUPPORT);
    EXPECT_FALSE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), TEXT_CHUNKS_UNSUPPORT);
    EXPECT_STREQ(response.getErrMsg().c_str(), "不支持的文本长度");
}

// ==================== 向量数据库错误码测试 ====================

TEST_F(BusinessFrameworkTest, Response_Vector_Database_Errors) {
    Response response(VECTOR_DATABASE_DATA_UNAVAILABLE);
    EXPECT_FALSE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), VECTOR_DATABASE_DATA_UNAVAILABLE);
    EXPECT_STREQ(response.getErrMsg().c_str(), "向量数据库数据不可用，数据库可能损坏或无法访问");
}

// ==================== 自定义错误消息测试 ====================

TEST_F(BusinessFrameworkTest, Response_Custom_Error_Message) {
    std::string custom_msg = "自定义错误消息";
    Response response(SYSTEM_ERR, custom_msg);
    
    EXPECT_FALSE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), SYSTEM_ERR);
    EXPECT_STREQ(response.getErrMsg().c_str(), custom_msg.c_str());
}

TEST_F(BusinessFrameworkTest, Response_Custom_Error_Message_With_Data) {
    std::string custom_msg = "带数据的自定义错误";
    nlohmann::json data = {
        {"error_details", "具体错误信息"},
        {"suggestion", "请检查输入参数"}
    };
    
    Response response(DM_ERROR_INVALID_INPUT, custom_msg, data);
    
    EXPECT_FALSE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), DM_ERROR_INVALID_INPUT);
    EXPECT_STREQ(response.getErrMsg().c_str(), custom_msg.c_str());
    EXPECT_EQ(response.getData()["error_details"], "具体错误信息");
    EXPECT_EQ(response.getData()["suggestion"], "请检查输入参数");
}

// ==================== 数据访问方法测试 ====================

TEST_F(BusinessFrameworkTest, Response_Data_Access_Methods) {
    nlohmann::json data = {
        {"name", "测试用户"},
        {"count", 42},
        {"active", true},
        {"tags", {"tag1", "tag2"}}
    };
    
    Response response(SUCCESS, data);
    
    // 测试getString方法
    EXPECT_STREQ(response.getString("name").c_str(), "测试用户");
    EXPECT_TRUE(response.getString("nonexistent").empty());  // 不存在的key返回空字符串
    
    // 测试getInt方法  
    EXPECT_EQ(response.getInt("count"), 42);
    EXPECT_EQ(response.getInt("nonexistent"), 0);  // 不存在的key返回0
}

// ==================== 序列化和反序列化测试 ====================

TEST_F(BusinessFrameworkTest, Response_Serialization) {
    nlohmann::json data = {
        {"result", "处理成功"},
        {"count", 100}
    };
    
    Response original(SUCCESS, data);
    std::string serialized = original.toString();
    
    // 验证序列化格式
    nlohmann::json parsed = nlohmann::json::parse(serialized);
    EXPECT_EQ(parsed["err_no"], SUCCESS);
    EXPECT_STREQ(parsed["err_msg"].get<std::string>().c_str(), "请求成功");
    EXPECT_EQ(parsed["data"]["result"], "处理成功");
    EXPECT_EQ(parsed["data"]["count"], 100);
}

TEST_F(BusinessFrameworkTest, Response_Clear_Method) {
    Response response(SYSTEM_ERR);
    response.add("test_key", "test_value");
    
    EXPECT_FALSE(response.isSuccess());
    EXPECT_FALSE(response.getData().is_null());
    
    response.clear();
    
    EXPECT_TRUE(response.isSuccess());
    EXPECT_EQ(response.getErrCode(), SUCCESS);
    EXPECT_STREQ(response.getErrMsg().c_str(), "请求成功");
    EXPECT_TRUE(response.getData().is_object());
}

// ==================== 边界情况测试 ====================

TEST_F(BusinessFrameworkTest, Response_Edge_Cases) {
    // 测试空数据
    Response response1(SUCCESS, nlohmann::json());
    EXPECT_TRUE(response1.isSuccess());
    EXPECT_TRUE(response1.getData().is_null());
    
    // 测试复杂嵌套数据
    nlohmann::json complex_data = {
        {"users", {
            {"user1", {"name", "Alice", "age", 25}},
            {"user2", {"name", "Bob", "age", 30}}
        }},
        {"metadata", {
            {"version", "1.0"},
            {"timestamp", 1234567890}
        }}
    };
    
    Response response2(SUCCESS, complex_data);
    EXPECT_TRUE(response2.isSuccess());
    EXPECT_EQ(response2.getData()["metadata"]["version"], "1.0");
}

// ==================== 错误码覆盖完整性测试 ====================

TEST_F(BusinessFrameworkTest, Response_Error_Code_Coverage) {
    // 验证所有在ERROR_MSG中定义的错误码都能正确创建Response
    for (const auto& [error_code, error_msg] : ERROR_MSG) {
        Response response(error_code);
        EXPECT_EQ(response.getErrCode(), error_code);
        EXPECT_STREQ(response.getErrMsg().c_str(), error_msg.c_str());
    }
}