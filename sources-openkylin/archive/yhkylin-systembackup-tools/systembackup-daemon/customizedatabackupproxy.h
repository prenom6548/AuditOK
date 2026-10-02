#ifndef CUSTOMIZEDATABACKUPPROXY_H
#define CUSTOMIZEDATABACKUPPROXY_H

#include "workerfactory.h"
#include "myprocess/calcbackupsize.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class CustomizeDataBackupProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(CustomizeDataBackupProxy)
#ifdef UNIT_TEST
    friend class TestCustomizeDataBackupProxy;
#endif
public:
    // 系统备份的几种场景
    enum CustomizeDataBackupScene {
        DATA_BACKUP,  // 系统备份
        INC_DATA_BACKUP,  // 增量系统备份
        TRY_DATA_BACKUP,  // 测试系统备份，可用于计算备份传输数据大小
        TRY_INC_DATA_BACKUP, // 测试增量系统备份，可用于计算备份传输数据大小
    };

    explicit CustomizeDataBackupProxy();
    virtual ~CustomizeDataBackupProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

public slots:
    // 任务取消
    virtual void cancelEx();

private slots:
    // 校验剩余空间是否满足备份
    bool checkFreeCapacity(qint64 itotalSize);

    // 备份
    void doBackup();

private:
    // 计算备份所需空间大小
    void calcSizeForBackup();

    /**
     * @brief 记录/backup/snapshots/backuplist.xml文件
     * @return true-成功；false-失败
     */
    bool recordBackupPoint();

    // 备份准备
    bool doPrepare();

    // 备份系统
    bool backupData();

protected:
    // 判断是否增量备份
    bool isIncBackup();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(CustomizeDataBackupScene scene);



    // 失败则删除相应数据
    virtual void deleteFailedData();

    // 计算备份空间大小的进程
    CalcBackupSize *m_calc;
    // 是否只是检测
    bool m_isOnlyCheck;
    // 是否完成
    bool m_isFinished;
    // 是否成功
    bool m_bSuccess;
    // 当前备份uuid
    QString m_curUuid;
    // 当前备份目标目录
    QString m_destPath;
    // 当前备份所需空间大小
    qint64 m_size;
    // 备份点用户备份路径文件
    QString m_userFile;
    // 备份点排除备份路径文件
    QString m_excludeUserFile;
    // 备份进程
    RsyncPathToDirProcess *m_p;
    // 当前备份节点
    ParseBackupList::BackupPoint m_backupPoint;
};

#endif // CUSTOMIZEDATABACKUPPROXY_H
