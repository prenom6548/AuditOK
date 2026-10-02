/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMERROR_H
#define KMERROR_H

#include <iostream>
#include <map>
#include <string>

/**
 * Error codes for library and dbus functions.
 */
typedef enum
{
    KM_ERROR_NO = 0,                         // success
    KM_ERROR_CHECK_ENV_FAILED,               // Check env failed.
    KM_ERROR_GET_SUMMARY_COMMIT_FAILED,      // Get commit from summary.idx failed
    KM_ERROR_INVALID_ARGS,                   // Input args are invalid.
    KM_ERROR_NEED_NEW_VERSION,               // The App/Runtime needs a more recent version of KM.
    KM_ERROR_ALREADY_INSTALLED,              // App/runtime/remote is already installed
    KM_ERROR_NOT_INSTALLED,                  // App/runtime is not installed
    KM_ERROR_INSTALL_ONLY_PULLED,            // App/runtime was only pulled into the local repository but not installed.
    KM_ERROR_DIFFERENT_REMOTE,               // The App/Runtime is already installed, but from a different remote.
    KM_ERROR_INSTALL_SKIPPED,                // The App/Runtime install was skipped due to earlier errors.
    KM_ERROR_REMOTE_NOT_FOUND,               // The specified remote was not found.
    KM_ERROR_REMOTE_ALREADY_EXISTS,          // The specified remote already exists
    KM_ERROR_REMOTE_USED,                    // Remote can't be uninstalled.
    KM_ERROR_RUNTIME_NOT_FOUND,              // A runtime needed for the app was not found.
    KM_ERROR_REF_NOT_FOUND,                  // The specified ref was not found.
    KM_ERROR_REF_NOT_INSTALLED,              // The specified ref is not installed.
    KM_ERROR_REF_COMMIT_NOT_FOUND,           // Subsummary ref commit not found.
    KM_ERROR_DOWNGRADE,                      // The pulled commit is a downgrade, and a downgrade wasn't specifically allowed.
    KM_ERROR_PULL_FAILED,                    // The pulled is failed.
    KM_ERROR_INVALID_REF,                    // A ref could not be parsed.
    KM_ERROR_INVALID_DATA,                   // Invalid data.
    KM_ERROR_SETUP_FAILED,                   // Sandbox setup failed.
    KM_ERROR_EXPORT_FAILED,                  // Exporting data failed.
    KM_ERROR_RUNTIME_USED,                   // Runtime can't be uninstalled.
    KM_ERROR_INVALID_NAME,                   // Application, runtime or remote name is invalid.
    KM_ERROR_OUT_OF_SPACE,                   // More disk space needed.
    KM_ERROR_WRONG_USER,                     // An operation is being attempted by the wrong user (such as root operating on a user installation).
    KM_ERROR_NOT_CACHED,                     // Cached data was requested, but it was not available.
    KM_ERROR_PERMISSION_DENIED,              // An operation was not allowed by the administrative policy. For example, an app is not allowed to be installed due to not complying with the parental controls policy.
    KM_ERROR_DECOMPRESSION_FAILED,           // decompression .gz file failed.
    KM_ERROR_CURL_DOWNLOAD_FAILED,           // Download .gz file by curl failed.
    KM_ERROR_CURL_CHECK_URL_FAILED,          // Check url is accessible by curl failed.
    KM_ERROR_SUMMARY_JSON_INVALID,           // summary.json is not a valid json file
    KM_ERROR_LOAD_FILE_FAILED,               // Load file failed.
    KM_ERROR_GET_SUBSUMMARY_GVARIANT_FAILED, // Get subsummary GVariant* failed.
    KM_ERROR_GET_REF_INFO_FAILED,            // Get ref info failed.
    KM_ERROR_NO_COLLECTIONID,                // NO collection_id
    KM_ERROR_NOTHING_TO_DO,                  // Nothing to do.
    KM_ERROR_GET_COMMIT_DATA_FAILED,         // Failed to get commit data from ostree api
    KM_ERROR_DEPLOY_DATA_FAILED,             // Failed to deploy data
    KM_ERROR_FOUND_REMOTE_REF_FAILED,        // Failed to find remote ref
    KM_ERROR_DELETE_EXPORT_FILES_FAILED,     // Failed to delete export files
    KM_ERROR_DELETE_DIRECTORY_FAILED,        // Failed to delete directory
    KM_ERROR_DELETE_DEPLOY_TAG_FAILED,       // Failed to delete deploy tag
    KM_ERROR_DELETE_REF_TAG_FAILED,          // Failed to delete ref tag
    KM_ERROR_PRUNE_REPO_FAILED,              // Failed to prune repo
    KM_ERROR_NOT_NEED_UPDATE,                // Not need to update
    KM_ERROR_LOAD_METADATA_FILE_FAILED,      // Failed to load metadate file
    KM_ERROR_UNINSTALL_FAILED,               // Failed to uninstall app
    KM_ERROR_INSTALL_FAILED,                 // Not all the package is online package or offline package
    KM_ERROR_NO_LAYERS_FOUND,                // No layers found
    KM_ERROR_NO_MD5_FILES,                   // No md5 files
    KM_ERROR_SIGN_FAILED,                    // Sign failed
    KM_ERROR_NOT_SUPPORT_ARCH,               // Not support arch
    KM_ERROR_DEPLOY_CONFICTS,                // Deploy conficts
    KM_ERROR_CONFIG_NOT_FOUND,               // No config file
    KM_ERROR_CONFIG_WRITE_FAILED,            // wrte config failed
    KM_ERROR_INVALID_PARAM,                  // Invalid param
    KM_ERROR_SETLABEL_FAILED,                // Failed to set label
    KM_ERROR_GET_SAFE_MODE_FAILED,           // Get safe mode failed
    KM_ERROR_USER_ABORTED,                   // User aborted installation
    KM_ERROR_CREATE_FILE_FAILED,             // Failed to create file
    KM_ERROR_MD5_CHECK_FAILED,               // MD5 check failed
    KM_ERROR_REPAIR_FAILED,                  // Can not repair
    KM_ERROR_CANNOT_UNINSTALL,               // Can not uninstall
    KM_ERROR_INSTALLING,                     // Is installing
    KM_ERROR_NOT_INSTALL_ALLPACKAGE,
    KM_ERROR_DBUS_CALL_FAILED,
    KM_ERROR_DBUS_CONNECTION_FAILED,
    KM_ERROR_UPDATE_FAILED,
    KM_ERROR_INSTALL_DELTA_FAILED,
    KM_ERROR_CANNOT_INSTALL,
    KM_ERROR_KARE_INSTALL_FAILED,
    KM_ERROR_SCRIPT_EXECUTION_FAILED,
    KM_ERROR_NETWORK_FAILED,
    KM_ERROR_DEPENDENCY_NOT_FOUND,
    KM_ERROR_NOT_SUPPORT_HOSTAPP,
    KM_ERROR_USER_CANCEL,                      // user cancle
    KM_ERROR_USER_PAUSED,                       // user paused
    KM_ERROR_PARTIAL_FAILED,                   // Partial download failed
    KM_ERROR_CHECKOUT_FAILED,
    KM_ERROR_NOT_DOWNLOAD

} KMErrorCodeEnum;

