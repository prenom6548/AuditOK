/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KYLINDBUSSECCONFIG_H__
#define __KYLINDBUSSECCONFIG_H__

#include <string>
#include <vector>
#include <memory>
#include <yaml-cpp/yaml.h>

class KylinDBusSecConfig : public std::enable_shared_from_this<KylinDBusSecConfig>
{
public:
    // 属性域
    std::vector<std::string> whitelist;

public:
    KylinDBusSecConfig();
    virtual ~KylinDBusSecConfig();

    /**
     * @brief : 工厂函数，根据yaml格式的创建对应对象
     * @param : [in] yaml，yaml文件
     * @return: std::shared_ptr<KylinDBusSecConfig>，创建成功则关联对象；
     * @throws: KMException，创建失败则函数抛出异常
     */  
    [[nodiscard]] static std::shared_ptr<KylinDBusSecConfig> loadFromYAML(const std::string& yaml);

private:
    KylinDBusSecConfig(YAML::Node);

    // 解析清单属性
    void parseFromYAML();

private:
    // 数据成员域

    // YAML文件
    std::string m_yaml;
    // YAML根节点
    YAML::Node m_rootNode;
};

#endif // !__KYLINDBUSSECCONFIG_H__
