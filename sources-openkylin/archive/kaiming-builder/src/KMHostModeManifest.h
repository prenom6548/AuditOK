/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMHOSTMODEMANIFEST_H__
#define __KMHOSTMODEMANIFEST_H__

#include <string>
#include <vector>
#include <memory>
#include <yaml-cpp/yaml.h>

class KMHostModeManifest : public std::enable_shared_from_this<KMHostModeManifest>
{
public:
    enum BuildSystem
    {
        BS_Unknow,
        BS_Qmake,
        BS_Cmake,
        BS_Meson,
        BS_Autotools,
        BS_Cmake_Ninja,
        BS_Simple
    };

    struct Depends
    {
        std::vector<std::string> debs;
        std::vector<std::string> kaimings;
    };

    /**
     * 清单内部类，用于描述package属性
     */ 
    struct Package
    {
        // appid，形如：top.openkylin.Calculator，必须
        std::string id;

        // 名称，形如：Calculator；必须
        std::string name;

        // 包描述信息；非必须
        std::string description;

        /*
            说明：此包包含的文件路径列表。如果没有配置此属性则默认此包包含所有的构建结果数据。路径写法支持：
            1. 绝对路径匹配。此路径以install的根目录为根，如：/opt/system/bin
            2. 基本名称匹配。文件或目录名。如：bin
            3. 支持通配符'*'。如: '*.so'，表示files的子目录中所有的.so结尾的文件。
            includes:
            - /opt/system/bin
            - /opt/system/resource/share/applications
            - "*.so"
         */
        std::vector<std::string> includes;

        /*
            依赖列表。每个元素对象包含两个key: deb和kaiming。deb的值为依赖的debian包列表；kaiming的值为依赖的原生部署开明包列表。格式为：${id}/${version}，其中id为必须说明，version可以没有（没有则默认源中最新的或当前系统中已经安装好的版本）。
            注意：
            1、deb包的版本号写法中支持>=、>、<=、< 4种符号，等于则不写比较符号。
            2、开明包的版本号写法中不支持比较符号。默认版本兼容的包即可使用。
            示例：
            depends:
                deb: 
                    - cmake/>3.15
                    - cmkae-data
                kaiming:
                    - top.openkylin.libmpv-dev/0.32.0
                    - top.openkylin.ffmpeg
        */
        Depends depends;
    };

public:
    // 构建清单属性域

    // 清单规范版本号。默认值1
    std::string standards_version;

    // 源码包名
    std::string source;

    // 源码版本号
    std::string version;

    /*
        构建依赖。包含两个key: deb和kaiming。deb的值为依赖的debian包列表；kaiming的值为依赖的原生部署开明包列表。格式为：${id}/${version}，其中id为必须说明，version可以没有（没有则默认源中最新的或当前系统中已经安装好的版本）。
        注意：
        1、deb包的版本号写法中支持>=、>、<=、< 4种符号，等于则不写比较符号。
        2、开明包的版本号写法中不支持比较符号。默认版本兼容的包即可使用。
        示例：
        build-depends:
            deb: 
                - cmake/>3.15
                - cmkae-data
            kaiming:
                - top.openkylin.libmpv-dev/0.32.0
                - top.openkylin.ffmpeg
    */
    Depends build_depends;

    // 构建系统使用: autotools, cmake, cmake-ninja, meson, qmake, simple。如果不设置则默认使用autotools构建系统
    std::string buildsystem;

    /*
        type: string
        说明：构建脚本，可以是多行字符串。buildsyste=simple时用它来自定义构建、安装行为。buildsyste=simple时不使用内置的构建行为，不使用下面的config-opts、build-opts、install-opts参数。
        注意：
        1、执行此脚本的工作目录为源码根目录。
        2、安装数据时，实际是临时安装在${source-root}/kaiming/${source}/files目录下。其中${source-root}为源码根目录，${source}为上面的清单属性source的值。后面的生成开明包就是根据这个目录里的数据生成的。
     */
    std::string build_script;

    /*
        将传递给配置环节的选项数组。常见的配置环节如下：
        cmake和cmake-ninja构建系统的配置环节：cmake .
        qmake构建系统的配置环节：qmake xxx.pro
        autotools构建系统的配置环节：./configure
        meson构建系统的配置环节：meson ../
        配置选项设置方式参考如下：
        config-opts:
            - -DBUILD_TESTING=OFF
            - -DCMAKE_INSTALL_PREFIX:PATH=/opt/apps/myappid/files 
            - --disable-static
            - --disable-gtk-doc
    */
    std::vector<std::string> config_opts;

    /*
        将传递给make及等同阶段的额外参数选项的数组。make及等同阶段包含：
        构建系统为autotools、qmake和cmake时，如：make -j8
        构建系统为meson和cmake-ninja时，如：ninja -j1
        示例：
        build-opts:
            - j12
    */
    std::vector<std::string> build_opts;

    /*
        将传递给make install及等同阶段的一组额外参数。
        make install及等同阶段包含：
        构建系统为autotools、qmake和cmake时，如：make install
        构建系统为meson和cmake-ninja时，如：ninja install
        示例：
        Install-opts:
            - prefix=/opt/myapp
            - DESTDIR=kaiming/myapp/files
    */
    std::vector<std::string> install_opts;

    /*
        在安装阶段（make install或等同阶段）之后运行的一组shell命令。例如，可以清理安装目录，或者安装额外的文件（如：自包含其他依赖库的处理等）。
        A. 执行该命令的工作目录为当前项目的源码根目录。
        B. 此处进行安装操作时，注意安装的目标目录为构建结果存放目录。使用kaiming-buildpackage命令构建时默认构建结果存放目录为${source-root}/kaiming/${source}/files目录(其中${source-root}表示源码根目录)。
        以kylin-video项目示例：
        post-install: |
            mkdir -p kaiming/kylin-video/files/opt/apps/cn.kylin.kylin-video/lib/
            # 进行依赖复制和rpath设置
            ./shells/bundle_dependencies_ok.sh
    */
    std::string post_build;

    // 包对象列表
    std::vector<Package> packages;

    // 许可证信息，SPDX格式。简单说明即可。如：Apache-2.0, MIT等
    std::string license;

    // 开发商或者项目网址
    std::string homepage;

    // 维护者邮箱，字符串数组
    std::vector<std::string> maintainers;

public:
    KMHostModeManifest();
    virtual ~KMHostModeManifest();

    /**
     * @brief : 工厂函数，根据yaml格式的清单文件创建清单对象
     * @param : [in] yaml，yaml清单文件
     * @return: std::shared_ptr<KMHostModeManifest>，创建成功则关联对象；
     * @throws: KMException，创建失败则函数抛出异常
     */  
    [[nodiscard]] static std::shared_ptr<KMHostModeManifest> loadFromYAML(const std::string& yaml);

private:
    KMHostModeManifest(YAML::Node);

    // 解析清单属性
    void parseFromYAML();
    // 解析subpackage属性
    void parsePackages(YAML::Node& packagesNode);

    // 验证必设字段
    void validate();

private:
    // 数据成员域

    // YAML文件
    std::string m_yaml;
    // YAML根节点
    YAML::Node m_rootNode;
};

#endif // !__KMHOSTMODEMANIFEST_H__
