/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMRUNNER_H__
#define __KMRUNNER_H__

#include "common/KMSubCommand.h"
#include <set>

/**
 * @brief 开明包运行功能封装
 */
class KMRunner final : public KMSubCommand
{
    DECLARE_DYNCREATE(KMRunner);

public:
    KMRunner();
    ~KMRunner() override = default;

    /**
     * @brief 开明子命令运行
     * @param argc 参数个数
     * @param argv 参数列表
     * @return 0 成功 非0 失败
     */
    int dispose(int argc, char **argv) override;

    /**
     * @brief 开始运行
     * @return true 成功 false 失败
     */
    bool start();

    /**
     * @brief 运行
     * @return true 成功 false 失败
     */
    bool run();

private:
    /**
     * @brief 解析命令行参数
     * @param argc 参数个数
     * @param argv 参数列表
     * @return true 成功 false 失败
     */
    bool parseArgs(int argc, char **argv);

    /**
     * @brief 获取app,base,runtime目录
     * @return true 成功 false 失败
     */
    bool getLayersDirs();

    /**
     * @brief 创建appdate子目录
     * @return true 成功 false 失败
     */
    bool ensureAppDataDir();

    /**
     *@brief 设置容器annotation信息
     */
    void setAnnotationInfo();

    /**
     * @brief 挂载app,base,runtime目录
     * @return true 成功 false 失败
     */
    bool mountLayers();

    /**
     * @brief 持久化存储家目录信息
     * @return true 成功 false 失败
    */
    bool PerStorageHome();

    /**
     * @brief 挂载基础路径
     * @return true 成功 false 失败
     */
    bool mountBase();

    /**
     * @brief 挂载var目录
     * @return true 成功 false 失败
     */
    bool mountVar();

    /**
     * @brief 挂载数据配置路径
     * @return true 成功 false 失败
     */
    bool mountData();

    /**
     * @brief 权限管控
     * @return true 成功 false 失败
     */
    bool permissionControl();

    /**
     * @brief 设置process
     * @return true 成功 false 失败
     */
    bool setProcessCmd();

    /**
     * @brief 挂载dev设备
     * @return true 成功 false 失败
     */
    bool mountDev();

    /**
     * @brief 设置网络访问权限
     * @return true 成功 false 失败
     */
    bool setNetwork();

    /**
     * @brief 挂载指定目录
     * @return true 成功 false 失败
     */
    bool mountFileSystem();

    /**
     * @brief 挂载box目录
     * @return true 成功 false 失败
     */
    bool mountBox();

    /**
     * @brief 挂载covers目录
     * @return true 成功 false 失败
     */
    bool mountCover();

    /**
     * @brief 挂载自动启动目录
     * @return true 成功 false 失败
     */
    bool mountAutoStart();

    /**
     * @brief 挂载依赖应用目录
     * @return true 成功 false 失败
     */
    bool mountDependAppsDir();

    /**
     * @brief 挂载启动程序
     * @return true 成功 false 失败
     */
    bool mountStartsh();

    /**
     * @brief 管控设备文件
     * @return true 成功 false 失败
     */
    bool permDev(std::string const &dev);

    /**
     * @brief 设置process的cap能力
     * @param cap 能力列表
     * @return true 成功 false 失败
     */
    bool setProcessCap(const std::vector<int> &cap);

    /**
     * @brief 挂载socket
     * @return true 成功 false 失败
     */
    bool mountSocket();

    /**
     * @brief 叠加share目录
     * @return true 成功 false 失败
     */
    bool overlayShare();

    /**
     * @brief 叠加/var目录 
    */
    void overlayVar();

    /**
     * @brief 挂载share,包含字体,icon等
     * @return true 成功 false 失败
     */
    bool mountShare();

    /**
     * @brief 挂载共享目录
     * @return true 成功 false 失败
     */
    bool mountShareData();

    /**
     * @brief 系统目录叠加挂载
     * @return true 成功 false 失败
     */
    void sysOverlay();

    /**
     * @brief 应用目录挂载
     * @return true 成功 false 失败
     */
    void appBindMount();

    /**
     * @brief 挂载etc目录
     * @return true 成功 false 失败
     */
    bool mountEtc();

