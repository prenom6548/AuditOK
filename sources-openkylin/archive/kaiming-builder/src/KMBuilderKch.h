/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDERKCH_H__
#define __KMBUILDERKCH_H__

#include <string>
#include <memory>
#include <vector>

#include "KMSubCommand.h"

class KMBuilderKch : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuilderKch);

public:
    KMBuilderKch();
    virtual ~KMBuilderKch();

    int dispose(int argc, char **argv) override;

private:
    /**
     * @brief : 初始化
     */
    void init(int argc, char **argv);

    /**
     * @brief : 逻辑
     */
    int run();

    /**
     * @brief : 创建changelog
     */
    void create();

    /**
     * @brief : 新增changlog记录项
     */
    void increment();

    /**
     * @brief : 新增changelog entry
     */
    void append(const std::vector<std::string>& entrys);

    /**
     * @brief : 输出最新版本号
     */
    void showLastVersion();

    /**
     * @brief : 在指定路径下查找构建清单文件
     */
    bool findManifest();

    /**
     * @brief : 获取到维护者信息
     */
    void initMaintainer();

    /**
     * @brief : 比较两个版本号的大小关系
     */
    bool compareVersions(const std::string& v1, const std::string& operation, const std::string& v2);
    bool compareDebianVersions(const std::string& v1, const std::string& operation, const std::string& v2);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILDERKCH_H__
