/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_MANIFEST_H__
#define __BUILDER_MANIFEST_H__

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <map>
#include <yaml-cpp/yaml.h>

#include "KMBuilderModule.h"
#include "common/KMInfoJson.h"

#define PACKGE_KIND_APP "app"
#define PACKGE_KIND_BASE "base"
#define PACKGE_KIND_RUNTIME "runtime"
#define PACKGE_KIND_DEPEND "depend"

/**
 * @brief : 构建清单类
 */ 
class KMBuilderManifest : public std::enable_shared_from_this<KMBuilderManifest>
{
public:
    /**
     * 清单内部类，用于描述package属性
     */ 
    struct Package
    {
        // appid，形如：top.openkylin.Calculator，必须
        std::string m_id;
        // 名称，形如：Calculator；非必须
        std::string m_name;
        // 包版本，必须
        std::string m_version;
        // 包类型，如：app、runtime、base等，必须
        std::string m_kind;
        // 包描述信息；非必须
        std::string m_description;
        //description[zh_CN]包描述信息；非必须
        std::map<std::string, std::string> m_descriptions;
        //name[zh_CN] 展示名称；非必须
        std::map<std::string, std::string> m_names;
        
    };

    /**
     * 清单内部类，用于描述subpackage属性
     */ 
    struct SubPackage
    {
        // appid，形如：top.openkylin.Calculator，必须
        std::string m_id;

        // 名称，形如：Calculator；必须
        std::string m_name;

        // 包类型，如：app、runtime、base等，必须
        std::string m_kind;

        // 包描述信息；非必须
        std::string m_description;

        // module: 子包模块名称，如：binary、devel。kind=app时默认为binary；kind=depend时默认为devel
        std::string m_module;

        // description[zh_CN]包描述信息；非必须
        std::map<std::string, std::string> m_descriptions;

        // name[zh_CN] 展示名称；非必须
        std::map<std::string, std::string> m_names;

        // kaiming run时默认运行的程序。数组的第一个参数为运行程序名，后面数组元素可以设置运行参数；非必须
        std::vector<std::string> m_commands;

        /*
            此子包包含的文件路径列表。路径写法支持：
            1. 绝对路径匹配。此路径以构建结果files目录为根。如：/bin/appA，表示files/bin/appA
            2. 基本名称匹配。文件或目录名。如：include
            3. 支持通配符'*'。如: '*.so'，表示files的子目录中所有的.so结尾的文件。

            注意：构建结果中可能包含依赖包数据，如果使用通配符或目录匹配可能会带入非必需的数据，建议写精确的文件路径。

            如：
            构建完成后，make install、post-install等安装阶段，文件A安装到了容器目录/opt/apps/${APP_ID}/files/bin/A,则此处可写/bin/A。
            例：
            includes:
            - /bin
            - /share/applications
            - '*.so'

            注意：构建结果目录build/files在容器中挂载在路径/opt/apps/${APP_ID}/files目录。make install、post-install等安装也是将文件安装在容器/opt/apps/${APP_ID}/files目录。
            构建结果build目录的结构如下：
            ├── files
            │   ├── bin
            │   │   └── filelight
            │   ├── etc
            │   │   └── xdg
            │   └── share
            │       ├── applications
            │       ├── doc
            │       ├── icons
            │       ├── kxmlgui5
            │       ├── locale
            │       ├── metainfo
            │       └── qlogging-categories5
            └── info.json
         */
        std::vector<std::string> m_includes;

        /*
            子包中的此属性主要作用：
            1. 用于描述depend类型包间关系。如果有depend类型的子包，此处用于描述此子包依赖的其他depend包。
            2. app类型的包需要用此属性描述此子包需要自包含的depend包列表。
            格式为：${id}/${version}，其中id为必须说明，version可以没有(没有版本则下载安装最新版本)。
            注意：
            此属性和顶层depends属性作用稍有不同。子包中的此属性的项可以引用顶层depends属性中的项，也可以是subpackages列表中前面拆分的子包。
            如：
              - top.openkylin.libkylin-ai-base2
              - top.openkylin.libkf5windowsystem5/5.92.0-ok5.3
         */
        std::vector<std::string> m_depends;
    };

public:
    KMBuilderManifest();
    virtual ~KMBuilderManifest();

    void toYaml(const std::string &path);

    /**
     * @brief : 工厂函数，根据yaml格式的清单文件创建清单对象
     * @param : [in] yaml，yaml清单文件
     * @return: std::shared_ptr<KMBuilderManifest>，创建成功则关联对象；
     * @throws: KMException，创建失败则函数抛出异常
     */  
    [[nodiscard]] static std::shared_ptr<KMBuilderManifest> loadFromYAML(const std::string& yaml);

private:
    KMBuilderManifest(YAML::Node);

