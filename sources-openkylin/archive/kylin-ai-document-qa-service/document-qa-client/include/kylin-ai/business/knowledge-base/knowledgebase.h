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

#pragma once
#include <kylin-ai/business/knowledge-base/common.h>

#include <memory>

namespace knowledgebase {

class KnowledgebaseSession {
public:
    KnowledgebaseSession();
    ~KnowledgebaseSession();

    /*!
     * \brief 判断知识库是否支持
     * \param success [out] 出参，返回架构是否支持的标记位
     * \return 成功 Status::success 为 true，失败则会返回对应错误码
     */
    Status isSupported(bool& success) const;

    /*!
     * \brief 创建知识库
     * \param knowledgebaseName [out] 出参，返回知识库名称
     * \return 成功 Status::success 为 true，失败则会返回对应错误码
     */
    Status createKnowledgebase(std::string& knowledgebaseName);

    /*!
     * \brief 删除知识库
     * \param knowledgebaseName [in] 入参
     * \return 成功 Status::success 为 true，失败则会返回对应错误码
     */
    Status deleteKnowledgebase(const std::string& knowledgebaseName);

    /*!
     * \brief 相似搜索
     * \param searchCondition [in] search conditional，字符串为json格式
     *  {
     *      "knowledgebase_name": "name",
     *      "search_condition": "linux内核安全选项"
     *  }
     * \param searchResult [out] searchResult，字符串为json格式
     *  [
     *      {
     *          "document": "xxxxxxx",
     *          "filepath": "/path/to/file1.txt",
     *          "similarity": 0.92
     *      },
     *      {
     *          "document": "xxxxxxx",
     *          "filepath": "/path/to/file2.txt",
     *          "similarity": 0.88
     *      }
     *  ]
     * \return 成功 Status::success 为 true，失败则会返回对应错误码
     */
    Status similaritySearch(const std::string& searchCondition, std::string& searchResult);

    /*!
     * \brief 添加文件到知识库
     * \param fileinfos [in]  fileinfos 添加文件到知识库的信息，字符串为json格式
     *  [
     *      {
     *          "knowledgebase_name": "name",
     *          "filepath": "/path/to/file1.txt"
     *      },
     *      {
     *          "knowledgebase_name": "name",
     *          "filepath": "/path/to/file2.txt"
     *      }
     *  ]
     * \return 成功 Status::success 为 true，失败则会返回对应错误码
     */
    Status addTextFiles(const std::string& fileinfos);

    /*!
     * \brief 获得文件的内容
     * \param filepaths [in]  filepaths 文件路径，字符串为json格式
     *  [
     *      {
     *          "filepath": "/path/to/file1.txt"
     *      },
     *      {
     *          "filepath": "/path/to/file2.txt"
     *      }
     *  ]
     * \param fileContent[out] 出参  fileContent 获得文件内容信息，字符串为json格式
     *  [
     *      {
     *          "filepath": "/path/to/file1.txt",
     *          "content": "file content"
     *      },
     *      {
     *          "filepath": "/path/to/file2.txt",
     *          "content": "file content"
     *      }
     *  ]

     * \return 成功 Status::success 为 true，失败则会返回对应错误码
     */
    Status getTextFileContents(const std::string& input, std::string& result);

private:
    struct KnowledgebaseImlp;
    std::unique_ptr<KnowledgebaseImlp> impl_;
};

}  // namespace knowledgebase