#ifndef SYSTEMRESTOREPROXY_H
#define SYSTEMRESTOREPROXY_H

#include "systemrestoreworker.h"

class SystemRestoreProxy : public SystemRestoreWorker
{
    Q_OBJECT
    DECLARE_DYNCREATE(SystemRestoreProxy)
public:
    // 系统还原的几种场景
    enum SystemRestoreScene {
        SYSTEM_RESTORE,  // 系统还原
        RESTORE_SYSTEM_WITH_DATA, // 保留用户数据还原
        EFI_RESTORE,  // efi还原
    };

    explicit SystemRestoreProxy();
    virtual ~SystemRestoreProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

private:
    // 还原efi
    bool restoreEfi();
    // 修复efi目录
    void repairEfi();
    // 同步efi
    bool rsyncEfi();
    // 系统还原
    void restoreSystem();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(SystemRestoreScene scene);

private:
    // 是否还原成功
    bool m_bSuccess;
    // 是否还原为oem
    bool m_isOem;
};

#endif // SYSTEMRESTOREPROXY_H
