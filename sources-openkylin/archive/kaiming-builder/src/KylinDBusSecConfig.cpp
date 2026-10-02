/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KylinDBusSecConfig.h"

#include <set>
#include <map>

#include "common/KMYAMLNodeHelper.h"
#include "KMBuilderUtils.h"

KylinDBusSecConfig::KylinDBusSecConfig()
{}

KylinDBusSecConfig::KylinDBusSecConfig(YAML::Node rootNode)
{
    m_rootNode = rootNode;
}

KylinDBusSecConfig::~KylinDBusSecConfig()
{}

/**
 * @brief : 工厂函数，根据yaml格式的创建对应对象
 * @param : [in] yaml，yaml文件
 * @return: std::shared_ptr<KylinDBusSecConfig>，创建成功则关联对象；
 * @throws: KMException，创建失败则函数抛出异常
 */  
std::shared_ptr<KylinDBusSecConfig> KylinDBusSecConfig::loadFromYAML(const std::string& yaml)
{
    YAML::Node rootNode = KMYAMLNodeHelper::loadFromYAML(yaml);
    std::shared_ptr<KylinDBusSecConfig> config;
    config.reset(new KylinDBusSecConfig(rootNode));
    config->m_yaml = yaml;

    config->parseFromYAML();

    return config;
}

void KylinDBusSecConfig::parseFromYAML()
{
    if (m_rootNode.IsMap())
    {
        for(YAML::Node::const_iterator iter = m_rootNode.begin() ; iter != m_rootNode.end() ; iter++)
        {
            std::string id = iter->first.as<std::string>();
            YAML::Node value = iter->second;
            if (value.IsMap())
            {
                for(YAML::Node::const_iterator iter1 = value.begin() ; iter1 != value.end() ; iter1++)
                {
                    std::string version = iter1->first.as<std::string>();
                    YAML::Node value1 = iter1->second;

                    if (value1.IsMap())
                    {
                        YAML::Node whitelistNode = value1["whitelist"];
                        if (!whitelistNode.IsDefined())
                        {
                            continue;
                        }

                        KMYAMLNodeHelper helper(whitelistNode);
                        std::string _default = helper.getStringValue("_default");
                        KMBuilderUtils::replace(_default, "\"", "");
                        KMBuilderUtils::replace(_default, "[", "");
                        KMBuilderUtils::replace(_default, "]", "");
                        whitelist = KMBuilderUtils::splitString(_default, ",");
                        if (!whitelist.empty())
                        {
                            return ;
                        }
                    }
                }
            }
        }
    }
}
