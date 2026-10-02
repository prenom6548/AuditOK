/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMCONTAINER_H__
#define __KMCONTAINER_H__

#include "KMConfigData.h"

#include <memory>
#ifdef CPPUNIT_TEST
#define private public
#endif

class KMContainer
{
public:
    KMContainer();
    ~KMContainer();

public:
    /**
     * @brief 设置appID,必须设置用于生成运行目录
     * @param id appID
     */
    void setAppID(const std::string &id);

    /**
     * @brief 设置base目录,必须设置用于修订挂载点信息
     * @param baseDir 目录
     */
    void setBaseDir(const std::string &baseDir);

    /**
     * @brief 获取appID
     * @return appID
     */
    std::string getAppID();

    /**
     * @brief 设置oci版本
     * @param version 版本号
     */
    void setOciVersion(const std::string &version);

    /**
     * @brief 获取版本
     * @return string 版本号
    */
    std::string getOciVersion();

    /**
     * @brief 设置主机名
     * @param hostName 主机名
     */
    void setHostName(const std::string &hostName);

    /**
     * @brief 获取容器主机名
     * @return string 主机名
     */

    std::string setHostName();
    /**
     * @brief 设置域名
     * @param domainName 域名
     */

    void setDomainName(const std::string &domainName);

    /**
     * @brief 获取域名
     * @return string 域名
     */
    std::string getDomainName();

    /**
     * @brief 设置uid映射
     * @param uidMappings uid映射数组
     */
    void setUidMappings(const std::vector<IDMap> &uidMappings);

    /**
     * @brief 设置gid映射
     * @param gidMappings gid映射数组
     */
    void setGidMappings(const std::vector<IDMap> &gidMappings);

    /**
     * @brief 获取命名空间
     */
    std::vector<NameSpace> getNameSpaces();

    /**
     * @brief 设置命名空间
     * @param nss 命名空间数组
     */
    void setNameSpace(const std::vector<std::string> &nss);

    /**
     * @brief 添加命名空间
     * @param ns 命名空间
     */
    void addNameSpace(const std::string &ns);

    /**
     * @brief 移除命名空间
     * @param ns 命名空间
     */
    void rmNameSpace(const std::string &ns);

    /**
     * @brief 设置/根挂载传播属性
     * @param 传播类型
     */
    void setRootfsPropagation(const std::string &propagation);

    /**
     * @brief 获取/根挂载传播属性
     * @param 传播类型
     * @return true 成功 false 失败
     */
    bool getRootfsPropagation(std::string &propagation);

    /**
     * @brief 设置根目录
     * @param root 根目录
     * @param readonly 是否只读系统
     */
    void setRoot(const std::string &root, bool readonly);

    /**
     * @brief 获取根结构数据
     * @param Root 数据
     */
    Root getRoot();

    /**
     * @brief 添加注释；包含任意传递给容器的元数据
     * @param key 键
     * @param value 值
     */
    void addAnnotation(const std::string &key, const std::string &value);

    /**
     * @brief 删除注释
     * @param key 键
     */
    void delAnnotation(const std::string &key);

    /**
     * @brief 获取注释；包含任意传递给容器的元数据
     * @return 注释
     */
    std::map<std::string, std::string> getAnnotations();

    /**
     * @brief 设置进程配置,设置所有配置项
     * @param process 进程配置
     */
    void setProcess(const Process &process);

    /**
     * @brief 获取进程配置
     * @return 进程配置
     */
    Process getProcess();

    /**
     * @brief 设置进程启动参数
     * @param args 启动参数
     */
    void setProcessArgs(const std::vector<std::string> &args);

    /**
     * @brief 设置进程当前工作目录
     * @param cwd 当前工作目录
     */
    void setProcessCwd(const std::string &cwd);

    /**
     * @brief 添加进程环境变量
     * @param env 环境变量
     */
    void addProcessEnv(const std::vector<std::string> &env);

    /**
     * @brief 获取进程启动参数
     * @return args 启动参数
     */
    std::vector<std::string> getProcessArgs();

    /**
     * @brief 设置进程环境变量
     * @param env 环境变量
     */
    void setProcessEnv(const std::vector<std::string> &env);

    /**
     * @brief 获取进程启动环境变量
     * @return env 环境变量
     */
    std::vector<std::string> getProcessEnv();

    /**
     * @brief 设置工作目录
     * @param cwd 工作目录
     */
    void setCwd(const std::string &cwd);

    /**
     * @brief 设置hooks挂载
     * @param hooks hooks挂载
     */
    void setHooks(const Hooks &hooks);

    /**
     * @brief 获取Hooks挂载数据
     * @return hooks 挂载数据
     */
    Hooks getHooks();

    /**
     * @brief 设置prestart钩子
     * @param perstart prestart结构
     */
    void setPrestart(const std::vector<Hook> &prestart);

    /**
     * @brief 添加createRuntime钩子
     * @param createRuntime createRuntime结构
     */
    void setCreateRuntime(const std::vector<Hook> &createRuntime);

    /**
     * @brief 设置createContainer钩子
     * @param createContainer createContainer结构
     */
    void setCreateContainer(const std::vector<Hook> &createContainer);

