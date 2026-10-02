#ifndef UDISKSYSTEMRESTOREPROXY_H
#define UDISKSYSTEMRESTOREPROXY_H

#include "systemrestoreworker.h"

class UDiskSystemRestoreProxy : public SystemRestoreWorker
{
    Q_OBJECT
    DECLARE_DYNCREATE(UDiskSystemRestoreProxy)
public:
    // 系统还原的几种场景
    enum SystemRestoreScene {
        RESTORE_SYSTEM_WITH_DATA, // 保留用户数据还原
        SYSTEM_RESTORE,  // 系统还原
        EFI_RESTORE,  // efi还原
    };

    explicit UDiskSystemRestoreProxy();
    virtual ~UDiskSystemRestoreProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

private slots:
    bool checkUdiskExists();

private:
    // 还原efi
    bool restoreEfi();
    // 修复efi目录
    void repairEfi();
    // 同步efi
    bool rsyncEfi();
    // 系统还原
    void restoreSystem();
    // 还原前准备
    bool doPrepare();
    // 异机还原时更新grub.cfg中的分区UUID
    void updateGrubUUid();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(SystemRestoreScene scene);

    // 是否还原结束
    bool m_isFinished;
    // 强制结束标志(stop后没反应的情况，系统处于睡眠状态)
    bool m_isForce;
};

#endif // UDISKSYSTEMRESTOREPROXY_H
