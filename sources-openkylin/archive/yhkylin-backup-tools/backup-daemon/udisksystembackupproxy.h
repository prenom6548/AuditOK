#ifndef UDISKSYSTEMBACKUPPROXY_H
#define UDISKSYSTEMBACKUPPROXY_H

#include "workerfactory.h"
#include "myprocess/calcbackupsize.h"
#include "myprocess/mksquashfsprocess.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class UDiskSystemBackupProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(UDiskSystemBackupProxy)

public:
    // 系统备份的几种场景
    enum UDiskSystemBackupScene {
         SYSTEM_BACKUP,  // 系统备份
         TRY_SYSTEM_BACKUP,  // 测试系统备份，可用于计算备份传输数据大小
         MKSQUASHFS,  // 生成img文件
         IMG_BACKUP,  // 备份img文件
    };

    explicit UDiskSystemBackupProxy();
    virtual ~UDiskSystemBackupProxy();

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

    // 备份
    void doBackup();

    // 任务取消
    virtual void cancelEx();

    /**
     * @brief 校验移动盘是否还在
     * @return: bool,存在返回true；不存在返回false
     * @author: zhaominyong
     * @since: 2021/05/24
     * @note:
     *      add by zhaominyong at 2021/05/24 for bug:54377 【备份还原】备份数据到U盘的过程中拔出U盘，备份还原工具仍然一直显示正在备份数据
     */
    bool checkDestDirExists();

private:
    // 判断是否增量备份
    bool isIncBackup();

    // 计算备份所需空间大小
    void calcSizeForBackup();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(UDiskSystemBackupScene scene);

    /**
     * @brief 记录/backup/snapshots/backuplist.xml文件
     * @return true-成功；false-失败
     */
    bool recordBackupPoint();

    // 备份准备
    bool doPrepare();

    // 备份系统
    bool backupSystem();

    // 备份img文件
    bool backupImg();

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
    // 备份进程
    RsyncPathToDirProcess *m_p;
    // 当前备份节点
    ParseBackupList::BackupPoint m_backupPoint;
    // 是否只是检测
    bool m_isOnlyCheck;
    // img文件存放路径
    QString m_imgPath;
    // 强制结束标志(stop后没反应的情况，系统处于睡眠状态)
    bool m_isForce;
};

#endif // UDISKSYSTEMBACKUPPROXY_H
