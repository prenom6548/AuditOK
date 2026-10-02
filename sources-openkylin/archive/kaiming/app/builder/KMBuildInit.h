/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDINIT_H__
#define __KMBUILDINIT_H__

#include <optional>
#include <filesystem>
#include <map>
#include <set>

#include "common/KMSubCommand.h"
#include "common/KMBuildinsUtils.h"
#include "common/KMCommonTypePrivate.h"
#include "common/KMJsonHelper.h"

namespace fs = std::filesystem;

/**
 * @brief 	 构建目录树初始化
 * @note	 供kaiming-builder构建流程中调用
 */
class KMBuildInit : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuildInit);

public:
    KMBuildInit();
    ~KMBuildInit() override;

    int dispose(int argc, char **argv) override;

    /**
     * @brief : 检查app与runtime或depend的base或runtime
     * @param : runtime或depend的路径
     * @param : true: 检查app与depend的runtime；false:检查app与runtime的base
     */
    bool checkBaseOrRuntime(const fs::path &infoPath , const bool runtimeFlag = false) const;

    /**
     * @brief : 获取depend路径
     * @param : depend名称
     * @return : depend所在路径
     */
    std::string getDependPath(const std::string &depend);

    /**
     * @brief: 处理depend依赖
     */
    void addDependsForApp(const std::vector<std::string> &depends, std::set<std::string>& noRepeat);
    void addDependsForDepend(const std::vector<std::string> &depends, std::set<std::string>& noRepeat);

private:
    void init(int argc, char **argv);

    int run();

    void generateDepends();

    void generateMetadata();
    
     /**
     * @brief  校验并确保指定开发模块（base/runtime）已安装
     * @param  [in] ref , KMRef对象，包含id和version信息
     * @param  [in] module, 模块名称，如 "devel"
     * @param  [in] kind, "base"或"runtime"
     * @return 返回对应的KMRef对象
     */ 
    KMRef ensureDevModuleInstalled(const KMRef& ref, const std::string& module, const std::string& kind);
    
    /**
     * @brief : 解析base/runtime等的pref
     * @param : [in] pref, 格式如：top.openkylin.base/stable/2.0.0
     * @return: std::optional<KMRef>
     */ 
    static std::optional<KMRef> parsePref(const std::string& pref);

    static void addEnv(std::map<std::string, std::string>& envs, const std::string &key, const std::string &value);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILDINIT_H__
