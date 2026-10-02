#ifndef AutoKareRestoreProxy_H
#define AutoKareRestoreProxy_H

#include "systemrestoreworker.h"
#include "myprocess/OstreeInterface.h"

class AutoKareRestoreProxy : public SystemRestoreWorker
{
    Q_OBJECT
    DECLARE_DYNCREATE(AutoKareRestoreProxy)
public:
    // 系统还原的几种场景
    enum CustomizeSystemRestoreScene {
        SYSTEM_RESTORE,  // 系统还原
        RESTORE_SYSTEM_WITH_DATA, // 保留用户数据还原
        RESTORE_EXTRA_DATA, // 还原额外数据
    };

    enum BackupPointType {
        MKSQUASHFS_COMPRESS, // mksquashfs压缩的备份点
        RSYNC_BACKUP, // rsync的备份点
    };

    explicit AutoKareRestoreProxy();
    virtual ~AutoKareRestoreProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

private:
    // 将img文件挂载到/backup/imgbackup目录
    bool mountImg();
    // 还原前准备
    bool doPrepare();
    // 系统还原
    void restoreSystem();
    // 还原额外数据
    void restoreExtraData();
    // 还原文件
    bool restoreFile(const QString &backupPath, const QString &targetPath);
    // 清理回滚的备份点元数据
    bool deleteXmlBackupPoint();
    // 清理回滚的备份点
    bool deleteBackupPointCommits();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(CustomizeSystemRestoreScene scene);

    // 压缩的img文件全名
    QString m_imgFileName;
    // 是否还原成功
    bool m_bSuccess;
    // 是否还原为oem
    bool m_isOem;

    // 额外备份数据目录
    QString m_extraDir;
    // 额外备份路径列表文件
    QString m_extraUserFile;
    // 额外数据还原进程
    RsyncPathToDirProcess* m_extraProcess;
    // 压缩备份点类型
    BackupPointType m_backupPointType = RSYNC_BACKUP;


    // ostree备份进程
    OstreeInterface *m_pOstree;
    //系统备份点commit Hash
    QMap<QString, QString> m_systemCommitHashMap;

    //最新一次的备份点
    ParseBackupList::BackupPoint m_lastBackupPoint;
};


#endif // AutoKareRestoreProxy_H

