#ifndef DELETEBACKUPPROXY_H
#define DELETEBACKUPPROXY_H

#include "workerfactory.h"
#include "parsebackuplist.h"
#include "myprocess/OstreeInterface.h"

class DeleteBackupProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(DeleteBackupProxy)
public:
    explicit DeleteBackupProxy();
    virtual ~DeleteBackupProxy();

public:
    // 任务处理
    virtual void doWorkEx();

private:
    bool deleteXmlBackupPoint();

    bool deleteBackupPointCommits();

    bool runOstreeCommand(const QStringList &args);

    ParseBackupList::BackupPoint m_backupPoint;

    //ostree 数据备份仓库名称
    QString repoPath;

    // ostree备份进程
    OstreeInterface *m_pOstree;

    struct FileEntry {
        QString hash;
        bool isDirectory;  // true=目录, false=文件

        bool operator< (const FileEntry &other);
    };

    QMap<QString, ParseBackupList::FileEntry> m_dataCommitHashMap;
    QMap<QString, QString> m_systemCommitHashMap;
    // 是否是最后一个移动设备备份点标志
    bool m_isLastMobileBackupPoint;
};

#endif // DELETEBACKUPPROXY_H
