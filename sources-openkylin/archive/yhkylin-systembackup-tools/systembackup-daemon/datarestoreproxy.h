#ifndef DATARESTOREPROXY_H
#define DATARESTOREPROXY_H

#include <QObject>
#include "systemrestoreworker.h"
#include "myprocess/OstreeInterface.h"

class DataRestoreProxy : public SystemRestoreWorker
{
    Q_OBJECT
    DECLARE_DYNCREATE(DataRestoreProxy)
#ifdef UNIT_TEST
    friend class TestDataRestoreProxy;
#endif
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

    explicit DataRestoreProxy();
    virtual ~DataRestoreProxy();

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
    void restoreData();
    // 还原额外数据
    void restoreExtraData();
    //更新进度条
    void updateProgress(int processed);

    bool rsyncCopy(const QString& source, const QString& dest, bool isDir);

    bool ostreeCheckout(QString commitHash,  QString targetDir);

    void clearDirectory(const QString &dirPath);

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

    //需要恢复的文件总数
    int m_totalItems;

    //ostree 数据备份仓库名称
    QString repoPath;

    // ostree备份进程
    OstreeInterface *m_pOstree;
    //系统备份点commit Hash
    // Key: 绝对路径, Value: 结构体（包含哈希和类型）

    struct FileEntry {
        QString hash;
        bool isDirectory;  // true=目录, false=文件

        bool operator< (const FileEntry &other);
    };

    QMap<QString, ParseBackupList::FileEntry> m_dataCommitHashMap;

    QString generate16DigitRandomNumber();

    //将还原之后的数据硬链接出去
    int linkFile(QString src, QString desc);

};

#endif // DATARESTOREPROXY_H
