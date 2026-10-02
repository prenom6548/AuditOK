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

#ifndef DATAMANAGEMENT_DATAMANAGEMENT_H
#define DATAMANAGEMENT_DATAMANAGEMENT_H

#include <kylin-ai/business/data-management/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DataManagementSession DataManagementSession;

/**
 * \brief 判断当前系统架构是否支持AI功能
 */
bool data_management_is_feature_supported();

/*!
 * \brief 创建数据管理会话
 * \return 失败将返回 nullptr
 */
DataManagementSession* data_management_create_session();

/*!
 * \brief 销毁数据管理会话
 */
void data_management_destroy_session(DataManagementSession* session);

/*!
 * \brief 释放请求返回的结果
 */
void data_management_destroy_result(char* result);

/*!
 * \brief 获取当前会话支持的功能状态
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param status 获取到的功能状态，入参不能为 nullptr，函数内会修改 status 的值
 * \return 成功将返回 SearchResult::SEARCH_SUCCESS，失败则会返回对应错误码
 * \example 示例用法
 * FeatureStatus status = FeatureStatus::UNKNOWN;
 * DataManagementStatus status = data_management_get_feature_status(session,
 * &status); if (status.success && status == FeatureStatus::AVAILABLE) {
 *     do_something();
 * }
 */
DataManagementStatus data_management_get_feature_status(DataManagementSession* session, FeatureStatus* status);

/*!
 * \brief 检查文件是否支持，返回识别到的文件格式，需要在调用
 * data_management_add_files 前使用 \param session 请求所使用的会话，通过
 * data_management_create_session 创建 \param filepath 检查的文件绝对路径 \param
 * result 检查的格式结果，为 json 格式，格式如下
 * {
 *   "supported": true
 *   "format" : "txt"
 * }
 * supported 字段代表当前文件是否支持，对于不支持的文件在
 * data_management_add_files 时会返回 DATA_MANAGEMENT_FILE_FORMAT_ERROR format
 * 字段代表识别到的格式，可以用作 data_management_add_files 时入参的 fileformat
 * result 需要使用 data_management_destroy_result 释放
 * \return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_check_file_format(DataManagementSession* session, const char* filepath,
                                                       char** check_result);

/*!
 * \brief 会通过语意相似度搜索文件内容，返回包含相关内容的文件
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param search_conditions 要搜索的内容，是一个 json 格式的字符串，格式如下：
 * {
 *   "text": "linux内核安全选项"
 *   "similarity-threshold" : 0.6
 * }
 * similarity-threshold
 * 代表相似度阈值，只会返回超过这个阈值的结果，如果不写则使用 默认的相似度阈值
 * \param search_result
 * 返回搜索结果，一般传入一个空指针，由SDK申请内存，在使用完后 通过
 * data_management_destroy_result 进行销毁。 搜索结果为一段 json
 * 格式的文件列表，包含"filepath"和"similarity" 两个字段，格式如下：
 * [
 *   {
 *     "filepath": "/path/to/file1.txt",
 *     "similarity": 0.92
 *   },
 *   {
 *     "filepath": "/path/to/file2.txt",
 *     "similarity": 0.85
 *   }
 * ]
 * 结果默认会根据相似度进行排序，越接近1认为相似度越高
 * \return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_similarity_search(DataManagementSession* session, const char* search_conditions,
                                                       char** search_result);

/*!
 * \brief 将新文件增加到会话的数据范围中
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要增加到搜索范围里的文件列表，使用 json 格式，格式参考如下：
 * [
 *   {
 *     "filepath": "/path/to/file3.pdf",
 *     "fileformat": "pdf"
 *   },
 *   {
 *     "filepath": "/path/to/file1.txt",
 *     "fileformat": "txt"
 *   }
 * ]
 * filepath 字段要求是文件的绝对路径
 * fileformat 字段目前支持 pdf, txt, docx, pptx 格式，可以通过
 * data_management_check_file_format 获取 如果 fileformat
 * 为空会默认使用文件的后缀 \return 成功 DataManagementStatus::success 为
 * true，失败则会返回对应错误码
 */
DataManagementStatus data_management_add_text_files(DataManagementSession* session, const char* fileinfos);