    // 解析清单属性
    void parseFromYAML();
    // 解析清单的permission属性
    void parsePermission(const YAML::Node& permissionNode);
    // 解析subpackage属性
    void parseSubPackages(YAML::Node& subpackagesNode);
    // 解析annotations属性
    void parseAnnotations(YAML::Node& annotationsNode);
    // 解析清单的components属性
    std::vector<std::shared_ptr<KMBuilderModule>> parseComponents(const YAML::Node& modules);
    // 加载清单共享节点(共享模块、共享源)
    YAML::Node loadShareNode(const YAML::Node& shareNode);
    // 解析清单中components下的sources属性
    std::vector<std::shared_ptr<KMBuilderSource>> parseSources(const YAML::Node& sources);
    // 解析构建选项
    std::shared_ptr<KMBuildOptions> parseBuildOptions(YAML::Node &node);
    // 验证必设字段
    void validate();
    // 检查app、base、runtime的channel是否一致
    void checkChannel();

    //////////////////////读写分割线////////////////////////////

    void packageToYaml();

    void permissionToYaml();
    void permissionFilesystemToYaml(YAML::Node &yamlNode);
    void permissionNetToYaml(YAML::Node &yamlNode);
    void permissionDeviceToYaml(YAML::Node &yamlNode);
    void permissionProcessManagerToYaml(YAML::Node &yamlNode);
    void permissionOthersToYaml(YAML::Node &yamlNode);

    void annotationToYaml();

    void componentToYaml(YAML::Node &yamlNode, const std::shared_ptr<KMBuilderModule> component);
    void buildOptionsToYaml(YAML::Node &yamlNode, const std::shared_ptr<KMBuildOptions> buildOptions);

public:
    // 清单属性域

    // 清单规范版本，必须
    std::string m_version;
    // 包基础信息，必须
    Package m_package;
    // kaiming run时默认运行的程序。数组的第一个参数为运行程序名，后面数组元素可以设置运行参数；非必须
    std::vector<std::string> m_commands;
    // 渠道/分支，如：stable；非必须
    std::string m_channel;
    // 权限；非必须
    KMInfoJson::Permission m_permission;
    // 模块类型，如：binary、devel(只有base和runtime才有，app不提供开发库)；非必须
    std::map<std::string, std::string> m_modules;
    // base，如：stable:dev.openkylin.base/23.0.0；必须
    std::string m_base;
    // 运行时，如：stable:dev.ukui.runtime/23.0.0；非必须
    std::string m_runtime;
    // 构建模块，必须
    std::vector<std::shared_ptr<KMBuilderModule>> m_components;
    // 构建结束后的清理文件；非必须
    std::vector<std::string> m_cleanups;
    // 环境变量；非必须
    std::map<std::string, std::string> m_envs;
    // 环境变量；应用编译依赖的所有包列表，仓库、base、runtime中未提供的需要通过模块依赖的方式添加，非必须
    std::vector<std::string> m_depends;
    // 许可证信息，SPDX格式。简单说明即可。如：Apache-2.0, MIT等，非必须
    std::string m_license;
    // 首页，非必须
    std::string m_homepage;
    // 维护者邮箱，字符串，如果有多个可用多行字符串或用逗号分割，非必须
    std::vector<std::string> maintainers;
    // 扩展字段，非必须
    KMInfoJson::Annotations m_annotations;

    /*
        导出的插件路径列表。此路径以构建结果的files目录为根。
        如：
        export_plugins: 
            - /usr/lib/mozilla/plugin
            - /opt/kingsoft/wps/office6
        1）构建出开明包后，安装时会将构建结果目录${build-dir}/files/usr/lib/mozilla/plugin和${build-dir}/files/opt/kingsoft/wps/office6导出到宿主机的目录/opt/kaiming/exports/plugins/usr/lib/mozilla/plugin和/opt/kaiming/exports/plugins/opt/kingsoft/wps/office6。
        2）安装时同时将导出插件目录记录到/opt/kaiming/exports/plugins/plugins.list中
        3）应用运行时，根据2）中文件记录的目录列表挂载到容器中
     */
    std::vector<std::string> m_exportPlugins;

    // 子包列表
    std::vector<SubPackage> m_subpackages;
    
private:
    // 数据成员域

    // YAML文件
    std::string m_yaml;
    // YAML根节点
    YAML::Node m_rootNode;
    // 反编列base目录
    std::string m_demarshalBaseDir;
};

#endif /***  __BUILDER_INIT_H__ */
