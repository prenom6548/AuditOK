/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMSUBPACKAGESJSON_H__
#define __KMSUBPACKAGESJSON_H__

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <map>

#include "KMNlohmannJsonHelper.h"

#define PACKGE_KIND_APP "app"
#define PACKGE_KIND_BASE "base"
#define PACKGE_KIND_RUNTIME "runtime"
#define PACKGE_KIND_DEPEND "depend"

/**
 * @brief: 于kaiming build-export生成开明包时拆分多个子包的场景用
 */
class KMSubPackagesJson : public std::enable_shared_from_this<KMSubPackagesJson>
{
public:
    struct SubPackage
    {
        // 子包属性

        // appid，形如：top.openkylin.Calculator，必须
        std::string id;

        // 名称，形如：Calculator；必须
        std::string name;

        // 包类型，如：app、runtime、base等，必须
        std::string kind;

        // 版本号；不要手动设置，此属性会自动继承清单中的package.version
        std::string version;

        // 包描述信息；非必须
        std::string description;

        // module: 子包模块名称，如：binary、devel。kind=app时默认为binary；kind=depend时默认为devel
        std::string module;

        // description[zh_CN]包描述信息；非必须
        std::map<std::string, std::string> descriptions;

        // name[zh_CN] 展示名称；非必须
        std::map<std::string, std::string> names;

        // kaiming run时默认运行的程序。数组的第一个参数为运行程序名，后面数组元素可以设置运行参数；非必须
        std::vector<std::string> commands;

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
        std::vector<std::string> includes;

        /*
            子包中的此属性主要作用：
            1. 用于描述depend类型包间关系。如果有depend类型的子包，此处用于描述此子包依赖的其他depend包。
            2. app类型的包需要用此属性描述此子包需要自包含的depend包列表。
            格式为：${id}/${version}，其中id为必须说明，version可以没有(没有版本则下载安装最新版本)。
         */
        std::vector<std::string> depends;
    };

public:
    KMSubPackagesJson();
    virtual ~KMSubPackagesJson();

    /**
     * @brief : 工厂函数，根据subpackages.json文件创建处理类
     * @param : [in] jsonFile, subpackages.json文件
     * @return: std::shared_ptr<KMSubPackagesJson>，创建成功则关联对象；
     * @throws: KMException，创建失败则函数抛出异常
     */  
    [[nodiscard]] static std::shared_ptr<KMSubPackagesJson> loadFile(const std::string& jsonFile);

    /**
     * @brief : 保存json对象到文件
     * @param : [in] jsonFile, json文件
     * @throw : KMException
     */ 
    void saveFile(const std::string& jsonFile);

private:
    KMSubPackagesJson(nlohmann::json rootNode);

    void fromJson();
    void parseSubpackages();
    void toJson();
    void subpackagesToJson();

public:
    // 属性域

    // 子包列表
    std::vector<SubPackage> subpackages;

private:
    // 数据成员域
    nlohmann::json m_root;
};

 #endif // !__KMSUBPACKAGESJSON_H__
