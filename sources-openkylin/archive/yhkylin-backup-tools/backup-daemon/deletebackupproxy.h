#ifndef DELETEBACKUPPROXY_H
#define DELETEBACKUPPROXY_H

#include "workerfactory.h"
#include "parsebackuplist.h"

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

    void deleteBackupPointDir();

    ParseBackupList::BackupPoint m_backupPoint;
};

#endif // DELETEBACKUPPROXY_H