    /**
     * @brief 挂载许可
     *@ return true 成功 false 失败
     */
    bool mountXauthority();

    /**
     * @brief 挂载pulseaudio配置
     * @return true 成功 false 失败
     */
    bool mountPulseaudio();

    /**
     * @brief 挂载alsa配置
     * @return true 成功 false 失败
     */
    bool mountAlsa();

    /**
     * @brief 挂载dbus配置
     * @return true 成功 false 失败
     */
    bool mountDbus();

    /**
     * @brief 挂载x11-wayland兼容层
     */
    void mountX11Wayland();

    /**
     * @brief : 挂载x11
     * @return true 成功 false 失败
     */
    bool mountX11();

    /**
     * @brief 挂载wayland
     * @return true 成功 false 失败
     */
    bool mountWayland();

    /**
     * @brief 组装运行命令
     * @return true 成功 false 失败
     */
    bool assembleCommand();

    /**
     * @brief 解析命令行
     * @return 命令行
     */
    std::string parseRunCommand(const std::string &commandLine);

    /**
     * @brief 组装运行环境变量
     * @return true 成功 false 失败
     */
    bool assembleProcEnv();

    /**
     * @brief 设置进程运行命令
     * @return true 成功 false 失败
     */
    bool setProcessCmdArgs();

    /**
     * @brief 叠加目录
     * @param dir overlay目录路径
     */
    void overlayDir(const std::string &dir);

    /**
     * @brief 挂载dconf目录
     * @return true 成功 false 失败
     */
    bool mountDconf();

    /**
     *@brief 获取appdata路径
     *@return true 成功 false 失败
     */
    bool getAppDataPath();

    /**
     *@brief 解析dbus地址
     *@return string 路径
     */
    std::string pathFromAddress(const std::string &address) const;

    /**
     * @brief 处理依赖应用问题
     * @return true 成功 false 失败
     */
    bool mountDepandApps();

    /**
     * @brief 提权
     * @return true 成功 false 失败
     */
    bool privileged();

    /**
     * @brief 挂载指定目
     */
    void mountDir(const std::string &hostDir, const std::string &containDir, const std::string &type, std::vector<std::string> &options);

    /**
     * @brief 判断是否有蓝牙权限
     * @return true 成功 false 失败
     */
    bool bluetoothRestricted();

    /**
     * @brief 家目录是否存在
     * @return true 存在 false 不存在
     */
    bool isExistHomeDir();

    /**
     * @brief XDG_RUNTIME_DIR是否存在
     * @return true 存在 false 不存在
     */
    bool isExistsXdgRuntimeDir();

    /**
     * @brief 创建socket服务套接字
     * @return true 创建成功 false 创建失败
    */
    bool startUnixSocketServer();

    /**
     * @brief 设置交互tty
     * @return true 成功 false 失败
    */
    bool setupInterTty();

    /**
     * @brief 构建run参数
     * @param runDir 运行目录
     * @return 返回参数列表
     */
    std::vector<std::string> assembleRunOptions(const std::string &runDir);

    /**
     * @brief 创建usr子目录软连接
     * @param link 软连接文件
     * @param target 软连接目标
     * @return true 成功 false 失败
     */
    void createUsrSymlinkSubdir(const std::string &link, const std::string &target);

    /**
     * @brief 叠加libexec目录
     */
    void overlaylibexec();

    /**
     * @brief 叠加applications目录
     */
    void overlayApplications();

    /**
     * @brief 挂载overlay
     * @param lowers目录
     * @param upper目录
     * @param merge目录
     * @return true 成功 false 失败
     */
    bool mountOverlay(const std::vector<std::string> &lowers, const std::string &upper, const std::string &merge);

    /**
     * @brief 挂载GPU显卡驱动
     * @return true 存在 false 不存在
     */
    bool mountGpu();

    /**
     * @brief 挂载插件 
     * @return true 存在 false 不存在
     */
    bool mountPlugins();

    /**
    * @brief 组装libgtk-3.so.0 提取版本号
    * @return true 成功 false 失败
    */
    bool adaptGtkTheme();

private:
    class Private;
    std::shared_ptr<Private> d_ptr;
};

#endif // KMRUNNER_H