    /**
     * @brief 添加startContainer钩子
     * @param startContainer startContainer结构
     */
    void setStartContainer(const std::vector<Hook> &startContainer);

    /**
     * @brief 设置poststart钩子
     * @param postStart postStart结构
     */
    void setPoststart(const std::vector<Hook> &postStart);

    /**
     * @brief 添加poststop钩子
     * @param postStop postStop结构
     */
    void setPostStop(const std::vector<Hook> &postStop);

    /**
     * @brief 设置linux平台配置;不设置则使用默认值
     * @param linuxConfig linux配置
     */
    void setLinux(const Linux &linuxConfig);

    /**
     * @brief 获取linux平台配置
     * @return Linux 平台信息
     */
    Linux getLinux();

    /**
     * @brief 设置挂载点
     * @param mounts 挂载点
     */
    void setMounts(const std::vector<Mount> &mounts);

    /**
     * @brief 添加挂载点
     * @param mount 挂载点
     */
    void addMount(const Mount &mount);

    /**
     * @brief 获取Mounts
     * @return mounts 挂载数据
     */
    std::vector<Mount> getMounts();

    /**
     * @brief 设置进程默认参数
     */
    void setProcessTerminal(const bool &terminal);

    /**
     * @brief 运行前准备
     * @param runDir 工作目录
     * @param containerID 容器ID
     * @param isBuild 是否是编译工作运行
     * @return 成功true，失败 false
     */
    bool preRun(std::string &runDir, std::string &containerid, const bool &isBuild = false);

    /**
     * @brief 运行容器,同步方式,前后端不分离
     * @param isBuild 是否是编译工作运行, 默认为true是为了兼容旧版本代码
     * @return 成功true，失败 false
     */
    bool run(const bool &isBuild = true);

public:
    /**
     * @brief 初始化模板
     */
    void initTemplate();

    /**
     * @brief 初始化环境变量
     */
    void initBaseEnv();

    /**
     * @brief 设置进程默认参数
     */
    void initProcessDefault();

    /**
     * @brief 生成配置文件
     * @param dir bundle目录
     * @return true 成功，false 失败
     */
    bool genConfig(const std::string &dir);

    /**
     * @brief 生成containerId
     * @return containerId的值
    */
    std::string generateUUID();

    /**
     * @brief 生成运行参数
     * @return true 成功，false 失败
    */
    bool genRunDir(std::string &dir, std::string &uuid, bool isBuild = false);

    /**
     * @brief 清空容器运行目录
     * @param dir 运行目录
     * @return true 成功 false 失败
     */
    bool clearRunDir(const std::string &dir, const bool &isBuild);

public:
    /**
     * @brief 设置seccomp配置
     * @param seccomp seccomp配置
     * @return true 成功 false 失败
     */
    bool setSeccomp(const Seccomp &seccomp);

    /**
     * @brief 设置Linux的capabilities的权限能力
     * @param ids 权限 ID 的列表
     */
    void setLinuxCapabilities(const std::vector<int> &ids);

    /**
     * @brief 设置process的capabilities的bounding的权限能力
     * @param ids 权限 ID 的列表
     */
    void setProcessBoundingCapabilities(const std::vector<int> &ids);

    /**
     * @brief 设置process的capabilities的permitted的权限能力
     * @param ids 权限 ID 的列表
     */
    void setProcessPermittedCapabilities(const std::vector<int> &ids);

    /**
     * @brief 设置process的capabilities的inheritable的权限能力
     * @param ids 权限 ID 的列表
     */
    void setProcessInheritableCapabilities(const std::vector<int> &ids);

    /**
     * @brief 设置process的capabilities的effective的权限能力
     * @param ids 权限 ID 的列表
     */
    void setProcessEffectiveCapabilities(const std::vector<int> &ids);

    /**
     * @brief 设置process的capabilities的ambient的权限能力
     * @param ids 权限 ID 的列表
     */
    void setProcessAmbientCapabilities(const std::vector<int> &ids);

    /**
     * @brief 生成config.json到指定目录
     * @param dir 指定目录
     */
    void genConfigTo(const std::string &dir);

    /**
     * @brief 获取KMConfigDbata对象
     * @return KMConfigDbata 
     */
    KMConfigDbata getConfig();

    /**
     * @brief 修改KMConfigDbata对象cfg
     * @param config, KMConfigDbata对象
     */
    void setCfg(const KMConfigDbata &config);

    /**
     * @brief 获取宿主主机名
     * @return string 主机名
     */
    std::string getHostName();

    /**
     * @brief 获取主机域名
     * @return string 主机名
     */
    std::string getHostDomainName();

    /**
     * @brief overlay支持rootless挂载处理
     */
    void overlayRootless();

private:
    class Private;
    std::unique_ptr<Private> d_ptr;

    /**
     * @brief 根据给定的权限 ID 列表，获取对应权限能力的字符串表示
     * @param ids 权限ID的列表
     * @return 权限字符串的列表，表示输入ID对应的权限能力
    */
    std::vector<std::string> getCapabilities(const std::vector<int> &ids);
};

#endif // __KMCONTAINER_H__
