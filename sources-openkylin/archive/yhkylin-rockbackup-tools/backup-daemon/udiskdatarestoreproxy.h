#ifndef UDISKDATARESTOREPROXY_H
#define UDISKDATARESTOREPROXY_H

#include "workerfactory.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class UDiskDataRestoreProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(UDiskDataRestoreProxy)
public:
    // 数据还原的几种场景
    enum DataRestoreScene {
        DATA_RESTORE,  // 数据还原
    };

    explicit UDiskDataRestoreProxy();
    virtual ~UDiskDataRestoreProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

private slots:
    bool checkUdiskExists();

private:
    // 数据还原
    void restoreData();

    /**
     * @brief 根据场景获取rsync命令参数
     * @param scene，场景
     * @return 组装好的rsync的参数信息
     */
    QStringList getRsyncArgs(DataRestoreScene scene);

    // .user.txt文件路径
    QString m_userFile;
    // .exclude.user.txt文件路径
    QString m_excludeUserFile;
    // 备份数据所在的data目录
    QString m_backupPath;

    // 是否还原成功
    bool m_bSuccess;
    // 是否完成
    bool m_isFinished;
    // 当前还原源目录
    QString m_srcPath;
    // 还原进程
    RsyncPathToDirProcess *m_p;
    // 当前备份节点
    ParseBackupList::BackupPoint m_backupPoint;
    // 强制结束标志(stop后没反应的情况，系统处于睡眠状态)
    bool m_isForce;
};

#endif // UDISKDATARESTOREPROXY_H
