#ifndef KMBUILDERMANIFEST_H
#define KMBUILDERMANIFEST_H

#include <QString>
#include <QVector>
#include <QMap>
#include <yaml-cpp/yaml.h>
#include <QMetaType>

#include "KMBuilderStringUtils.h"
#include "KMYAMLNodeHelper.h"

class KMBuilderManifest
{

public:
    KMBuilderManifest() = default;
    KMBuilderManifest(const KMBuilderManifest&) = delete;
    KMBuilderManifest& operator=(const KMBuilderManifest&) = delete;
    enum SOURCEType
    {
        UNKNOWN,
        ARCHIVE,
        GIT,
        DIR,
        FILE,
        PATCH
    };

public:

    QString m_version = "1";

    // 包基础信息，必须
    struct Package
    {
        // appid，形如：top.openkylin.Calculator，必须
        QString m_id;
        // 名称，形如：Calculator；非必须
        QString m_name;
        // 包版本，必须
        QString m_version;
        // 包类型，如：app、runtime、base等，必须
        QString m_kind = "app";
        // 包描述信息；非必须
        QString m_description;
        //description[zh_CN]包描述信息；非必须
        QMap<QString, QString> m_descriptions;
        //name[zh_CN] 展示名称；非必须
        QMap<QString, QString> m_names;

    };
    Package m_package;
    // 包中可执行程序列表，kaiming run时默认运行第一个，故需要注意书写顺序，建议只配置一个核心可执行程序；非必须
    QVector<QString> m_commands;
    // 渠道/分支，如：stable；非必须
    QString m_channel = "stable";
    // 权限；非必须
    struct Permission
    {
        struct Filesystem
        {
            bool m_xdgUserDirs = true;
            QStringList m_filesystemDirs;
        };

        /**
         * 清单内部类，用于描述permission net属性
         */
        struct Net
        {
            bool m_network = false;
            bool m_scanning = false;
        };

       /**
         * 清单内部类，用于描述permission device属性
         */
        struct Device
        {
            bool m_bluetooth = false;
            bool m_microphone = false;
            bool m_camera = false;
            bool m_usb = false;
            bool m_serial = false;
        };

        /**
         * 清单内部类，用于描述permission processManager属性
         */
        struct ProcessManager
        {
            bool m_autostart = false;
        };

        /**
         * 清单内部类，用于描述permission others属性
         */
        struct Others
        {
            bool m_systemNotice = false;
            bool m_systemTime = false;
            bool m_systemServer = false;
            bool m_windowTop = false;
            bool m_preventSleep = false;
            bool m_systemDisplayResolution = false;
            bool m_systemDisplayBrightness = false;
            bool m_systemAudioSetting = false;
            bool m_systemAudioSize = false;
            bool m_systemCrontab = false;
            bool m_systemPowerMode = false;
            bool m_systemPowerPolicy = false;
        };

        bool m_none = false;
        Filesystem m_filesystem;
        Net m_net;
        Device m_device;
        ProcessManager m_processManager;
        Others m_others;
    };
    Permission m_permission;

    // 模块类型，如：binary、devel(只有base和runtime才有，app不提供开发库)；非必须
    QMap<QString, QString> m_modules;
    // base，如：stable:dev.openkylin.base/23.0.0；必须
    QString m_base;
    // 运行时，如：stable:dev.ukui.runtime/23.0.0；非必须
    QString m_runtime;

    struct Component
    {

        QString m_name;
        QString m_buildSystem;
        QString m_buildScript;
        QVector<QString> m_configOpts;
        struct BuildOptions
        {

            QString m_cFlags = "";
            QString m_cppFlags = "";
            QString m_cxxFlags = "";
            QString m_ldFlags = "";
            QString m_appendPath = "";
            QString m_prependPath = "";
            QString m_appendLdLibraryPath = "";
            QString m_prependLdLibraryPath = "";
            QString m_appendPkgConfigPath = "";
            QString m_prependPkgConfigPath = "";
            QStringList m_makeArgs;
            QStringList m_makeInstallArgs;
            QMap<QString, QString> m_env;
        };
        BuildOptions m_buildOptions;
        QString m_preInstall;
        QString m_postInstall;
        QVector<QString> m_cleanup;

        struct Source
        {
            QString m_base;

            QVector<QString> m_onlyArches;
            QVector<QString> m_skipArches;
            QString m_command;
            SOURCEType m_type = UNKNOWN;
            int m_index = -1;

            struct Archive
            {
                QString m_path;
                QString m_url;
                QString m_sha256;
                QString m_sha512;
                QString m_destFilename;
            };

            struct Git
            {
                QString m_url;
                QString m_tag;
                QString m_branch;
                QString m_commit;
            };

            struct Dir
            {
                QString m_path;
                QVector<QString> m_skip;

                // 用于type=dir/file时，选择本地原始源路径，构建之前需要将其拷贝到path指定的路径中
                QString m_originalPath;
            };

