#ifndef SYSTEMBACKUPPROXY_H
#define SYSTEMBACKUPPROXY_H

#include "workerfactory.h"
#include "myprocess/calcbackupsize.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class SystemBackupProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(SystemBackupProxy)
public:
    // 系统备份的几种场景
    enum SystemBackupScene {
        SYSTEM_BACKUP,  // 系统备份
        INC_SYSTEM_BACKUP,  // 增量系统备份
        INC_SYSTEM_BACKUP_AT_BASE, // 在原备份点里增量备份
        TRY_SYSTEM_BACKUP,  // 测试系统备份，可用于计算备份传输数据大小
        TRY_INC_SYSTEM_BACKUP, // 测试增量系统备份，可用于计算备份传输数据大小
        TRY_INC_SYSTEM_BACKUP_AT_BASE, // 测试增量系统备份，在原备份点里增量备份，可用于计算备份传输数据大小
        EFI_BACKUP, // 备份/boot/efi
    };

    explicit SystemBackupProxy();
    virtual ~SystemBackupProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

    // 任务取消
    virtual void cancelEx();

private:
    // 判断是否增量备份
    bool isIncBackup();

    // 校验剩余空间是否满足备份
    void checkFreeCapacity(qint64 itotalSize);

    // 计算备份所需空间大小
    void calcSizeForBackup();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(SystemBackupScene scene);

    /**
     * @brief 记录/backup/snapshots/backuplist.xml文件
     * @return true-成功；false-失败
     */
    bool recordBackupPoint();

    // 备份准备
    bool doPrepare();

    // 备份
    void doBackup();

    // 备份/boot/efi
    bool backupEfi();

    // 备份系统
    bool backupSystem();

    void do_kylin_security(const QString& dstDir);

    // 失败则删除相应数据
    void deleteFailedData();

    // 是否只是检测
    bool m_isOnlyCheck;
    // 是否备份成功
    bool m_bSuccess;
    // 是否完成
    bool m_isFinished;
    // 当前备份uuid
    QString m_curUuid;
    // 当前备份目标目录
    QString m_destPath;
    // 当前备份所需空间大小
    qint64 m_size;
    // 计算备份空间大小的进程
    CalcBackupSize *m_calc;
    // 备份进程
    RsyncPathToDirProcess *m_p;
    // 当前备份节点
    ParseBackupList::BackupPoint m_backupPoint;

};

#endif // SYSTEMBACKUPPROXY_H
