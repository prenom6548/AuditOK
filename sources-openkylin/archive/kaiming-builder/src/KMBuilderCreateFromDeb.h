/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDERCREATEFROMDEB_H__
#define __KMBUILDERCREATEFROMDEB_H__

#include <algorithm>

#include "KMSubCommand.h"
#include "KMBuilderModule.h"

class KMBuilderCreateFromDeb : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuilderCreateFromDeb);
public:
    KMBuilderCreateFromDeb();
    ~ KMBuilderCreateFromDeb();
    int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv );
    int run();

public:
    /**
     * @brief: 将debian中的数据写入manifest中
     */
    void debianToManifest();

    /**
     * @brief: 加载dpkg.list中软件包列表
     */
    void loadDpkgList(const std::string &path);

    /**
     * @brief: 加载本地及远程summary.json中的依赖
     */
    void loadDepends();

    /**
     * @brief: 确定deb包所需依赖是否存在于base/runtime/depend中，或作为component添加
     */
    bool isExists(std::pair<const std::string, std::string> &depend);
    void matchDepends(std::map<std::string, std::string> &debianDepends);
    void matchDepends(std::vector<std::map<std::string, std::string>> &debianDependsOr);

    /**
     * @brief: 比较deb规则版本号
     */
    bool compareVersion(const std::string &ver1, const std::string &relation, const std::string &ver2);

    /**
     * @brief: 填充清单列表
     */
    void fillManifest();

    /**
     * @brief: 获取base或runtime的安装路径，info格式为{channel}:{id}/{version}
     */
    std::string getDeployDir(const std::string &kind, const std::string &info);

    /**
     * @brief: 确定项目构建方式
     */
    std::string matchBuildSystem(const std::string &sourcePath);

    /**
     * @brief: 设置项目组件
     */
    std::shared_ptr<KMBuilderModule> setComponent();

    /**
     * @brief: 将未找到的依赖填充到depends后面
     */
    void fillDepends();

    /**
     * @brief: 设置build-script
     */
    void setBuildScript(std::shared_ptr<KMBuilderModule> &component);

    /**
     * @brief: 匹配符合依赖包的depend
     */
    std::string matchDependByDeb(const std::vector<std::string> &depends, const std::string debName);

private:
    void generateChangelog();

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;

};

#endif