            struct File
            {
                QString m_path;
                QString m_url;
                QString m_sha256;
                QString m_sha512;
                QString m_destFilename;

                // 用于type=dir/file时，选择本地原始源路径，构建之前需要将其拷贝到path指定的路径中
                QString m_originalPath;
            };

            struct Patch
            {
                QString m_path;
                QVector<QString> m_paths;
                int m_stripComponents = -1;
            };

            Archive m_archive;
            Git m_git;
            Dir m_dir;
            File m_file;
            Patch m_patch;
        };
        QVector<Source> m_sources;
    };

    // 构建模块，必须
    QVector<Component> m_components;
    // 构建结束后的清理文件；非必须
    QVector<QString> m_cleanups;
    // 环境变量；非必须
    QMap<QString, QString> m_envs;
    // 环境变量；应用编译依赖的所有包列表，仓库、base、runtime中未提供的需要通过模块依赖的方式添加，非必须
    QVector<QString> m_depends;
    // 许可证信息，SPDX格式。简单说明即可。如：Apache-2.0, MIT等，非必须
    QString m_license;
    // 首页，非必须
    QString m_homepage;
    // 维护者邮箱，字符串，如果有多个可用多行字符串或用逗号分割，非必须
    QVector<QString> m_maintainers;
    struct Annotations
    {
        // 是否提权。应用如果需要sudo运行，则设置为true
        bool m_kylinPrivilege = false;

        // 是否系统应用，系统应用使用overlay方式挂载并安装在/usr目录下
        bool m_kylinSysapp = false;

        //依赖的其他应用的程序，每个数组元素都是一个可执行程序。可以是绝对路径，也可以只写程序名，建议和本应用中使用方式一致。
        QStringList m_kylinDependApps;
    };
    Annotations m_annotations;

private:
    YAML::Node yaml;

    QString m_project_path = "";

public:

    /**
     * @brief 生成yaml
     * @param path: 生成yaml路径
     */
    bool toYaml(QString path);

    void packageToYaml();

    void permissionToYaml();
    void permissionFilesystemToYaml(YAML::Node &yamlNode);
    void permissionNetToYaml(YAML::Node &yamlNode);
    void permissionDeviceToYaml(YAML::Node &yamlNode);
    void permissionProcessManagerToYaml(YAML::Node &yamlNode);
    void permissionOthersToYaml(YAML::Node &yamlNode);

    void annotationToYaml();

    static void componentToYaml(YAML::Node &yamlNode, const Component component);
    static void buildOptionsToYaml(YAML::Node &yamlNode, const Component::BuildOptions buildOptions);
    static void sourceToYaml(YAML::Node &yamlNode, const Component::Source source);
    static void archiveToYaml(YAML::Node &yamlNode, const Component::Source::Archive archiveValue);
    static void gitToYaml(YAML::Node &yamlNode, const Component::Source::Git gitValue);
    static void dirToYaml(YAML::Node &yamlNode, const Component::Source::Dir dirValue);
    static void fileToYaml(YAML::Node &yamlNode, const Component::Source::File fileValue);
    static void patchToYaml(YAML::Node &yamlNode, const Component::Source::Patch patchValue);

    static void vectorToYaml(YAML::Node &yamlNode, QString key, QVector<QString> values);
    static void QStringListToYaml(YAML::Node &yamlNode, const QString &key, const QStringList &values);
    static void mapToYaml(YAML::Node &yamlNode, QMap<QString, QString>values);

    /**
     * @brief yamlNode转为字符串
     * @param yamlNode:yaml节点
     * @return yamlNode转换后的字符串
     */
    static QString nodeToString(const YAML::Node &yamlNode);


public:
    void cleanup();

    void loadFromYaml(const QString &path);
    void parseFromYaml();

    void parsePermission(const YAML::Node &permissionNode);
    void parseComponents(const YAML::Node &componentsNode);
    void parseBuildOptions(YAML::Node buildOptionsNode, Component::BuildOptions &buildOptions);
    void parseSources(YAML::Node sourcesNode, QVector<Component::Source> &sources, QString base_path="");

    void setType(const QString &typeStr, SOURCEType &type);

    YAML::Node loadShareNode(const YAML::Node &shareNode);
};
Q_DECLARE_METATYPE(KMBuilderManifest::Permission)
Q_DECLARE_METATYPE(KMBuilderManifest::Component)
Q_DECLARE_METATYPE(KMBuilderManifest::Component::BuildOptions)
Q_DECLARE_METATYPE(KMBuilderManifest::Component::Source)
Q_DECLARE_METATYPE(KMBuilderManifest::Component::Source::Git)
Q_DECLARE_METATYPE(KMBuilderManifest::Component::Source::Archive)
Q_DECLARE_METATYPE(KMBuilderManifest::Component::Source::Dir)
Q_DECLARE_METATYPE(KMBuilderManifest::Component::Source::File)
#endif

