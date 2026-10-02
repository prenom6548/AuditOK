/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef UKUISEARCH_H
#define UKUISEARCH_H

#include "result-item.h"
#include "data-queue.h"
#include "search-result-property.h"
namespace UkuiSearch {
class UkuiSearchTaskPrivate;
class UkuiSearchTask : public QObject
{
    Q_OBJECT
public:
    explicit UkuiSearchTask(QObject *parent = nullptr);
    ~UkuiSearchTask();

    /**
     * @brief init
     * 初始化搜索结果队列,必须在搜索之前调用
     * @return 返回一个用于接收搜索结果的队列指针,用户无需析构
     */
    DataQueue<ResultItem>* init();

    /**
     * @brief addSearchDir
     * 增加一个搜索目录作为搜索范围(文件)
     * @param path 要添加的目录
     */
    void addSearchDir(const QString &path);

    /**
     * @brief setRecurse
     * 设置是否执行递归搜索(文件),默认为递归搜索
     * @param recurse 是否递归
     */
    void setRecurse(bool recurse = true);

    /**
     * @brief addKeyword
     * 增加一个搜索关键词
     * @param keyword 关键词
     */
    void addKeyword(const QString &keyword);
    /**
     * @brief setMatchAllIfNoKeyword
     * 是否在未添加关键词的情况下进行所有文件名匹配
     * @param matchAll
     */
    void setMatchAllIfNoKeyword(bool matchAll);

    /**
     * @brief addFileLabel
     * 增加一个peony文件标签作为搜索条件
     * @param label 文件标签
     */
    void addFileLabel(const QString &label);

    /**
     * @brief setOnlySearchFile
     * 设置仅搜索文件(文件搜索)
     * @param onlySearchFile
     */
    void setOnlySearchFile(bool onlySearchFile);

    /**
     * @brief setOnlySearchDir
     * 设置仅搜索目录(文件搜索)
     * @param onlySearchDir
     */
    void setOnlySearchDir(bool onlySearchDir);

    /**
     * @brief setSearchOnlineApps
     * 设置是否搜索应用商店中已上架但未安装的应用
     * @param searchOnlineApps
     */
    void setSearchOnlineApps(bool searchOnlineApps);
    /**
     * @brief setSearchHiddenFiles
     * 是否搜索隐藏文件,仅支持文件名搜索
     * @param searchHiddenFiles
     */
    void setSearchHiddenFiles(bool searchHiddenFiles);
    /**
     * @brief initSearchPlugin
     * 初始化搜索插件
     * @param searchType 搜索插件枚举
     * @param customSearchType 当searchType为Custom时,传入用户自定义插件类型名称
     */
    void initSearchPlugin(SearchProperty::SearchType searchType, const QString& customSearchType = QString());
    /**
     * @brief setResultDataType
     * 设置某个搜索插件searchType的搜索结果数据类型
     * @param searchType 搜索插件枚举
     * @param searchResultProperties 指定的结果数据类型列表
     */
    bool setResultProperties(SearchProperty::SearchType searchType, SearchResultProperties searchResultProperties);

    /**
     * @brief setCustomResultDataType
     * 当initSearchPlugin中设置的SearchType为custom时,可以用此接口设置对应的返回数据类型
     * @param customSearchType 自定义插件名称
     * @param dataType 数据类型列表
     */
    void setCustomResultDataType(QString customSearchType, QStringList dataType);

    /**
     * @brief clearAllConditions
     * 清空所有搜索条件
     */
    void clearAllConditions();

    /**
     * @brief clearKeyWords
     * 清空关键词条件
     */
    void clearKeyWords();

    /**
     * @brief clearSearchDir
     * 清空搜索目录条件
     */
    void clearSearchDir();

    /**
     * @brief clearFileLabel
     * 清空文件标签条件
     */
    void clearFileLabel();
    /**
     * @brief setMaxResultNum
     * 设置最大结果数量
     * @param maxResults
     */
    void setMaxResultNum(unsigned int maxResults = 99999999);
    /**
     * @brief setInformNum
     * 设置搜索结果提醒数量,每当搜索结果数量达到@param num时,会发送reachInformNum信号然后重新开始计数
     * @param num
     */
    void setInformNum(int num);

    /**
     * @brief startSearch
     * 启动搜索
     * @param searchtype 搜索插件
     * @param customSearchType 外部插件类型，当searchType为Custom时可用
     * @return 返回一个代表本次搜索的唯一id
     */
    size_t startSearch(SearchProperty::SearchType searchtype, QString customSearchType = QString());
    /**
     * @brief stop 停止搜索
     */
    void stop();
    /**
     * @brief isSearching 查询某个插件是否处于搜索中
     * @return true表示正在搜索,false表示未在搜索
     */
    bool isSearching(SearchProperty::SearchType searchtype, QString customSearchType = {});

Q_SIGNALS:
    /**
     * @brief searchFinished
     * 搜索完成时发送
     * @param searchId 此次搜索的唯一id
     */
    void searchFinished(size_t searchId);

    /**
     * @brief searchError
     * 搜索发生错误时发送
     * @param searchId 此次搜索的唯一id
     * @param msg 错误信息
     */
    void searchError(size_t searchId, QString msg);

    /**
     * @brief reachInformNum
     * 配合setInformNum使用,当搜索结果数量达到设置值时发送
     */
    void reachInformNum();

private:
    UkuiSearchTaskPrivate* d = nullptr;
};
}

#endif // UKUISEARCH_H
