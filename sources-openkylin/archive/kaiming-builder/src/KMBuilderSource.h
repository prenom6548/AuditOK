/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILD_SOURCE_H__
#define __BUILD_SOURCE_H__

#include <any>
#include <memory>
#include <string>
#include <vector>
#include <sys/stat.h>

#include <yaml-cpp/yaml.h>

#include "common/KMDynamicCreator.h"
#include "common/KMException.h"
#include "common/KMObject.h"
#include "common/KMYAMLNodeHelper.h"
#include "KMBuilderUtils.h"

/**
 * @brief : 源基类
 */ 
class KMBuilderSource : public KMObject
{
public:
    enum Type
    {
        SRC_Unknow,
        SRC_Archive,
        SRC_File,
        SRC_Dir,
        SRC_Script,
        SRC_Shell,
        SRC_ExtraData,
        SRC_Patch,
        SRC_Git,
        SRC_Bzr,
        SRC_Svn
    };

    KMBuilderSource();
    ~KMBuilderSource();

    /**
     * @brief : 源工厂方法，根据源类型创建对应的源类型对象
     * @param : [in]，sourceType 源类型
     * @return: std::shared_ptr<KMBuilderSource>，根据源类型创建对应的源类型对象或空
     */ 
    static std::shared_ptr<KMBuilderSource> createSource(const std::string &sourceType);

    /**
     * @brief : 反序列化
     * @param : [in]，sourceNode，清单中的源节点
     */ 
    void deserialize(YAML::Node& sourceNode);
    
    /**
     * @brief : 需要子类实现自身的反序列化
     * @param : [in]，sourceNode，清单中的源节点
     */ 
    virtual void deserializeEx(YAML::Node& sourceNode) {}

    /**
     * @brief : 需要子类实现将数据写入yaml节点
     * @param : [in], sourceNode, 待写入节点
     */
    virtual void toYaml(YAML::Node& sourceNode) {}

    Type type()
    {
        return m_type;
    }

    void setBaseDir(const std::string &baseDir);
    std::string getBaseDir() const;
    std::string getDest() const;
    std::vector<std::string> getOnlyArches() const;
    std::vector<std::string> getSkipArches() const;
    std::string getCommand() const;
    void setCommand(std::string path);
    void executeCommand();

    virtual bool isEnable();
    virtual bool downloadSource(const std::string &destDir);
    virtual bool updateSource() { return true; }
    virtual bool ensureSource() = 0;
    virtual bool unzipSource() { return true; }
    
    virtual bool processNode(std::string path) = 0;

protected:
    // 属性域

    // 将源目录中的数据提取到此目录中
    std::string m_dest;
    // 如果不是空的，只在列出的arch上构建模块
    std::vector<std::string> m_onlyArches;
    // 不要在列出的任何arch上建造
    std::vector<std::string> m_skipArches;
    std::string m_command;
    std::string m_scriptFile;

protected:
    // 数据成员域
    
    // 源类型
    Type m_type = SRC_Unknow;

    // base目录
    std::string m_baseDir;
};

#define REGISTER_SOURCE_TYPE_DYNCREATE(source_type, class_type) REGISTER_DYNCREATE(SourceType_ ## source_type, class_type)

#endif
