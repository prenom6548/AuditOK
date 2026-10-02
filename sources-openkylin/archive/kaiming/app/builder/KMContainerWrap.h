/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMCONTAINERWRAP_H__
#define __KMCONTAINERWRAP_H__

#include <memory>
#include <vector>
#include <map>

#include "common/KMConfigData.h"
#include "common/KMContext.h"
#include "common/KMStorageDir.h"

#define DEFAULT_SHELL "/bin/bash"

class KMContainerWrap
{
public:
    KMContainerWrap();
    ~KMContainerWrap();

    void setUpperdir(const std::string &upperdir);

    static void lowerToUpperForSubDir(const std::string &lowerdir, const std::string &upperdir);

    void setAppID(const std::string &id);
    void initMinimalEnvs(bool devel, bool useLdSoCache);
    void addEnv(const std::string &key, const std::string &value);
    void removeEnv(const std::string &key);

    void addMount(const std::string &src, const std::string &dest, const std::string &type, const std::vector<std::string> &options);
    void addShareMount(const std::string &name, const std::string &content, const std::string &dest, const std::vector<std::string> &options);
    void createUpperdirFile(const std::string &filename, const std::string &content);

    void setupOverlayfs(const std::string &base, const std::string &runtime, const std::string &upper, const std::string &workdir, const std::vector<std::string> &depends);
    void setupBase(const std::shared_ptr<KMDeploy> &baseDeploy);
    void setupRuntime(const std::shared_ptr<KMDeploy> &runtimeDeploy);
    void setupAppDataDir(const std::string &id);
    void setupAppContext(const std::shared_ptr<KMContext> &context);

    void setProcess(const Process &p);
    void setupProcessEnvs();
    void setupUserMap();
    Process getProcess();

    /**
     * @brief 设置uid映射
     * @param uidMappings uid映射数组
     */
    void setUidMappings(const std::vector<IDMap> &uidMappings);

    /**
     * @brief 设置gid映射
     * @param gidMappings gid映射数组
     */
    void setGidMappings(const std::vector<IDMap> &gidMappings);

    bool run();

protected:
    // static void dupUpperDirTree(const std::string &src, const std::string &dest);

    void mountBase();
    void setupTZData();
    void setupMonitorPath();
    void applyContextEnvVars(const std::shared_ptr<KMContext> &context);
    void setupDev();
    void setupFileSystem();
    void setupWayland();
    void setupX11();
    void setupSsh();
    void setupPulseaudio();
    void setupPcsc();
    void setupCups();
    void setupGpgAgent();
    void setupSessionDbus();
    void setupSystemDbus();
    void setupA11yDbus();
    void setupGssproxy();
    void setupResolved();
    void setupJournal();
    void setupBluetooth();
    void setProcessCap(const std::vector<int> &cap);

private:
    static std::string getPulseaudioServerUserConfig(const std::string &path);
    static std::string getPulseaudioServer();
    static std::string parsePulseServer(const std::string &value, bool &remote);
    static bool cupsCheckServerIsSocket(const std::string &server);
    static std::string getCupsServerNameConfig(const std::string &path);
    static std::string getCupsServerName();
    static std::string extractUnixPathFromDbusAddress(const std::string &address);

private:
    static const std::map<std::string, std::string> m_defalutEnvs;
    static const std::map<std::string, std::string> m_develEnvs;
    static const std::vector<std::string> m_copyEnvs;
    static const std::vector<std::string> m_copyNoDevelEnvs;
    static const std::vector<std::string> m_overlayfs;

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMCONTAINERWRAP_H__