/**
 * @brief 操作类型枚举
 */
typedef enum
{
    KM_INSTALL_OPERATION = 0, // 安装操作
    KM_UPDATE_OPERATION,      // 升级操作
    KM_UNINSTALL_OPERATION,   // 卸载操作
    KM_DEPLOY_OPERATION,      // 部署操作
} KMOperationType;

namespace KMErrorMsg
{
    const std::map<int, std::string> Messages = {
        {KM_ERROR_NO, "没有错误，操作成功"},
        {KM_ERROR_CHECK_ENV_FAILED, "检测系统环境失败"},
        {KM_ERROR_GET_SUMMARY_COMMIT_FAILED, "得到摘要commit失败"},
        {KM_ERROR_INVALID_ARGS, "非法传入参数"},
        {KM_ERROR_NEED_NEW_VERSION, "软件包或运行时需要更新的版本"},
        {KM_ERROR_ALREADY_INSTALLED, "系统已安装该软件包"},
        {KM_ERROR_NOT_INSTALLED, "系统未安装软件包"},
        {KM_ERROR_INSTALL_ONLY_PULLED, "软件包或运行时只拉取到本地仓库，但不安装"},
        {KM_ERROR_DIFFERENT_REMOTE, "软件包或运行时已安装，但是来自于不同的远程仓库源"},
        {KM_ERROR_INSTALL_SKIPPED, "软件包运行或软件或运行时因为更早的错误跳过安装"},
        {KM_ERROR_REMOTE_NOT_FOUND, "未找到具体的远程仓库源"},
        {KM_ERROR_RUNTIME_NOT_FOUND, "未找到运行时"},
        {KM_ERROR_REF_NOT_FOUND, "未找到完整的ref引用"},
        {KM_ERROR_REF_NOT_INSTALLED, "未安装指定ref"},
        {KM_ERROR_REF_COMMIT_NOT_FOUND, "未找到ref对应的commit信息"},
        {KM_ERROR_DOWNGRADE, "拉取的commit是一个更新包，但更新包不被允许安装"},
        {KM_ERROR_PULL_FAILED, "拉取失败"},
        {KM_ERROR_INVALID_REF, "非法的ref引用"},
        {KM_ERROR_INVALID_DATA, "非法的数据"},
        {KM_ERROR_SETUP_FAILED, "沙箱建立失败"},
        {KM_ERROR_EXPORT_FAILED, "导出数据失败"},
        {KM_ERROR_REMOTE_USED, "远程仓库不允许卸载"},
        {KM_ERROR_RUNTIME_USED, "运行时不允许被卸载"},
        {KM_ERROR_INVALID_NAME, "非法的软件包、运行时或仓库源名称"},
        {KM_ERROR_OUT_OF_SPACE, "磁盘用尽，需要更多磁盘空间"},
        {KM_ERROR_WRONG_USER, "错误用户试图操作，比如非root用户试图执行root权限操作"},
        {KM_ERROR_NOT_CACHED, "得到摘要失败"},
        {KM_ERROR_PERMISSION_DENIED, "由于系统策略，该操作不被允许"},
        {KM_ERROR_DECOMPRESSION_FAILED, "文件解压失败"},
        {KM_ERROR_CURL_DOWNLOAD_FAILED, "curl下载文件失败,请检查网络"},
        {KM_ERROR_CURL_CHECK_URL_FAILED, "url不可用"},
        {KM_ERROR_SUMMARY_JSON_INVALID, "summary.json无效"},
        {KM_ERROR_LOAD_FILE_FAILED, "加载文件失败"},
        {KM_ERROR_GET_SUBSUMMARY_GVARIANT_FAILED, "解析摘要信息失败"},
        {KM_ERROR_GET_REF_INFO_FAILED, "获取ref信息失败"},
        {KM_ERROR_NO_COLLECTIONID, "没有collection id"},
        {KM_ERROR_NOTHING_TO_DO, "无事可做"},
        {KM_ERROR_GET_COMMIT_DATA_FAILED, "获取commit数据失败"},
        {KM_ERROR_DEPLOY_DATA_FAILED, "部署数据到系统失败"},
        {KM_ERROR_FOUND_REMOTE_REF_FAILED, "未找到远程仓库对应的ref"},
        {KM_ERROR_DELETE_EXPORT_FILES_FAILED, "删除exports文件失败"},
        {KM_ERROR_DELETE_DIRECTORY_FAILED, "删除目录失败"},
        {KM_ERROR_DELETE_DEPLOY_TAG_FAILED, "删除deploy tag失败"},
        {KM_ERROR_DELETE_REF_TAG_FAILED, "删除ref tag失败"},
        {KM_ERROR_PRUNE_REPO_FAILED, "精简仓库失败"},
        {KM_ERROR_NOT_NEED_UPDATE, "不需要更新"},
        {KM_ERROR_LOAD_METADATA_FILE_FAILED, "加载 metadata 文件失败"},
        {KM_ERROR_UNINSTALL_FAILED, "卸载应用失败"},
        {KM_ERROR_INSTALL_FAILED, "需要所有软件包都是离线或者在线包"},
        {KM_ERROR_NO_LAYERS_FOUND, "无法找到layers"},
        {KM_ERROR_NO_MD5_FILES, "签名文件不存在"},
        {KM_ERROR_SIGN_FAILED, "软件包验证失败，拒绝安装。"},
        {KM_ERROR_NOT_SUPPORT_ARCH, "不支持本机架构"},
        {KM_ERROR_DEPLOY_CONFICTS, "安装冲突"},
        {KM_ERROR_CONFIG_NOT_FOUND, "config文件未找到"},
        {KM_ERROR_CONFIG_WRITE_FAILED, "修改config文件失败"},
        {KM_ERROR_INVALID_PARAM, "无效参数"},
        {KM_ERROR_SETLABEL_FAILED, "设置label失败"},
        {KM_ERROR_GET_SAFE_MODE_FAILED, "获取安装模式失败"},
        {KM_ERROR_USER_ABORTED, "用户忽略安装信息"},
        {KM_ERROR_CREATE_FILE_FAILED, "创建文件失败"},
        {KM_ERROR_MD5_CHECK_FAILED, "MD5检测失败"},
        {KM_ERROR_REPAIR_FAILED, "repair修复失败"},
        {KM_ERROR_CANNOT_UNINSTALL, "无法卸载该软件包"},
        {KM_ERROR_INSTALLING, "已有安装程序在运行"},
        {KM_ERROR_NOT_INSTALL_ALLPACKAGE, "部分软件包安装失败"},
        {KM_ERROR_INSTALL_DELTA_FAILED, "未安装该增量包低版本"},
	    {KM_ERROR_CANNOT_INSTALL,"无法安装该应用"},
        {KM_ERROR_KARE_INSTALL_FAILED,"kare安装失败"},
        {KM_ERROR_NETWORK_FAILED, "网络错误"},
        {KM_ERROR_DEPENDENCY_NOT_FOUND,"依赖项未找到"},
        {KM_ERROR_NOT_SUPPORT_HOSTAPP, "不支持此类型应用安装，请检查应用类型或更新应用"},
        {KM_ERROR_USER_CANCEL, "用户取消操作"},
        {KM_ERROR_USER_PAUSED, "用户暂停操作"},
        {KM_ERROR_PARTIAL_FAILED, "部分下载失败" },                  // Partial download failed
        {KM_ERROR_CHECKOUT_FAILED, "checkout失败"},
        {KM_ERROR_NOT_DOWNLOAD, "未下载该软件包"}
    };

    static std::string getErrorMessage(int errorCode)
    {
        auto it = Messages.find(errorCode);
        return it != Messages.end() ? it->second : "Unknown error";
    }
}

#endif // KMERROR_H
