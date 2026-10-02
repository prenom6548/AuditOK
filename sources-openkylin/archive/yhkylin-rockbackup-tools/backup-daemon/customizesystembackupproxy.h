#ifndef CUSTOMIZESYSTEMBACKUPPROXY_H
#define CUSTOMIZESYSTEMBACKUPPROXY_H

#include "workerfactory.h"
#include "myprocess/calcbackupsize.h"
#include "myprocess/mksquashfsprocess.h"
#include "parsebackuplist.h"

class CustomizeSystemBackupProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(CustomizeSystemBackupProxy)

public:
    // 系统备份的几种场景
    enum CustomizeSystemBackupScene {
         TRY_SYSTEM_BACKUP,  // 测试系统备份，可用于计算备份传输数据大小
         MKSQUASHFS,  // 生成img文件
    };

    explicit CustomizeSystemBackupProxy();
    virtual ~CustomizeSystemBackupProxy();

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

    // 任务取消
    virtual void cancelEx();

private:
    // 计算备份所需空间大小
    void calcSizeForBackup();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(CustomizeSystemBackupScene scene);

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
};

#endif // CUSTOMIZESYSTEMBACKUPPROXY_H
