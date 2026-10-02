/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMHostModeManifest.h"

#include <set>
#include <map>

#include "KMBuilderUtils.h"
#include "common/KMException.h"
#include "common/KMYAMLNodeHelper.h"

KMHostModeManifest::KMHostModeManifest()
{}

KMHostModeManifest::KMHostModeManifest(YAML::Node rootNode)
{
    m_rootNode = rootNode;
}

KMHostModeManifest::~KMHostModeManifest()
{}

/**
 * @brief : 工厂函数，根据yaml格式的清单文件创建清单对象
 * @param : [in] yaml，yaml清单文件
 * @return: std::shared_ptr<KMHostModeManifest>，创建成功则关联对象；
 * @throws: KMException，创建失败则函数抛出异常
 */  
std::shared_ptr<KMHostModeManifest> KMHostModeManifest::loadFromYAML(const std::string& yaml)
{
    YAML::Node rootNode = KMYAMLNodeHelper::loadFromYAML(yaml);
    std::shared_ptr<KMHostModeManifest> manifest;
    manifest.reset(new KMHostModeManifest(rootNode));
    manifest->m_yaml = yaml;

    manifest->parseFromYAML();

    return manifest;
}

void KMHostModeManifest::parseFromYAML()
{
    // 此方法主要解析顶层属性
    KMYAMLNodeHelper helper(m_rootNode);
    standards_version = helper.getStringValue("standards-version", "1");
    source = helper.getStringValue("source");
    version = helper.getStringValue("version");
    buildsystem = helper.getStringValue("buildsystem");
    build_script = helper.getStringValue("build-script");
    config_opts = helper.getStringListValue("config-opts");
    build_opts = helper.getStringListValue("build-opts");
    install_opts = helper.getStringListValue("install-opts");
    post_build = helper.getStringValue("post-build");
    license = helper.getStringValue("license");
    homepage = helper.getStringValue("homepage");
    maintainers = helper.getStringListValue("maintainer");

    YAML::Node buildDependsNode = m_rootNode["build-depends"];
    if (buildDependsNode.IsDefined())
    {
        KMYAMLNodeHelper nodeHelper(buildDependsNode);
        build_depends.debs = nodeHelper.getStringListValue("deb");
        build_depends.kaimings = nodeHelper.getStringListValue("kaiming");
    }

    YAML::Node annotationsNode = m_rootNode["annotations"];
    if (annotationsNode.IsDefined())
    {
        // nothing
    }

    // package属性
    YAML::Node packagesNode = m_rootNode["packages"];
    if (packagesNode.IsDefined())
    {
        parsePackages(packagesNode);
    }

    validate();
}

void KMHostModeManifest::parsePackages(YAML::Node& packagesNode)
{
    if (packagesNode.IsSequence())
    {
        std::set<std::string> ids;
        for (YAML::Node::const_iterator iter = packagesNode.begin(); iter != packagesNode.end(); iter++)
        {
            YAML::Node packageNode = *iter;
            if (packageNode.IsMap())
            {
                KMYAMLNodeHelper helper(packageNode);
                Package package;
                package.id = helper.getStringValue("id");
                if (package.id.empty())
                {
                    throw KMException("Please set 'id' property for packages in manifest.yaml");
                }
                if (ids.count(package.id) > 0)
                {
                    throw KMException("Repeat 'id' property for package : " + package.id + " in manifest.yaml");
                }
                ids.insert(package.id);
                package.name = helper.getStringValue("name");
                if (package.name.empty())
                {
                    throw KMException("Please set 'name' property for packages in manifest.yaml");
                }
                package.description = helper.getStringValue("description");
                package.includes = helper.getStringListValue("includes");
                if (package.includes.empty())
                {
                    package.includes.push_back("*");
                }

                YAML::Node dependsNode = m_rootNode["depends"];
                if (dependsNode.IsDefined())
                {
                    KMYAMLNodeHelper nodeHelper(dependsNode);
                    package.depends.debs = nodeHelper.getStringListValue("deb");
                    package.depends.kaimings = nodeHelper.getStringListValue("kaiming");
                }

                packages.push_back(package);
            }
        }
    }
}

void KMHostModeManifest::validate()
{
    if (source.empty())
    {
        throw KMException("Must set 'source' in manifest file");
    }

    if (version.empty())
    {
        throw KMException("Must set 'version' in manifest file");
    }

    if (buildsystem == "simple" && build_script.empty())
    {
        throw KMException("Must config 'build-script' when buildsystem=simple in manifest file");
    }

    if (packages.empty())
    {
        throw KMException("Must config 'packages' in manifest file");
    }
}

