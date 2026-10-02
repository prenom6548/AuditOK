#ifndef MOUNTBACKUPPROCESS_H
#define MOUNTBACKUPPROCESS_H

#include <QProcess>
#include <sys/mount.h>

class MountBackupProcess : public QObject {
    Q_OBJECT
public:
    MountBackupProcess(QObject* parent = nullptr);

    bool Do(const QString& diskUuid);

    bool umountBackupPartition();

    bool umount(const QString& mountPath);
    bool mount(const QString& source, const QString& target, const QString& options = "");

private:

    QProcess* m_p;
};


#endif // MOUNTBACKUPPROCESS_H
