#ifndef AUTOSYSTEMBACKUPPROXY_H
#define AUTOSYSTEMBACKUPPROXY_H

#include "workerfactory.h"
#include "myprocess/calcbackupsize.h"
#include "myprocess/mksquashfsprocess.h"
#include "parsebackuplist.h"
#include "myprocess/rsyncpathtodirprocess.h"

class AutoSystemBackupProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(AutoSystemBackupProxy)

public:
    // 自动系统备份的几种场景
    enum AutoSystemBackupScene {
         TRY_SYSTEM_BACKUP,  // 测试自动系统备份，可用于计算备份传输数据大小
         TRY_INC_SYSTEM_BACKUP_AT_BASE, // 测试增量系统备份，在原备份点里增量备份，可用于计算备份传输数据大小
         MKSQUASHFS,  // mksquashfs生成img文件
         SYSTEM_BACKUP, // rsync备份系统
         TRY_EXTRA_DATA_BACKUP, // 计算额外数据备份所需空间大小
         EXTRA_DATA_BACKUP, // 额外数据备份
    };

    // 计算备份空间的方法
    enum CalcMethod {
        NONCOMPRESS_INC,     //非压缩增量备份计算方法
        NONCOMPRESS_FULL,    //非压缩全量备份计算方法
        MKSQUASHFS_COMPRESS, //mksquashfs压缩全量备份计算方法
    };

    explicit AutoSystemBackupProxy();
    virtual ~AutoSystemBackupProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

signals:

private slots:
    // 校验剩余空间是否满足备份
    void checkFreeCapacity(qint64 itotalSize);

    // mksqushfs
    void doMksqushfs();

    // 备份系统
    bool backupSystem();

    // 任务取消
    virtual void cancelEx();

private:
    void loadExistsBackupPoint();

    // 计算备份所需空间大小
    void calcSizeForBackup();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(AutoSystemBackupScene scene);

    /**
     * @brief 记录/backup/snapshots/backuplist.xml文件
     * @return true-成功；false-失败
     */
    bool recordBackupPoint();

    // 备份准备
    bool doPrepare();

    bool backup(const QStringList &args);

    void do_kylin_security(const QString& dstDir);

    // 失败则删除相应数据
    void deleteFailedData();

    // 删除指定备份点
    void removeAutoBackupPoint(const QString& uuid);

    // 额外数据备份
    void extraDataBackup();

    // 额外数据路径列表
    QStringList extraDataPaths();

    // 计算额外备份数据所需空间大小
    int calcSizeForExtraBackup();

    // 计算备份空间大小的进程
    CalcBackupSize *m_calc;
    // 压缩进程
    MkSquashFSProcess *m_mksquashfs;

    // 是否完成
    bool m_isFinished;
    // 是否备份成功
    bool m_bSuccess;
    // 当前备份uuid
    QString m_curUuid;
    // 当前备份目标目录
    QString m_destPath;
    // 当前备份所需空间大小
    qint64 m_size;
    // 当前备份节点
    ParseBackupList::BackupPoint m_backupPoint;
    // 是否只是检测
    bool m_isOnlyCheck;

    // 额外数据的备份目录
    QString m_extraPath;
    // 额外备份的数据列表
    QStringList m_backExtraPaths;
    // 额外数据备份进程
    RsyncPathToDirProcess *m_p;
    // 计算额外备份空间大小的进程
    CalcBackupSize *m_calcExtra;

    // 计算方法
    CalcMethod m_calcMethod = MKSQUASHFS_COMPRESS;
    // 是否有备份分区
    bool m_hasBackupPartition = true;
    // rsync备份进程
    RsyncPathToDirProcess* m_rsync;
    // loop, only connect once
    bool m_hasConnectCalcProcess = false;
    // 备份点xml文件名
    QString m_xmlFile;
};


#endif // AUTOSYSTEMBACKUPPROXY_H
