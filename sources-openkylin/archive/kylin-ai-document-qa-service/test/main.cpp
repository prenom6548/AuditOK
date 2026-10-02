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

#include <gio/gio.h>
#include <kylin-ai/business/knowledge-base/knowledgebase.h>

#include <filesystem>
#include <iostream>
#include <sstream>

// 定义一个回调函数，当定时器触发时调用该函数
gboolean quit_loop(gpointer data) {
    GMainLoop* loop = static_cast<GMainLoop*>(data);
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;  // 返回 G_SOURCE_REMOVE 以移除定时器
}
int main() {
    std::cout << " test start " << std::endl;
    std::cout << "test isSupported start." << std::endl;
    /*平台支持与否判断*/
    knowledgebase::KnowledgebaseSession d;
    bool ok = false;
    knowledgebase::Status status = d.isSupported(ok);
    if (!ok) {
        std::cout << "test isSupported failed." << std::endl;
    }

    std::cout << "test isSupported end." << std::endl;

    std::cout << "test createKnowledgebase start." << std::endl;
    std::string name;
    status = d.createKnowledgebase(name);
    if (!status.success) {
        std::cout << "test createKnowledgebase failed, errorCode:" << status.errorCode
                  << " errorMsg:" << status.errorMsg << std::endl;
    }
    std::cout << "name: " << name << std::endl;
    std::cout << "test createKnowledgebase end." << std::endl;

    std::cout << "test addFiles start." << std::endl;

    /*添加数据*/
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "公司报销制度及流程.docx";
    std::string filepath = path.string();

    std::ostringstream first;
    first << R"(
    [
        {
            "knowledgebase_name": ")"
          << name << R"(",
           "filepath": ")"
          << filepath << R"("
        }
    ]
    )";
    std::string files = first.str();
    status = d.addTextFiles(files);
    if (!status.success) {
        std::cout << "test addFiles failed, errorMsg: " << status.errorCode << " errorMsg:" << status.errorMsg
                  << std::endl;
    }

    std::cout << "test addFiles end." << std::endl;

#if 1
    /*获得所有数据*/
    std::string contents;
    std::cout << "test getTextFileContents start." << std::endl;

    std::ostringstream second;
    second << R"(
    [
        {
           "filepath": ")"
           << filepath << R"("
        }
    ]
    )";

    std::string filesContent = second.str();

    status = d.getTextFileContents(filesContent, contents);
    if (!status.success) {
        std::cout << "test getTextFileContents failed, errorMsg: " << status.errorMsg << std::endl;
    }

    std::cout << "contents: " << contents << std::endl;
    std::cout << "test getTextFileContents end." << std::endl;
#endif

#if 1
    /*搜索*/
    std::string searchResult;
    std::ostringstream oss;
    oss << R"({
                      "knowledgebase_name": ")"
        << name << R"(",
                      "search_condition": "公司报销"
                    })";
    std::string searchcondition = oss.str();
    std::cout << "test similaritySearch start." << std::endl;
    status = d.similaritySearch(searchcondition, searchResult);
    std::cout << "searchResult:" << searchResult << std::endl;
    if (!status.success) {
        std::cout << "test similaritySearch failed, errorCode:" << status.errorCode << " errorMsg:" << status.errorMsg
                  << std::endl;
    }
    std::cout << "test similaritySearch end." << std::endl;
#endif

    /*删除数据库collection*/
    std::cout << "test deleteKnowledgebase start: " << name << std::endl;
    status = d.deleteKnowledgebase(name);
    if (!status.success) {
        std::cout << "test deleteKnowledgebase failed, errorMsg: " << status.errorCode
                  << " errorMsg:" << status.errorMsg << std::endl;
    }

    std::cout << "test deleteKnowledgebase end." << std::endl;

    std::cout << " test end " << std::endl;

    GMainLoop* loop = g_main_loop_new(nullptr, false);
    g_timeout_add_seconds(30, quit_loop, loop);
    g_main_loop_run(loop);
    return 0;
}