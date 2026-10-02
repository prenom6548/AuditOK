/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDFLOW_H__
#define __KMBUILDFLOW_H__

#include "KMSubCommand.h"
#include "KMBuilderContext.h"

/**
 * @brief: 构建流程
 * @note : 框架是新的，但业务逻辑是旧的，从原来的KMBuilderMain.cpp和KMBuilderManifest类中迁移了一部分逻辑过来，后续有时间可以再行优化
 */ 
class KMBuildFlow : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuildFlow);

public:
    KMBuildFlow();
    virtual ~KMBuildFlow();

    int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);
    int run();

    /**
     * @brief: 设置ccache目录
     * @param: 是否启用
     * @param: ccache缓存目录
     */
    bool setCcache(bool enable, const std::string &ccacheDir);

    /**
     * @brief : 编译初始化
     * @return : true-成功; false-失败
     */
    bool buildInit();

    /**
    * @brief : 下载
    * @return : true-成功; false-失败
    */
    bool downloadFile(std::map<std::shared_ptr<KMBuilderModule>, std::string> &moudle_str_map);

    /**
     * @brief : 编译
     * @return : true-成功; false-失败
     */
    bool build(std::map<std::shared_ptr<KMBuilderModule>, std::string> &moudle_str_map);

    /**
     * @brief : 初始化参数
     * @return : true-成功; false-失败
     */
    bool initArgs(std::vector<std::string> &args);

    /**
     * @brief : 展开所有的builderModules
     * @param modules : 所有的builderModules
     * @param expandModules  : 展开后的builderModules
     * @return : true-成功; false-失败
     */
    void expandModules(const std::vector<std::shared_ptr<KMBuilderModule>> &modules);

    /**
     * @brief : 是否需要构建
     * @param module : builderModule模块
     * @return : true-需要; false-不需要
     */
    bool isShouldBuild(const std::shared_ptr<KMBuilderModule> &module);

    /**
     * @brief : 修正depends数据
     */
    void amendDepends();

    /**
     * @brief : 构建流程中的cleanup环节
     */ 
    void cleanup();

    /**
     * @brief : 构建流程中的finish环节
     */
    void buildFinish();

    /**
     * @brief : 填充info.json的部分信息
     */
    void fillInfoJson();

    /**
     * @brief : 获取dbus服务的白名单列表
     * @return: dbus服务的白名单列表。每个数组元素都是一个可执行程序完整路径。dbus-daemon管控的判断逻辑是：
                (1)判断/proc/进程id/exe 和白名单中的名字是否一致
                (2)判断白名单中的文件是否存在
     */
    std::vector<std::string> getDbusWhitelist();

    /**
     * @brief : 将changelog等文件封装到构建结果中
     */
    void finishKaimingDir();

    /**
     * @brief : 构建流程中的export环节
     */ 
    void buildExport();

    /**
     * @brief : 生成subpackages.json文件
     */ 
    void generateSubPackagesJson();

    /**
     * @brief : 收集子开明包包含的路径列表
     * @param : [in] path, 指定路径; files，指定的文件列表
     * @param : [in] prefix, 前缀，匹配比较时，如果match是相对路径，需要去掉文件/目录的前缀后比较
     * @param : [in] matchs, 匹配规则
     * @param : [out] includes，子开明包包含的路径列表
     */ 
    static void collectSubPackagesPath(const std::string& path, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &includes);

    /**
     * @brief : 收集指定路径下或文件列表中匹配规则的文件/目录列表
     * @param : [in] path, 指定路径; files，指定的文件列表
     * @param : [in] prefix, 前缀，匹配比较时，如果match是相对路径，需要去掉文件/目录的前缀后比较
     * @param : [in] matchs, 匹配规则
     * @param : [out] toRemoves，待删除的路径
     */ 
    static void collectCleanupForPath(const std::string& path, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &toRemoves);
    static void collectCleanupForPath(const std::vector<std::string>& files, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &toRemoves);

    static void addEnv(std::map<std::string, std::string>& envs, const std::string &key, const std::string &value);

private:
    std::vector<std::shared_ptr<KMBuilderModule>> m_expandModules;
    std::map<std::string, std::vector<std::string>> m_moduleToFiles;
    std::vector<std::string> m_lastFiles;
    bool m_addComponents = false;
};

#endif // !__KMBUILDFLOW_H__