/*!
 * \brief 将新文件增加到会话的数据中
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要增加到搜索范围里的文件列表，使用 json 格式，格式参考如下：
 * [
 *   {
 *     "filepath": "/path/to/file3.jpeg",
 *     "fileformat": "jpeg"
 *   },
 *   {
 *     "filepath": "/path/to/file2.png",
 *     "fileformat": "png"
 *   }
 * ]
 * fileformat 目前支持 png, jpg, jepg 格式
 * \return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_add_image_files(DataManagementSession* session, const char* fileinfos);

/*!
 * \brief 将文件从会话的数据中删除
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要从会话的文件里删除的文件列表，字符串为 json
 * 格式，其中路径为绝对路径
 * [
 *   {
 *     "filepath": "/path/to/file2.png"
 *   },
 *   {
 *     "filepath": "/path/to/file1.pdf"
 *   }
 * ]
 * \return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_delete_files(DataManagementSession* session, const char* fileinfos);

/*!
 * \brief 更新文件名，不重建向量
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要从会话的文件里重命名的文件列表，字符串为 json
 * 格式，其中路径为绝对路径
 * [
 *   {
 *     "old_filepath": "/path/to/file2.png",
 *     "new_filepath": "/path/to/newfile2.png"
 *   },
 *   {
 *     "old_filepath": "/path/to/file1.txt",
 *     "new_filepath": "/path/to/newfile1.txt"
 *   }
 * ]
 * \param new_fileinfos
 * 新文件文件列表，格式和旧文件列表一致，和旧文件列表需要是一一对应关系 \return
 * 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_update_files_name(DataManagementSession* session, const char* fileinfos);

/*!
 * \brief 更新文件的向量数据
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos 要从会话的文件要更新的文件列表，字符串为 json
 * 格式，其中路径为绝对路径
 * [
 *   {
 *     "filepath": "/path/to/file2.png",
 *     "fileformat": "png"
 *   },
 *   {
 *     "filepath": "/path/to/file1.pdf",
 *     "fileformat": "pdf"
 *   }
 * ]
 * \return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_update_files_content(DataManagementSession* session, const char* fileinfos);

/*!
 * \brief 获取当前会话所有文件及修改时间列表
 * \param session 请求所使用的会话，通过 data_management_create_session 创建
 * \param fileinfos_result 文件列表，返回查询结果，一般传入一个
 * nullptr，由SDK申请内存， 需要通过 data_management_destroy_result 销毁
 * 返回的结果为 json 格式，包含文件的绝对路径和文件修改时间的UNIX时间戳
 * [
 *   {
 *     "filepath": "/path/to/file3.jpeg",
 *     "timestamp": 123456
 *   },
 *   {
 *     "filepath": "/path/to/file2.png",
 *     "timestamp": 654321
 *   }
 * ]
 * \return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_get_all_fileinfos(DataManagementSession* session, char** fileinfos_result);

/**
 * @brief 获取文件的标签
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [in] files 'files'做为key的Json字符串，示例：
 * {
 *     "files": ["xxxx1","xxxx2"...]
 * }
 * @param [out] result 返回的Json array字符串，示例：
 * [
 *      {
 *          "filepath": xxx,
 *          "tags": ["xxxx1","xxxx2"...]
 *      },
 *      ...
 * ]
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_get_tags_of_files(DataManagementSession* session, const char* files,
                                                       char** result);

/**
 * @brief 获取当前存储的所有标签名称
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [out] tags_result 返回的Json array字符串，示例：["xxxx1","xxxx2"...]
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_get_all_tags(DataManagementSession* session, char** result);

/**
 * @brief 搜索相似标签
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [in] tags 'tags'做为key的Json字符串，示例：
 * {
 *     "tags": ["xxxx1","xxxx2"...]
 * }
 * @param [out] result 返回的Json array字符串，示例：["xxxx1","xxxx2"...]
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_search_similar_tags(DataManagementSession* session, const char* tags,
                                                         char** result);

/**
 * @brief 根据标签搜索文件
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [in] tags 'tags'做为key的Json字符串，示例：
 * {
 *     "tags": ["xxxx1","xxxx2"...]
 * }
 * @param [out] result 返回的Json array字符串，示例：["xxxx1","xxxx2"...]
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_search_files_about_tags(DataManagementSession* session, const char* tags,
                                                             char** result);

/**
 * @brief 判断文件和标签是否相关
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [in] fileinfos 'file' 'tags'做为key的Json字符串，示例：
 * {
 *     "file": xxxx,
 *     "tags": ["xxxx1","xxxx2"...]
 * }
 * @param [out] isRelevant true 相关；false 不相关
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_is_file_relevant_to_tags(DataManagementSession* session, const char* fileInfos,
                                                              bool* isRelevant);

/**
 * @brief 判断文件和内容是否相关
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [in] fileinfos 'file' 'description'做为key的Json字符串，示例：
 * {
 *     "file": xxxx,
 *     "description": xxxxx
 * }
 * @param [out] isRelevant true 相关；false 不相关
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_is_file_relevant_to_description(DataManagementSession* session,
                                                                     const char* fileInfos, bool* isRelevant);

/**
 * @brief 根据描述生成搜索条件结构
 * @param session 请求所使用的会话，通过 data_management_create_session 创建
 * @param description 'description'做为key的Json字符串，示例：
 * {
 *      "description": xxxxx
 * }
 * @param result 搜索条件结构
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_extract_search_conditions(DataManagementSession* session, const char* description,
                                                               char** result);

/**
 * @brief 获取文件的摘要
 * @param [in] session 请求所使用的会话，通过 data_management_create_session 创建
 * @param [in] files 'files'做为key的Json字符串，示例：
 * {
 *     "files": ["xxxx1","xxxx2"...]
 * }
 * @param [out] result 返回的Json array字符串，示例：
 * [
 *      {
 *          "filepath": xxxx1,
 *          "summary": xxxxxxxxx
 *      },
 *      ...
 * ]
 * @return 成功 DataManagementStatus::success 为 true，失败则会返回对应错误码
 */
DataManagementStatus data_management_get_summarys_of_files(DataManagementSession* session, const char* files,
                                                           char** result);

#ifdef __cplusplus
}
#endif

#endif  // DATAMANAGEMENT_DATAMANAGEMENT_H
