#ifndef MYMOUNTPROXY_H
#define MYMOUNTPROXY_H

#include <QObject>
#include "myprocess/mountbackupprocess.h"
#include "../common/mydefine.h"

class MyMountProxy : public QObject {
    Q_OBJECT
public:

    MyMountProxy(QObject* parent = nullptr);

    MountResult mountBackupPartition();

    bool umountBackupPartition();

    bool remountPartitionsForCasperMode();

private:
    MountBackupProcess* m_processMount;
};

#endif // MYMOUNTPROXY_H
