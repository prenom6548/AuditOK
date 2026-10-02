/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_QUERY_H__
#define __BUILDER_QUERY_H__

#include<filesystem>
#include<nlohmann/json.hpp>
#include<algorithm>

#include"KMSubCommand.h"
#include "KMBuilderUtils.h"
#include "common/KMBuildinOptions.h"
#include "common/KMNlohmannJsonHelper.h"
#include "common/KMException.h"
#include "common/KMParseSummaryJson.h"
#include "common/KMRef.h"
#include "common/KMFormatter.h"
#include "common/KMProcessUtils.h"

namespace fs = std::filesystem;

class KMBuilderQuery: public KMSubCommand
{
DECLARE_DYNCREATE(KMBuilderQuery);
public:
    KMBuilderQuery();
    ~KMBuilderQuery();
    

public:
    void init(int argc , char **argv);
    int dispose(int argc , char **agrv) override;
    int run();
   
private:
    class Options;
    class Private;
    std::unique_ptr<Private>d;

public:
    /**
     * @brief: 获取当前目录下的指定文件的绝对路径
     * @param: path:待检查路径
     * @param: info_paths: 保存找到的info.json的绝对路径
     * @param: target_filename: 目标文件
     */ 
    void findTargetFile(const std::string &path, std::vector<std::string>&info_paths, const std::string &target_filename);

    /**
     * @brief: 检查summary.json是否合理
     */
    bool checkSummary(const std::string &path);

    /**
     * @brief: 从summary.json中查找满足条件的包信息
     */
    std::vector<KMRef> getRefsFromSummary(const std::string &arch="", const std::string &kind="", const std::string &model="", const std::string &id="", const std::string &version="", const std::string &base="", const std::string &runtime="");
    
    /**
     * @brief: 从本地查找满足条件的包信息
     */
    std::vector<KMRef> getRefsFromLocal(const std::string &arch="", const std::string &kind="", const std::string &model="", const std::string &id="", const std::string &version="", const std::string &base="", const std::string &runtime="", bool binary=true);
    
    /**
     * @brief: 在终端打印软件包信息
     */
    void printRef(const std::vector<KMRef>&refs);

};


#endif 